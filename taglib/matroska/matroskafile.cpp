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
#include <memory>
#include "matroskatag.h"
#include "matroskaattachments.h"
#include "matroskaattachedfile.h"
#include "matroskachapter.h"
#include "matroskachapteredition.h"
#include "matroskachapters.h"
#include "matroskaseekhead.h"
#include "matroskacues.h"
#include "matroskasegment.h"
#include "ebmlutils.h"
#include "ebmlelement.h"
#include "ebmlmasterelement.h"
#include "ebmlstringelement.h"
#include "ebmluintelement.h"
#include "ebmlmkinfo.h"
#include "ebmlmkseekhead.h"
#include "ebmlmksegment.h"
#include "ebmlmktags.h"
#include "ebmlmktracks.h"
#include "tlist.h"
#include "tdebug.h"
#include "tagutils.h"
#include "tpropertymap.h"

using namespace TagLib;

class Matroska::File::FilePrivate
{
public:
  FilePrivate() = default;
  ~FilePrivate() = default;

  FilePrivate(const FilePrivate &) = delete;
  FilePrivate &operator=(const FilePrivate &) = delete;

  std::unique_ptr<Tag> tag;
  std::unique_ptr<Attachments> attachments;
  std::unique_ptr<Chapters> chapters;
  std::unique_ptr<SeekHead> seekHead;
  std::unique_ptr<Cues> cues;
  std::unique_ptr<Segment> segment;
  std::unique_ptr<Properties> properties;
  bool partialRead = false;
  bool tagStateResolved = true;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool Matroska::File::isSupported(IOStream *stream)
{
  const ByteVector id = Utils::readHeader(stream, 4, false);
  return id.startsWith("\x1A\x45\xDF\xA3");
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::File::File(FileName file, bool readProperties,
                     Properties::ReadStyle readStyle) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>())
{
  if(!isOpen()) {
    debug("Failed to open matroska file");
    setValid(false);
    return;
  }
  read(readProperties, readStyle);
}

Matroska::File::File(IOStream *stream, bool readProperties,
                     Properties::ReadStyle readStyle) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>())
{
  if(!isOpen()) {
    debug("Failed to open matroska file");
    setValid(false);
    return;
  }
  read(readProperties, readStyle);
}

Matroska::File::~File() = default;

Matroska::Properties *Matroska::File::audioProperties() const
{
  return d->properties.get();
}

Tag *Matroska::File::tag() const
{
  return tag(true);
}

Matroska::Tag *Matroska::File::tag(bool create) const
{
  if(!ensureTagResolved()) {
    return nullptr;
  }

  if(!d->tag && create) {
    d->tag = std::make_unique<Tag>();
    if(d->properties) {
      d->tag->setSegmentTitle(d->properties->title());
    }
  }
  return d->tag.get();
}

PropertyMap Matroska::File::properties() const
{
  if(!ensureTagResolved()) {
    return PropertyMap();
  }
  return d->tag ? d->tag->properties() : PropertyMap();
}

void Matroska::File::removeUnsupportedProperties(const StringList &properties)
{
  if(!ensureTagResolved()) {
    return;
  }
  if(d->tag) {
    d->tag->removeUnsupportedProperties(properties);
  }
}

PropertyMap Matroska::File::setProperties(const PropertyMap &properties)
{
  if(!ensureTagResolved()) {
    return properties;
  }
  if(!d->tag) {
    d->tag = std::make_unique<Tag>();
  }
  return d->tag->setProperties(properties);
}

namespace {

  constexpr offset_t FastScanLimit = static_cast<offset_t>(512 * 1024);

  struct FastSegmentReadResult
  {
    bool success = false;
    bool tagStateResolved = false;
  };

  String keyForAttachedFile(const Matroska::AttachedFile &attachedFile)
  {
    if(attachedFile.mediaType().startsWith("image/")) {
      return "PICTURE";
    }
    if(!attachedFile.fileName().isEmpty()) {
      return attachedFile.fileName();
    }
    if(!attachedFile.mediaType().isEmpty()) {
      return attachedFile.mediaType();
    }
    return String::fromULongLong(attachedFile.uid());
  }

