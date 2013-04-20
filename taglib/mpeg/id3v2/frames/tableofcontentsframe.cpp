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
#include "tableofcontentsframe.h"

using namespace TagLib;
using namespace ID3v2;

class TableOfContentsFrame::TableOfContentsFramePrivate
{
public:
  ByteVector elementID;
  bool isTopLevel;
  bool isOrdered;
  ByteVectorList childElements;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

TableOfContentsFrame::TableOfContentsFrame(const ByteVector &data) :
    ID3v2::Frame(data)
{
  d = new TableOfContentsFramePrivate;
  setData(data);
}

TableOfContentsFrame::TableOfContentsFrame(const ByteVector &eID, const ByteVectorList &ch) :
    ID3v2::Frame("CTOC")
{
  d = new TableOfContentsFramePrivate;
  d->elementID = eID;
  d->childElements = ch;
}

TableOfContentsFrame::~TableOfContentsFrame()
{
  delete d;
}

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

unsigned char TableOfContentsFrame::entryCount() const
{
  return (unsigned char)(d->childElements.size());
}

ByteVectorList TableOfContentsFrame::childElements const
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

String TableOfContentsFrame::toString() const
{
  return String::null;
}

PropertyMap TableOfContentsFrame::asProperties() const
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

TableOfContentsFrame *TableOfContentsFrame::findByElementID(const ID3v2::Tag *tag, const ByteVector &eID) // static
{
  ID3v2::FrameList comments = tag->frameList("CTOC");

  for(ID3v2::FrameList::ConstIterator it = comments.begin();
      it != comments.end();
      ++it)
  {
    TableOfContentsFrame *frame = dynamic_cast<TableOfContentsFrame *>(*it);
    if(frame && frame->elementID() == eID)
      return frame;
  }

  return 0;
}

void TableOfContentsFrame::parseFields(const ByteVector &data)
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

ByteVector TableOfContentsFrame::renderFields() const
{
  //DODELAT
  ByteVector data;

  data.append(d->owner.data(String::Latin1));
  data.append(char(0));
  data.append(d->identifier);

  return data;
}

TableOfContentsFrame::TableOfContentsFrame(const ByteVector &data, Header *h) :
  Frame(h)
{
  d = new TableOfContentsFramePrivate;
  parseFields(fieldData(data));
}
