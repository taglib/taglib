/***************************************************************************
    copyright            : (C) 2008 by Scott Wheeler
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

#include "wavfile.h"

#include <optional>
#include <utility>
#include <vector>

#include "tcharset.h"
#include "tdebug.h"
#include "tpropertymap.h"
#include "tagutils.h"
#include "infotag.h"
#include "tagunion.h"

using namespace TagLib;

namespace
{
  enum { ID3v2Index = 0, InfoIndex = 1 };

  /*!
   * Render CSET chunk.
   *
   * A CSET chunk declares the charset of the file's text and is laid out as
   *   <CSET chunk> -> CSET( <wCodePage:WORD> <wCountryCode:WORD>
   *                         <wLanguageCode:WORD> <wDialect:WORD> )
   * so the code page is its first 16 bits. The other three fields name the
   * country, language and dialect the text was written for; zero is the
   * spec's "no opinion" value for each of them.
   * https://www.robotplanet.dk/audio/wav_meta_data/riff_mci.pdf#page=25
   */
  ByteVector renderCSET(unsigned int codePage)
  {
    constexpr unsigned int CSETChunkSize = 8;

    ByteVector cset = ByteVector::fromShort(static_cast<short>(codePage), false);
    cset.resize(CSETChunkSize, 0);
    return cset;
  }

  /*!
   * Reads the code page out of a CSET payload. Two bytes are enough: that is
   * the whole of wCodePage, and the three fields behind it only add detail the
   * code page does not depend on. A payload shorter than that is malformed, as
   * it cannot hold the number the chunk exists to declare, so it is rejected
   * rather than guessed at and the file keeps the RIFF default.
   */
  std::optional<unsigned int> parseCSET(const ByteVector &data)
  {
    if(data.size() >= 2)
      return static_cast<unsigned short>(data.toShort(0, false));
    return std::nullopt;
  }
} // namespace

class RIFF::WAV::File::FilePrivate
{
public:
  FilePrivate(const ID3v2::FrameFactory *frameFactory)
        : ID3v2FrameFactory(frameFactory ? frameFactory
                                         : ID3v2::FrameFactory::instance())
  {
  }

  ~FilePrivate() = default;

  const ID3v2::FrameFactory *ID3v2FrameFactory;
  std::unique_ptr<Properties> properties;
  TagUnion tag;

  bool hasID3v2 { false };
  bool hasInfo { false };
  bool hasCSET { false };
  bool hasiXML { false };
  bool hasBEXT { false };
  bool hasDISP { false };
  bool dispModified { false };

  String iXMLData;
  ByteVector bextData;
  ByteVectorList dispData;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool RIFF::WAV::File::isSupported(IOStream *stream)
{
  // A WAV file has to start with "RIFF????WAVE", or with the long-form "RF64" or
  // "BW64" magic used past 4 GB.

  const ByteVector id = Utils::readHeader(stream, 12, false);
  return (id.startsWith("RIFF") || id.startsWith("RF64") || id.startsWith("BW64")) &&
         id.containsAt("WAVE", 8);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RIFF::WAV::File::File(FileName file, bool readProperties, Properties::ReadStyle,
                      ID3v2::FrameFactory *frameFactory) :
  RIFF::File(file, LittleEndian),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties);
}

RIFF::WAV::File::File(IOStream *stream, bool readProperties, Properties::ReadStyle,
                      ID3v2::FrameFactory *frameFactory) :
  RIFF::File(stream, LittleEndian),
  d(std::make_unique<FilePrivate>(frameFactory))
{
  if(isOpen())
    read(readProperties);
}

RIFF::WAV::File::~File() = default;

TagLib::Tag *RIFF::WAV::File::tag() const
{
  return &d->tag;
}

ID3v2::Tag *RIFF::WAV::File::ID3v2Tag() const
{
  return d->tag.access<ID3v2::Tag>(ID3v2Index, false);
}

RIFF::Info::Tag *RIFF::WAV::File::InfoTag() const
{
  return d->tag.access<RIFF::Info::Tag>(InfoIndex, false);
}

