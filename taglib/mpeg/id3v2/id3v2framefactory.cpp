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

#include "id3v2framefactory.h"

#include <array>
#include <utility>

#include "tdebug.h"
#include "tzlib.h"
#include "id3v2synchdata.h"
#include "id3v1genres.h"
#include "frames/attachedpictureframe.h"
#include "frames/commentsframe.h"
#include "frames/relativevolumeframe.h"
#include "frames/textidentificationframe.h"
#include "frames/uniquefileidentifierframe.h"
#include "frames/unknownframe.h"
#include "frames/generalencapsulatedobjectframe.h"
#include "frames/urllinkframe.h"
#include "frames/unsynchronizedlyricsframe.h"
#include "frames/popularimeterframe.h"
#include "frames/privateframe.h"
#include "frames/ownershipframe.h"
#include "frames/synchronizedlyricsframe.h"
#include "frames/eventtimingcodesframe.h"
#include "frames/chapterframe.h"
#include "frames/tableofcontentsframe.h"
#include "frames/podcastframe.h"

using namespace TagLib;
using namespace ID3v2;

namespace
{
  void updateGenre(TextIdentificationFrame *frame)
  {
    StringList fields = frame->fieldList();
    StringList newfields;

    for(auto s : std::as_const(fields)) {
      int offset = 0;
      int end = 0;

      while(static_cast<int>(s.length()) > offset && s[offset] == '(' &&
            (end = s.find(")", offset + 1)) > offset) {
        // "(12)Genre"
        const String genreCode = s.substr(offset + 1, end - 1);
        s = s.substr(end + 1);
        bool ok;
        int number = genreCode.toInt(&ok);
        if((ok && number >= 0 && number <= 255 &&
            ID3v1::genre(number) != s) ||
           genreCode == "RX" || genreCode == "CR")
          newfields.append(genreCode);
      }
      if(!s.isEmpty())
        // "Genre" or "12"
        newfields.append(s);
    }

    if(newfields.isEmpty())
      fields.append(String());

    frame->setText(newfields);
  }
}  // namespace

class FrameFactory::FrameFactoryPrivate
{
public:
  String::Type defaultEncoding { String::Latin1 };
  bool useDefaultEncoding { false };

  template <class T> void setTextEncoding(T *frame)
  {
    if(useDefaultEncoding)
      frame->setTextEncoding(defaultEncoding);
  }
};

FrameFactory FrameFactory::factory;

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FrameFactory *FrameFactory::instance()
{
  return &factory;
}

std::pair<Frame::Header *, bool> FrameFactory::prepareFrameHeader(
  ByteVector &data, const Header *tagHeader) const
{
  unsigned int version = tagHeader->majorVersion();
  auto header = new Frame::Header(data, version);
  ByteVector frameID = header->frameID();

  // A quick sanity check -- make sure that the frameID is 4 uppercase Latin1
  // characters.  Also make sure that there is data in the frame.

  if(frameID.size() != (version < 3U ? 3U : 4U) ||
     header->frameSize() <= static_cast<unsigned int>(header->dataLengthIndicator() ? 4 : 0) ||
     header->frameSize() > data.size())
  {
    delete header;
    return {nullptr, false};
  }

#ifndef NO_ITUNES_HACKS
  if(version == 3 && frameID[3] == '\0') {
    // iTunes v2.3 tags store v2.2 frames - convert now
    frameID = frameID.mid(0, 3);
    header->setFrameID(frameID);
    header->setVersion(2);
    updateFrame(header);
    header->setVersion(3);
  }
#endif

  if(std::any_of(frameID.cbegin(), frameID.cend(),
      [](auto c) { return (c < 'A' || c > 'Z') && (c < '0' || c > '9'); })) {
    delete header;
    return { nullptr, false };
  }

  if(version > 3 && (tagHeader->unsynchronisation() || header->unsynchronisation())) {
    // Data lengths are not part of the encoded data, but since they are synch-safe
    // integers they will be never actually encoded.
    ByteVector frameData = data.mid(header->size(), header->frameSize());
    frameData = SynchData::decode(frameData);
    data = data.mid(0, header->size()) + frameData;
  }

  // TagLib doesn't mess with encrypted frames, so just treat them
  // as unknown frames.

  if(!zlib::isAvailable() && header->compression()) {
    debug("Compressed frames are currently not supported.");
    return {header, false};
  }

  if(header->encryption()) {
    debug("Encrypted frames are currently not supported.");
    return {header, false};
  }

  if(!updateFrame(header)) {
    header->setTagAlterPreservation(true);
    return {header, false};
  }

  return {header, true};
}

