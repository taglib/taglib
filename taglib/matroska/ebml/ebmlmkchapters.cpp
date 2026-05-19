/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
 ***************************************************************************/

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

#include "ebmlmkchapters.h"
#include "ebmlstringelement.h"
#include <atomic>
#include "ebmluintelement.h"
#include "matroskachapters.h"
#include "matroskachapteredition.h"

using namespace TagLib;

namespace {

// Counter for synthesising a unique ChapterUID when the source file omits
// the MkChapterUID element (out-of-spec but produced by some muxers, e.g.
// older FFmpeg or audiobook generators). MKVToolNix / MediaInfo / FFmpeg
// all tolerate UID-less chapters; without this synth, the call sites in
// MkChapters::parse() filter such chapters out via the `chapter.uid()`
// check and they are silently lost.
//
// High bit set as a marker; counter starts at 1 and increments per atom
// so each parsed UID-less chapter gets a distinct value within the
// process lifetime. Real ChapterUIDs are random 64-bit values from
// muxers; the (1ULL << 63) | n encoding makes collision practically
// impossible while keeping the distinction local to TagLib.
static std::atomic<Matroska::Chapter::UID> synthesisedChapterUidCounter{1};

Matroska::Chapter parseChapterAtom(
  const std::unique_ptr<EBML::Element> &atomElement)
{
  Matroska::Chapter::UID chapterUid = 0;
  Matroska::Chapter::Time chapterTimeStart = 0;
  Matroska::Chapter::Time chapterTimeEnd = 0;
  List<Matroska::Chapter::Display> chapterDisplays;
  bool chapterHidden = false;

  const auto chapterAtom = EBML::element_cast<EBML::Element::Id::MkChapterAtom>(atomElement);
  for(const auto &chapterChild : *chapterAtom) {
    if(const EBML::Element::Id cid = chapterChild->getId(); cid == EBML::Element::Id::MkChapterUID)
      chapterUid = EBML::element_cast<EBML::Element::Id::MkChapterUID>(chapterChild)->getValue();
    else if(cid == EBML::Element::Id::MkChapterTimeStart)
      chapterTimeStart = EBML::element_cast<EBML::Element::Id::MkChapterTimeStart>(chapterChild)->getValue();
    else if(cid == EBML::Element::Id::MkChapterTimeEnd)
      chapterTimeEnd = EBML::element_cast<EBML::Element::Id::MkChapterTimeEnd>(chapterChild)->getValue();
    else if(cid == EBML::Element::Id::MkChapterFlagHidden)
      chapterHidden = EBML::element_cast<EBML::Element::Id::MkChapterFlagHidden>(chapterChild)->getValue() != 0;
    else if (cid == EBML::Element::Id::MkChapterDisplay) {
      const auto display = EBML::element_cast<EBML::Element::Id::MkChapterDisplay>(chapterChild);
      String displayString;
      String displayLanguage;
      for(const auto &displayChild : *display) {
        if (const EBML::Element::Id did = displayChild->getId(); did == EBML::Element::Id::MkChapString)
          displayString = EBML::element_cast<EBML::Element::Id::MkChapString>(displayChild)->getValue();
        else if(did == EBML::Element::Id::MkChapLanguage)
          displayLanguage = EBML::element_cast<EBML::Element::Id::MkChapLanguage>(displayChild)->getValue();
      }
      if(!displayString.isEmpty()) {
        chapterDisplays.append(Matroska::Chapter::Display(displayString, displayLanguage));
      }
    }
  }
  // Spec-noncompliant: ChapterAtom missing ChapterUID. Synthesise a
  // process-unique UID so the chapter survives downstream `chapter.uid()`
  // filters. See synthesisedChapterUidCounter comment above.
  if(chapterUid == 0)
    chapterUid = (1ULL << 63) |
                 synthesisedChapterUidCounter.fetch_add(1, std::memory_order_relaxed);

  return Matroska::Chapter(chapterTimeStart, chapterTimeEnd, chapterDisplays,
    chapterUid, chapterHidden);
}

} // namespae

EBML::MkChapters::MkChapters(int sizeLength, offset_t dataSize, offset_t offset):
  MasterElement(Id::MkChapters, sizeLength, dataSize, offset)
{
}

EBML::MkChapters::MkChapters(Id, int sizeLength, offset_t dataSize, offset_t offset):
  MasterElement(Id::MkChapters, sizeLength, dataSize, offset)
{
}

EBML::MkChapters::MkChapters() :
  MasterElement(Id::MkChapters, 0, 0, 0)
{
}

std::unique_ptr<Matroska::Chapters> EBML::MkChapters::parse() const
{
  auto chapters = std::make_unique<Matroska::Chapters>();
  chapters->setOffset(offset);
  chapters->setSize(getSize() + padding);

  // Collect any orphan ChapterAtom elements not wrapped in an EditionEntry.
  // The Matroska spec requires ChapterAtom to be inside an EditionEntry, but
  // some muxers produce files with ChapterAtom directly under Chapters.
  // MKVToolNix and FFmpeg handle this case by treating the orphan atoms as
  // belonging to an implicit default edition.
  List<Matroska::Chapter> orphanChapters;

  for(const auto &element : elements) {
    if(element->getId() == Id::MkChapterAtom) {
      if(auto chapter = parseChapterAtom(element); chapter.uid()) {
        orphanChapters.append(chapter);
      }
      continue;
    }
    if(element->getId() != Id::MkEditionEntry)
      continue;

    List<Matroska::Chapter> editionChapters;
    Matroska::ChapterEdition::UID editionUid = 0;
    bool editionIsDefault = false;
    bool editionIsOrdered = false;
    const auto edition = element_cast<Id::MkEditionEntry>(element);
    for(const auto &editionChild : *edition) {
      if(const Id id = editionChild->getId(); id == Id::MkEditionUID)
        editionUid = element_cast<Id::MkEditionUID>(editionChild)->getValue();
      else if(id == Id::MkEditionFlagDefault)
        editionIsDefault = element_cast<Id::MkEditionFlagDefault>(editionChild)->getValue() != 0;
      else if(id == Id::MkEditionFlagOrdered)
        editionIsOrdered = element_cast<Id::MkEditionFlagOrdered>(editionChild)->getValue() != 0;
      else if(id == Id::MkChapterAtom) {
        if(auto chapter = parseChapterAtom(editionChild); chapter.uid()) {
          editionChapters.append(chapter);
        }
      }
    }
    if(!editionChapters.isEmpty()) {
      chapters->addChapterEdition(Matroska::ChapterEdition(
        editionChapters, editionIsDefault, editionIsOrdered, editionUid));
    }
  }

  // If orphan chapters were found, wrap them in an implicit default edition
  // so they are not silently lost.
  if (!orphanChapters.isEmpty()) {
    chapters->addChapterEdition(Matroska::ChapterEdition(
      orphanChapters, true, false, 0));
  }

  return chapters;
}
