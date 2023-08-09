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

#include "id3v2frame.h"

#include <array>
#include <bitset>

#include "tdebug.h"
#include "tstringlist.h"
#include "tzlib.h"

#include "id3v2tag.h"
#include "id3v2synchdata.h"

#include "tpropertymap.h"
#include "frames/textidentificationframe.h"
#include "frames/urllinkframe.h"
#include "frames/unsynchronizedlyricsframe.h"
#include "frames/commentsframe.h"
#include "frames/uniquefileidentifierframe.h"
#include "frames/unknownframe.h"
#include "frames/podcastframe.h"

using namespace TagLib;
using namespace ID3v2;

class Frame::FramePrivate
{
public:
  FramePrivate() :
    header(nullptr)
    {}

  ~FramePrivate()
  {
    delete header;
  }

  FramePrivate(const FramePrivate &) = delete;
  FramePrivate &operator=(const FramePrivate &) = delete;

  Frame::Header *header;
};

namespace
{
  bool isValidFrameID(const ByteVector &frameID)
  {
    if(frameID.size() != 4)
      return false;

    return std::none_of(frameID.begin(), frameID.end(), [](auto c) { return (c < 'A' || c > 'Z') && (c < '0' || c > '9'); });
  }
}  // namespace

////////////////////////////////////////////////////////////////////////////////
// static methods
////////////////////////////////////////////////////////////////////////////////

ByteVector Frame::textDelimiter(String::Type t)
{
  if(t == String::UTF16 || t == String::UTF16BE || t == String::UTF16LE)
    return ByteVector(2, '\0');
  return ByteVector(1, '\0');
}

const String Frame::instrumentPrefix("PERFORMER:");
const String Frame::commentPrefix("COMMENT:");
const String Frame::lyricsPrefix("LYRICS:");
const String Frame::urlPrefix("URL:");

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

unsigned int Frame::headerSize()
{
  return d->header->size();
}

Frame *Frame::createTextualFrame(const String &key, const StringList &values) //static
{
  // check if the key is contained in the key<=>frameID mapping
  ByteVector frameID = keyToFrameID(key);
  if(!frameID.isEmpty()) {
    // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number), GRP1 (Grouping) are in fact text frames.
    if(frameID[0] == 'T' || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN" || frameID == "GRP1"){ // text frame
      auto frame = new TextIdentificationFrame(frameID, String::UTF8);
      frame->setText(values);
      return frame;
    } if((frameID[0] == 'W') && (values.size() == 1)){  // URL frame (not WXXX); support only one value
        auto frame = new UrlLinkFrame(frameID);
        frame->setUrl(values.front());
        return frame;
    } if(frameID == "PCST") {
      return new PodcastFrame();
    }
  }
  if(key == "MUSICBRAINZ_TRACKID" && values.size() == 1) {
    auto frame = new UniqueFileIdentifierFrame("http://musicbrainz.org", values.front().data(String::UTF8));
    return frame;
  }
  // now we check if it's one of the "special" cases:
  // -LYRICS: depending on the number of values, use USLT or TXXX (with description=LYRICS)
  if((key == "LYRICS" || key.startsWith(lyricsPrefix)) && values.size() == 1){
    auto frame = new UnsynchronizedLyricsFrame(String::UTF8);
    frame->setDescription(key == "LYRICS" ? key : key.substr(lyricsPrefix.size()));
    frame->setText(values.front());
    return frame;
  }
  // -URL: depending on the number of values, use WXXX or TXXX (with description=URL)
  if((key == "URL" || key.startsWith(urlPrefix)) && values.size() == 1){
    auto frame = new UserUrlLinkFrame(String::UTF8);
    frame->setDescription(key == "URL" ? key : key.substr(urlPrefix.size()));
    frame->setUrl(values.front());
    return frame;
  }
  // -COMMENT: depending on the number of values, use COMM or TXXX (with description=COMMENT)
  if((key == "COMMENT" || key.startsWith(commentPrefix)) && values.size() == 1){
    auto frame = new CommentsFrame(String::UTF8);
    if (key != "COMMENT"){
      frame->setDescription(key.substr(commentPrefix.size()));
    }
    frame->setText(values.front());
    return frame;
  }
  // if non of the above cases apply, we use a TXXX frame with the key as description
  return new UserTextIdentificationFrame(keyToTXXX(key), values, String::UTF8);
}

Frame::~Frame() = default;

ByteVector Frame::frameID() const
{
  if(d->header)
    return d->header->frameID();
  return ByteVector();
}

