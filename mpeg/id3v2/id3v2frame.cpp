/***************************************************************************
    copyright            : (C) 2002, 2003 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
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
 ***************************************************************************/

#include <tdebug.h>

#include "id3v2frame.h"
#include "id3v2synchdata.h"

using namespace TagLib;
using namespace ID3v2;

class Frame::FramePrivate
{
public:
  FramePrivate() {
    header = 0;
  }

  ~FramePrivate() {
    delete header;
  }

  Frame::Header *header;
};

////////////////////////////////////////////////////////////////////////////////
// static methods
////////////////////////////////////////////////////////////////////////////////

TagLib::uint Frame::headerSize()
{
  return Header::size();
}

ByteVector Frame::textDelimiter(String::Type t)
{
  ByteVector d = char(0);
  if(t == String::UTF16 || t == String::UTF16BE)
    d.append(char(0));
  return d;
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Frame::~Frame()
{
  delete d;
}

ByteVector Frame::frameID() const
{
  if(d->header)
    return d->header->frameID();
  else
    return ByteVector::null;
}

TagLib::uint Frame::size() const
{
  if(d->header)
    return d->header->frameSize();
  else
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

Frame::Frame(const ByteVector &data)
{
  d = new FramePrivate();
  d->header = new Header(data);
}

Frame::Frame(Header *h)
{
  d = new FramePrivate();
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

  // size() is the lenght of the field data
  parseFields(data.mid(Header::size(), size()));
}

////////////////////////////////////////////////////////////////////////////////
// Frame::Header class
////////////////////////////////////////////////////////////////////////////////

class Frame::Header::HeaderPrivate
{
public:
  HeaderPrivate() : frameSize(0) {}

  ByteVector frameID;
  uint frameSize;
  static const unsigned int size = 10;
};

////////////////////////////////////////////////////////////////////////////////
// static members (Frame::Header)
////////////////////////////////////////////////////////////////////////////////

TagLib::uint Frame::Header::size()
{
  return HeaderPrivate::size;
}

////////////////////////////////////////////////////////////////////////////////
// public members (Frame::Header)
////////////////////////////////////////////////////////////////////////////////

Frame::Header::Header(const ByteVector &data, bool synchSafeInts)
{
  d = new HeaderPrivate;
  setData(data, synchSafeInts);
}

Frame::Header::~Header()
{
  delete d;
}

void Frame::Header::setData(const ByteVector &data, bool synchSafeInts)
{
  if(data.size() < 4) {
    debug("You must at least specify a frame ID.");
    return;
  }

  // set the frame ID -- the first four bytes

  d->frameID = data.mid(0, 4);

  // If the full header information was not passed in, do not continue to the
  // steps to parse the frame size and flags.

  if(data.size() < 10) {
    d->frameSize = 0;
    return;
  }

  // Set the size -- the frame size is the four bytes starting at byte four in
  // the frame header (structure 4)

  if(synchSafeInts)
    d->frameSize = SynchData::toUInt(data.mid(4, 4));
  else
    d->frameSize = data.mid(4, 4).toUInt();

  // read flags
  // ...
}

ByteVector Frame::Header::frameID() const
{
  return d->frameID;
}

void Frame::Header::setFrameID(const ByteVector &id)
{
  d->frameID = id.mid(0, 4);
}

TagLib::uint Frame::Header::frameSize() const
{
  return d->frameSize;
}

void Frame::Header::setFrameSize(uint size)
{
  d->frameSize = size;
}

ByteVector Frame::Header::render() const
{
  ByteVector flags(2, char(0)); // just blank for the moment

  ByteVector v = d->frameID + SynchData::fromUInt(d->frameSize) + flags;

  return v;
}
