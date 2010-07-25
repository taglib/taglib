/**************************************************************************
    copyright            : (C) 2010 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <taglib.h>
#include <tdebug.h>
#include "flacpicture.h"

using namespace TagLib;

class FLAC::Picture::PicturePrivate 
{
public:
  PicturePrivate() :
    type(ID3v2::AttachedPictureFrame::Other),
    width(0),
    height(0),
    colorDepth(0),
    numColors(0)
    {}

  Type type;
  String mimeType;
  String description;
  int width;
  int height;
  int colorDepth;
  int numColors;
  ByteVector data;
};

FLAC::Picture::Picture()
{
  d = new PicturePrivate;
}

FLAC::Picture::Picture(const ByteVector &data)
{
  d = new PicturePrivate;
  parse(data);
}

FLAC::Picture::~Picture()
{
  delete d;
}

bool FLAC::Picture::parse(const ByteVector &data)
{
  if(data.size() < 32) {
    debug("A picture block must contain at least 5 bytes.");
    return false;
  }

  int pos = 0;
  d->type = TagLib::ID3v2::AttachedPictureFrame::Type(data.mid(pos, 4).toUInt());
  pos += 4;
  uint mimeTypeLength = data.mid(pos, 4).toUInt();
  pos += 4;
  if(pos + mimeTypeLength + 24 > data.size()) {
    debug("Invalid picture block.");
    return false;
  }
  d->mimeType = String(data.mid(pos, mimeTypeLength), String::UTF8);
  pos += mimeTypeLength;
  uint descriptionLength = data.mid(pos, 4).toUInt();
  pos += 4;
  if(pos + descriptionLength + 20 > data.size()) {
    debug("Invalid picture block.");
    return false;
  }
  d->description = String(data.mid(pos, descriptionLength), String::UTF8);
  pos += descriptionLength;
  d->width = data.mid(pos, 4).toUInt();
  pos += 4;
  d->height = data.mid(pos, 4).toUInt();
  pos += 4;
  d->colorDepth = data.mid(pos, 4).toUInt();
  pos += 4;
  d->numColors = data.mid(pos, 4).toUInt();
  pos += 4;
  uint dataLength = data.mid(pos, 4).toUInt();
  pos += 4;
  if(pos + dataLength > data.size()) {
    debug("Invalid picture block.");
    return false;
  }
  d->data = data.mid(pos, dataLength);

  return true;  
}

FLAC::Picture::Type FLAC::Picture::type() const
{
  return d->type;
}

void FLAC::Picture::setType(FLAC::Picture::Type type)
{
  d->type = type;
}

String FLAC::Picture::mimeType() const
{
  return d->mimeType;
}

void FLAC::Picture::setMimeType(const String &mimeType)
{
  d->mimeType = mimeType;
}

String FLAC::Picture::description() const
{
  return d->description;
}

void FLAC::Picture::setDescription(const String &description)
{
  d->description = description;
}

int FLAC::Picture::width() const
{
  return d->width;
}

void FLAC::Picture::setWidth(int width)
{
  d->width = width;
}

int FLAC::Picture::height() const
{
  return d->height;
}

void FLAC::Picture::setHeight(int height)
{
  d->height = height;
}

int FLAC::Picture::colorDepth() const
{
  return d->colorDepth;
}

void FLAC::Picture::setColorDepth(int colorDepth)
{
  d->colorDepth = colorDepth;
}

int FLAC::Picture::numColors() const
{
  return d->numColors;
}

void FLAC::Picture::setNumColors(int numColors)
{
  d->numColors = numColors;
}

ByteVector FLAC::Picture::data() const
{
  return d->data;
}

void FLAC::Picture::setData(const ByteVector &data)
{
  d->data = data;
}