  bool keyMatchesAttachedFile(const String &key, const Matroska::AttachedFile &attachedFile)
  {
    return !key.isEmpty() && (
      (key == "PICTURE" && attachedFile.mediaType().startsWith("image/")) ||
      key == attachedFile.fileName() ||
      key == attachedFile.mediaType() ||
      key == String::fromULongLong(attachedFile.uid())
    );
  }

  template <EBML::Element::Id Id, typename ElementType>
  std::unique_ptr<ElementType> readElementAt(Matroska::File &file,
                                             offset_t offset,
                                             offset_t maxOffset)
  {
    if(offset < 0 || offset >= maxOffset) {
      return nullptr;
    }

    file.seek(offset);
    auto element = EBML::Element::factory(file);
    if(!element || element->getId() != Id) {
      return nullptr;
    }

    auto typed = EBML::element_cast<Id>(std::move(element));
    if(!typed || !typed->read(file)) {
      return nullptr;
    }
    return typed;
  }

  FastSegmentReadResult readFastSegmentData(Matroska::File &file,
                                            const EBML::MkSegment &segment,
                                            bool readProperties,
                                            std::unique_ptr<Matroska::SeekHead> &seekHead,
                                            std::unique_ptr<Matroska::Tag> &tag,
                                            std::unique_ptr<Matroska::Properties> &properties)
  {
    const offset_t fileLength = file.length();
    const offset_t segmentDataOffset = segment.segmentDataOffset();
    if(segmentDataOffset < 0 || segmentDataOffset >= fileLength) {
      return {};
    }

    bool haveInfo = !readProperties;
    bool haveTracks = !readProperties;
    bool tagStateResolved = tag != nullptr;
    offset_t scanLimit = segmentDataOffset + FastScanLimit;
    if(scanLimit > fileLength) {
      scanLimit = fileLength;
    }

    file.seek(segmentDataOffset);
    while(file.tell() < scanLimit) {
      auto element = EBML::Element::factory(file);
      if(!element) {
        break;
      }

      switch(element->getId()) {
      case EBML::Element::Id::MkSeekHead: {
        auto mkSeekHead = EBML::element_cast<EBML::Element::Id::MkSeekHead>(std::move(element));
        if(!mkSeekHead || !mkSeekHead->read(file)) {
          return {};
        }
        seekHead = mkSeekHead->parse(segmentDataOffset);
        break;
      }
      case EBML::Element::Id::MkInfo:
        if(readProperties && properties && !haveInfo) {
          auto mkInfo = EBML::element_cast<EBML::Element::Id::MkInfo>(std::move(element));
          if(!mkInfo || !mkInfo->read(file)) {
            return {};
          }
          mkInfo->parse(properties.get());
          haveInfo = true;
        }
        else {
          element->skipData(file);
        }
        break;
      case EBML::Element::Id::MkTracks:
        if(readProperties && properties && !haveTracks) {
          auto mkTracks = EBML::element_cast<EBML::Element::Id::MkTracks>(std::move(element));
          if(!mkTracks || !mkTracks->read(file)) {
            return {};
          }
          mkTracks->parse(properties.get());
          haveTracks = true;
        }
        else {
          element->skipData(file);
        }
        break;
      case EBML::Element::Id::MkTags:
        if(!tag) {
          auto mkTags = EBML::element_cast<EBML::Element::Id::MkTags>(std::move(element));
          if(!mkTags || !mkTags->read(file)) {
            return {};
          }
          tag = mkTags->parse();
          tagStateResolved = true;
        }
        else {
          element->skipData(file);
        }
        break;
      default:
        element->skipData(file);
        break;
      }

      if(haveInfo && haveTracks && tagStateResolved) {
        return {true, true};
      }
    }

    bool hasTagsEntry = false;
    if(seekHead) {
      for(const auto &[idValue, relativeOffset] : seekHead->entryList()) {
        const auto id = static_cast<EBML::Element::Id>(idValue);
        const offset_t absoluteOffset = segmentDataOffset + relativeOffset;
        switch(id) {
        case EBML::Element::Id::MkInfo:
          if(readProperties && properties && !haveInfo) {
            auto mkInfo = readElementAt<EBML::Element::Id::MkInfo, EBML::MkInfo>(
              file, absoluteOffset, fileLength);
            if(!mkInfo) {
              return {};
            }
            mkInfo->parse(properties.get());
            haveInfo = true;
          }
          break;
        case EBML::Element::Id::MkTracks:
          if(readProperties && properties && !haveTracks) {
            auto mkTracks = readElementAt<EBML::Element::Id::MkTracks, EBML::MkTracks>(
              file, absoluteOffset, fileLength);
            if(!mkTracks) {
              return {};
            }
            mkTracks->parse(properties.get());
            haveTracks = true;
          }
          break;
        case EBML::Element::Id::MkTags:
          hasTagsEntry = true;
          if(!tag) {
            auto mkTags = readElementAt<EBML::Element::Id::MkTags, EBML::MkTags>(
              file, absoluteOffset, fileLength);
            if(!mkTags) {
              return {};
            }
            tag = mkTags->parse();
          }
          tagStateResolved = true;
          break;
        default:
          break;
        }

        if(haveInfo && haveTracks && tagStateResolved) {
          return {true, true};
        }
      }
    }

    if(seekHead && !hasTagsEntry && !tag) {
      tagStateResolved = true;
    }

    return {haveInfo && haveTracks, tagStateResolved};
  }

}

