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

#include <tfile.h>
#include <tdebug.h>

#include "id3v2tag.h"
#include "id3v2header.h"
#include "id3v2extendedheader.h"
#include "id3v2footer.h"
#include "id3v2synchdata.h"
#include "tbytevector.h"
#include "id3v1genres.h"
#include "tpropertymap.h"

#include "frames/textidentificationframe.h"
#include "frames/commentsframe.h"
#include "frames/urllinkframe.h"
#include "frames/uniquefileidentifierframe.h"
#include "frames/unsynchronizedlyricsframe.h"
#include "frames/unknownframe.h"

using namespace TagLib;
using namespace ID3v2;

class ID3v2::Tag::TagPrivate
{
public:
  TagPrivate() : file(0), tagOffset(-1), extendedHeader(0), footer(0), paddingSize(0)
  {
    frameList.setAutoDelete(true);
  }
  ~TagPrivate()
  {
    delete extendedHeader;
    delete footer;
  }

  File *file;
  long tagOffset;
  const FrameFactory *factory;

  Header header;
  ExtendedHeader *extendedHeader;
  Footer *footer;

  int paddingSize;

  FrameListMap frameListMap;
  FrameList frameList;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ID3v2::Tag::Tag() : TagLib::Tag()
{
  d = new TagPrivate;
  d->factory = FrameFactory::instance();
}

ID3v2::Tag::Tag(File *file, long tagOffset, const FrameFactory *factory) :
  TagLib::Tag()
{
  d = new TagPrivate;

  d->file = file;
  d->tagOffset = tagOffset;
  d->factory = factory;

  read();
}

ID3v2::Tag::~Tag()
{
  delete d;
}


String ID3v2::Tag::title() const
{
  if(!d->frameListMap["TIT2"].isEmpty())
    return d->frameListMap["TIT2"].front()->toString();
  return String::null;
}

String ID3v2::Tag::artist() const
{
  if(!d->frameListMap["TPE1"].isEmpty())
    return d->frameListMap["TPE1"].front()->toString();
  return String::null;
}

String ID3v2::Tag::album() const
{
  if(!d->frameListMap["TALB"].isEmpty())
    return d->frameListMap["TALB"].front()->toString();
  return String::null;
}

String ID3v2::Tag::comment() const
{
  const FrameList &comments = d->frameListMap["COMM"];

  if(comments.isEmpty())
    return String::null;

  for(FrameList::ConstIterator it = comments.begin(); it != comments.end(); ++it)
  {
    CommentsFrame *frame = dynamic_cast<CommentsFrame *>(*it);

    if(frame && frame->description().isEmpty())
      return (*it)->toString();
  }

  return comments.front()->toString();
}

String ID3v2::Tag::genre() const
{
  // TODO: In the next major version (TagLib 2.0) a list of multiple genres
  // should be separated by " / " instead of " ".  For the moment to keep
  // the behavior the same as released versions it is being left with " ".

  if(d->frameListMap["TCON"].isEmpty() ||
     !dynamic_cast<TextIdentificationFrame *>(d->frameListMap["TCON"].front()))
  {
    return String::null;
  }

  // ID3v2.4 lists genres as the fields in its frames field list.  If the field
  // is simply a number it can be assumed that it is an ID3v1 genre number.
  // Here was assume that if an ID3v1 string is present that it should be
  // appended to the genre string.  Multiple fields will be appended as the
  // string is built.

  TextIdentificationFrame *f = static_cast<TextIdentificationFrame *>(
    d->frameListMap["TCON"].front());

  StringList fields = f->fieldList();

  StringList genres;

  for(StringList::Iterator it = fields.begin(); it != fields.end(); ++it) {

    if((*it).isEmpty())
      continue;

    bool ok;
    int number = (*it).toInt(&ok);
    if(ok && number >= 0 && number <= 255) {
      *it = ID3v1::genre(number);
    }

    if(std::find(genres.begin(), genres.end(), *it) == genres.end())
      genres.append(*it);
  }

  return genres.toString();
}

TagLib::uint ID3v2::Tag::year() const
{
  if(!d->frameListMap["TDRC"].isEmpty())
    return d->frameListMap["TDRC"].front()->toString().substr(0, 4).toInt();
  return 0;
}

TagLib::uint ID3v2::Tag::track() const
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

