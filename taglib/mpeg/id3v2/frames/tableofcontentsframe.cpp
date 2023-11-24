/***************************************************************************
    copyright            : (C) 2013 by Lukas Krejci
    email                : krejclu6@fel.cvut.cz
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

#include "tableofcontentsframe.h"

#include <utility>

#include "tpropertymap.h"
#include "tdebug.h"

using namespace TagLib;
using namespace ID3v2;

class TableOfContentsFrame::TableOfContentsFramePrivate
{
public:
  TableOfContentsFramePrivate()
  {
    embeddedFrameList.setAutoDelete(true);
  }

  const ID3v2::Header *tagHeader { nullptr };
  ByteVector elementID;
  bool isTopLevel { false };
  bool isOrdered { false };
  ByteVectorList childElements;
  FrameListMap embeddedFrameListMap;
  FrameList embeddedFrameList;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

TableOfContentsFrame::TableOfContentsFrame(const ID3v2::Header *tagHeader, const ByteVector &data) :
  ID3v2::Frame(data),
  d(std::make_unique<TableOfContentsFramePrivate>())
{
  d->tagHeader = tagHeader;
  setData(data);
}

TableOfContentsFrame::TableOfContentsFrame(const ByteVector &elementID,
                                           const ByteVectorList &children,
                                           const FrameList &embeddedFrames) :
  ID3v2::Frame("CTOC"),
  d(std::make_unique<TableOfContentsFramePrivate>())
{
  d->elementID = elementID;
  d->childElements = children;

  for(const auto &frame : embeddedFrames)
    addEmbeddedFrame(frame);
}

TableOfContentsFrame::~TableOfContentsFrame() = default;

ByteVector TableOfContentsFrame::elementID() const
{
  return d->elementID;
}

bool TableOfContentsFrame::isTopLevel() const
{
  return d->isTopLevel;
}

bool TableOfContentsFrame::isOrdered() const
{
  return d->isOrdered;
}

unsigned int TableOfContentsFrame::entryCount() const
{
  return d->childElements.size();
}

ByteVectorList TableOfContentsFrame::childElements() const
{
  return d->childElements;
}

void TableOfContentsFrame::setElementID(const ByteVector &eID)
{
  d->elementID = eID;
}

void TableOfContentsFrame::setIsTopLevel(const bool &t)
{
  d->isTopLevel = t;
}

void TableOfContentsFrame::setIsOrdered(const bool &o)
{
  d->isOrdered = o;
}

void TableOfContentsFrame::setChildElements(const ByteVectorList &l)
{
  d->childElements = l;
}

void TableOfContentsFrame::addChildElement(const ByteVector &cE)
{
  d->childElements.append(cE);
}

void TableOfContentsFrame::removeChildElement(const ByteVector &cE)
{
  auto it = d->childElements.find(cE);

  if(it == d->childElements.end())
    it = d->childElements.find(cE + ByteVector("\0"));

  if(it != d->childElements.end())
    d->childElements.erase(it);
}

const FrameListMap &TableOfContentsFrame::embeddedFrameListMap() const
{
  return d->embeddedFrameListMap;
}

const FrameList &TableOfContentsFrame::embeddedFrameList() const
{
  return d->embeddedFrameList;
}

const FrameList &TableOfContentsFrame::embeddedFrameList(const ByteVector &frameID) const
{
  return d->embeddedFrameListMap[frameID];
}

void TableOfContentsFrame::addEmbeddedFrame(Frame *frame)
{
  d->embeddedFrameList.append(frame);
  d->embeddedFrameListMap[frame->frameID()].append(frame);
}

void TableOfContentsFrame::removeEmbeddedFrame(Frame *frame, bool del)
{
  // remove the frame from the frame list
  auto it = d->embeddedFrameList.find(frame);
  if(it != d->embeddedFrameList.end())
    d->embeddedFrameList.erase(it);

  // ...and from the frame list map
  FrameList &mappedList = d->embeddedFrameListMap[frame->frameID()];
  it = mappedList.find(frame);
  if(it != mappedList.end())
    mappedList.erase(it);

  // ...and delete as desired
  if(del)
    delete frame;
}

void TableOfContentsFrame::removeEmbeddedFrames(const ByteVector &id)
{
  const FrameList frames = d->embeddedFrameListMap[id];
  for(const auto &frame : frames)
    removeEmbeddedFrame(frame, true);
}

String TableOfContentsFrame::toString() const
{
  String s = String(d->elementID) +
             ": top level: " + (d->isTopLevel ? "true" : "false") +
             ", ordered: " + (d->isOrdered ? "true" : "false");

  if(!d->childElements.isEmpty()) {
    s+= ", chapters: [ " + String(d->childElements.toByteVector(", ")) + " ]";
  }

  if(!d->embeddedFrameList.isEmpty()) {
    StringList frameIDs;
    for(const auto &frame : std::as_const(d->embeddedFrameList))
      frameIDs.append(frame->frameID());
    s += ", sub-frames: [ " + frameIDs.toString(", ") + " ]";
  }

  return s;
}

PropertyMap TableOfContentsFrame::asProperties() const
{
  PropertyMap map;

  map.addUnsupportedData(frameID() + String("/") + d->elementID);

  return map;
}

TableOfContentsFrame *TableOfContentsFrame::findByElementID(const ID3v2::Tag *tag,
                                                            const ByteVector &eID) // static
{
  for(const auto &table : std::as_const(tag->frameList("CTOC"))) {
    auto frame = dynamic_cast<TableOfContentsFrame *>(table);
    if(frame && frame->elementID() == eID)
      return frame;
  }

  return nullptr;
}

TableOfContentsFrame *TableOfContentsFrame::findTopLevel(const ID3v2::Tag *tag) // static
{
  for(const auto &table : std::as_const(tag->frameList("CTOC"))) {
    auto frame = dynamic_cast<TableOfContentsFrame *>(table);
    if(frame && frame->isTopLevel())
      return frame;
  }

  return nullptr;
}

void TableOfContentsFrame::parseFields(const ByteVector &data)
{
  unsigned int size = data.size();
  if(size < 6) {
    debug("A CTOC frame must contain at least 6 bytes (1 byte element ID terminated by "
          "null, 1 byte flags, 1 byte entry count and 1 byte child element ID terminated "
          "by null.");
    return;
  }

  int pos = 0;
  unsigned int embPos = 0;
  d->elementID = readStringField(data, String::Latin1, &pos).data(String::Latin1);
  d->isTopLevel = (data.at(pos) & 2) != 0;
  d->isOrdered = (data.at(pos++) & 1) != 0;
  unsigned int entryCount = static_cast<unsigned char>(data.at(pos++));
  for(unsigned int i = 0; i < entryCount; i++) {
    ByteVector childElementID = readStringField(data, String::Latin1, &pos).data(String::Latin1);
    d->childElements.append(childElementID);
  }

  size -= pos;

  if(size < header()->size())
    return;

  while(embPos < size - header()->size()) {
    Frame *frame = FrameFactory::instance()->createFrame(data.mid(pos + embPos), d->tagHeader);

    if(!frame)
      return;

    // Checks to make sure that frame parsed correctly.
    if(frame->size() <= 0) {
      delete frame;
      return;
    }

    embPos += frame->size() + header()->size();
    addEmbeddedFrame(frame);
  }
}

ByteVector TableOfContentsFrame::renderFields() const
{
  ByteVector data;

  data.append(d->elementID);
  data.append('\0');
  char flags = 0;
  if(d->isTopLevel)
    flags += 2;
  if(d->isOrdered)
    flags += 1;
  data.append(flags);
  data.append(static_cast<char>(entryCount()));
  for(const auto &element : std::as_const(d->childElements)) {
    data.append(element);
    data.append('\0');
  }
  for(const auto &frame : std::as_const(d->embeddedFrameList)) {
    frame->header()->setVersion(header()->version());
    data.append(frame->render());
  }

  return data;
}

TableOfContentsFrame::TableOfContentsFrame(const ID3v2::Header *tagHeader,
                                           const ByteVector &data, Header *h) :
  Frame(h),
  d(std::make_unique<TableOfContentsFramePrivate>())
{
  d->tagHeader = tagHeader;
  parseFields(fieldData(data));
}