Frame *FrameFactory::createFrame(const ByteVector &origData,
                                 const Header *tagHeader) const
{
  ByteVector data = origData;
  auto [header, ok] = prepareFrameHeader(data, tagHeader);
  if(!ok) {
    // check if frame is valid and return as UnknownFrame
    return header ? new UnknownFrame(data, header) : nullptr;
  }
  return createFrame(data, header, tagHeader);
}

Frame *FrameFactory::createFrame(const ByteVector &data, Frame::Header *header,
                                 const Header *tagHeader) const {
  ByteVector frameID = header->frameID();

  // This is where things get necessarily nasty.  Here we determine which
  // Frame subclass (or if none is found simply a Frame) based
  // on the frame ID.  Since there are a lot of possibilities, that means
  // a lot of if blocks.

  // Text Identification (frames 4.2)

  // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number), GRP1 (Grouping) are in fact text frames.
  if(frameID.startsWith("T") || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN" || frameID == "GRP1") {

    TextIdentificationFrame *f = frameID != "TXXX"
      ? new TextIdentificationFrame(data, header)
      : new UserTextIdentificationFrame(data, header);

    d->setTextEncoding(f);

    if(frameID == "TCON")
      updateGenre(f);

    return f;
  }

  // Comments (frames 4.10)

  if(frameID == "COMM") {
    auto f = new CommentsFrame(data, header);
    d->setTextEncoding(f);
    return f;
  }

  // Attached Picture (frames 4.14)

  if(frameID == "APIC") {
    auto f = new AttachedPictureFrame(data, header);
    d->setTextEncoding(f);
    return f;
  }

  // ID3v2.2 Attached Picture

  if(frameID == "PIC") {
    AttachedPictureFrame *f = new AttachedPictureFrameV22(data, header);
    d->setTextEncoding(f);
    return f;
  }

  // Relative Volume Adjustment (frames 4.11)

  if(frameID == "RVA2")
    return new RelativeVolumeFrame(data, header);

  // Unique File Identifier (frames 4.1)

  if(frameID == "UFID")
    return new UniqueFileIdentifierFrame(data, header);

  // General Encapsulated Object (frames 4.15)

  if(frameID == "GEOB") {
    auto f = new GeneralEncapsulatedObjectFrame(data, header);
    d->setTextEncoding(f);
    return f;
  }

  // URL link (frames 4.3)

  if(frameID.startsWith("W")) {
    if(frameID != "WXXX") {
      return new UrlLinkFrame(data, header);
    }
    auto f = new UserUrlLinkFrame(data, header);
    d->setTextEncoding(f);
    return f;
  }

  // Unsynchronized lyric/text transcription (frames 4.8)

  if(frameID == "USLT") {
    auto f = new UnsynchronizedLyricsFrame(data, header);
    if(d->useDefaultEncoding)
      f->setTextEncoding(d->defaultEncoding);
    return f;
  }

  // Synchronized lyrics/text (frames 4.9)

  if(frameID == "SYLT") {
    auto f = new SynchronizedLyricsFrame(data, header);
    if(d->useDefaultEncoding)
      f->setTextEncoding(d->defaultEncoding);
    return f;
  }

  // Event timing codes (frames 4.5)

  if(frameID == "ETCO")
    return new EventTimingCodesFrame(data, header);

  // Popularimeter (frames 4.17)

  if(frameID == "POPM")
    return new PopularimeterFrame(data, header);

  // Private (frames 4.27)

  if(frameID == "PRIV")
    return new PrivateFrame(data, header);

  // Ownership (frames 4.22)

  if(frameID == "OWNE") {
    auto f = new OwnershipFrame(data, header);
    d->setTextEncoding(f);
    return f;
  }

  // Chapter (ID3v2 chapters 1.0)

  if(frameID == "CHAP")
    return new ChapterFrame(tagHeader, data, header);

  // Table of contents (ID3v2 chapters 1.0)

  if(frameID == "CTOC")
    return new TableOfContentsFrame(tagHeader, data, header);

  // Apple proprietary PCST (Podcast)

  if(frameID == "PCST")
    return new PodcastFrame(data, header);

  return new UnknownFrame(data, header);
}