StringList Matroska::File::complexPropertyKeys() const
{
  if(d->partialRead && !ensureFullyParsed()) {
    return TagLib::File::complexPropertyKeys();
  }

  StringList keys = TagLib::File::complexPropertyKeys();
  if(d->attachments) {
    const auto &attachedFiles = d->attachments->attachedFileList();
    for(const auto &attachedFile : attachedFiles) {
      if(String key = keyForAttachedFile(attachedFile);
         !key.isEmpty() && !keys.contains(key)) {
        keys.append(key);
      }
    }
  }
  if(d->chapters && !d->chapters->chapterEditionList().isEmpty()) {
    keys.append("CHAPTERS");
  }
  return keys;
}

List<VariantMap> Matroska::File::complexProperties(const String &key) const
{
  List<VariantMap> props = TagLib::File::complexProperties(key);
  if(d->partialRead && !ensureFullyParsed()) {
    return props;
  }

  if(key.upper() == "CHAPTERS") {
    if(d->chapters) {
      for(const auto &edition : d->chapters->chapterEditionList()) {
        VariantMap property;
        if(const auto uid = edition.uid()) {
          property.insert("uid", uid);
        }
        if(const auto isDefault = edition.isDefault()) {
          property.insert("isDefault", isDefault);
        }
        if(const auto isOrdered = edition.isOrdered()) {
          property.insert("isOrdered", isOrdered);
        }
        if(auto chapters = edition.chapterList(); !chapters.isEmpty()) {
          VariantList chaps;
          for(const auto &chapter : chapters) {
            VariantMap chap;
            if(const auto uid = chapter.uid()) {
              chap.insert("uid", uid);
            }
            if(const auto isHidden = chapter.isHidden()) {
              chap.insert("isHidden", isHidden);
            }
            chap.insert("timeStart", chapter.timeStart());
            if(const auto timeEnd = chapter.timeEnd()) {
              chap.insert("timeEnd", timeEnd);
            }
            if(auto displays = chapter.displayList(); !displays.isEmpty()) {
              VariantList disps;
              for(const auto &display : displays) {
                VariantMap disp;
                if(auto str = display.string(); !str.isEmpty()) {
                  disp.insert("string", str);
                }
                if(auto language = display.language(); !language.isEmpty()) {
                  disp.insert("language", language);
                }
                disps.append(disp);
              }
              chap.insert("displays", disps);
            }
            chaps.append(chap);
          }
          property.insert("chapters", chaps);
        }
        props.append(property);
      }
    }
  }
  if(d->attachments) {
    const auto &attachedFiles = d->attachments->attachedFileList();
    for(const auto &attachedFile : attachedFiles) {
      if(keyMatchesAttachedFile(key, attachedFile)) {
        VariantMap property;
        property.insert("data", attachedFile.data());
        property.insert("mimeType", attachedFile.mediaType());
        property.insert("description", attachedFile.description());
        property.insert("fileName", attachedFile.fileName());
        property.insert("uid", attachedFile.uid());
        props.append(property);
      }
    }
  }
  return props;
}

