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

#include "id3v2framefactory.h"

#include "frames/unknownframe.h"
#include "frames/textidentificationframe.h"
#include "frames/commentsframe.h"

using namespace TagLib;
using namespace ID3v2;

class FrameFactory::FrameFactoryPrivate
{
public:
  FrameFactoryPrivate() :
    defaultEncoding(String::Latin1),
    useDefaultEncoding(false) {}

  String::Type defaultEncoding;
  bool useDefaultEncoding;
};

FrameFactory *FrameFactory::factory = 0;

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FrameFactory *FrameFactory::instance()
{
  if(!factory)
    factory = new FrameFactory;
  return factory;
}

Frame *FrameFactory::createFrame(const ByteVector &data, bool synchSafeInts) const
{
  return createFrame(data, uint(synchSafeInts ? 4 : 3));
}

Frame *FrameFactory::createFrame(const ByteVector &data, uint version) const
{
  Frame::Header *header = new Frame::Header(data, version);

  TagLib::ByteVector frameID = header->frameID();

  // A quick sanity check -- make sure that the frameID is 4 uppercase Latin1
  // characters.  Also make sure that there is data in the frame.

  if(!frameID.size() == (version < 3 ? 3 : 4) || header->frameSize() <= 0)
      return 0;

  for(ByteVector::ConstIterator it = frameID.begin(); it != frameID.end(); it++) {
    if( (*it < 'A' || *it > 'Z') && (*it < '1' || *it > '9') ) {
      delete header;
      return 0;
    }
  }

  if(!updateFrame(header)) {
    delete header;
    return 0;
  }

  frameID = header->frameID();

  // This is where things get necissarily nasty.  Here we determine which
  // Frame subclass (or if none is found simply an Frame) based
  // on the frame ID.  Since there are a lot of possibilities, that means
  // a lot of if blocks.

  // Text Identification (frames 4.2)

  if(frameID.startsWith("T") && frameID != "TXXX") {
    TextIdentificationFrame *f = new TextIdentificationFrame(data, header);
    if(d->useDefaultEncoding)
      f->setTextEncoding(d->defaultEncoding);
    return f;
  }

  // Comments (frames 4.10)

  if(frameID == "COMM") {
    CommentsFrame *f = new CommentsFrame(data, header);
    if(d->useDefaultEncoding)
      f->setTextEncoding(d->defaultEncoding);
    return f;
  }

  return new UnknownFrame(data, header);
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

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

FrameFactory::FrameFactory()
{
  d = new FrameFactoryPrivate;
}

FrameFactory::~FrameFactory()
{
  delete d;
}

bool FrameFactory::updateFrame(Frame::Header *header) const
{
  TagLib::ByteVector frameID = header->frameID();
  if(frameID == "EQUA" ||
     frameID == "RVAD" ||
     frameID == "TIME" ||
     frameID == "TRDA" ||
     frameID == "TSIZ")
  {
    debug("ID3v2.4 no longer supports the frame type " + String(frameID) +
          ".  It will be discarded from the tag.");
    return false;
  }

  convertFrame("TDAT", "TRDC", header);
  convertFrame("TORY", "TDOR", header);
  convertFrame("TYER", "TRDC", header);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void FrameFactory::convertFrame(const ByteVector &from, const ByteVector &to,
                                Frame::Header *header) const
{
  if(header->frameID() != from)
    return;

  // debug("ID3v2.4 no longer supports the frame type " + String(from) + "  It has" +
  //       "been converted to the type " + String(to) + ".");

  header->setFrameID(to);
}
