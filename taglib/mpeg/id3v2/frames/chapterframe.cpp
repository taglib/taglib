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

ChapterFrame::ChapterFrame(const ByteVector &eID, const int &sT, const int &eT, const int &sO, const int &eO) :
    ID3v2::Frame("CHAP")
{
  d = new ChapterFramePrivate;
  d->elementID = eID;
  d->startTime = sT;
  d->endTime = eT;
  d->startOffset = sO;
  d->endOffset = e0;
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

String UniqueFileIdentifierFrame::toString() const
{
  return String::null;
}

PropertyMap ChapterFrame::asProperties() const
{
  //DODELAT
  PropertyMap map;
  if(d->owner == "http://musicbrainz.org") {
    map.insert("MUSICBRAINZ_TRACKID", String(d->identifier));
  }
  else {
    map.unsupportedData().append(frameID() + String("/") + d->owner);
  }
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
  //DODELAT
  if(data.size() < 1) {
    debug("An UFID frame must contain at least 1 byte.");
    return;
  }

  int pos = 0;
  d->owner = readStringField(data, String::Latin1, &pos);
  d->identifier = data.mid(pos);
}

ByteVector ChapterFrame::renderFields() const
{
  //DODELAT
  ByteVector data;

  data.append(d->owner.data(String::Latin1));
  data.append(char(0));
  data.append(d->identifier);

  return data;
}

ChapterFrame::ChapterFrame(const ByteVector &data, Header *h) :
  Frame(h)
{
  d = new ChapterFramePrivate;
  parseFields(fieldData(data));
}