  if(!d->frameListMap["COMM"].isEmpty())
    d->frameListMap["COMM"].front()->setText(s);
  else {
    CommentsFrame *f = new CommentsFrame(d->factory->defaultTextEncoding());
    addFrame(f);
    f->setText(s);
  }
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

void ID3v2::Tag::setYear(uint i)
{
  if(i <= 0) {
    removeFrames("TDRC");
    return;
  }
  setTextFrame("TDRC", String::number(i));
}

void ID3v2::Tag::setTrack(uint i)
{
  if(i <= 0) {
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
  return d->extendedHeader;
}

Footer *ID3v2::Tag::footer() const
{
  return d->footer;
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
  FrameList::Iterator it = d->frameList.find(frame);
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
  FrameList l = d->frameListMap[id];
  for(FrameList::Iterator it = l.begin(); it != l.end(); ++it)
    removeFrame(*it, true);
}

PropertyMap ID3v2::Tag::properties() const
{
  PropertyMap properties;
  for(FrameList::ConstIterator it = frameList().begin(); it != frameList().end(); ++it)
    properties.merge((*it)->asProperties());
  return properties;
}

PropertyMap ID3v2::Tag::setProperties(const PropertyMap &origProps)
{
  FrameList toRemove;
  PropertyMap properties = origProps;
  // first find out what frames to remove; we do not remove in-place
  // because that would invalidate FrameListMap iterators.
  // At the moment, we remove _all_ frames that don't contain unsupported data,
  // and create new ones in the next step; this is to avoid clumsy technicalities
  // arising when trying to do this more efficient. For example, if the current tag
  // contains one URL attribute stored in an WXXX frame, but the given \a properties
  // contain two URL values, we would need to remove the WXXX frame (which supports
  // only one value), and create a TXXX frame with description=URL.
  // The same may happen with COMM and USLT. Additionally, handling of TIPL and TMCL is
  // complicated.
  // In the future, someone might come up with a more clever sync algorithm. :-)
  for(FrameListMap::ConstIterator it = frameListMap().begin(); it != frameListMap().end(); ++it) {
    String key = Frame::frameIDToKey(it->first);
    // for unsupported (binary) frame types, as well as frames that need special treatment
    // (TXXX, WXXX, COMM, TMCL, TIPL, USLT), key will be String::null
    if(key.isNull())
      continue;
    // else: non-user text or url frame -> there should be only one frame of this type,
    // and it's asProperties() method should return a PropertyMap with exactly one key
    // and empty unsupportedData().
    if(it->second.size() != 1)
      debug("invalid ID3 tag: found more than one " + it->first + " frame");
    PropertyMap frameMap = it->second[0]->asProperties();
    if(properties.contains(key) && frameMap[key] == properties[key])
      properties.erase(key);
    else
      toRemove.append(it->second[0]);
  }

  // now handle the special cases
  // first: TXXX frames
  FrameList &userTextFrames = frameList("TXXX");
  for(FrameList::ConstIterator it = userTextFrames.begin(); it != userTextFrames.end(); ++it) {
    PropertyMap frameMap = (*it)->asProperties();
    if(!frameMap.unsupportedData().isEmpty())
      // don't touch unsupported frames
      continue;
    // TXXX frames yield only one key, so it must be begin()->first
    String &key = frameMap.begin()->first;
    if(!Frame::keyToFrameID(key).isNull())
      // TXXX frame which a description (=key) for which there is a dedicated frame.
      // We don't want this, so remove the frame, the appropriate T*** or W*** frame
      // will be created later on.
      toRemove.append(*it);
    if(key.find(":") > 0)
      // colon-separated key: this should be inside a TMCL frame.
      toRemove.append(*it);
    // More (ugly) exceptions: If the user provides more than one COMMENT, LYRICS, or URL
    // tag, we store all of these in a TXXX, because COMM, USLT and WXXX. Otherwise there
    // should not be such a TXXX frame.
    if(key == "COMMENT") {
      if(properties.contains("COMMENT")
          && properties["COMMENT"].size() >= 2
          && properties["COMMENT"] == frameMap.begin()->second)
        properties.erase("COMMENT");
      else
        toRemove.append(*it);
    }else if(key == "LYRICS") {
      if(properties.contains("LYRICS")
          && properties["LYRICS"].size() >= 2
          && properties["LYRICS"] == frameMap.begin()->second)
        properties.erase("LYRICS");
      else
        toRemove.append(*it);
    }else if(key == "URL") {
       if(properties.contains("URL")
           && properties["URL"].size() >= 2
           && properties["URL"] == frameMap.begin()->second)
         properties.erase("URL");
       else
         toRemove.append(*it);
    }
  }

  // next: WXXX frames
  FrameList &userUrlFrames = frameList("WXXX");
  for(FrameList::ConstIterator it = userUrlFrames.begin(); it != userUrlFrames.end(); ++it) {
    PropertyMap frameMap = (*it)->asProperties();
    if(!frameMap.unsupportedData().isEmpty())
      // don't touch unsupported frames
      continue;
    // WXXX frames yield only one key, so it must be begin()->first
    String &key = frameMap.begin()->first;
    if(!Frame::keyToFrameID(key).isNull())
      // WXXX frame which a description (=key) for which there is a dedicated frame.
      // We don't want this, so remove the frame, the appropriate T*** or W*** frame
      // will be created later on.
      toRemove.append(*it);
    else if(key.find(":") > 0)
      // colon-separated key: this should be inside a TMCL frame.
      toRemove.append(*it);
    // More exceptions: we don't allow COMMENT and LYRICS in WXXX; they should be in COMM and USLT
    // (or TXXX, see above).
    else if(key == "COMMENT" || key == "LYRICS")
      toRemove.append(*it);
    // now, the key is either URL or some other string that neither has a dedicated text frame, nor
    // a colon. We accept the frame if it's contents match the values in properties. However, if
    // key != URL and the values are changed, they will be stored inside a TXXX frame instead, since
    // we can't distinguish free-form text from free-form URL keys (possible fix: use URL:REASON like
    // in TMCL / TIPL?).
    else if(properties.contains(key) && properties[key] == frameMap.begin()->second)
      properties.erase(key);
    else
      toRemove.append(*it);
  }
  for(FrameList::ConstIterator it = toRemove.begin(); it != toRemove.end(); ++it)
    removeFrame(*it);

  // next: TIPL
  PropertyMap existingTipl;
  if(!frameList("TIPL").isEmpty())
    existingTipl = frameList("TIPL").front()->asProperties();
  PropertyMap requestedTipl;
  KeyConversionMap::ConstIterator it = TextIdentificationFrame::involvedPeopleMap().begin();
  bool rebuildTipl = false;
  for(; it != TextIdentificationFrame::involvedPeopleMap().end(); ++it) {
    if(properties.contains(it->first)){
      requestedTipl.insert(it->first, properties[it->first]);
      properties.erase(it->first); // it's ensured now that this key gets handled correctly
      if(!existingTipl.contains(it->first) || existingTipl[it->first] != requestedTipl[it->first])
        rebuildTipl = true;
    } else if(existingTipl.contains(it->first))
      rebuildTipl = true;
  }
  if(rebuildTipl){
    removeFrames("TIPL");
    addFrame(TextIdentificationFrame::createTIPLFrame(requestedTipl));
  }

  // next: create frames for everything still in properties except for TMCL ("PERFORMER:<instrument>")
  // keys, which are collected in a dedicated map
  PropertyMap requestedTmcl;
  for(PropertyMap::ConstIterator it = properties.begin(); it != properties.end(); ++it){
    if(it->first.find(":") != -1)
      requestedTmcl.insert(it->first, it->second);
    else{
      // phew. Now we are in the simple case that our key=<value list> pair can be represented by a
      // single frame, either a T*** (not TIPL, TMCL) or W*** frame.
      ByteVector id = Frame::keyToFrameID(it->first);
    }
  }

    // next: TMCL
  PropertyMap existingTmcl;
  if(!frameList("TMCL").isEmpty())
    existingTmcl = frameList("TMCL").front()->asProperties();
  bool rebuildTmcl = false;
  // search for TMCL keys ("PERFORMER:<instrument>") in properties
  for(PropertyMap::ConstIterator it = properties.begin(); it != properties.end(); ++it){
    if(it->first.find(":") != -1)
      requestedTmcl.insert(it->first, it->second);
  }
}

ByteVector ID3v2::Tag::render() const
{
  return render(4);
}

void ID3v2::Tag::downgradeFrames(FrameList *frames, FrameList *newFrames) const
{
  const char *unsupportedFrames[] = {
    "ASPI", "EQU2", "RVA2", "SEEK", "SIGN", "TDRL", "TDTG",
    "TMOO", "TPRO", "TSOA", "TSOT", "TSST", "TSOP", 0
  };
  ID3v2::TextIdentificationFrame *frameTDOR = 0;
  ID3v2::TextIdentificationFrame *frameTDRC = 0;
  ID3v2::TextIdentificationFrame *frameTIPL = 0;
  ID3v2::TextIdentificationFrame *frameTMCL = 0;
  for(FrameList::Iterator it = d->frameList.begin(); it != d->frameList.end(); it++) {
    ID3v2::Frame *frame = *it;
    ByteVector frameID = frame->header()->frameID();
    for(int i = 0; unsupportedFrames[i]; i++) {
      if(frameID == unsupportedFrames[i]) {
        debug("A frame that is not supported in ID3v2.3 \'"
          + String(frameID) + "\' has been discarded");
        frame = 0;
        break;
      }
    }
    if(frame && frameID == "TDOR") {
      frameTDOR = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
      frame = 0;
    }
    if(frame && frameID == "TDRC") {
      frameTDRC = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
      frame = 0;
    }
    if(frame && frameID == "TIPL") {
      frameTIPL = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
      frame = 0;
    }
    if(frame && frameID == "TMCL") {
      frameTMCL = dynamic_cast<ID3v2::TextIdentificationFrame *>(frame);
      frame = 0;
    }
    if(frame) {
      frames->append(frame);
    }
  }
  if(frameTDOR) {
    String content = frameTDOR->toString();
    if(content.size() >= 4) {
      ID3v2::TextIdentificationFrame *frameTORY = new ID3v2::TextIdentificationFrame("TORY", String::Latin1);
      frameTORY->setText(content.substr(0, 4));
      frames->append(frameTORY);
      newFrames->append(frameTORY);
    }
  }
  if(frameTDRC) {
    String content = frameTDRC->toString();
    if(content.size() >= 4) {
      ID3v2::TextIdentificationFrame *frameTYER = new ID3v2::TextIdentificationFrame("TYER", String::Latin1);
      frameTYER->setText(content.substr(0, 4));
      frames->append(frameTYER);
      newFrames->append(frameTYER);
      if(content.size() >= 10 && content[4] == '-' && content[7] == '-') {
        ID3v2::TextIdentificationFrame *frameTDAT = new ID3v2::TextIdentificationFrame("TDAT", String::Latin1);
        frameTDAT->setText(content.substr(8, 2) + content.substr(5, 2));
        frames->append(frameTDAT);
        newFrames->append(frameTDAT);
        if(content.size() >= 16 && content[10] == 'T' && content[13] == ':') {
          ID3v2::TextIdentificationFrame *frameTIME = new ID3v2::TextIdentificationFrame("TIME", String::Latin1);
          frameTIME->setText(content.substr(11, 2) + content.substr(14, 2));
          frames->append(frameTIME);
          newFrames->append(frameTIME);
        }
      }
    }
  }
  if(frameTIPL || frameTMCL) {
    ID3v2::TextIdentificationFrame *frameIPLS = new ID3v2::TextIdentificationFrame("IPLS", String::Latin1);
    StringList people;
    if(frameTMCL) {
      StringList v24People = frameTMCL->fieldList();
      for(uint i = 0; i + 1 < v24People.size(); i += 2) {
        people.append(v24People[i]);
        people.append(v24People[i+1]);
      }
    }
    if(frameTIPL) {
      StringList v24People = frameTIPL->fieldList();
      for(uint i = 0; i + 1 < v24People.size(); i += 2) {
        people.append(v24People[i]);
        people.append(v24People[i+1]);
      }
    }
    frameIPLS->setText(people);
    frames->append(frameIPLS);
    newFrames->append(frameIPLS);
  }
}


ByteVector ID3v2::Tag::render(int version) const
{
  // We need to render the "tag data" first so that we have to correct size to
  // render in the tag's header.  The "tag data" -- everything that is included
  // in ID3v2::Header::tagSize() -- includes the extended header, frames and
  // padding, but does not include the tag's header or footer.

  ByteVector tagData;

  if(version != 3 && version != 4) {
    debug("Unknown ID3v2 version, using ID3v2.4");
    version = 4;
  }

  // TODO: Render the extended header.

  // Loop through the frames rendering them and adding them to the tagData.

  FrameList newFrames;
  newFrames.setAutoDelete(true);

  FrameList frameList;
  if(version == 4) {
    frameList = d->frameList;
  }
  else {
    downgradeFrames(&frameList, &newFrames);
  }

  for(FrameList::Iterator it = frameList.begin(); it != frameList.end(); it++) {
    (*it)->header()->setVersion(version);
    if((*it)->header()->frameID().size() != 4) {
      debug("A frame of unsupported or unknown type \'"
          + String((*it)->header()->frameID()) + "\' has been discarded");
      continue;
    }
    if(!(*it)->header()->tagAlterPreservation())
      tagData.append((*it)->render());
  }

  // Compute the amount of padding, and append that to tagData.

  uint paddingSize = 0;
  uint originalSize = d->header.tagSize();

  if(tagData.size() < originalSize)
    paddingSize = originalSize - tagData.size();
  else
    paddingSize = 1024;

  tagData.append(ByteVector(paddingSize, char(0)));

  // Set the version and data size.
  d->header.setMajorVersion(version);
  d->header.setTagSize(tagData.size());

  // TODO: This should eventually include d->footer->render().
  return d->header.render() + tagData;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void ID3v2::Tag::read()
{
  if(d->file && d->file->isOpen()) {

    d->file->seek(d->tagOffset);
    d->header.setData(d->file->readBlock(Header::size()));

    // if the tag size is 0, then this is an invalid tag (tags must contain at
    // least one frame)

    if(d->header.tagSize() == 0)
      return;

    parse(d->file->readBlock(d->header.tagSize()));
  }
}

void ID3v2::Tag::parse(const ByteVector &origData)
{
  ByteVector data = origData;

  if(d->header.unsynchronisation() && d->header.majorVersion() <= 3)
    data = SynchData::decode(data);

  uint frameDataPosition = 0;
  uint frameDataLength = data.size();

  // check for extended header

  if(d->header.extendedHeader()) {
    if(!d->extendedHeader)
      d->extendedHeader = new ExtendedHeader;
    d->extendedHeader->setData(data);
    if(d->extendedHeader->size() <= data.size()) {
      frameDataPosition += d->extendedHeader->size();
      frameDataLength -= d->extendedHeader->size();
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

  while(frameDataPosition < frameDataLength - Frame::headerSize(d->header.majorVersion())) {

    // If the next data is position is 0, assume that we've hit the padding
    // portion of the frame data.

    if(data.at(frameDataPosition) == 0) {
      if(d->header.footerPresent())
        debug("Padding *and* a footer found.  This is not allowed by the spec.");

      d->paddingSize = frameDataLength - frameDataPosition;
      return;
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

    frameDataPosition += frame->size() + Frame::headerSize(d->header.majorVersion());
    addFrame(frame);
  }
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
    TextIdentificationFrame *f = new TextIdentificationFrame(id, encoding);
    addFrame(f);
    f->setText(value);
  }
}