bool Matroska::File::setComplexProperties(const String &key, const List<VariantMap> &value)
{
  if(d->partialRead && !ensureFullyParsed()) {
    return false;
  }

  if(TagLib::File::setComplexProperties(key, value)) {
    return true;
  }

  if(key.upper() == "CHAPTERS") {
    chapters(true)->clear();
    for(const auto &ed : value) {
      List<Chapter> editionChapters;
      const auto chaps = ed.value("chapters").toList();
      for(const auto &chapVar : chaps) {
        auto chap = chapVar.toMap();
        const auto disps = chap.value("displays").toList();
        List<Chapter::Display> chapterDisplays;
        for(const auto &dispVar : disps) {
          auto disp = dispVar.toMap();
          chapterDisplays.append(Chapter::Display(
            disp.value("string").toString(),
            disp.value("language").toString()));
        }
        editionChapters.append(Chapter(
          chap.value("timeStart").toULongLong(),
          chap.value("timeEnd").toULongLong(),
          chapterDisplays,
          chap.value("uid", 0ULL).toULongLong(),
          chap.value("isHidden", false).toBool()));
      }
      d->chapters->addChapterEdition(ChapterEdition(
        editionChapters,
        ed.value("isDefault", false).toBool(),
        ed.value("isOrdered", false).toBool(),
        ed.value("uid", 0ULL).toULongLong()));
    }
    return true;
  }

  List<AttachedFile> &files = attachments(true)->attachedFiles();
  for(auto it = files.begin(); it != files.end();) {
    if(keyMatchesAttachedFile(key, *it)) {
      it = files.erase(it);
    }
    else {
      ++it;
    }
  }

  for(const auto &property : value) {
    if(property.isEmpty())
      continue;
    auto mimeType = property.value("mimeType").value<String>();
    auto data = property.value("data").value<ByteVector>();
    auto fileName = property.value("fileName").value<String>();
    auto uid = property.value("uid").value<unsigned long long>();
    bool ok;
    if(key.upper() == "PICTURE" && !mimeType.startsWith("image/")) {
      mimeType = data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")
                ? "image/png" : "image/jpeg";
    }
    else if(mimeType.isEmpty() && key.find("/") != -1) {
      mimeType = key;
    }
    else if(fileName.isEmpty() && key.find(".") != -1) {
      fileName = key;
    }
    else if(unsigned long long uidKey;
            !uid && ((uidKey = key.toULongLong(&ok))) && ok) {
      uid = uidKey;
    }
    if(fileName.isEmpty() && !mimeType.isEmpty()) {
      const int slashPos = mimeType.rfind('/');
      String ext = mimeType.substr(slashPos + 1);
      if(ext == "jpeg") {
        ext = "jpg";
      }
      fileName = "attachment." + ext;
    }
    if(!mimeType.isEmpty() && !fileName.isEmpty()) {
      d->attachments->addAttachedFile(AttachedFile(
        data, fileName, mimeType, uid,
        property.value("description").value<String>()));
    }
  }
  return true;
}

Matroska::Attachments *Matroska::File::attachments(bool create) const
{
  if(d->partialRead && !ensureFullyParsed()) {
    return nullptr;
  }
  if(!d->attachments && create)
    d->attachments = std::make_unique<Attachments>();
  return d->attachments.get();
}

Matroska::Chapters *Matroska::File::chapters(bool create) const
{
  if(d->partialRead && !ensureFullyParsed()) {
    return nullptr;
  }
  if(!d->chapters && create)
    d->chapters = std::make_unique<Chapters>();
  return d->chapters.get();
}

