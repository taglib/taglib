/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
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

#include <tbytevector.h>
#include <tstring.h>
#include <tdebug.h>

#include "mpcfile.h"
#include "id3v1tag.h"
#include "id3v2header.h"
#include "apetag.h"
#include "mpctag.h"

using namespace TagLib;

class MPC::File::FilePrivate
{
public:
  FilePrivate() :
    APETag(0),
    APEFooter(-1),
    APELocation(-1),
    APESize(0),
    ID3v1Tag(0),
    ID3v1Location(-1),
    ID3v2Header(0),
    ID3v2Location(-1),
    ID3v2Size(0),
    tag(0),
    properties(0),
    scanned(false),
    hasAPE(false),
    hasID3v1(false),
    hasID3v2(false) {}

  ~FilePrivate()
  {
    delete ID3v1Tag;
    delete properties;
  }

  APE::Tag *APETag;
  long APEFooter;
  long APELocation;
  uint APESize;

  ID3v1::Tag *ID3v1Tag;
  long ID3v1Location;

  ID3v2::Header *ID3v2Header;
  long ID3v2Location;
  uint ID3v2Size;

  Tag *tag;

  Properties *properties;
  bool scanned;

  bool hasAPE;
  bool hasID3v1;
  bool hasID3v2;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPC::File::File(const char *file, bool readProperties,
                Properties::ReadStyle propertiesStyle) : TagLib::File(file)
{
  d = new FilePrivate;
  read(readProperties, propertiesStyle);
}

MPC::File::~File()
{
  delete d;
}

TagLib::Tag *MPC::File::tag() const
{
  return d->tag;
}

MPC::Properties *MPC::File::audioProperties() const
{
  return d->properties;
}

bool MPC::File::save()
{

  // Update ID3v1 tag

  if(d->ID3v1Tag)
  {
    if(d->hasID3v1)
    {
        seek(-128, End);
        writeBlock(d->ID3v1Tag->render());
    }
    else
    {
        seek(0, End);
        writeBlock(d->ID3v1Tag->render());
    }
  }

  // Update APE tag

  if(d->APETag)
  {
    if(d->hasAPE)
    {
        insert(d->APETag->render(), d->APELocation, d->APESize);
    }
    else
      if (d->hasID3v1)
      {
        seek(-128, End);
        insert(d->APETag->render(), tell(), 0);
      }
      else
      {
        seek(0, End);
        writeBlock(d->APETag->render());
      }
  }

  return true;
}

ID3v1::Tag *MPC::File::ID3v1Tag(bool create)
{
  if(!create || d->ID3v1Tag)
    return d->ID3v1Tag;

  // no ID3v1 tag exists and we've been asked to create one

  d->ID3v1Tag = new ID3v1::Tag;
  if (d->APETag)
    d->tag = new CombinedTag(d->APETag, d->ID3v1Tag);
  else
    d->tag = d->ID3v1Tag;
  return d->ID3v1Tag;
}

APE::Tag *MPC::File::APETag(bool create)
{
  if(!create || d->APETag)
    return d->APETag;

  // no APE tag exists and we've been asked to create one

  d->APETag = new APE::Tag;
  if (d->ID3v1Tag)
    d->tag = new CombinedTag(d->APETag, d->ID3v1Tag);
  else
    d->tag = d->APETag;
  return d->APETag;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPC::File::read(bool readProperties, Properties::ReadStyle /* propertiesStyle */)
{
  // Look for an ID3v1 tag

  d->ID3v1Location = findID3v1();

  if(d->ID3v1Location >= 0) {
    d->ID3v1Tag = new ID3v1::Tag(this, d->ID3v1Location);
    d->hasID3v1 = true;
  }

  // Look for an APE tag

  findAPE();

  if(d->APELocation >= 0) {
    d->APETag = new APE::Tag(this, d->APEFooter);
    d->hasAPE = true;
  }

  if (d->hasID3v1 && d->hasAPE)
    d->tag = new CombinedTag(d->APETag, d->ID3v1Tag);
  else
  if (d->hasID3v1)
    d->tag = d->ID3v1Tag;
  else
  if (d->hasAPE)
    d->tag = d->APETag;
  else
    d->tag = d->APETag = new APE::Tag();

  // Look for and skip an ID3v2 tag

  d->ID3v2Location = findID3v2();

  if(d->ID3v2Location >= 0) {
    seek(d->ID3v2Location);
    d->ID3v2Header = new ID3v2::Header(readBlock(ID3v2::Header::size()));
    d->ID3v2Size = d->ID3v2Header->completeTagSize();
    d->hasID3v2 = true;
  }

  if(d->hasID3v2)
    seek(d->ID3v2Location + d->ID3v2Size);
  else
    seek(0);

  // Look for MPC metadata

  if(readProperties) {
    d->properties = new Properties(readBlock(MPC::HeaderSize),
                                   length() - d->ID3v2Size - d->APESize);
  }
}

bool MPC::File::findAPE()
{
  if(!isValid())
    return false;

  if (d->hasID3v1)
    seek(-160, End);
  else
    seek(-32, End);
  long p = tell();

  ByteVector footer = readBlock(32);
  if(footer.mid(0,8) == APE::Tag::fileIdentifier()) {
     d->APEFooter = p;
     d->APESize = APE::Tag::tagSize(footer);
     d->APELocation = p + 32 - d->APESize;
     return true;
  }

  return false;
}

long MPC::File::findID3v1()
{
  if(!isValid())
    return -1;

  seek(-128, End);
  long p = tell();

  if(readBlock(3) == ID3v1::Tag::fileIdentifier())
    return p;

  return -1;
}

long MPC::File::findID3v2()
{
  if(!isValid())
    return -1;

  seek(0);

  if(readBlock(3) == ID3v2::Header::fileIdentifier())
    return 0;

  return -1;
}