String RIFF::WAV::File::iXMLData() const
{
  return d->iXMLData;
}

void RIFF::WAV::File::setiXMLData(const String &data)
{
  d->iXMLData = data;
}

ByteVector RIFF::WAV::File::BEXTData() const
{
  return d->bextData;
}

void RIFF::WAV::File::setBEXTData(const ByteVector &data)
{
  d->bextData = data;
}

ByteVectorList RIFF::WAV::File::DISPData() const
{
  return d->dispData;
}

void RIFF::WAV::File::setDISPData(const ByteVectorList &data)
{
  d->dispData = data;
  d->dispModified = true;
}

void RIFF::WAV::File::strip(TagTypes tags)
{
  removeTagChunks(tags);

  if(tags & ID3v2)
    d->tag.set(ID3v2Index, new ID3v2::Tag(nullptr, 0, d->ID3v2FrameFactory));

  if(tags & Info)
    d->tag.set(InfoIndex, new RIFF::Info::Tag());
}

PropertyMap RIFF::WAV::File::properties() const
{
  return d->tag.properties();
}

void RIFF::WAV::File::removeUnsupportedProperties(const StringList &unsupported)
{
  d->tag.removeUnsupportedProperties(unsupported);
}

PropertyMap RIFF::WAV::File::setProperties(const PropertyMap &properties)
{
  InfoTag()->setProperties(properties);
  return ID3v2Tag()->setProperties(properties);
}

RIFF::WAV::Properties *RIFF::WAV::File::audioProperties() const
{
  return d->properties.get();
}

bool RIFF::WAV::File::save()
{
  return RIFF::WAV::File::save(AllTags);
}

