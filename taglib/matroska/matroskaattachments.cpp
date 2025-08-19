#include "matroskaattachments.h"
#include <memory>
#include "matroskaattachedfile.h"
#include "ebmlmkattachments.h"
#include "ebmlmasterelement.h"
#include "ebmlstringelement.h"
#include "ebmlbinaryelement.h"
#include "ebmluintelement.h"
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
  List<AttachedFile *> files;
};

Matroska::Attachments::Attachments() :
  Element(static_cast<ID>(EBML::Element::Id::MkAttachments)),
  d(std::make_unique<AttachmentsPrivate>())
{
  d->files.setAutoDelete(true);
}
Matroska::Attachments::~Attachments() = default;

void Matroska::Attachments::addAttachedFile(AttachedFile *file)
{
  d->files.append(file);
}

void Matroska::Attachments::removeAttachedFile(AttachedFile *file)
{
  auto it = d->files.find(file);
  if(it != d->files.end()) {
    delete *it;
    d->files.erase(it);
  }
}

void Matroska::Attachments::clear()
{
  d->files.clear();
}

const Matroska::Attachments::AttachedFileList &Matroska::Attachments::attachedFileList() const
{
  return d->files;
}

bool Matroska::Attachments::render()
{
  EBML::MkAttachments attachments;
  for(const auto attachedFile : d->files) {
    auto attachedFileElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFile>();

    // Filename
    auto fileNameElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFileName>();
    fileNameElement->setValue(attachedFile->fileName());
    attachedFileElement->appendElement(std::move(fileNameElement));

    // Media/MIME type
    auto mediaTypeElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFileMediaType>();
    mediaTypeElement->setValue(attachedFile->mediaType());
    attachedFileElement->appendElement(std::move(mediaTypeElement));

    // Description
    const String &description = attachedFile->description();
    if(!description.isEmpty()) {
      auto descriptionElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFileDescription>();
      descriptionElement->setValue(description);
      attachedFileElement->appendElement(std::move(descriptionElement));
    }

    // Data
    auto dataElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFileData>();
    dataElement->setValue(attachedFile->data());
    attachedFileElement->appendElement(std::move(dataElement));

    // UID
    auto uidElement = EBML::make_unique_element<EBML::Element::Id::MkAttachedFileUID>();
    AttachedFile::UID uid = attachedFile->uid();
    if(!uid)
      uid = EBML::randomUID();
    uidElement->setValue(uid);
    attachedFileElement->appendElement(std::move(uidElement));

    attachments.appendElement(std::move(attachedFileElement));
  }

  auto beforeSize = size();
  auto data = attachments.render();
  auto afterSize = data.size();
  if(beforeSize != afterSize) {
    if(!emitSizeChanged(afterSize - beforeSize))
      return false;
  }
  setData(data);
  return true;
}
