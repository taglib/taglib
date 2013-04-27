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

#include <tbytevectorlist.h>
#include <tpropertymap.h>
#include <tdebug.h>

#include "id3v2tag.h"
#include "chapterframe.h"

using namespace TagLib;
using namespace ID3v2;

class ChapterFrame::ChapterFramePrivate
{
public:
  ByteVector elementID;
  uint startTime;
  uint endTime;
  uint startOffset;
  uint endOffset;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

ChapterFrame::ChapterFrame(const ByteVector &data) :
    ID3v2::Frame(data)
{
  d = new ChapterFramePrivate;
  setData(data);
}

ChapterFrame::ChapterFrame(const ByteVector &eID, const uint &sT, const uint &eT, const uint &sO, const uint &eO) :
    ID3v2::Frame("CHAP")
{
  d = new ChapterFramePrivate;
  d->elementID = eID;
  d->startTime = sT;
  d->endTime = eT;
  d->startOffset = sO;
  d->endOffset = eO;
}

ChapterFrame::~ChapterFrame()
{
  delete d;
}

ByteVector ChapterFrame::elementID() const
{
    return d->elementID;
}

uint ChapterFrame::startTime() const
{
  return d->startTime;
}

uint ChapterFrame::endTime() const
{
  return d->endTime;
}

uint ChapterFrame::startOffset() const
{
  return d->startOffset;
}

uint ChapterFrame::endOffset() const
{
  return d->endOffset;
}

void ChapterFrame::setElementID(const ByteVector &eID)
{
  d->elementID = eID;
  if(eID.at(eID.size() - 1) != char(0))
    d->elementID.append(char(0));
}

void ChapterFrame::setStartTime(const uint &sT)
{
  d->startTime = sT;
}

void ChapterFrame::setEndTime(const uint &eT)
{
  d->endTime = eT;
}

void ChapterFrame::setStartOffset(const uint &sO)
{
  d->startOffset = sO;
}

void ChapterFrame::setEndOffset(const uint &eO)
{
  d->endOffset = eO;
}

String ChapterFrame::toString() const
{
  return String::null;
}

PropertyMap ChapterFrame::asProperties() const
{
  PropertyMap map;

  map.unsupportedData().append(frameID() + String("/") + d->elementID);
  
  return map;
}

ChapterFrame *ChapterFrame::findByElementID(const Tag *tag, const ByteVector &eID) // static
{
  ID3v2::FrameList comments = tag->frameList("CHAP");

  for(ID3v2::FrameList::ConstIterator it = comments.begin();
      it != comments.end();
      ++it)
  {
    ChapterFrame *frame = dynamic_cast<ChapterFrame *>(*it);
    if(frame && frame->elementID() == eID)
      return frame;
  }

  return 0;
}

void ChapterFrame::parseFields(const ByteVector &data)
{
  if(data.size() < 18) {
    debug("An CHAP frame must contain at least 18 bytes (1 byte element ID terminated by null and 4x4 bytes for start and end time and offset).");
    return;
  }

  int pos = 0;
  d->elementID = readStringField(data, String::Latin1, &pos).data(String::Latin1);
  d->elementID.append(char(0));
  d->startTime = data.mid(pos, 4).toUInt(true);
  pos += 4;
  d->endTime = data.mid(pos, 4).toUInt(true);
  pos += 4;
  d->startOffset = data.mid(pos, 4).toUInt(true);
  pos += 4;
  d->endOffset = data.mid(pos, 4).toUInt(true);
}

ByteVector ChapterFrame::renderFields() const
{
  ByteVector data;

  data.append(d->elementID);
  data.append(ByteVector::fromUInt(d->startTime, true));
  data.append(ByteVector::fromUInt(d->endTime, true));
  data.append(ByteVector::fromUInt(d->startOffset, true));
  data.append(ByteVector::fromUInt(d->endOffset, true));

  return data;
}

ChapterFrame::ChapterFrame(const ByteVector &data, Header *h) :
  Frame(h)
{
  d = new ChapterFramePrivate;
  parseFields(fieldData(data));
}
