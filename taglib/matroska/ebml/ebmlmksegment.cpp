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

#include "ebmlmksegment.h"
#include "ebmlmktags.h"
#include "ebmlmkattachments.h"
#include "ebmlmkseekhead.h"
#include "ebmlmkinfo.h"
#include "ebmlmktracks.h"
#include "ebmlutils.h"
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskaattachments.h"
#include "matroskaseekhead.h"
#include "matroskasegment.h"

using namespace TagLib;

EBML::MkSegment::~MkSegment()
{
  delete tags;
  delete attachments;
  delete seekHead;
  delete info;
  delete tracks;
}

bool EBML::MkSegment::read(File &file)
{
  offset_t maxOffset = file.tell() + dataSize;
  Element *element = nullptr;
  int i = 0;
  int seekHeadIndex = -1;
  while((element = findNextElement(file, maxOffset))) {
    Id id = element->getId();
    if(id == ElementIDs::MkSeekHead) {
      seekHeadIndex = i;
      seekHead = static_cast<MkSeekHead *>(element);
      if(!seekHead->read(file))
        return false;
    }
    else if(id == ElementIDs::MkInfo) {
      info = static_cast<MkInfo *>(element);
      if(!info->read(file))
        return false;
    }
    else if(id == ElementIDs::MkTracks) {
      tracks = static_cast<MkTracks *>(element);
      if(!tracks->read(file))
        return false;
    }
    else if(id == ElementIDs::MkTags) {
      tags = static_cast<MkTags *>(element);
      if(!tags->read(file))
        return false;
    }
    else if(id == ElementIDs::MkAttachments) {
      attachments = static_cast<MkAttachments *>(element);
      if(!attachments->read(file))
        return false;
    }
    else {
      if(id == ElementIDs::VoidElement
         && seekHead
         && seekHeadIndex == i - 1)
        seekHead->setPadding(element->getSize());

      element->skipData(file);
      delete element;
    }
    i++;
  }
  return true;
}

Matroska::Tag *EBML::MkSegment::parseTag()
{
  return tags ? tags->parse() : nullptr;
}

Matroska::Attachments *EBML::MkSegment::parseAttachments()
{
  return attachments ? attachments->parse() : nullptr;
}

Matroska::SeekHead *EBML::MkSegment::parseSeekHead()
{
  return seekHead ? seekHead->parse() : nullptr;
}

Matroska::Segment *EBML::MkSegment::parseSegment()
{
  return new Matroska::Segment(sizeLength, dataSize, offset + idSize(id));
}

void EBML::MkSegment::parseInfo(Matroska::Properties *properties)
{
  if(info) {
    info->parse(properties);
  }
}

void EBML::MkSegment::parseTracks(Matroska::Properties *properties)
{
  if (tracks) {
    tracks->parse(properties);
  }
}
