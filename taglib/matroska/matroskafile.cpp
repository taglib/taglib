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
#include "matroskaseekhead.h"
#include "matroskasegment.h"
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
    delete seekHead;
  }

  FilePrivate(const FilePrivate &) = delete;
  FilePrivate &operator=(const FilePrivate &) = delete;
  Matroska::Tag *tag = nullptr;
  Attachments *attachments = nullptr;
  SeekHead *seekHead = nullptr;
  Segment *segment = nullptr;
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

  // Read the segment into memory from file
  if(!segment->read(*this)) {
    debug("Failed to read segment");
    setValid(false);
    return;
  }
  
  // Parse the elements
  d->segment = segment->parseSegment();
  d->seekHead = segment->parseSeekHead();
  d->tag = segment->parseTag();
  d->attachments = segment->parseAttachments();

  setValid(true);
}

bool Matroska::File::save()
{
  if(readOnly()) {
    debug("Matroska::File::save() -- File is read only.");
    return false;
  }
  if(!isValid()) {
    debug("Matroska::File::save() -- File is not valid.");
    return false;
  }

  List<Element*> renderList;
  List<Element*> newElements;

  // List of all possible elements we can write
  List<Element*> elements {
    d->attachments,
    d->tag
  };

 /* Build render list. New elements will be added
  * to the end of the file. For new elements,
  * the order is from least likely to change,
  * to most likely to change:
  *   1. Bookmarks (todo)
  *   2. Attachments
  *   3. Tags
  */
  for (auto element : elements) {
    if (!element)
      continue;
    if (element->size())
      renderList.append(element);
    else {
      element->setOffset(length());
      newElements.append(element);
    }
  }
  if (renderList.isEmpty())
    return true;

  auto sortAscending = [](const auto a, const auto b) { return a->offset() < b->offset(); };
  renderList.sort(sortAscending);
  renderList.append(newElements);

  // Add our new elements to the Seek Head (if the file has one)
  if (d->seekHead) {
    auto segmentDataOffset = d->segment->dataOffset();
    for (auto element : newElements)
      d->seekHead->addEntry(element->id(), element->offset() - segmentDataOffset);
    d->seekHead->sort();
  }

  // Set up listeners, add seek head and segment length to the end
  for(auto it = renderList.begin(); it != renderList.end(); ++it) {
    for (auto it2 = std::next(it); it2 != renderList.end(); ++it2)
      (*it)->addSizeListener(*it2);
    if (d->seekHead)
      (*it)->addSizeListener(d->seekHead);
    (*it)->addSizeListener(d->segment);
  }
  if(d->seekHead) {
    d->seekHead->addSizeListeners(renderList);
    renderList.append(d->seekHead);
  }
  d->segment->addSizeListeners(renderList);
  renderList.append(d->segment);

  // Render the elements
  for(auto element : renderList) {
    if (!element->render())
      return false;
  }

  // Write out to file
  renderList.sort(sortAscending);
  for(auto element : renderList)
    element->write(*this);

  return true;
}