bool Matroska::File::ensureFullyParsed() const
{
  if(!d->partialRead) {
    return true;
  }

  auto &file = *const_cast<Matroska::File *>(this);
  const offset_t fileLength = file.length();
  file.seek(0);

  const auto head = EBML::element_cast<EBML::Element::Id::EBMLHeader>(
    EBML::Element::factory(file));
  if(!head || head->getId() != EBML::Element::Id::EBMLHeader) {
    debug("Failed to find EBML head");
    file.setValid(false);
    return false;
  }
  head->skipData(file);

  const std::unique_ptr<EBML::MkSegment> segment(
    EBML::element_cast<EBML::Element::Id::MkSegment>(
      EBML::findElement(file, EBML::Element::Id::MkSegment, fileLength - file.tell())
    )
  );
  if(!segment) {
    debug("Failed to find Matroska segment");
    file.setValid(false);
    return false;
  }

  if(!segment->read(file)) {
    debug("Failed to read segment");
    file.setValid(false);
    return false;
  }

  auto existingTag = std::move(d->tag);

  d->segment = segment->parseSegment();
  d->seekHead = segment->parseSeekHead();
  d->cues = segment->parseCues();
  d->attachments = segment->parseAttachments();
  d->chapters = segment->parseChapters();

  if(existingTag) {
    if(d->properties) {
      existingTag->setSegmentTitle(d->properties->title());
    }
    d->tag = std::move(existingTag);
  }
  else {
    d->tag = segment->parseTag();
    if(d->tag && d->properties) {
      d->tag->setSegmentTitle(d->properties->title());
    }
  }

  d->partialRead = false;
  d->tagStateResolved = true;
  return true;
}

bool Matroska::File::ensureTagResolved() const
{
  if(d->tagStateResolved) {
    return true;
  }
  return ensureFullyParsed();
}

