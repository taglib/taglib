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

#include "id3v2tag.h"

#include <algorithm>
#include <array>
#include <utility>

#include "tdebug.h"
#include "tfile.h"
#include "tpropertymap.h"
#include "id3v2header.h"
#include "id3v2extendedheader.h"
#include "id3v2footer.h"
#include "id3v2synchdata.h"
#include "id3v1genres.h"
#include "frames/attachedpictureframe.h"
#include "frames/generalencapsulatedobjectframe.h"
#include "frames/textidentificationframe.h"
#include "frames/commentsframe.h"
#include "frames/urllinkframe.h"
#include "frames/uniquefileidentifierframe.h"
#include "frames/unsynchronizedlyricsframe.h"
#include "frames/unknownframe.h"

using namespace TagLib;
using namespace ID3v2;

namespace
{
  const ID3v2::Latin1StringHandler defaultStringHandler;
  const ID3v2::Latin1StringHandler *stringHandler = &defaultStringHandler;

  const long MinPaddingSize = 1024;
  const long MaxPaddingSize = 1024 * 1024;
}  // namespace

class ID3v2::Tag::TagPrivate
{
public:
  TagPrivate()
  {
    frameList.setAutoDelete(true);
  }

  const FrameFactory *factory { nullptr };

  File *file { nullptr };
  offset_t tagOffset { 0 };

  Header header;
  std::unique_ptr<ExtendedHeader> extendedHeader;
  std::unique_ptr<Footer> footer;

  FrameListMap frameListMap;
  FrameList frameList;
};

class ID3v2::Latin1StringHandler::Latin1StringHandlerPrivate
{
};

////////////////////////////////////////////////////////////////////////////////
// StringHandler implementation
////////////////////////////////////////////////////////////////////////////////

Latin1StringHandler::Latin1StringHandler() = default;

Latin1StringHandler::~Latin1StringHandler() = default;

