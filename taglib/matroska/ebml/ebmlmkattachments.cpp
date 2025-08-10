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
#include "matroskaattachments.h"
#include "matroskaattachedfile.h"

using namespace TagLib;

Matroska::Attachments *EBML::MkAttachments::parse()
{
  auto attachments = new Matroska::Attachments();
  attachments->setOffset(offset);
  attachments->setSize(getSize());

  for(auto element : elements) {
    if(element->getId() != ElementIDs::MkAttachedFile)
      continue;

    const String *filename = nullptr;
    const String *description = nullptr;
    const String *mediaType = nullptr;
    const ByteVector *data = nullptr;
    Matroska::AttachedFile::UID uid = 0;
    auto attachedFile = static_cast<MasterElement *>(element);
    for(auto attachedFileChild : *attachedFile) {
      Id id = attachedFileChild->getId();
      if(id == ElementIDs::MkAttachedFileName)
        filename = &(static_cast<UTF8StringElement *>(attachedFileChild)->getValue());
      else if(id == ElementIDs::MkAttachedFileData)
        data = &(static_cast<BinaryElement *>(attachedFileChild)->getValue());
      else if(id == ElementIDs::MkAttachedFileDescription)
        description = &(static_cast<UTF8StringElement *>(attachedFileChild)->getValue());
      else if(id == ElementIDs::MkAttachedFileMediaType)
        mediaType = &(static_cast<Latin1StringElement *>(attachedFileChild)->getValue());
      else if(id == ElementIDs::MkAttachedFileUID)
        uid = static_cast<UIntElement *>(attachedFileChild)->getValue();
    }
    if(!(filename && data))
      continue;

    auto file = new Matroska::AttachedFile();
    file->setFileName(*filename);
    file->setData(*data);
    if(description)
      file->setDescription(*description);
    if(mediaType)
      file->setMediaType(*mediaType);
    if(uid)
      file->setUID(uid);

    attachments->addAttachedFile(file);
  }
  return attachments;
}