void FrameFactory::rebuildAggregateFrames(ID3v2::Tag *tag) const
{
  if(tag->header()->majorVersion() < 4 &&
     tag->frameList("TDRC").size() == 1 &&
     tag->frameList("TDAT").size() == 1)
  {
    auto tdrc =
      dynamic_cast<TextIdentificationFrame *>(tag->frameList("TDRC").front());
    auto tdat = dynamic_cast<UnknownFrame *>(tag->frameList("TDAT").front());

    if(tdrc &&
       tdrc->fieldList().size() == 1 &&
       tdrc->fieldList().front().size() == 4 &&
       tdat &&
       tdat->data().size() >= 5)
    {
      String date(tdat->data().mid(1), static_cast<String::Type>(tdat->data()[0]));
      if(date.length() == 4) {
        tdrc->setText(tdrc->toString() + '-' + date.substr(2, 2) + '-' + date.substr(0, 2));
        if(tag->frameList("TIME").size() == 1) {
          auto timeframe = dynamic_cast<UnknownFrame *>(tag->frameList("TIME").front());
          if(timeframe && timeframe->data().size() >= 5) {
            String time(timeframe->data().mid(1), static_cast<String::Type>(timeframe->data()[0]));
            if(time.length() == 4) {
              tdrc->setText(tdrc->toString() + 'T' + time.substr(0, 2) + ':' + time.substr(2, 2));
            }
          }
        }
      }
    }
  }
}

String::Type FrameFactory::defaultTextEncoding() const
{
  return d->defaultEncoding;
}

void FrameFactory::setDefaultTextEncoding(String::Type encoding)
{
  d->useDefaultEncoding = true;
  d->defaultEncoding = encoding;
}