unsigned int Frame::size() const
{
  if(d->header)
    return d->header->frameSize();
  return 0;
}

void Frame::setData(const ByteVector &data)
{
  parse(data);
}

void Frame::setText(const String &)
{

}

ByteVector Frame::render() const
{
  ByteVector fieldData = renderFields();
  d->header->setFrameSize(fieldData.size());
  ByteVector headerData = d->header->render();

  return headerData + fieldData;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

Frame::Frame(const ByteVector &data) :
  d(std::make_unique<FramePrivate>())
{
  d->header = new Header(data);
}

Frame::Frame(Header *h) :
  d(std::make_unique<FramePrivate>())
{
  d->header = h;
}

Frame::Header *Frame::header() const
{
  return d->header;
}

void Frame::setHeader(Header *h, bool deleteCurrent)
{
  if(deleteCurrent)
    delete d->header;

  d->header = h;
}

void Frame::parse(const ByteVector &data)
{
  if(d->header)
    d->header->setData(data);
  else
    d->header = new Header(data);

  parseFields(fieldData(data));
}

ByteVector Frame::fieldData(const ByteVector &frameData) const
{
  unsigned int headerSize = d->header->size();

  unsigned int frameDataOffset = headerSize;
  unsigned int frameDataLength = size();

  if(d->header->compression() || d->header->dataLengthIndicator()) {
    frameDataLength = SynchData::toUInt(frameData.mid(headerSize, 4));
    frameDataOffset += 4;
  }

  if(zlib::isAvailable() && d->header->compression() && !d->header->encryption()) {
    if(frameData.size() <= frameDataOffset) {
      debug("Compressed frame doesn't have enough data to decode");
      return ByteVector();
    }

    const ByteVector outData = zlib::decompress(frameData.mid(frameDataOffset));
    if(!outData.isEmpty() && frameDataLength != outData.size()) {
      debug("frameDataLength does not match the data length returned by zlib");
    }

    return outData;
  }

  return frameData.mid(frameDataOffset, frameDataLength);
}

String Frame::readStringField(const ByteVector &data, String::Type encoding, int *position)
{
  int start = 0;

  if(!position)
    position = &start;

  ByteVector delimiter = textDelimiter(encoding);

  int end = data.find(delimiter, *position, delimiter.size());

  if(end < *position)
    return String();

  String str;
  if(encoding == String::Latin1)
    str = Tag::latin1StringHandler()->parse(data.mid(*position, end - *position));
  else
    str = String(data.mid(*position, end - *position), encoding);

  *position = end + delimiter.size();

  return str;
}

String::Type Frame::checkTextEncoding(const StringList &fields, String::Type encoding) const
{
  if((encoding == String::UTF8 || encoding == String::UTF16BE) && header()->version() != 4)
    return String::UTF16;

  if(encoding != String::Latin1)
    return encoding;

  for(auto it = fields.begin(); it != fields.end(); ++it) {
    if(!(*it).isLatin1()) {
      if(header()->version() == 4) {
        debug("Frame::checkEncoding() -- Rendering using UTF8.");
        return String::UTF8;
      }
      debug("Frame::checkEncoding() -- Rendering using UTF16.");
      return String::UTF16;
    }
  }

  return String::Latin1;
}

namespace
{
  constexpr std::array frameTranslation {
    // Text information frames
    std::pair("TALB", "ALBUM"),
    std::pair("TBPM", "BPM"),
    std::pair("TCOM", "COMPOSER"),
    std::pair("TCON", "GENRE"),
    std::pair("TCOP", "COPYRIGHT"),
    std::pair("TDEN", "ENCODINGTIME"),
    std::pair("TDLY", "PLAYLISTDELAY"),
    std::pair("TDOR", "ORIGINALDATE"),
    std::pair("TDRC", "DATE"),
    // std::pair("TRDA", "DATE"), // id3 v2.3, replaced by TDRC in v2.4
    // std::pair("TDAT", "DATE"), // id3 v2.3, replaced by TDRC in v2.4
    // std::pair("TYER", "DATE"), // id3 v2.3, replaced by TDRC in v2.4
    // std::pair("TIME", "DATE"), // id3 v2.3, replaced by TDRC in v2.4
    std::pair("TDRL", "RELEASEDATE"),
    std::pair("TDTG", "TAGGINGDATE"),
    std::pair("TENC", "ENCODEDBY"),
    std::pair("TEXT", "LYRICIST"),
    std::pair("TFLT", "FILETYPE"),
    // std::pair("TIPL", "INVOLVEDPEOPLE"), handled separately
    std::pair("TIT1", "WORK"), // 'Work' in iTunes
    std::pair("TIT2", "TITLE"),
    std::pair("TIT3", "SUBTITLE"),
    std::pair("TKEY", "INITIALKEY"),
    std::pair("TLAN", "LANGUAGE"),
    std::pair("TLEN", "LENGTH"),
    // std::pair("TMCL", "MUSICIANCREDITS"), handled separately
    std::pair("TMED", "MEDIA"),
    std::pair("TMOO", "MOOD"),
    std::pair("TOAL", "ORIGINALALBUM"),
    std::pair("TOFN", "ORIGINALFILENAME"),
    std::pair("TOLY", "ORIGINALLYRICIST"),
    std::pair("TOPE", "ORIGINALARTIST"),
    std::pair("TOWN", "OWNER"),
    std::pair("TPE1", "ARTIST"),
    std::pair("TPE2", "ALBUMARTIST"), // id3's spec says 'PERFORMER', but most programs use 'ALBUMARTIST'
    std::pair("TPE3", "CONDUCTOR"),
    std::pair("TPE4", "REMIXER"),     // could also be ARRANGER
    std::pair("TPOS", "DISCNUMBER"),
    std::pair("TPRO", "PRODUCEDNOTICE"),
    std::pair("TPUB", "LABEL"),
    std::pair("TRCK", "TRACKNUMBER"),
    std::pair("TRSN", "RADIOSTATION"),
    std::pair("TRSO", "RADIOSTATIONOWNER"),
    std::pair("TSOA", "ALBUMSORT"),
    std::pair("TSOC", "COMPOSERSORT"),
    std::pair("TSOP", "ARTISTSORT"),
    std::pair("TSOT", "TITLESORT"),
    std::pair("TSO2", "ALBUMARTISTSORT"), // non-standard, used by iTunes
    std::pair("TSRC", "ISRC"),
    std::pair("TSSE", "ENCODING"),
    std::pair("TSST", "DISCSUBTITLE"),
    // URL frames
    std::pair("WCOP", "COPYRIGHTURL"),
    std::pair("WOAF", "FILEWEBPAGE"),
    std::pair("WOAR", "ARTISTWEBPAGE"),
    std::pair("WOAS", "AUDIOSOURCEWEBPAGE"),
    std::pair("WORS", "RADIOSTATIONWEBPAGE"),
    std::pair("WPAY", "PAYMENTWEBPAGE"),
    std::pair("WPUB", "PUBLISHERWEBPAGE"),
    // std::pair("WXXX", "URL"), handled specially
    // Other frames
    std::pair("COMM", "COMMENT"),
    // std::pair("USLT", "LYRICS"), handled specially
    // Apple iTunes proprietary frames
    std::pair("PCST", "PODCAST"),
    std::pair("TCAT", "PODCASTCATEGORY"),
    std::pair("TDES", "PODCASTDESC"),
    std::pair("TGID", "PODCASTID"),
    std::pair("WFED", "PODCASTURL"),
    std::pair("MVNM", "MOVEMENTNAME"),
    std::pair("MVIN", "MOVEMENTNUMBER"),
    std::pair("GRP1", "GROUPING"),
    std::pair("TCMP", "COMPILATION"),
  };

  constexpr std::array txxxFrameTranslation {
    std::pair("MUSICBRAINZ ALBUM ID", "MUSICBRAINZ_ALBUMID"),
    std::pair("MUSICBRAINZ ARTIST ID", "MUSICBRAINZ_ARTISTID"),
    std::pair("MUSICBRAINZ ALBUM ARTIST ID", "MUSICBRAINZ_ALBUMARTISTID"),
    std::pair("MUSICBRAINZ ALBUM RELEASE COUNTRY", "RELEASECOUNTRY"),
    std::pair("MUSICBRAINZ ALBUM STATUS", "RELEASESTATUS"),
    std::pair("MUSICBRAINZ ALBUM TYPE", "RELEASETYPE"),
    std::pair("MUSICBRAINZ RELEASE GROUP ID", "MUSICBRAINZ_RELEASEGROUPID"),
    std::pair("MUSICBRAINZ RELEASE TRACK ID", "MUSICBRAINZ_RELEASETRACKID"),
    std::pair("MUSICBRAINZ WORK ID", "MUSICBRAINZ_WORKID"),
    std::pair("ACOUSTID ID", "ACOUSTID_ID"),
    std::pair("ACOUSTID FINGERPRINT", "ACOUSTID_FINGERPRINT"),
    std::pair("MUSICIP PUID", "MUSICIP_PUID"),
  };

  // list of deprecated frames and their successors
  constexpr std::array deprecatedFrames {
    std::pair("TRDA", "TDRC"), // 2.3 -> 2.4 (http://en.wikipedia.org/wiki/ID3)
    std::pair("TDAT", "TDRC"), // 2.3 -> 2.4
    std::pair("TYER", "TDRC"), // 2.3 -> 2.4
    std::pair("TIME", "TDRC"), // 2.3 -> 2.4
  };
}  // namespace

String Frame::frameIDToKey(const ByteVector &id)
{
  ByteVector id24 = id;
  for(const auto &[o, t] : deprecatedFrames) {
    if(id24 == o) {
      id24 = t;
      break;
    }
  }
  for(const auto &[o, t] : frameTranslation) {
    if(id24 == o)
      return t;
  }
  return String();
}

ByteVector Frame::keyToFrameID(const String &s)
{
  const String key = s.upper();
  for(const auto &[o, t] : frameTranslation) {
    if(key == t)
      return o;
  }
  return ByteVector();
}

String Frame::txxxToKey(const String &description)
{
  const String d = description.upper();
  for(const auto &[o, t] : txxxFrameTranslation) {
    if(d == o)
      return t;
  }
  return d;
}

String Frame::keyToTXXX(const String &s)
{
  const String key = s.upper();
  for(const auto &[o, t] : txxxFrameTranslation) {
    if(key == t)
      return o;
  }
  return s;
}

PropertyMap Frame::asProperties() const
{
  if(dynamic_cast< const UnknownFrame *>(this)) {
    PropertyMap m;
    m.unsupportedData().append("UNKNOWN/" + frameID());
    return m;
  }
  const ByteVector &id = frameID();
  PropertyMap m;
  m.unsupportedData().append(id);
  return m;
}

void Frame::splitProperties(const PropertyMap &original, PropertyMap &singleFrameProperties,
          PropertyMap &tiplProperties, PropertyMap &tmclProperties)
{
  singleFrameProperties.clear();
  tiplProperties.clear();
  tmclProperties.clear();
  for(auto it = original.begin(); it != original.end(); ++it) {
    if(TextIdentificationFrame::involvedPeopleMap().contains(it->first))
      tiplProperties.insert(it->first, it->second);
    else if(it->first.startsWith(TextIdentificationFrame::instrumentPrefix))
      tmclProperties.insert(it->first, it->second);
    else
      singleFrameProperties.insert(it->first, it->second);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Frame::Header class
////////////////////////////////////////////////////////////////////////////////

class Frame::Header::HeaderPrivate
{
public:
  HeaderPrivate() :
    frameSize(0),
    version(4),
    tagAlterPreservation(false),
    fileAlterPreservation(false),
    readOnly(false),
    groupingIdentity(false),
    compression(false),
    encryption(false),
    unsynchronisation(false),
    dataLengthIndicator(false)
    {}

  ByteVector frameID;
  unsigned int frameSize;
  unsigned int version;

  // flags

  bool tagAlterPreservation;
  bool fileAlterPreservation;
  bool readOnly;
  bool groupingIdentity;
  bool compression;
  bool encryption;
  bool unsynchronisation;
  bool dataLengthIndicator;
};

////////////////////////////////////////////////////////////////////////////////
// public members (Frame::Header)
////////////////////////////////////////////////////////////////////////////////

unsigned int Frame::Header::size()
{
  switch(d->version) {
  case 0:
  case 1:
  case 2:
    return 6;
  case 3:
  case 4:
  default:
    return 10;
  }
}

Frame::Header::Header(const ByteVector &data, unsigned int version) :
  d(std::make_unique<HeaderPrivate>())
{
  setData(data, version);
}

Frame::Header::~Header() = default;

void Frame::Header::setData(const ByteVector &data, unsigned int version)
{
  d->version = version;

  switch(version) {
  case 0:
  case 1:
  case 2:
  {
    // ID3v2.2

    if(data.size() < 3) {
      debug("You must at least specify a frame ID.");
      return;
    }

    // Set the frame ID -- the first three bytes

    d->frameID = data.mid(0, 3);

    // If the full header information was not passed in, do not continue to the
    // steps to parse the frame size and flags.

    if(data.size() < 6) {
      d->frameSize = 0;
      return;
    }

    d->frameSize = data.toUInt(3, 3, true);

    break;
  }
  case 3:
  {
    // ID3v2.3

    if(data.size() < 4) {
      debug("You must at least specify a frame ID.");
      return;
    }

    // Set the frame ID -- the first four bytes

    d->frameID = data.mid(0, 4);

    // If the full header information was not passed in, do not continue to the
    // steps to parse the frame size and flags.

    if(data.size() < 10) {
      d->frameSize = 0;
      return;
    }

    // Set the size -- the frame size is the four bytes starting at byte four in
    // the frame header (structure 4)

    d->frameSize = data.toUInt(4U);

    { // read the first byte of flags
      std::bitset<8> flags(data[8]);
      d->tagAlterPreservation  = flags[7]; // (structure 3.3.1.a)
      d->fileAlterPreservation = flags[6]; // (structure 3.3.1.b)
      d->readOnly              = flags[5]; // (structure 3.3.1.c)
    }

    { // read the second byte of flags
      std::bitset<8> flags(data[9]);
      d->compression         = flags[7]; // (structure 3.3.1.i)
      d->encryption          = flags[6]; // (structure 3.3.1.j)
      d->groupingIdentity    = flags[5]; // (structure 3.3.1.k)
    }
    break;
  }
  case 4:
  default:
  {
    // ID3v2.4

    if(data.size() < 4) {
      debug("You must at least specify a frame ID.");
      return;
    }

    // Set the frame ID -- the first four bytes

    d->frameID = data.mid(0, 4);

    // If the full header information was not passed in, do not continue to the
    // steps to parse the frame size and flags.

    if(data.size() < 10) {
      d->frameSize = 0;
      return;
    }

    // Set the size -- the frame size is the four bytes starting at byte four in
    // the frame header (structure 4)

    d->frameSize = SynchData::toUInt(data.mid(4, 4));
#ifndef NO_ITUNES_HACKS
    // iTunes writes v2.4 tags with v2.3-like frame sizes
    if(d->frameSize > 127) {
      if(!isValidFrameID(data.mid(d->frameSize + 10, 4))) {
        unsigned int uintSize = data.toUInt(4U);
        if(isValidFrameID(data.mid(uintSize + 10, 4))) {
          d->frameSize = uintSize;
        }
      }
    }
#endif

    { // read the first byte of flags
      std::bitset<8> flags(data[8]);
      d->tagAlterPreservation  = flags[6]; // (structure 4.1.1.a)
      d->fileAlterPreservation = flags[5]; // (structure 4.1.1.b)
      d->readOnly              = flags[4]; // (structure 4.1.1.c)
    }

    { // read the second byte of flags
      std::bitset<8> flags(data[9]);
      d->groupingIdentity    = flags[6]; // (structure 4.1.2.h)
      d->compression         = flags[3]; // (structure 4.1.2.k)
      d->encryption          = flags[2]; // (structure 4.1.2.m)
      d->unsynchronisation   = flags[1]; // (structure 4.1.2.n)
      d->dataLengthIndicator = flags[0]; // (structure 4.1.2.p)
    }
    break;
  }
  }
}

ByteVector Frame::Header::frameID() const
{
  return d->frameID;
}

void Frame::Header::setFrameID(const ByteVector &id)
{
  d->frameID = id.mid(0, 4);
}

unsigned int Frame::Header::frameSize() const
{
  return d->frameSize;
}

void Frame::Header::setFrameSize(unsigned int size)
{
  d->frameSize = size;
}

unsigned int Frame::Header::version() const
{
  return d->version;
}

void Frame::Header::setVersion(unsigned int version)
{
  d->version = version;
}

bool Frame::Header::tagAlterPreservation() const
{
  return d->tagAlterPreservation;
}

void Frame::Header::setTagAlterPreservation(bool preserve)
{
  d->tagAlterPreservation = preserve;
}

bool Frame::Header::fileAlterPreservation() const
{
  return d->fileAlterPreservation;
}

bool Frame::Header::readOnly() const
{
  return d->readOnly;
}

bool Frame::Header::groupingIdentity() const
{
  return d->groupingIdentity;
}

bool Frame::Header::compression() const
{
  return d->compression;
}

bool Frame::Header::encryption() const
{
  return d->encryption;
}

bool Frame::Header::unsycronisation() const
{
  return unsynchronisation();
}

bool Frame::Header::unsynchronisation() const
{
  return d->unsynchronisation;
}

bool Frame::Header::dataLengthIndicator() const
{
  return d->dataLengthIndicator;
}

ByteVector Frame::Header::render() const
{
  ByteVector flags(2, static_cast<char>(0)); // just blank for the moment

  ByteVector v = d->frameID +
    (d->version == 3
      ? ByteVector::fromUInt(d->frameSize)
      : SynchData::fromUInt(d->frameSize)) +
    flags;

  return v;
}