String Latin1StringHandler::parse(const ByteVector &data) const
{
  return String(data, String::Latin1);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ID3v2::Tag::Tag() :
  d(std::make_unique<TagPrivate>())
{
  d->factory = FrameFactory::instance();
}

ID3v2::Tag::Tag(File *file, offset_t tagOffset, const FrameFactory *factory) :
  d(std::make_unique<TagPrivate>())
{
  d->factory = factory;
  d->file = file;
  d->tagOffset = tagOffset;

  read();
}

ID3v2::Tag::~Tag() = default;

String ID3v2::Tag::title() const
{
  if(!d->frameListMap["TIT2"].isEmpty())
    return d->frameListMap["TIT2"].front()->toString();
  return String();
}

String ID3v2::Tag::artist() const
{
  if(!d->frameListMap["TPE1"].isEmpty())
    return d->frameListMap["TPE1"].front()->toString();
  return String();
}

String ID3v2::Tag::album() const
{
  if(!d->frameListMap["TALB"].isEmpty())
    return d->frameListMap["TALB"].front()->toString();
  return String();
}

String ID3v2::Tag::comment() const
{
  const FrameList &comments = d->frameListMap["COMM"];

  if(comments.isEmpty())
    return String();

  for(const auto &comment : comments) {
    auto frame = dynamic_cast<CommentsFrame *>(comment);
    if(frame && frame->description().isEmpty())
      return comment->toString();
  }

  return comments.front()->toString();
}

String ID3v2::Tag::genre() const
{
  // TODO: In the next major version (TagLib 2.0) a list of multiple genres
  // should be separated by " / " instead of " ".  For the moment to keep
  // the behavior the same as released versions it is being left with " ".

  const FrameList &tconFrames = d->frameListMap["TCON"];
  if(tconFrames.isEmpty())
  {
    return String();
  }

  auto f = dynamic_cast<TextIdentificationFrame *>(tconFrames.front());
  if(!f)
  {
    return String();
  }

  // ID3v2.4 lists genres as the fields in its frames field list.  If the field
  // is simply a number it can be assumed that it is an ID3v1 genre number.
  // Here was assumed that if an ID3v1 string is present then it should be
  // appended to the genre string.  Multiple fields will be appended as the
  // string is built.

  StringList genres;

  for(auto &field : f->fieldList()) {

    if(field.isEmpty())
      continue;

    bool ok;
    int number = field.toInt(&ok);
    if(ok && number >= 0 && number <= 255) {
      field = ID3v1::genre(number);
    }

    if(std::find(genres.begin(), genres.end(), field) == genres.end())
      genres.append(field);
  }

  return genres.toString();
}

unsigned int ID3v2::Tag::year() const
{
  if(!d->frameListMap["TDRC"].isEmpty())
    return d->frameListMap["TDRC"].front()->toString().substr(0, 4).toInt();
  return 0;
}

unsigned int ID3v2::Tag::track() const
{
  if(!d->frameListMap["TRCK"].isEmpty())
    return d->frameListMap["TRCK"].front()->toString().toInt();
  return 0;
}

void ID3v2::Tag::setTitle(const String &s)
{
  setTextFrame("TIT2", s);
}

void ID3v2::Tag::setArtist(const String &s)
{
  setTextFrame("TPE1", s);
}

void ID3v2::Tag::setAlbum(const String &s)
{
  setTextFrame("TALB", s);
}

void ID3v2::Tag::setComment(const String &s)
{
  if(s.isEmpty()) {
    removeFrames("COMM");
    return;
  }

  const FrameList &comments = d->frameListMap["COMM"];

  if(!comments.isEmpty()) {
    for(const auto &comment : comments) {
      auto frame = dynamic_cast<CommentsFrame *>(comment);
      if(frame && frame->description().isEmpty()) {
        comment->setText(s);
        return;
      }
    }

    comments.front()->setText(s);
    return;
  }

  auto f = new CommentsFrame(d->factory->defaultTextEncoding());
  addFrame(f);
  f->setText(s);
}

void ID3v2::Tag::setGenre(const String &s)
{
  if(s.isEmpty()) {
    removeFrames("TCON");
    return;
  }

  // iTunes can't handle correctly encoded ID3v2.4 numerical genres.  Just use
  // strings until iTunes sucks less.

#ifdef NO_ITUNES_HACKS

  int index = ID3v1::genreIndex(s);

  if(index != 255)
    setTextFrame("TCON", String::number(index));
  else
    setTextFrame("TCON", s);

#else

  setTextFrame("TCON", s);

#endif
}

void ID3v2::Tag::setYear(unsigned int i)
{
  if(i == 0) {
    removeFrames("TDRC");
    return;
  }
  setTextFrame("TDRC", String::number(i));
}

void ID3v2::Tag::setTrack(unsigned int i)
{
  if(i == 0) {
    removeFrames("TRCK");
    return;
  }
  setTextFrame("TRCK", String::number(i));
}

bool ID3v2::Tag::isEmpty() const
{
  return d->frameList.isEmpty();
}

Header *ID3v2::Tag::header() const
{
  return &(d->header);
}

ExtendedHeader *ID3v2::Tag::extendedHeader() const
{
  return d->extendedHeader.get();
}

const FrameListMap &ID3v2::Tag::frameListMap() const
{
  return d->frameListMap;
}

const FrameList &ID3v2::Tag::frameList() const
{
  return d->frameList;
}

const FrameList &ID3v2::Tag::frameList(const ByteVector &frameID) const
{
  return d->frameListMap[frameID];
}

void ID3v2::Tag::addFrame(Frame *frame)
{
  d->frameList.append(frame);
  d->frameListMap[frame->frameID()].append(frame);
}

void ID3v2::Tag::removeFrame(Frame *frame, bool del)
{
  // remove the frame from the frame list
  auto it = d->frameList.find(frame);
  d->frameList.erase(it);

  // ...and from the frame list map
  it = d->frameListMap[frame->frameID()].find(frame);
  d->frameListMap[frame->frameID()].erase(it);

  // ...and delete as desired
  if(del)
    delete frame;
}

void ID3v2::Tag::removeFrames(const ByteVector &id)
{
  const FrameList frames = d->frameListMap[id];
  for(const auto &frame : frames)
    removeFrame(frame, true);
}

PropertyMap ID3v2::Tag::properties() const
{
  PropertyMap properties;
  for(const auto &frame : std::as_const(frameList())) {
    PropertyMap props = frame->asProperties();
    properties.merge(props);
  }
  return properties;
}

void ID3v2::Tag::removeUnsupportedProperties(const StringList &properties)
{
  for(const auto &property : properties) {
    if(property.startsWith("UNKNOWN/")) {
      String frameID = property.substr(String("UNKNOWN/").size());
      if(frameID.size() != 4)
        continue; // invalid specification
      ByteVector id = frameID.data(String::Latin1);
      // delete all unknown frames of given type
      const FrameList frames = frameList(id);
      for(const auto &frame : frames)
        if(dynamic_cast<const UnknownFrame *>(frame) != nullptr)
          removeFrame(frame);
    }
    else if(property.size() == 4) {
      ByteVector id = property.data(String::Latin1);
      removeFrames(id);
    }
    else {
      ByteVector id = property.substr(0, 4).data(String::Latin1);
      if(property.size() <= 5)
        continue; // invalid specification
      String description = property.substr(5);
      Frame *frame = nullptr;
      if(id == "TXXX")
        frame = UserTextIdentificationFrame::find(this, description);
      else if(id == "WXXX")
        frame = UserUrlLinkFrame::find(this, description);
      else if(id == "COMM")
        frame = CommentsFrame::findByDescription(this, description);
      else if(id == "USLT")
        frame = UnsynchronizedLyricsFrame::findByDescription(this, description);
      else if(id == "UFID")
        frame = UniqueFileIdentifierFrame::findByOwner(this, description);
      if(frame)
        removeFrame(frame);
    }
  }
}

PropertyMap ID3v2::Tag::setProperties(const PropertyMap &origProps)
{
  FrameList framesToDelete;
  // we split up the PropertyMap into the "normal" keys and the "complicated" ones,
  // which are those according to TIPL or TMCL frames.
  PropertyMap properties;
  PropertyMap tiplProperties;
  PropertyMap tmclProperties;
  Frame::splitProperties(origProps, properties, tiplProperties, tmclProperties);
  for(const auto &[tag, frames] : std::as_const(frameListMap())) {
    for(const auto &frame : frames) {
      PropertyMap frameProperties = frame->asProperties();
      if(tag == "TIPL") {
        if (tiplProperties != frameProperties)
          framesToDelete.append(frame);
        else
          tiplProperties.erase(frameProperties);
      }
      else if(tag == "TMCL") {
        if (tmclProperties != frameProperties)
          framesToDelete.append(frame);
        else
          tmclProperties.erase(frameProperties);
      }
      else if(!properties.contains(frameProperties))
        framesToDelete.append(frame);
      else
        properties.erase(frameProperties);
    }
  }
  for(const auto &frame : std::as_const(framesToDelete))
    removeFrame(frame);

  // now create remaining frames:
  // start with the involved people list (TIPL)
  if(!tiplProperties.isEmpty())
      addFrame(TextIdentificationFrame::createTIPLFrame(tiplProperties));
  // proceed with the musician credit list (TMCL)
  if(!tmclProperties.isEmpty())
      addFrame(TextIdentificationFrame::createTMCLFrame(tmclProperties));
  // now create the "one key per frame" frames
  for(const auto &[tag, frames] : std::as_const(properties))
      addFrame(Frame::createTextualFrame(tag, frames));
  return PropertyMap(); // ID3 implements the complete PropertyMap interface, so an empty map is returned
}

StringList ID3v2::Tag::complexPropertyKeys() const
{
  StringList keys;
  if(d->frameListMap.contains("APIC")) {
    keys.append("PICTURE");
  }
  if(d->frameListMap.contains("GEOB")) {
    keys.append("GENERALOBJECT");
  }
  return keys;
}

List<VariantMap> ID3v2::Tag::complexProperties(const String &key) const
{
  List<VariantMap> properties;
  const String uppercaseKey = key.upper();
  if(uppercaseKey == "PICTURE") {
    const FrameList pictures = d->frameListMap.value("APIC");
    for(const Frame *frame : pictures) {
      auto picture = static_cast<const AttachedPictureFrame *>(frame);
      VariantMap property;
      property.insert("data", picture->picture());
      property.insert("mimeType", picture->mimeType());
      property.insert("description", picture->description());
      property.insert("pictureType",
        AttachedPictureFrame::typeToString(picture->type()));
      properties.append(property);
    }
  }
  else if(uppercaseKey == "GENERALOBJECT") {
    const FrameList geobs = d->frameListMap.value("GEOB");
    for(const Frame *frame : geobs) {
      auto geob = static_cast<const GeneralEncapsulatedObjectFrame *>(frame);
      VariantMap property;
      property.insert("data", geob->object());
      property.insert("mimeType", geob->mimeType());
      property.insert("description", geob->description());
      property.insert("fileName", geob->fileName());
      properties.append(property);
    }
  }
  return properties;
}

bool ID3v2::Tag::setComplexProperties(const String &key, const List<VariantMap> &value)
{
  const String uppercaseKey = key.upper();
  if(uppercaseKey == "PICTURE") {
    removeFrames("APIC");

    for(auto property : value) {
      auto picture = new AttachedPictureFrame;
      picture->setPicture(property.value("data").value<ByteVector>());
      picture->setMimeType(property.value("mimeType").value<String>());
      picture->setDescription(property.value("description").value<String>());
      picture->setType(AttachedPictureFrame::typeFromString(
        property.value("pictureType").value<String>()));
      addFrame(picture);
    }
  }
  else if(uppercaseKey == "GENERALOBJECT") {
    removeFrames("GEOB");

    for(auto property : value) {
      auto geob = new GeneralEncapsulatedObjectFrame;
      geob->setObject(property.value("data").value<ByteVector>());
      geob->setMimeType(property.value("mimeType").value<String>());
      geob->setDescription(property.value("description").value<String>());
      geob->setFileName(property.value("fileName").value<String>());
      addFrame(geob);
    }
  }
  else {
    return false;
  }
  return true;
}

ByteVector ID3v2::Tag::render() const
{
  return render(ID3v2::v4);
}

void ID3v2::Tag::downgradeFrames(FrameList *frames, FrameList *newFrames) const
{
#ifdef NO_ITUNES_HACKS
  static constexpr std::array unsupportedFrames {
    "ASPI", "EQU2", "RVA2", "SEEK", "SIGN", "TDRL", "TDTG",
    "TMOO", "TPRO", "TSOA", "TSOT", "TSST", "TSOP"
  };
#else
  // iTunes writes and reads TSOA, TSOT, TSOP to ID3v2.3.
  static constexpr std::array unsupportedFrames {
    "ASPI", "EQU2", "RVA2", "SEEK", "SIGN", "TDRL", "TDTG",
    "TMOO", "TPRO", "TSST"
  };
#endif
  ID3v2::TextIdentificationFrame *frameTDOR = nullptr;
  ID3v2::TextIdentificationFrame *frameTDRC = nullptr;
  ID3v2::TextIdentificationFrame *frameTIPL = nullptr;
  ID3v2::TextIdentificationFrame *frameTMCL = nullptr;
  ID3v2::TextIdentificationFrame *frameTCON = nullptr;

  for(const auto &frame : std::as_const(d->frameList)) {
    ByteVector frameID = frame->header()->frameID();

    if(std::any_of(unsupportedFrames.begin(), unsupportedFrames.end(),
                   [&frameID](auto m){ return frameID == m; })) {
      debug("A frame that is not supported in ID3v2.3 \'" + String(frameID) +
            "\' has been discarded");
      continue;
    }

    if(frameID == "TDOR")
      frameTDOR = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
    else if(frameID == "TDRC")
      frameTDRC = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
    else if(frameID == "TIPL")
      frameTIPL = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
    else if(frameID == "TMCL")
      frameTMCL = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
    else if(frame && frameID == "TCON")
      frameTCON = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
    else
      frames->append(frame);
  }

  if(frameTDOR) {
    String content = frameTDOR->toString();

    if(content.size() >= 4) {
      auto frameTORY = new ID3v2::TextIdentificationFrame("TORY", String::Latin1);
      frameTORY->setText(content.substr(0, 4));
      frames->append(frameTORY);
      newFrames->append(frameTORY);
    }
  }

  if(frameTDRC) {
    String content = frameTDRC->toString();
    if(content.size() >= 4) {
      auto frameTYER = new ID3v2::TextIdentificationFrame("TYER", String::Latin1);
      frameTYER->setText(content.substr(0, 4));
      frames->append(frameTYER);
      newFrames->append(frameTYER);
      if(content.size() >= 10 && content[4] == '-' && content[7] == '-') {
        auto frameTDAT = new ID3v2::TextIdentificationFrame("TDAT", String::Latin1);
        frameTDAT->setText(content.substr(8, 2) + content.substr(5, 2));
        frames->append(frameTDAT);
        newFrames->append(frameTDAT);
        if(content.size() >= 16 && content[10] == 'T' && content[13] == ':') {
          auto frameTIME = new ID3v2::TextIdentificationFrame("TIME", String::Latin1);
          frameTIME->setText(content.substr(11, 2) + content.substr(14, 2));
          frames->append(frameTIME);
          newFrames->append(frameTIME);
        }
      }
    }
  }

  if(frameTIPL || frameTMCL) {
    auto frameIPLS = new ID3v2::TextIdentificationFrame("IPLS", String::Latin1);

    StringList people;

    if(frameTMCL) {
      StringList v24People = frameTMCL->fieldList();
      for(unsigned int i = 0; i + 1 < v24People.size(); i += 2) {
        people.append(v24People[i]);
        people.append(v24People[i+1]);
      }
    }
    if(frameTIPL) {
      StringList v24People = frameTIPL->fieldList();
      for(unsigned int i = 0; i + 1 < v24People.size(); i += 2) {
        people.append(v24People[i]);
        people.append(v24People[i+1]);
      }
    }

    frameIPLS->setText(people);
    frames->append(frameIPLS);
    newFrames->append(frameIPLS);
  }

  if(frameTCON) {
    const StringList genres = frameTCON->fieldList();
    String combined;
    String genreText;
    const bool hasMultipleGenres = genres.size() > 1;

    // If there are multiple genres, add them as multiple references to ID3v1
    // genres if such a reference exists. The first genre for which no ID3v1
    // genre number exists can be finally added as a refinement.
    for(const auto &genre : genres) {
      bool ok = false;
      int number = genre.toInt(&ok);
      if((ok && number >= 0 && number <= 255) || genre == "RX" || genre == "CR")
        combined += '(' + genre + ')';
      else if(hasMultipleGenres && (number = ID3v1::genreIndex(genre)) != 255)
        combined += '(' + String::number(number) + ')';
      else if(genreText.isEmpty())
        genreText = genre;
    }
    if(!genreText.isEmpty())
      combined += genreText;

    frameTCON = new ID3v2::TextIdentificationFrame("TCON", String::Latin1);
    frameTCON->setText(combined);
    frames->append(frameTCON);
    newFrames->append(frameTCON);
  }
}

ByteVector ID3v2::Tag::render(Version version) const
{
  // We need to render the "tag data" first so that we have to correct size to
  // render in the tag's header.  The "tag data" -- everything that is included
  // in ID3v2::Header::tagSize() -- includes the extended header, frames and
  // padding, but does not include the tag's header or footer.

  // TODO: Render the extended header.

  // Downgrade the frames that ID3v2.3 doesn't support.

  FrameList newFrames;
  newFrames.setAutoDelete(true);

  FrameList frameList;
  if(version == v4) {
    frameList = d->frameList;
  }
  else {
    downgradeFrames(&frameList, &newFrames);
  }

  // Reserve a 10-byte blank space for an ID3v2 tag header.

  ByteVector tagData(Header::size(), '\0');

  // Loop through the frames rendering them and adding them to the tagData.

  for(const auto &frame : std::as_const(frameList)) {
    frame->header()->setVersion(version == v3 ? 3 : 4);
    if(frame->header()->frameID().size() != 4) {
      debug("An ID3v2 frame of unsupported or unknown type \'"
            + String(frame->header()->frameID()) + "\' has been discarded");
      continue;
    }
    if(!frame->header()->tagAlterPreservation()) {
      const ByteVector frameData = frame->render();
      if(frameData.size() == frame->headerSize()) {
        debug("An empty ID3v2 frame \'"
              + String(frame->header()->frameID()) + "\' has been discarded");
        continue;
      }
      tagData.append(frameData);
    }
  }

  // Compute the amount of padding, and append that to tagData.

  long originalSize = d->header.tagSize();
  long paddingSize = originalSize - (tagData.size() - Header::size());

  if(paddingSize <= 0) {
    paddingSize = MinPaddingSize;
  }
  else {
    // Padding won't increase beyond 1% of the file size or 1MB.

    offset_t threshold = d->file ? d->file->length() / 100 : 0;
    threshold = std::max<offset_t>(threshold, MinPaddingSize);
    threshold = std::min<offset_t>(threshold, MaxPaddingSize);

    if(paddingSize > threshold)
      paddingSize = MinPaddingSize;
  }

  tagData.resize(static_cast<unsigned int>(tagData.size() + paddingSize), '\0');

  // Set the version and data size.
  d->header.setMajorVersion(version);
  d->header.setTagSize(tagData.size() - Header::size());

  // TODO: This should eventually include d->footer->render().
  const ByteVector headerData = d->header.render();
  std::copy(headerData.begin(), headerData.end(), tagData.begin());

  return tagData;
}

Latin1StringHandler const *ID3v2::Tag::latin1StringHandler()
{
  return stringHandler;
}

void ID3v2::Tag::setLatin1StringHandler(const Latin1StringHandler *handler)
{
  if(handler)
    stringHandler = handler;
  else
    stringHandler = &defaultStringHandler;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void ID3v2::Tag::read()
{
  if(!d->file)
    return;

  if(!d->file->isOpen())
    return;

  d->file->seek(d->tagOffset);
  d->header.setData(d->file->readBlock(Header::size()));

  // If the tag size is 0, then this is an invalid tag (tags must contain at
  // least one frame)

  if(d->header.tagSize() != 0)
    parse(d->file->readBlock(d->header.tagSize()));

  // Look for duplicate ID3v2 tags and treat them as an extra blank of this one.
  // It leads to overwriting them with zero when saving the tag.

  // This is a workaround for some faulty files that have duplicate ID3v2 tags.
  // Unfortunately, TagLib itself may write such duplicate tags until v1.10.

  unsigned int extraSize = 0;

  while(true) {

    d->file->seek(d->tagOffset + d->header.completeTagSize() + extraSize);

    const ByteVector data = d->file->readBlock(Header::size());
    if(data.size() < Header::size() || !data.startsWith(Header::fileIdentifier()))
      break;

    extraSize += Header(data).completeTagSize();
  }

  if(extraSize != 0) {
    debug("ID3v2::Tag::read() - Duplicate ID3v2 tags found.");
    d->header.setTagSize(d->header.tagSize() + extraSize);
  }
}

void ID3v2::Tag::parse(const ByteVector &origData)
{
  ByteVector data = origData;

  if(d->header.unsynchronisation() && d->header.majorVersion() <= 3)
    data = SynchData::decode(data);

  unsigned int frameDataPosition = 0;
  unsigned int frameDataLength = data.size();

  // check for extended header

  if(d->header.extendedHeader()) {
    if(!d->extendedHeader)
      d->extendedHeader = std::make_unique<ExtendedHeader>();
    d->extendedHeader->setData(data);
    if(d->extendedHeader->size() <= data.size()) {
      frameDataPosition += d->extendedHeader->size();
    }
  }

  // check for footer -- we don't actually need to parse it, as it *must*
  // contain the same data as the header, but we do need to account for its
  // size.

  if(d->header.footerPresent() && Footer::size() <= frameDataLength)
    frameDataLength -= Footer::size();

  // parse frames

  // Make sure that there is at least enough room in the remaining frame data for
  // a frame header.

  while(frameDataPosition < frameDataLength - TagLib::ID3v2::Header::size()) {

    // If the next data is position is 0, assume that we've hit the padding
    // portion of the frame data.

    if(data.at(frameDataPosition) == 0) {
      if(d->header.footerPresent()) {
        debug("Padding *and* a footer found.  This is not allowed by the spec.");
      }

      break;
    }

    Frame *frame = d->factory->createFrame(data.mid(frameDataPosition),
                                           &d->header);

    if(!frame)
      return;

    // Checks to make sure that frame parsed correctly.

    if(frame->size() <= 0) {
      delete frame;
      return;
    }

    frameDataPosition += frame->size() + frame->headerSize();
    addFrame(frame);
  }

  d->factory->rebuildAggregateFrames(this);
}

void ID3v2::Tag::setTextFrame(const ByteVector &id, const String &value)
{
  if(value.isEmpty()) {
    removeFrames(id);
    return;
  }

  if(!d->frameListMap[id].isEmpty())
    d->frameListMap[id].front()->setText(value);
  else {
    const String::Type encoding = d->factory->defaultTextEncoding();
    auto f = new TextIdentificationFrame(id, encoding);
    addFrame(f);
    f->setText(value);
  }
}
