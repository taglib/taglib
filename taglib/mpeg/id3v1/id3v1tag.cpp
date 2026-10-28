/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include "id3v1tag.h"

#include <memory>

#include "tcharset.h"
#include "tdebug.h"
#include "tutils.h"
#include "tfile.h"
#include "id3v1genres.h"

using namespace TagLib;
using namespace ID3v1;

namespace
{
  const ID3v1::StringHandler defaultStringHandler;
  const ID3v1::StringHandler *stringHandler = &defaultStringHandler;

  //! The code page ID3v1::Tag::setAssumedCodePage() states.
  //!
  //! Used when a tag is read without an explicit one.
  //! 0 keeps the ISO 8859-1 the ID3v1 specification prescribes.
  unsigned int codePageAssumed = 0;

  //! Look up charset for \a codePage.
  //!
  //! The charset a tag's code page selects.  ID3v1 declares no charset, so the
  //! code page is only ever what the caller states: the constructor's, or the
  //! assumed one for the unspecified 0.  An unsupported number falls back to
  //! the assumed one too, rather than dropping the tag over it.
  Charset::Type charsetForCodePage(unsigned int codePage)
  {
    if(codePage == 0) {
      codePage = codePageAssumed;
    }
    else if(!Charset::typeForCodePage(codePage)) {
      debug(Utils::formatString(
        "ID3v1::Tag::Tag() - Unsupported code page %u, using the assumed one.",
        codePage));
      codePage = codePageAssumed;
    }

    // Fast path without lookup for the likely default case.
    if(codePage == 0)
      return Charset::Type::Latin1;

    // If we get here, setAssumedCodePage() stored a verified code page, not 0,
    // the condition will always be true.
    if(const auto charset = Charset::typeForCodePage(codePage))
      return *charset;

    return Charset::Type::Latin1;
  }

  String decodeField(const ByteVector &data, Charset::Type charset)
  {
    // ID3v1 fields are fixed width and space padded, so the padding is stripped
    // here rather than in Charset::decode, which the RIFF decoder shares.
    return Charset::decode(data, charset).stripWhiteSpace();
  }

  ByteVector encodeField(const String &s, Charset::Type charset)
  {
    return Charset::encode(s, charset);
  }
} // namespace

class ID3v1::Tag::TagPrivate
{
public:
  File *file { nullptr };
  offset_t tagOffset { 0 };

  String title;
  String artist;
  String album;
  String year;
  String comment;

  //! The charset the tag's code page selects, resolved once when the tag is
  //! read.
  Charset::Type charset { Charset::Type::Latin1 };

  unsigned char track { 0 };
  unsigned char genre { 255 };

  String parseField(const ByteVector &data) const
  {
    // A handler the user installed with setStringHandler() is an explicit
    // override, so it takes precedence over the code page that was stated. With
    // no handler installed the fields are decoded with the charset.
    if(stringHandler != &defaultStringHandler)
      return stringHandler->parse(data);
    return decodeField(data, charset);
  }

  ByteVector renderField(const String &s) const
  {
    if(stringHandler != &defaultStringHandler)
      return stringHandler->render(s);
    return encodeField(s, charset);
  }
};

class ID3v1::StringHandler::StringHandlerPrivate
{
};

////////////////////////////////////////////////////////////////////////////////
// StringHandler implementation
////////////////////////////////////////////////////////////////////////////////

StringHandler::StringHandler() = default;

StringHandler::~StringHandler() = default;

String ID3v1::StringHandler::parse(const ByteVector &data) const
{
  return String(data, String::Latin1).stripWhiteSpace();
}

ByteVector ID3v1::StringHandler::render(const String &s) const
{
  if(s.isLatin1())
    return s.data(String::Latin1);
  return ByteVector();
}

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

ID3v1::Tag::Tag() :
  d(std::make_unique<TagPrivate>())
{
}

ID3v1::Tag::Tag(File *file, offset_t tagOffset) :
  ID3v1::Tag(file, tagOffset, 0)
{
}

ID3v1::Tag::Tag(File *file, offset_t tagOffset, unsigned int codePage) :
  d(std::make_unique<TagPrivate>())
{
  d->file = file;
  d->tagOffset = tagOffset;
  d->charset = charsetForCodePage(codePage);

  read();
}

ID3v1::Tag::~Tag() = default;

