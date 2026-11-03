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

#include "aiffnativetag.h"

#include <algorithm>
#include <array>
#include <utility>

#include "tcharset.h"
#include "tbytevector.h"
#include "tdebug.h"
#include "tutils.h"
#include "tpropertymap.h"

using namespace TagLib;
using namespace RIFF::AIFF;

namespace
{
  //! The code page NativeTag::setAssumedCodePage() states.
  //!
  //! Used when a tag is read from a file, which declares none anywhere.
  //! 0, the initial value, states none either and means Mac OS Roman, the
  //! charset Macintosh software wrote these chunks in; any other number is
  //! the one the caller stated itself.
  unsigned int codePageAssumed = 0;

  //! The charset the assumed code page selects, decided when a tag is read.
  Charset::Type assumedCharset()
  {
    // No code page stated is Mac OS Roman: these chunks were written by
    // Macintosh software.
    const unsigned int codePage = codePageAssumed != 0 ? codePageAssumed : 10000;

    if(const auto charset = Charset::typeForCodePage(codePage))
      return *charset;

    // setAssumedCodePage() only ever stores a page this build can convert, so
    // the number reaching here with no table is the initial 0 of a build
    // without the charset tables: its ASCII, the specification's own text,
    // reads the same as ISO 8859-1.
    return Charset::Type::Latin1;
  }

  //! The four native text chunks, which is also the field list of the class.
  const std::array<ByteVector, 4> nativeFieldIds {
    ByteVector("NAME"), ByteVector("AUTH"), ByteVector("(c) "), ByteVector("ANNO")
  };

  //! Reads the text \a data holds.
  String decodeField(const ByteVector &data, Charset::Type charset)
  {
    // The chunk is text from end to end, so nothing is skipped here; decode()
    // itself stops at a NUL, which is how a writer that terminated its text
    // like a C string comes out without one.
    return Charset::decode(data, charset);
  }

  const Map<ByteVector, String> propertyKeyForId = {
    {"NAME", "TITLE"},
    {"AUTH", "ARTIST"},
    {"(c) ", "COPYRIGHT"},
    {"ANNO", "COMMENT"}
  };
}  // namespace

class RIFF::AIFF::NativeTag::NativeTagPrivate
{
public:
  Map<ByteVector, String> fieldListMap;