bool FrameFactory::isUsingDefaultTextEncoding() const
{
  return d->useDefaultEncoding;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

FrameFactory::FrameFactory() :
  d(std::make_unique<FrameFactoryPrivate>())
{
}

FrameFactory::~FrameFactory() = default;

namespace
{
  // Frame conversion table ID3v2.2 -> 2.4
  constexpr std::array frameConversion2 {
    std::pair("BUF", "RBUF"),
    std::pair("CNT", "PCNT"),
    std::pair("COM", "COMM"),
    std::pair("CRA", "AENC"),
    std::pair("ETC", "ETCO"),
    std::pair("GEO", "GEOB"),
    std::pair("IPL", "TIPL"),
    std::pair("MCI", "MCDI"),
    std::pair("MLL", "MLLT"),
    std::pair("POP", "POPM"),
    std::pair("REV", "RVRB"),
    std::pair("SLT", "SYLT"),
    std::pair("STC", "SYTC"),
    std::pair("TAL", "TALB"),
    std::pair("TBP", "TBPM"),
    std::pair("TCM", "TCOM"),
    std::pair("TCO", "TCON"),
    std::pair("TCP", "TCMP"),
    std::pair("TCR", "TCOP"),
    std::pair("TDY", "TDLY"),
    std::pair("TEN", "TENC"),
    std::pair("TFT", "TFLT"),
    std::pair("TKE", "TKEY"),
    std::pair("TLA", "TLAN"),
    std::pair("TLE", "TLEN"),
    std::pair("TMT", "TMED"),
    std::pair("TOA", "TOAL"),
    std::pair("TOF", "TOFN"),
    std::pair("TOL", "TOLY"),
    std::pair("TOR", "TDOR"),
    std::pair("TOT", "TOAL"),
    std::pair("TP1", "TPE1"),
    std::pair("TP2", "TPE2"),
    std::pair("TP3", "TPE3"),
    std::pair("TP4", "TPE4"),
    std::pair("TPA", "TPOS"),
    std::pair("TPB", "TPUB"),
    std::pair("TRC", "TSRC"),
    std::pair("TRD", "TDRC"),
    std::pair("TRK", "TRCK"),
    std::pair("TS2", "TSO2"),
    std::pair("TSA", "TSOA"),
    std::pair("TSC", "TSOC"),
    std::pair("TSP", "TSOP"),
    std::pair("TSS", "TSSE"),
    std::pair("TST", "TSOT"),
    std::pair("TT1", "TIT1"),
    std::pair("TT2", "TIT2"),
    std::pair("TT3", "TIT3"),
    std::pair("TXT", "TOLY"),
    std::pair("TXX", "TXXX"),
    std::pair("TYE", "TDRC"),
    std::pair("UFI", "UFID"),
    std::pair("ULT", "USLT"),
    std::pair("WAF", "WOAF"),
    std::pair("WAR", "WOAR"),
    std::pair("WAS", "WOAS"),
    std::pair("WCM", "WCOM"),
    std::pair("WCP", "WCOP"),
    std::pair("WPB", "WPUB"),
    std::pair("WXX", "WXXX"),

    // Apple iTunes nonstandard frames
    std::pair("PCS", "PCST"),
    std::pair("TCT", "TCAT"),
    std::pair("TDR", "TDRL"),
    std::pair("TDS", "TDES"),
    std::pair("TID", "TGID"),
    std::pair("WFD", "WFED"),
    std::pair("MVN", "MVNM"),
    std::pair("MVI", "MVIN"),
    std::pair("GP1", "GRP1"),
  };

  // Frame conversion table ID3v2.3 -> 2.4
  constexpr std::array frameConversion3 {
    std::pair("TORY", "TDOR"),
    std::pair("TYER", "TDRC"),
    std::pair("IPLS", "TIPL"),
  };
}  // namespace

bool FrameFactory::updateFrame(Frame::Header *header) const
{
  const ByteVector frameID = header->frameID();

  switch(header->version()) {

  case 2: // ID3v2.2
  {
    if(frameID == "CRM" ||
       frameID == "EQU" ||
       frameID == "LNK" ||
       frameID == "RVA" ||
       frameID == "TIM" ||
       frameID == "TSI" ||
       frameID == "TDA")
    {
      debug("ID3v2.4 no longer supports the frame type " + String(frameID) +
            ".  It will be discarded from the tag.");
      return false;
    }

    // ID3v2.2 only used 3 bytes for the frame ID, so we need to convert all
    // the frames to their 4 byte ID3v2.4 equivalent.

    for(const auto &[o, t] : frameConversion2) {
      if(frameID == o) {
        header->setFrameID(t);
        break;
      }
    }

    break;
  }

  case 3: // ID3v2.3
  {
    if(frameID == "EQUA" ||
       frameID == "RVAD" ||
       frameID == "TIME" ||
       frameID == "TRDA" ||
       frameID == "TSIZ" ||
       frameID == "TDAT")
    {
      debug("ID3v2.4 no longer supports the frame type " + String(frameID) +
            ".  It will be discarded from the tag.");
      return false;
    }

    for(const auto &[o, t] : frameConversion3) {
      if(frameID == o) {
        header->setFrameID(t);
        break;
      }
    }

    break;
  }

  default:

    // This should catch a typo that existed in TagLib up to and including
    // version 1.1 where TRDC was used for the year rather than TDRC.

    if(frameID == "TRDC")
      header->setFrameID("TDRC");

    break;
  }

  return true;
}

Frame *FrameFactory::createFrameForProperty(const String &key, const StringList &values) const
{
  // check if the key is contained in the key<=>frameID mapping
  if(ByteVector frameID = Frame::keyToFrameID(key); !frameID.isEmpty()) {
    // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number), GRP1 (Grouping) are in fact text frames.
    if(frameID[0] == 'T' || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN" || frameID == "GRP1"){ // text frame
      auto frame = new TextIdentificationFrame(frameID, String::UTF8);
      frame->setText(values);
      return frame;
    } if(frameID[0] == 'W' && values.size() == 1){  // URL frame (not WXXX); support only one value
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
  if((key == "LYRICS" || key.startsWith(Frame::lyricsPrefix)) && values.size() == 1){
    auto frame = new UnsynchronizedLyricsFrame(String::UTF8);
    frame->setDescription(key == "LYRICS" ? key : key.substr(Frame::lyricsPrefix.size()));
    frame->setText(values.front());
    return frame;
  }
  // -URL: depending on the number of values, use WXXX or TXXX (with description=URL)
  if((key == "URL" || key.startsWith(Frame::urlPrefix)) && values.size() == 1){
    auto frame = new UserUrlLinkFrame(String::UTF8);
    frame->setDescription(key == "URL" ? key : key.substr(Frame::urlPrefix.size()));
    frame->setUrl(values.front());
    return frame;
  }
  // -COMMENT: depending on the number of values, use COMM or TXXX (with description=COMMENT)
  if((key == "COMMENT" || key.startsWith(Frame::commentPrefix)) && values.size() == 1){
    auto frame = new CommentsFrame(String::UTF8);
    if (key != "COMMENT"){
      frame->setDescription(key.substr(Frame::commentPrefix.size()));
    }
    frame->setText(values.front());
    return frame;
  }
  // if none of the above cases apply, we use a TXXX frame with the key as description
  return new UserTextIdentificationFrame(
    UserTextIdentificationFrame::keyToTXXX(key), values, String::UTF8);
}
