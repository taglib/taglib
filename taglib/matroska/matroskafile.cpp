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
#include "matroskaattachedfile.h"
#include "matroskachapter.h"
#include "matroskachapteredition.h"
#include "matroskachapters.h"
#include "matroskaseekhead.h"
#include "matroskacues.h"
#include "matroskasegment.h"
#include "ebmlutils.h"
#include "ebmlelement.h"
#include "ebmlstringelement.h"
#include "ebmluintelement.h"
#include "ebmlmksegment.h"
#include "tlist.h"
#include "tdebug.h"
#include "tagutils.h"
#include "tpropertymap.h"

#include <memory>

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
  return d->tag ? d->tag->properties() : PropertyMap();
}

void Matroska::File::removeUnsupportedProperties(const StringList &properties)
{
  if(d->tag) {
    d->tag->removeUnsupportedProperties(properties);
  }
}

PropertyMap Matroska::File::setProperties(const PropertyMap &properties)
{
  if(!d->tag) {
    d->tag = std::make_unique<Tag>();
  }
  return d->tag->setProperties(properties);
}

namespace {

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

}

StringList Matroska::File::complexPropertyKeys() const
{
  StringList keys = TagLib::File::complexPropertyKeys();
  if(d->attachments) {
    const auto &attachedFiles = d->attachments->attachedFileList();
    for(const auto &attachedFile : attachedFiles) {
      String key = keyForAttachedFile(attachedFile);
      if(!key.isEmpty() && !keys.contains(key)) {
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
  if(key.upper() == "CHAPTERS") {
    if(d->chapters) {
      for(const auto &edition : d->chapters->chapterEditionList()) {
        VariantMap property;
        if(auto uid = edition.uid()) {
          property.insert("uid", uid);
        }
        if(auto isDefault = edition.isDefault()) {
          property.insert("isDefault", isDefault);
        }
        if(auto isOrdered = edition.isOrdered()) {
          property.insert("isOrdered", isOrdered);
        }
        if(auto chapters = edition.chapterList(); !chapters.isEmpty()) {
          VariantList chaps;
          for(const auto &chapter : chapters) {
            VariantMap chap;
            if(auto uid = chapter.uid()) {
              chap.insert("uid", uid);
            }
            if(auto isHidden = chapter.isHidden()) {
              chap.insert("isHidden", isHidden);
            }
            chap.insert("timeStart", chapter.timeStart());
            if(auto timeEnd = chapter.timeEnd()) {
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
    unsigned long long uidKey;
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
    else if(!uid && ((uidKey = key.toULongLong(&ok))) && ok) {
      uid = uidKey;
    }
    if(fileName.isEmpty() && !mimeType.isEmpty()) {
      int slashPos = mimeType.rfind('/');
      String ext = mimeType.substr(slashPos + 1);
      if(ext == "jpeg") {
        ext = "jpg";
      }
      fileName = "attachment." + ext;
    }
    if(!mimeType.isEmpty() && !fileName.isEmpty()) {
      AttachedFile attachedFile;
      attachedFile.setData(data);
      attachedFile.setMediaType(mimeType);
      attachedFile.setDescription(property.value("description").value<String>());
      attachedFile.setFileName(fileName);
      attachedFile.setUID(uid);
      d->attachments->addAttachedFile(attachedFile);
    }
  }
  return true;
}

Matroska::Attachments *Matroska::File::attachments(bool create) const
{
  if(!d->attachments && create)
    d->attachments = std::make_unique<Attachments>();
  return d->attachments.get();
}

Matroska::Chapters *Matroska::File::chapters(bool create) const
{
  if(!d->chapters && create)
    d->chapters = std::make_unique<Chapters>();
  return d->chapters.get();
}

void Matroska::File::read(bool readProperties, Properties::ReadStyle readStyle)
{
  offset_t fileLength = length();

  // Find the EBML Header
  auto head = EBML::element_cast<EBML::Element::Id::EBMLHeader>(
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
  std::unique_ptr<EBML::MkSegment> segment(
    EBML::element_cast<EBML::Element::Id::MkSegment>(
      EBML::findElement(*this, EBML::Element::Id::MkSegment, fileLength - tell())
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
  d->cues = segment->parseCues();
  d->tag = segment->parseTag();
  d->attachments = segment->parseAttachments();
  d->chapters = segment->parseChapters();

  if(readProperties) {
    d->properties = std::make_unique<Properties>(this);

    for(const auto &element : *head) {
      auto id = element->getId();
      if (id == EBML::Element::Id::DocType) {
        d->properties->setDocType(
          EBML::element_cast<EBML::Element::Id::DocType>(element)->getValue());
      }
      else if (id == EBML::Element::Id::DocTypeVersion) {
        d->properties->setDocTypeVersion(static_cast<int>(
          EBML::element_cast<EBML::Element::Id::DocTypeVersion>(element)->getValue()));
      }
    }

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
    auto segmentDataOffset = d->segment->dataOffset();
    for(auto element : newElements)
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
    for(auto element : renderList) {
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
  for(auto element : renderList)
    element->write(*this);

  return true;
}
