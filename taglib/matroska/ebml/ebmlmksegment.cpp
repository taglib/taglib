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
#include "ebmlutils.h"
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskaattachments.h"
#include "matroskachapters.h"
#include "matroskacues.h"
#include "matroskaseekhead.h"
#include "matroskasegment.h"

using namespace TagLib;

EBML::MkSegment::MkSegment(int sizeLength, offset_t dataSize, offset_t offset):
  MasterElement(Id::MkSegment, sizeLength, dataSize, offset)
{
}

EBML::MkSegment::MkSegment(Id, int sizeLength, offset_t dataSize, offset_t offset):
  MasterElement(Id::MkSegment, sizeLength, dataSize, offset)
{
}

EBML::MkSegment::~MkSegment() = default;

offset_t EBML::MkSegment::segmentDataOffset() const
{
  return offset + idSize(id) + sizeLength;
}

bool EBML::MkSegment::read(File &file)
{
  const offset_t maxOffset = file.tell() + dataSize;
  std::unique_ptr<Element> element;
  int i = 0;
  int seekHeadIndex = -1;
  while((element = findNextElement(file, maxOffset))) {
    if(const Id id = element->getId(); id == Id::MkSeekHead) {
      seekHeadIndex = i;
      seekHead = element_cast<Id::MkSeekHead>(std::move(element));
      if(!seekHead->read(file))
        return false;
    }
    else if(id == Id::MkCues) {
      cues = element_cast<Id::MkCues>(std::move(element));
      if(!cues->read(file))
        return false;
    }
    else if(id == Id::MkInfo) {
      info = element_cast<Id::MkInfo>(std::move(element));
      if(!info->read(file))
        return false;
    }
    else if(id == Id::MkTracks) {
      tracks = element_cast<Id::MkTracks>(std::move(element));
      if(!tracks->read(file))
        return false;
    }
    else if(id == Id::MkTags) {
      tags = element_cast<Id::MkTags>(std::move(element));
      if(!tags->read(file))
        return false;
    }
    else if(id == Id::MkAttachments) {
      attachments = element_cast<Id::MkAttachments>(std::move(element));
      if(!attachments->read(file))
        return false;
    }
    else if(id == Id::MkChapters) {
      chapters = element_cast<Id::MkChapters>(std::move(element));
      if(!chapters->read(file))
        return false;
    }
    else {
      if(id == Id::VoidElement
         && seekHead
         && seekHeadIndex == i - 1)
        seekHead->setPadding(element->getSize());

      element->skipData(file);
    }
    i++;
  }
  return true;
}

std::unique_ptr<Matroska::Tag> EBML::MkSegment::parseTag() const
{
  return tags ? tags->parse() : nullptr;
}

std::unique_ptr<Matroska::Attachments> EBML::MkSegment::parseAttachments() const
{
  return attachments ? attachments->parse() : nullptr;
}

std::unique_ptr<Matroska::Chapters> EBML::MkSegment::parseChapters() const
{
  return chapters ? chapters->parse() : nullptr;
}

std::unique_ptr<Matroska::SeekHead> EBML::MkSegment::parseSeekHead() const
{
  return seekHead ? seekHead->parse(segmentDataOffset()) : nullptr;
}

std::unique_ptr<Matroska::Cues> EBML::MkSegment::parseCues() const
{
  return cues ? cues->parse(segmentDataOffset()) : nullptr;
}

std::unique_ptr<Matroska::Segment> EBML::MkSegment::parseSegment() const
{
  return std::make_unique<Matroska::Segment>(sizeLength, dataSize, offset + idSize(id));
}

void EBML::MkSegment::parseInfo(Matroska::Properties *properties) const
{
  if(info) {
    info->parse(properties);
  }
}

void EBML::MkSegment::parseTracks(Matroska::Properties *properties) const
{
  if(tracks) {
    tracks->parse(properties);
  }
}