ByteVector ID3v1::Tag::render() const
{
  ByteVector data;

  data.append(fileIdentifier());
  data.append(d->renderField(d->title).resize(30));
  data.append(d->renderField(d->artist).resize(30));
  data.append(d->renderField(d->album).resize(30));
  data.append(d->renderField(d->year).resize(4));
  data.append(d->renderField(d->comment).resize(28));
  data.append(static_cast<char>(0));
  data.append(static_cast<char>(d->track));
  data.append(static_cast<char>(d->genre));

  return data;
}

ByteVector ID3v1::Tag::fileIdentifier()
{
  return ByteVector::fromCString("TAG");
}

String ID3v1::Tag::title() const
{
  return d->title;
}

String ID3v1::Tag::artist() const
{
  return d->artist;
}

String ID3v1::Tag::album() const
{
  return d->album;
}

String ID3v1::Tag::comment() const
{
  return d->comment;
}

String ID3v1::Tag::genre() const
{
  return ID3v1::genre(d->genre);
}

unsigned int ID3v1::Tag::year() const
{
  return d->year.toInt();
}

unsigned int ID3v1::Tag::track() const
{
  return d->track;
}

void ID3v1::Tag::setTitle(const String &s)
{
  d->title = s;
}

void ID3v1::Tag::setArtist(const String &s)
{
  d->artist = s;
}

void ID3v1::Tag::setAlbum(const String &s)
{
  d->album = s;
}

void ID3v1::Tag::setComment(const String &s)
{
  d->comment = s;
}

void ID3v1::Tag::setGenre(const String &s)
{
  d->genre = static_cast<unsigned char>(ID3v1::genreIndex(s));
}

void ID3v1::Tag::setYear(unsigned int i)
{
  d->year = i > 0 ? String::number(i) : String();
}

void ID3v1::Tag::setTrack(unsigned int i)
{
  d->track = static_cast<unsigned char>(i < 256 ? i : 0);
}

unsigned int ID3v1::Tag::genreNumber() const
{
  return d->genre;
}

void ID3v1::Tag::setGenreNumber(unsigned int i)
{
  d->genre = static_cast<unsigned char>(i < 256 ? i : 255);
}

void ID3v1::Tag::setStringHandler(const StringHandler *handler)
{
  if(handler)
    stringHandler = handler;
  else
    stringHandler = &defaultStringHandler;
}

unsigned int ID3v1::Tag::assumedCodePage()
{
  return codePageAssumed;
}

void ID3v1::Tag::setAssumedCodePage(unsigned int codePage)
{
  if(codePage != 0 && !Charset::typeForCodePage(codePage)) {
    debug(Utils::formatString(
      "ID3v1::Tag::setAssumedCodePage() - Unsupported code page %u, ignored.",
      codePage));
    return;
  }

  codePageAssumed = codePage;
}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

void ID3v1::Tag::read()
{
  if(d->file && d->file->isValid()) {
    d->file->seek(d->tagOffset);
    // read the tag -- always 128 bytes
    // some initial sanity checking
    if(const ByteVector data = d->file->readBlock(128);
       data.size() == 128 && data.startsWith("TAG"))
      parse(data);
    else
      debug("ID3v1 tag is not valid or could not be read at the specified offset.");
  }
}

void ID3v1::Tag::parse(const ByteVector &data)
{
  int offset = 3;

  d->title = d->parseField(data.mid(offset, 30));
  offset += 30;

  d->artist = d->parseField(data.mid(offset, 30));
  offset += 30;

  d->album = d->parseField(data.mid(offset, 30));
  offset += 30;

  d->year = d->parseField(data.mid(offset, 4));
  offset += 4;

  // Check for ID3v1.1 -- Note that ID3v1 *does not* support "track zero" -- this
  // is not a bug in TagLib.  Since a zeroed byte is what we would expect to
  // indicate the end of a C-String, specifically the comment string, a value of
  // zero must be assumed to be just that.

  if(data[offset + 28] == 0 && data[offset + 29] != 0) {
    // ID3v1.1 detected

    d->comment = d->parseField(data.mid(offset, 28));
    d->track   = static_cast<unsigned char>(data[offset + 29]);
  }
  else
    d->comment = d->parseField(data.mid(offset, 30));

  offset += 30;

  d->genre = static_cast<unsigned char>(data[offset]);
}
