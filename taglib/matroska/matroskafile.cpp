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

#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskaattachments.h"
#include "ebmlutils.h"
#include "ebmlelement.h"
#include "ebmlmksegment.h"
#include "tlist.h"
#include "tdebug.h"
#include "tutils.h"

#include <memory>
#include <algorithm>
#include <vector>

using namespace TagLib;

class Matroska::File::FilePrivate
{
public:
  FilePrivate() {}
  ~FilePrivate()
  {
    delete tag;
    delete attachments;
  }

  FilePrivate(const FilePrivate &) = delete;
  FilePrivate &operator=(const FilePrivate &) = delete;
  Matroska::Tag *tag = nullptr;
  Matroska::Attachments *attachments = nullptr;
  offset_t segmentSizeOffset = 0;
  offset_t segmentSizeLength = 0;
  offset_t segmentDataSize = 0;

};

Matroska::File::File(FileName file, bool readProperties)
: TagLib::File(file),
  d(std::make_unique<FilePrivate>())
{
  if(!isOpen()) {
    debug("Failed to open matroska file");
    setValid(false);
    return;
  }
  read(readProperties);
}
Matroska::File::File(IOStream *stream, bool readProperties)
: TagLib::File(stream),
  d(std::make_unique<FilePrivate>())
{
  if(!isOpen()) {
    debug("Failed to open matroska file");
    setValid(false);
    return;
  }
  read(readProperties);
}
Matroska::File::~File() = default;

TagLib::Tag* Matroska::File::tag() const
{
  return tag(true);
}

Matroska::Tag* Matroska::File::tag(bool create) const
{
  if(d->tag)
    return d->tag;
  else {
    if(create)
      d->tag = new Matroska::Tag();
    return d->tag;
  }
}

Matroska::Attachments* Matroska::File::attachments(bool create) const
{
  if(d->attachments)
    return d->attachments;
  else {
    if(create)
      d->attachments = new Attachments();
    return d->attachments;
  }
}

void Matroska::File::read(bool readProperties)
{
  offset_t fileLength = length();

  // Find the EBML Header
  std::unique_ptr<EBML::Element> head(EBML::Element::factory(*this));
  if(!head || head->getId() != EBML::ElementIDs::EBMLHeader) {
    debug("Failed to find EBML head");
    setValid(false);
    return;
  }
  head->skipData(*this);

  // Find the Matroska segment in the file
  std::unique_ptr<EBML::MkSegment> segment(
    static_cast<EBML::MkSegment*>(
      EBML::findElement(*this, EBML::ElementIDs::MkSegment, fileLength - tell())
    )
  );
  if(!segment) {
    debug("Failed to find Matroska segment");
    setValid(false);
    return;
  }
  d->segmentSizeLength = segment->getSizeLength();
  d->segmentSizeOffset = tell() - d->segmentSizeLength;
  d->segmentDataSize = segment->getDataSize();

  // Read the segment into memory from file
  if(!segment->read(*this)) {
    debug("Failed to read segment");
    setValid(false);
    return;
  }

  // Parse the tag
  d->tag = segment->parseTag();

  // Parse the attachments
  d->attachments = segment->parseAttachments();

  setValid(true);
}

bool Matroska::File::save()
{
  std::vector<Element*> renderListExisting;
  std::vector<Element*> renderListNew;
  offset_t newSegmentDataSize = d->segmentDataSize;


  if(d->tag) {
    if(d->tag->size())
      renderListExisting.push_back(d->tag);
    else
      renderListNew.push_back(d->tag);
  }

  if(d->attachments) {
    //d->attachments->setOffset(d->tag->offset());
    //renderListExisting.push_back(d->attachments);
    if(d->attachments->size())
      renderListExisting.push_back(d->attachments);
    else
      renderListNew.push_back(d->attachments);

      
  }


  // Render from end to beginning so we don't have to shift
  // the file offsets
  std::sort(renderListExisting.begin(),
    renderListExisting.end(),
    [](auto a, auto b) { return a->offset() > b->offset(); }
  );

  // Overwrite existing elements
  for(auto element : renderListExisting) {
    offset_t offset = element->offset();
    offset_t originalSize = element->size();
    ByteVector data = element->render();
    insert(data, offset, originalSize);
    newSegmentDataSize += (data.size() - originalSize);
  }

  // Add new elements to the end of file
  for(auto element : renderListNew) {
    offset_t offset = length();
    ByteVector data = element->render();
    insert(data, offset, 0);
    newSegmentDataSize += data.size();
  }

  // Write the new segment data size
  if(newSegmentDataSize != d->segmentDataSize) {
    auto segmentDataSizeBuffer = EBML::renderVINT(newSegmentDataSize, d->segmentSizeLength);
    insert(segmentDataSizeBuffer, d->segmentSizeOffset, d->segmentSizeLength);
    d->segmentDataSize = newSegmentDataSize;
  }

/*
  auto&& renderElements [&d->segmentDataSize](Element *element) {
    auto offset = element->offset();
    if(!offset)
  }
  
  if(d->tag) {
    ByteVector tag = d->tag->render();
    auto tagsOriginalSize = d->tag->size();
    auto tagsOffset = d->tag->offset();
    
    if(!tagsOriginalSize) {
      tagsOffset = d->segmentSizeOffset + d->segmentSizeLength + d->segmentDataSize;
    }
    insert(tag, tagsOffset, tagsOriginalSize);
    d->segmentDataSize += (tag.size() - tagsOriginalSize);
    auto segmentDataSizeBuffer = EBML::renderVINT(d->segmentDataSize, d->segmentSizeLength);
    insert(segmentDataSizeBuffer, d->segmentSizeOffset, d->segmentSizeLength);
  }
  */
  /*
  if(d->attachments) {
    ByteVector attachments = d->attachments->render();
  }
  */

  return true;
}
