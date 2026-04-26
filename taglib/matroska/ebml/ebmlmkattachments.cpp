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

#include "ebmlmkattachments.h"
#include "ebmlstringelement.h"
#include "ebmluintelement.h"
#include "ebmlbinaryelement.h"
#include "ebmlutils.h"
#include "tfile.h"
#include "matroskaattachments.h"
#include "matroskaattachedfile.h"

using namespace TagLib;

EBML::MkAttachments::MkAttachments(int sizeLength, offset_t dataSize, offset_t offset) :
  MasterElement(Id::MkAttachments, sizeLength, dataSize, offset)
{
}

EBML::MkAttachments::MkAttachments(Id, int sizeLength, offset_t dataSize, offset_t offset) :
  MasterElement(Id::MkAttachments, sizeLength, dataSize, offset)
{
}

EBML::MkAttachments::MkAttachments() :
  MasterElement(Id::MkAttachments, 0, 0, 0)
{
}

bool EBML::MkAttachments::readMetadataOnly(File& file)
{
  const offset_t maxOffset = file.tell() + dataSize;
  std::unique_ptr<Element> element;
  while ((element = findNextElement(file, maxOffset))) {
    if (element->getId() != Id::MkAttachedFile) {
      element->skipData(file);
      continue;
    }
    // Manually iterate MkAttachedFile children so we can skip the binary
    // MkAttachedFileData payload, which can be very large (cover art, fonts).
    auto attachedFile = std::make_unique<MasterElement>(Id::MkAttachedFile);
    const offset_t childMaxOffset = file.tell() + element->getDataSize();
    std::unique_ptr<Element> child;
    while ((child = findNextElement(file, childMaxOffset))) {
      if (child->getId() == Id::MkAttachedFileData) {
        // Record size but skip the actual data read
        child->skipData(file);
      }
      else if (!child->read(file)) {
        return false;
      }
      attachedFile->appendElement(std::move(child));
    }
    elements.push_back(std::move(attachedFile));
  }
  return file.tell() == maxOffset;
}

std::unique_ptr<Matroska::Attachments> EBML::MkAttachments::parse() const
{
  auto attachments = std::make_unique<Matroska::Attachments>();
  attachments->setOffset(offset);
  attachments->setSize(getSize());

  for (const auto &element : elements) {
    if (element->getId() != Id::MkAttachedFile)
      continue;

    const String* filename = nullptr;
    const String* description = nullptr;
    const String* mediaType = nullptr;
    const ByteVector* data = nullptr;
    Matroska::AttachedFile::UID uid = 0;
    static const ByteVector emptyData;
    const auto attachedFile = element_cast<Id::MkAttachedFile>(element);
    for (const auto& attachedFileChild : *attachedFile) {
      if (const Id id = attachedFileChild->getId(); id == Id::MkAttachedFileName)
        filename = &element_cast<Id::MkAttachedFileName>(attachedFileChild)->getValue();
      else if (id == Id::MkAttachedFileData)
        data = &element_cast<Id::MkAttachedFileData>(attachedFileChild)->getValue();
      else if (id == Id::MkAttachedFileDescription)
        description = &element_cast<Id::MkAttachedFileDescription>(attachedFileChild)->getValue();
      else if (id == Id::MkAttachedFileMediaType)
        mediaType = &element_cast<Id::MkAttachedFileMediaType>(attachedFileChild)->getValue();
      else if (id == Id::MkAttachedFileUID)
        uid = element_cast<Id::MkAttachedFileUID>(attachedFileChild)->getValue();
    }
    // In Fast mode, MkAttachedFileData has been skipped — emit the
    // attachment with an empty data ByteVector so callers still see its
    // metadata (filename, media type, UID, description).
    if (!filename)
      continue;

    attachments->addAttachedFile(Matroska::AttachedFile(
      data ? *data : emptyData, *filename, mediaType ? *mediaType : String(),
      uid, description ? *description : String()));
  }
  return attachments;
}
