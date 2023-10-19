#include <memory>
#include "matroskaattachments.h"
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
  AttachmentsPrivate() {}
  ~AttachmentsPrivate() = default;
  AttachmentsPrivate(const AttachmentsPrivate &) = delete;
  AttachmentsPrivate &operator=(const AttachmentsPrivate &) = delete;
  List<AttachedFile*> files;

};

Matroska::Attachments::Attachments()
: d(std::make_unique<AttachmentsPrivate>())
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

const Matroska::Attachments::AttachedFileList& Matroska::Attachments::attachedFileList() const
{
  return d->files;
}

ByteVector Matroska::Attachments::render()
{
  EBML::MkAttachments attachments;
  for(const auto attachedFile : d->files) {
    auto attachedFileElement = new EBML::MasterElement(EBML::ElementIDs::MkAttachedFile);

    // Filename
    auto fileNameElement = new EBML::UTF8StringElement(EBML::ElementIDs::MkAttachedFileName);
    fileNameElement->setValue(attachedFile->fileName());
    attachedFileElement->appendElement(fileNameElement);

    // Media/MIME type
    auto mediaTypeElement = new EBML::Latin1StringElement(EBML::ElementIDs::MkAttachedFileMediaType);
    mediaTypeElement->setValue(attachedFile->mediaType());
    attachedFileElement->appendElement(mediaTypeElement);

    // Description
    const String &description = attachedFile->description();
    if(!description.isEmpty()) {
      auto descriptionElement = new EBML::UTF8StringElement(EBML::ElementIDs::MkAttachedFileDescription);
      descriptionElement->setValue(description);
      attachedFileElement->appendElement(descriptionElement);
    }

    // Data
    auto dataElement = new EBML::BinaryElement(EBML::ElementIDs::MkAttachedFileData);
    dataElement->setValue(attachedFile->data());
    attachedFileElement->appendElement(dataElement);

    // UID
    auto uidElement = new EBML::UIntElement(EBML::ElementIDs::MkAttachedFileUID);
    AttachedFile::UID uid = attachedFile->uid();
    if(!uid)
      uid = EBML::randomUID();
    uidElement->setValue(uid);
    attachedFileElement->appendElement(uidElement);

    attachments.appendElement(attachedFileElement);
  }

  return attachments.render();
}