void Matroska::File::read(bool readProperties, Properties::ReadStyle readStyle)
{
  const offset_t fileLength = length();

  // Find the EBML Header
  const auto head = EBML::element_cast<EBML::Element::Id::EBMLHeader>(
    EBML::Element::factory(*this));
  if(!head || head->getId() != EBML::Element::Id::EBMLHeader) {
    debug("Failed to find EBML head");
    setValid(false);
    return;
  }
  if(readProperties) {
    head->read(*this);
  }
  else {
    head->skipData(*this);
  }

  // Find the Matroska segment in the file
  const std::unique_ptr<EBML::MkSegment> segment(
    EBML::element_cast<EBML::Element::Id::MkSegment>(
      EBML::findElement(*this, EBML::Element::Id::MkSegment, fileLength - tell())
    )
  );
  if(!segment) {
    debug("Failed to find Matroska segment");
    setValid(false);
    return;
  }

  d->segment = segment->parseSegment();

  if(readProperties) {
    d->properties = std::make_unique<Properties>(this);

    for(const auto &element : *head) {
      if(const auto id = element->getId(); id == EBML::Element::Id::DocType) {
        d->properties->setDocType(
          EBML::element_cast<EBML::Element::Id::DocType>(element)->getValue());
      }
      else if(id == EBML::Element::Id::DocTypeVersion) {
        d->properties->setDocTypeVersion(static_cast<int>(
          EBML::element_cast<EBML::Element::Id::DocTypeVersion>(element)->getValue()));
      }
    }
  }

  if(readStyle == AudioProperties::ReadStyle::Fast) {
    const auto fastRead = readFastSegmentData(*this, *segment, readProperties,
                                              d->seekHead, d->tag, d->properties);
    if(fastRead.success) {
      if(d->tag && d->properties) {
        d->tag->setSegmentTitle(d->properties->title());
      }
      d->partialRead = true;
      d->tagStateResolved = fastRead.tagStateResolved;
      setValid(true);
      return;
    }

    d->seekHead.reset();
    d->cues.reset();
    d->tag.reset();
    d->attachments.reset();
    d->chapters.reset();
    d->partialRead = false;
    d->tagStateResolved = true;
    if(readProperties) {
      d->properties = std::make_unique<Properties>(this);
      for(const auto &element : *head) {
        if(const auto id = element->getId(); id == EBML::Element::Id::DocType) {
          d->properties->setDocType(
            EBML::element_cast<EBML::Element::Id::DocType>(element)->getValue());
        }
        else if(id == EBML::Element::Id::DocTypeVersion) {
          d->properties->setDocTypeVersion(static_cast<int>(
            EBML::element_cast<EBML::Element::Id::DocTypeVersion>(element)->getValue()));
        }
      }
    }
    else {
      d->properties.reset();
    }

    seek(segment->segmentDataOffset());
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
  d->cues = segment->parseCues();
  d->tag = segment->parseTag();
  d->attachments = segment->parseAttachments();
  d->chapters = segment->parseChapters();
  d->partialRead = false;
  d->tagStateResolved = true;

  if(readProperties) {
    segment->parseInfo(d->properties.get());
    segment->parseTracks(d->properties.get());
    if(d->tag) {
      d->tag->setSegmentTitle(d->properties->title());
    }
  }

  if(readStyle == AudioProperties::Accurate &&
     ((d->seekHead && !d->seekHead->isValid(*this)) ||
      (d->cues && !d->cues->isValid(*this)))) {
    setValid(false);
    return;
  }
  setValid(true);
}

bool Matroska::File::save()
{
  if(d->partialRead && !ensureFullyParsed()) {
    return false;
  }
  if(readOnly()) {
    debug("Matroska::File::save() -- File is read only.");
    return false;
  }
  if(!isValid()) {
    debug("Matroska::File::save() -- File is not valid.");
    return false;
  }

  // Do not create new attachments, chapters or tags and corresponding
  // seek head entries if only empty objects were created.
  if(d->chapters && d->chapters->chapterEditionList().isEmpty() &&
     d->chapters->size() == 0 && d->chapters->offset() == 0 &&
     d->chapters->data().isEmpty()) {
    d->chapters.reset();
  }
  if(d->attachments && d->attachments->attachedFileList().isEmpty() &&
     d->attachments->size() == 0 && d->attachments->offset() == 0 &&
     d->attachments->data().isEmpty()) {
    d->attachments.reset();
  }
  if(d->tag && d->tag->isEmpty() &&
     d->tag->size() == 0 && d->tag->offset() == 0 &&
     d->tag->data().isEmpty()) {
    d->tag.reset();
  }

  List<Element *> renderList;
  List<Element *> newElements;

  // List of all possible elements we can write
  List<Element *> elements {
    d->chapters.get(),
    d->attachments.get(),
    d->tag.get()
  };

  /* Build render list. New elements will be added
   * to the end of the file. For new elements,
   * the order is from least likely to change,
   * to most likely to change:
   *   1. Chapters
   *   2. Attachments
   *   3. Tags
   */
  for(auto element : elements) {
    if(!element)
      continue;
    if(element->size())
      renderList.append(element);
    else {
      element->setOffset(length());
      newElements.append(element);
    }
  }
  if(renderList.isEmpty() && newElements.isEmpty())
    return true;

  auto sortAscending = [](const auto a, const auto b) { return a->offset() < b->offset(); };
  renderList.sort(sortAscending);
  renderList.append(newElements);

  // Add our new elements to the Seek Head (if the file has one)
  if(d->seekHead) {
    const auto segmentDataOffset = d->segment->dataOffset();
    for(const auto element : newElements)
      d->seekHead->addEntry(element->id(), element->offset() - segmentDataOffset);
    d->seekHead->sort();
  }

  // Set up listeners, add seek head and segment length to the end
  for(auto it = renderList.begin(); it != renderList.end(); ++it) {
    for(auto it2 = std::next(it); it2 != renderList.end(); ++it2)
      (*it)->addSizeListener(*it2);
    if(d->cues)
      (*it)->addSizeListener(d->cues.get());
    if(d->seekHead)
      (*it)->addSizeListener(d->seekHead.get());
    (*it)->addSizeListener(d->segment.get());
  }
  if(d->cues) {
    renderList.append(d->cues.get());
    d->cues->addSizeListeners(renderList);
    if(d->seekHead) {
      d->cues->addSizeListener(d->seekHead.get());
    }
    d->cues->addSizeListener(d->segment.get());
  }
  if(d->seekHead) {
    renderList.append(d->seekHead.get());
    d->seekHead->addSizeListeners(renderList);
    d->seekHead->addSizeListener(d->segment.get());
  }
  d->segment->addSizeListeners(renderList);
  renderList.append(d->segment.get());

  // Render the elements.
  // Because size changes of elements can cause segment offset updates and
  // size changes in other elements, we might need multiple rounds until no more
  // element needs rendering.
  int renderRound = 0;
  bool rendering = true;
  while(rendering && renderRound < 5) {
    rendering = false;
    for(const auto element : renderList) {
      if(element->needsRender()) {
        rendering = true;
        if(!element->render()) {
          return false;
        }
      }
    }
    ++renderRound;
  }

  // Write out to file
  renderList.sort(sortAscending);
  for(const auto element : renderList)
    element->write(*this);

  return true;
}
