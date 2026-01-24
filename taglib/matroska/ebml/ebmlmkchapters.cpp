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
#include "ebmluintelement.h"
#include "matroskachapters.h"
#include "matroskachapteredition.h"

using namespace TagLib;

EBML::MkChapters::MkChapters(int sizeLength, offset_t dataSize, offset_t offset):
  MasterElement(Id::MkChapters, sizeLength, dataSize, offset)
{
}

EBML::MkChapters::MkChapters(Id, int sizeLength, offset_t dataSize, offset_t offset):
  MasterElement(Id::MkChapters, sizeLength, dataSize, offset)
{
}

EBML::MkChapters::MkChapters():
  MasterElement(Id::MkChapters, 0, 0, 0)
{
}

std::unique_ptr<Matroska::Chapters> EBML::MkChapters::parse() const
{
  auto chapters = std::make_unique<Matroska::Chapters>();
  chapters->setOffset(offset);
  chapters->setSize(getSize());

  for(const auto &element : elements) {
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
        Matroska::Chapter::UID chapterUid = 0;
        Matroska::Chapter::Time chapterTimeStart = 0;
        Matroska::Chapter::Time chapterTimeEnd = 0;
        List<Matroska::Chapter::Display> chapterDisplays;
        bool chapterHidden = false;
        const auto chapterAtom = element_cast<Id::MkChapterAtom>(editionChild);
        for(const auto &chapterChild : *chapterAtom) {
          if(const Id cid = chapterChild->getId(); cid == Id::MkChapterUID)
            chapterUid = element_cast<Id::MkChapterUID>(chapterChild)->getValue();
          else if(cid == Id::MkChapterTimeStart)
            chapterTimeStart = element_cast<Id::MkChapterTimeStart>(chapterChild)->getValue();
          else if(cid == Id::MkChapterTimeEnd)
            chapterTimeEnd = element_cast<Id::MkChapterTimeEnd>(chapterChild)->getValue();
          else if(cid == Id::MkChapterFlagHidden)
            chapterHidden = element_cast<Id::MkChapterFlagHidden>(chapterChild)->getValue() != 0;
          else if(cid == Id::MkChapterDisplay) {
            const auto display = element_cast<Id::MkChapterDisplay>(chapterChild);
            String displayString;
            String displayLanguage;
            for(const auto &displayChild : *display) {
              if(const Id did = displayChild->getId(); did == Id::MkChapString)
                displayString = element_cast<Id::MkChapString>(displayChild)->getValue();
              else if(did == Id::MkChapLanguage)
                displayLanguage = element_cast<Id::MkChapLanguage>(displayChild)->getValue();
            }
            if(!displayString.isEmpty()) {
              chapterDisplays.append(Matroska::Chapter::Display(displayString, displayLanguage));
            }
          }
        }
        if(chapterUid) {
          editionChapters.append(Matroska::Chapter(
            chapterTimeStart, chapterTimeEnd, chapterDisplays, chapterUid, chapterHidden));
        }
      }
    }
    if(!editionChapters.isEmpty()) {
      chapters->addChapterEdition(Matroska::ChapterEdition(
        editionChapters, editionIsDefault, editionIsOrdered, editionUid));
    }
  }
  return chapters;
}