  //! The charset the tag's code page selects, resolved once when the tag is
  //! read, and the one its text is written back with.
  Charset::Type charset { Charset::Type::Latin1 };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::AIFF::NativeTag::NativeTag() :
  NativeTag(FieldDataMap())
{
}

RIFF::AIFF::NativeTag::NativeTag(const FieldDataMap &fields) :
  d(std::make_unique<NativeTagPrivate>())
{
  d->charset = assumedCharset();

  for(const auto &[id, payload] : fields) {
    if(isNativeField(id)) {
      if(String text = decodeField(payload, d->charset); !text.isEmpty())
        d->fieldListMap[id] = text;
    }
  }
}

RIFF::AIFF::NativeTag::~NativeTag() = default;

String RIFF::AIFF::NativeTag::title() const
{
  return fieldText("NAME");
}

String RIFF::AIFF::NativeTag::artist() const
{
  return fieldText("AUTH");
}

String RIFF::AIFF::NativeTag::album() const
{
  return String();
}

String RIFF::AIFF::NativeTag::comment() const
{
  return fieldText("ANNO");
}

String RIFF::AIFF::NativeTag::genre() const
{
  return String();
}

unsigned int RIFF::AIFF::NativeTag::year() const
{
  return 0;
}

unsigned int RIFF::AIFF::NativeTag::track() const
{
  return 0;
}

void RIFF::AIFF::NativeTag::setTitle(const String &s)
{
  setFieldText("NAME", s);
}

void RIFF::AIFF::NativeTag::setArtist(const String &s)
{
  setFieldText("AUTH", s);
}

void RIFF::AIFF::NativeTag::setAlbum(const String &)
{
}

void RIFF::AIFF::NativeTag::setComment(const String &s)
{
  setFieldText("ANNO", s);
}

void RIFF::AIFF::NativeTag::setGenre(const String &)
{
}

void RIFF::AIFF::NativeTag::setYear(unsigned int)
{
}

void RIFF::AIFF::NativeTag::setTrack(unsigned int)
{
}

bool RIFF::AIFF::NativeTag::isEmpty() const
{
  return d->fieldListMap.isEmpty();
}

PropertyMap RIFF::AIFF::NativeTag::properties() const
{
  PropertyMap props;
  for(const auto &[id, val] : std::as_const(d->fieldListMap)) {
    if(const String key = propertyKeyForId.value(id); !key.isEmpty())
      props[key].append(val);
  }
  return props;
}

void RIFF::AIFF::NativeTag::removeUnsupportedProperties(const StringList &props)
{
  // properties() reports unsupported data for none of its keys - all four
  // chunks have one - so anything here is an id a caller passed itself.
  for(const auto &id : props)
    d->fieldListMap.erase(id.data(String::Latin1));
}

PropertyMap RIFF::AIFF::NativeTag::setProperties(const PropertyMap &props)
{
  static const Map<String, ByteVector> idForPropertyKey = [] {
    Map<String, ByteVector> map;
    for(const auto &[id, key] : propertyKeyForId)
      map[key] = id;
    return map;
  }();

  const PropertyMap origProps = properties();
  for(const auto &[key, _] : origProps) {
    if(!props.contains(key) || props.value(key).isEmpty())
      d->fieldListMap.erase(idForPropertyKey.value(key));
  }

  PropertyMap ignoredProps;
  for(const auto &[key, val] : props) {
    if(ByteVector id = idForPropertyKey.value(key);
       !id.isEmpty() && !val.isEmpty()) {
      d->fieldListMap[id] = val.front();
    }
    else {
      ignoredProps.insert(key, val);
    }
  }
  return ignoredProps;
}

bool RIFF::AIFF::NativeTag::isNativeField(const ByteVector &id)
{
  return std::find(nativeFieldIds.begin(), nativeFieldIds.end(), id) !=
         nativeFieldIds.end();
}

String RIFF::AIFF::NativeTag::fieldText(const ByteVector &id) const
{
  if(d->fieldListMap.contains(id))
    return d->fieldListMap[id];
  return String();
}

void RIFF::AIFF::NativeTag::setFieldText(const ByteVector &id, const String &s)
{
  if(!isNativeField(id)) {
    debug("RIFF::AIFF::NativeTag::setFieldText() - Not a native AIFF field, ignored.");
    return;
  }

  if(!s.isEmpty())
    d->fieldListMap[id] = s;
  else
    removeField(id);
}

void RIFF::AIFF::NativeTag::removeField(const ByteVector &id)
{
  if(d->fieldListMap.contains(id))
    d->fieldListMap.erase(id);
}

RIFF::AIFF::NativeTag::FieldDataMap RIFF::AIFF::NativeTag::render() const
{
  FieldDataMap chunks;

  for(const auto &id : nativeFieldIds) {
    chunks[id] = d->fieldListMap.contains(id)
      ? Charset::encode(d->fieldListMap[id], d->charset)
      : ByteVector();
  }

  return chunks;
}

unsigned int RIFF::AIFF::NativeTag::assumedCodePage()
{
  return codePageAssumed;
}

void RIFF::AIFF::NativeTag::setAssumedCodePage(unsigned int codePage)
{
  if(codePage != 0 && !Charset::typeForCodePage(codePage)) {
    debug(Utils::formatString(
      "RIFF::AIFF::NativeTag::setAssumedCodePage() - Unsupported code page %u, ignored.",
      codePage));
    return;
  }

  codePageAssumed = codePage;
}