bool RIFF::WAV::File::save(TagTypes tags, StripTags strip, ID3v2::Version version)
{
  if(readOnly()) {
    debug("RIFF::WAV::File::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("RIFF::WAV::File::save() -- Trying to save invalid file.");
    return false;
  }

  if(strip == StripOthers)
    File::strip(static_cast<TagTypes>(AllTags & ~tags));

  if(!d->bextData.isEmpty()) {
    removeChunk("bext");
    setChunkData("bext", d->bextData);
    d->hasBEXT = true;
  }
  else if(d->hasBEXT) {
    removeChunk("bext");
    d->hasBEXT = false;
  }

  if(!d->iXMLData.isEmpty()) {
    removeChunk("iXML");
    setChunkData("iXML", d->iXMLData.data(String::UTF8));
    d->hasiXML = true;
  }
  else if(d->hasiXML) {
    removeChunk("iXML");
    d->hasiXML = false;
  }

  if(tags & ID3v2) {
    removeTagChunks(ID3v2);

    if(ID3v2Tag() && !ID3v2Tag()->isEmpty()) {
      setChunkData("ID3 ", ID3v2Tag()->render(version));
      d->hasID3v2 = true;
    }
  }

  if(tags & Info) {
    removeTagChunks(Info);

    // render() is empty when no field survives, which is also when there is
    // nothing for a CSET chunk to describe.
    const ByteVector info = InfoTag() ? InfoTag()->render() : ByteVector();
    if(!info.isEmpty()) {
      setChunkData("LIST", info, true);

      // The INFO tag was just rendered as UTF-8, so say so.
      // Without this a reader would fall back to the RIFF default of Latin1 and decode wrongly.
      setChunkData("CSET", renderCSET(Charset::codePageForType(Charset::Type::UTF8)));
      d->hasInfo = true;
      d->hasCSET = true;
    }
  }

  if(d->dispModified) {
    // There can be several DISP chunks, one per clipboard format.  Those
    // already in the file are overwritten in place to keep their position,
    // then the extra ones are appended or the surplus ones removed.
    std::vector<unsigned int> chunkIndexes;
    for(unsigned int i = 0; i < chunkCount(); ++i) {
      if(chunkName(i) == "DISP")
        chunkIndexes.push_back(i);
    }

    size_t count = 0;
    for(const auto &data : std::as_const(d->dispData)) {
      if(count < chunkIndexes.size())
        setChunkData(chunkIndexes[count], data);
      else
        appendChunk("DISP", data);
      ++count;
    }

    // Remove from the end, as removing a chunk shifts the indexes behind it.
    for(size_t i = chunkIndexes.size(); i > count; --i)
      removeChunk(chunkIndexes[i - 1]);

    d->hasDISP = !d->dispData.isEmpty();
    d->dispModified = false;
  }

  return true;
}

bool RIFF::WAV::File::hasID3v2Tag() const
{
  return d->hasID3v2;
}

bool RIFF::WAV::File::hasInfoTag() const
{
  return d->hasInfo;
}

bool RIFF::WAV::File::hasiXMLData() const
{
  return d->hasiXML;
}

bool RIFF::WAV::File::hasBEXTData() const
{
  return d->hasBEXT;
}

bool RIFF::WAV::File::hasDISPData() const
{
  return d->hasDISP;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void RIFF::WAV::File::read(bool readProperties)
{
  std::optional<unsigned int> codePage;
  ByteVector infoData;

  for(unsigned int i = 0; i < chunkCount(); ++i) {
    if(const ByteVector name = chunkName(i); name == "ID3 " || name == "id3 ") {
      if(!d->tag[ID3v2Index]) {
        d->tag.set(ID3v2Index, new ID3v2::Tag(this, chunkOffset(i),
                                              d->ID3v2FrameFactory));
        d->hasID3v2 = true;
      }
      else {
        debug("RIFF::WAV::File::read() - Duplicate ID3v2 tag found.");
      }
    }
    else if(name == "CSET") {
      if(d->hasCSET) {
        debug("RIFF::WAV::File::read() - Duplicate CSET chunk found.");
      }
      else if(const ByteVector data = chunkData(i); const auto page = parseCSET(data)) {
        codePage = *page;
        d->hasCSET = true;
      }
      else {
        debug("RIFF::WAV::File::read() - Invalid CSET chunk found.");
      }
    }
    else if(name == "LIST") {
      if(const ByteVector data = chunkData(i); data.startsWith("INFO")) {
        if(!infoData.isEmpty()) {
          debug("RIFF::WAV::File::read() - Duplicate INFO tag found.");
        }
        else {
          infoData = data;
        }
      }
    }
    else if(name == "iXML") {
      d->hasiXML = true;
      d->iXMLData = String(chunkData(i), String::UTF8);
    }
    else if(name == "bext") {
      d->hasBEXT = true;
      d->bextData = chunkData(i);
    }
    else if(name == "DISP") {
      d->hasDISP = true;
      d->dispData.append(chunkData(i));
    }
  }

  if(!infoData.isEmpty()) {
    d->tag.set(InfoIndex, new RIFF::Info::Tag(infoData, codePage.value_or(0)));
    d->hasInfo = true;
  }

  if(!d->tag[ID3v2Index])
    d->tag.set(ID3v2Index, new ID3v2::Tag(nullptr, 0, d->ID3v2FrameFactory));

  if(!d->tag[InfoIndex])
    d->tag.set(InfoIndex, new RIFF::Info::Tag());

  if(readProperties)
    d->properties = std::make_unique<Properties>(this, Properties::Average);
}

void RIFF::WAV::File::removeTagChunks(TagTypes tags)
{
  if((tags & ID3v2) && d->hasID3v2) {
    removeChunk("ID3 ");
    removeChunk("id3 ");

    d->hasID3v2 = false;
  }

  if((tags & Info) && (d->hasInfo || d->hasCSET)) {
    for(int i = static_cast<int>(chunkCount()) - 1; i >= 0; --i) {
      if(chunkName(i) == "LIST" && chunkData(i).startsWith("INFO"))
        removeChunk(i);
    }

    // A CSET chunk is only meaningful together with an INFO tag, so it goes
    // when the INFO tag goes. save() writes a new one if it writes the tag back.
    removeChunk("CSET");

    d->hasInfo = false;
    d->hasCSET = false;
  }
}
