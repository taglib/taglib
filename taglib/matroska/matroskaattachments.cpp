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

#include "matroskaattachments.h"
#include "matroskaattachedfile.h"
#include "ebmlmkattachments.h"
#include "ebmlmasterelement.h"
#include "ebmlstringelement.h"
#include "ebmlbinaryelement.h"
#include "ebmluintelement.h"
#include "ebmlutils.h"
#include "tlist.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::Attachments::AttachmentsPrivate
{
public:
  AttachmentsPrivate() = default;
  ~AttachmentsPrivate() = default;
  AttachmentsPrivate(const AttachmentsPrivate &) = delete;
  AttachmentsPrivate &operator=(const AttachmentsPrivate &) = delete;
  AttachedFileList files;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::Attachments::Attachments() :
  Element(static_cast<ID>(EBML::Element::Id::MkAttachments)),
  d(std::make_unique<AttachmentsPrivate>())
{
}

Matroska::Attachments::~Attachments() = default;

void Matroska::Attachments::addAttachedFile(const AttachedFile &file)
{
  d->files.append(file);
  setNeedsRender(true);
}

void Matroska::Attachments::removeAttachedFile(unsigned long long uid)
{
  const auto it = std::find_if(d->files.begin(), d->files.end(),
    [uid](const AttachedFile &file) {
      return file.uid() == uid;
    });
  if(it != d->files.end()) {
    d->files.erase(it);
    setNeedsRender(true);
  }
}

void Matroska::Attachments::clear()
{
  d->files.clear();
  setNeedsRender(true);
}

const Matroska::Attachments::AttachedFileList &Matroska::Attachments::attachedFileList() const
{
  return d->files;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

Matroska::Attachments::AttachedFileList &Matroska::Attachments::attachedFiles()
{
  setNeedsRender(true);
  return d->files;
}

ByteVector Matroska::Attachments::renderInternal()
{
  if(d->files.isEmpty()) {
    // Avoid writing an Attachments element without AttachedFile element.
    return {};
  }

  EBML::MkAttachments attachments;
  for(const auto &attachedFile : std::as_const(d->files)) {
    auto attachedFileElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFile>();

    // Filename
    auto fileNameElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFileName>();
    fileNameElement->setValue(attachedFile.fileName());
    attachedFileElement->appendElement(std::move(fileNameElement));

    // Media/MIME type
    auto mediaTypeElement =
      EBML::make_unique_element<EBML::Element::Id::MkAttachedFileMediaType>();
    mediaTypeElement->setValue(attachedFile.mediaType());
    attachedFileElement->appendElement(std::move(mediaTypeElement));

    // Description
    if(const String &description = attachedFile.description(); !description.isEmpty()) {
      auto descriptionElement =
        EBML::make_unique_element<EBML::Element::Id::MkAttachedFileDescription>();
      descriptionElement->setValue(description);
      attachedFileElement->appendElement(std::move(descriptionElement));
    }

    // Data
    auto dataElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFileData>();
    dataElement->setValue(attachedFile.data());
    attachedFileElement->appendElement(std::move(dataElement));

    // UID
    auto uidElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFileUID>();
    AttachedFile::UID uid = attachedFile.uid();
    if(!uid)
      uid = EBML::randomUID();
    uidElement->setValue(uid);
    attachedFileElement->appendElement(std::move(uidElement));

    attachments.appendElement(std::move(attachedFileElement));
  }
  return attachments.render();
}
