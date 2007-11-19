/***************************************************************************
    copyright            : (C) 2006 by Lukáš Lalinský
    email                : lalinsky@gmail.com

    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
                           (original MPC implementation)
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <tbytevector.h>
#include <tstring.h>
#include <tdebug.h>

#include "wavpackfile.h"
#include "id3v1tag.h"
#include "id3v2header.h"
#include "apetag.h"
#include "apefooter.h"
#include "combinedtag.h"

using namespace TagLib;

class WavPack::File::FilePrivate
{
public:
  FilePrivate() :
    APETag(0),
    APELocation(-1),
    APESize(0),
    ID3v1Tag(0),
    ID3v1Location(-1),
    tag(0),
    properties(0),
    scanned(false),
    hasAPE(false),
    hasID3v1(false) {}

  ~FilePrivate()
  {
    if (tag != ID3v1Tag && tag != APETag) delete tag;
    delete ID3v1Tag;
    delete APETag;
    delete properties;
  }

  APE::Tag *APETag;
  long APELocation;
  uint APESize;

  ID3v1::Tag *ID3v1Tag;
  long ID3v1Location;

  Tag *tag;

  Properties *properties;
  bool scanned;

  // These indicate whether the file *on disk* has these tags, not if
  // this data structure does.  This is used in computing offsets.

  bool hasAPE;
  bool hasID3v1;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

WavPack::File::File(FileName file, bool readProperties,
                Properties::ReadStyle propertiesStyle) : TagLib::File(file)
{
  d = new FilePrivate;
  read(readProperties, propertiesStyle);
}

WavPack::File::~File()
{
  delete d;
}

TagLib::Tag *WavPack::File::tag() const
{
  return d->tag;
}

WavPack::Properties *WavPack::File::audioProperties() const
{
  return d->properties;
}

bool WavPack::File::save()
{
  if(readOnly()) {
    debug("WavPack::File::save() -- File is read only.");
    return false;
  }

  // Update ID3v1 tag

  if(d->ID3v1Tag) {
    if(d->hasID3v1) {
      seek(d->ID3v1Location);
      writeBlock(d->ID3v1Tag->render());
    }
    else {
      seek(0, End);
      d->ID3v1Location = tell();
      writeBlock(d->ID3v1Tag->render());
      d->hasID3v1 = true;
    }
  } else
    if(d->hasID3v1) {
      removeBlock(d->ID3v1Location, 128);
      d->hasID3v1 = false;
      if(d->hasAPE) {
        if(d->APELocation > d->ID3v1Location)
          d->APELocation -= 128;
      }
    }

  // Update APE tag

  if(d->APETag) {
    if(d->hasAPE)
      insert(d->APETag->render(), d->APELocation, d->APESize);
    else {
      if(d->hasID3v1)  {
        insert(d->APETag->render(), d->ID3v1Location, 0);
        d->APESize = d->APETag->footer()->completeTagSize();
        d->hasAPE = true;
        d->APELocation = d->ID3v1Location;
        d->ID3v1Location += d->APESize;
      }
      else {
        seek(0, End);
        d->APELocation = tell();
        writeBlock(d->APETag->render());
        d->APESize = d->APETag->footer()->completeTagSize();
        d->hasAPE = true;
      }
    }
  }
  else
    if(d->hasAPE) {
      removeBlock(d->APELocation, d->APESize);
      d->hasAPE = false;
      if(d->hasID3v1) {
        if (d->ID3v1Location > d->APELocation)
          d->ID3v1Location -= d->APESize;
      }
    }

   return true;
}

ID3v1::Tag *WavPack::File::ID3v1Tag(bool create)
{
  if(!create || d->ID3v1Tag)
    return d->ID3v1Tag;

  // no ID3v1 tag exists and we've been asked to create one

  d->ID3v1Tag = new ID3v1::Tag;

  if(d->APETag)
    d->tag = new CombinedTag(d->APETag, d->ID3v1Tag);
  else
    d->tag = d->ID3v1Tag;

  return d->ID3v1Tag;
}

APE::Tag *WavPack::File::APETag(bool create)
{
  if(!create || d->APETag)
    return d->APETag;

  // no APE tag exists and we've been asked to create one

  d->APETag = new APE::Tag;

  if(d->ID3v1Tag)
    d->tag = new CombinedTag(d->APETag, d->ID3v1Tag);
  else
    d->tag = d->APETag;

  return d->APETag;
}

void WavPack::File::remove(int tags)
{
  if(tags & ID3v1) {
    delete d->ID3v1Tag;
    d->ID3v1Tag = 0;

    if(d->APETag)
      d->tag = d->APETag;
    else
      d->tag = d->APETag = new APE::Tag;
  }

  if(tags & APE) {
    delete d->APETag;
    d->APETag = 0;

    if(d->ID3v1Tag)
      d->tag = d->ID3v1Tag;
    else
      d->tag = d->APETag = new APE::Tag;
  }
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void WavPack::File::read(bool readProperties, Properties::ReadStyle /* propertiesStyle */)
{
  // Look for an ID3v1 tag

  d->ID3v1Location = findID3v1();

  if(d->ID3v1Location >= 0) {
    d->ID3v1Tag = new ID3v1::Tag(this, d->ID3v1Location);
    d->hasID3v1 = true;
  }

  // Look for an APE tag

  d->APELocation = findAPE();

  if(d->APELocation >= 0) {
    d->APETag = new APE::Tag(this, d->APELocation);
    d->APESize = d->APETag->footer()->completeTagSize();
    d->APELocation = d->APELocation + d->APETag->footer()->size() - d->APESize;
    d->hasAPE = true;
  }

  if(d->hasID3v1 && d->hasAPE)
    d->tag = new CombinedTag(d->APETag, d->ID3v1Tag);
  else {
    if(d->hasID3v1)
      d->tag = d->ID3v1Tag;
    else {
      if(d->hasAPE)
        d->tag = d->APETag;
      else
        d->tag = d->APETag = new APE::Tag;
    }
  }

  // Look for WavPack audio properties

  if(readProperties) {
    seek(0);
    d->properties = new Properties(readBlock(WavPack::HeaderSize),
                                   length() - d->APESize);
  }
}

long WavPack::File::findAPE()
{
  if(!isValid())
    return -1;

  if(d->hasID3v1)
    seek(-160, End);
  else
    seek(-32, End);

  long p = tell();

  if(readBlock(8) == APE::Tag::fileIdentifier())
    return p;

  return -1;
}

long WavPack::File::findID3v1()
{
  if(!isValid())
    return -1;

  seek(-128, End);
  long p = tell();

  if(readBlock(3) == ID3v1::Tag::fileIdentifier())
    return p;

  return -1;
}
