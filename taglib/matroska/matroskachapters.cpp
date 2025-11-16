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

#include "matroskachapters.h"
#include "matroskachapteredition.h"
#include "ebmlstringelement.h"
#include "ebmlbinaryelement.h"
#include "ebmlmkchapters.h"
#include "ebmluintelement.h"
#include "ebmlutils.h"
#include "tlist.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::Chapters::ChaptersPrivate
{
public:
  ChaptersPrivate() = default;
  ~ChaptersPrivate() = default;
  ChaptersPrivate(const ChaptersPrivate &) = delete;
  ChaptersPrivate &operator=(const ChaptersPrivate &) = delete;
  ChapterEditionList editions;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::Chapters::Chapters() :
  Element(static_cast<ID>(EBML::Element::Id::MkChapters)),
  d(std::make_unique<ChaptersPrivate>())
{
}

Matroska::Chapters::~Chapters() = default;

void Matroska::Chapters::addChapterEdition(const ChapterEdition &edition)
{
  d->editions.append(edition);
  setNeedsRender(true);
}

void Matroska::Chapters::removeChapterEdition(unsigned long long uid)
{
  const auto it = std::find_if(d->editions.begin(), d->editions.end(),
    [uid](const ChapterEdition &file) {
      return file.uid() == uid;
    });
  if(it != d->editions.end()) {
    d->editions.erase(it);
    setNeedsRender(true);
  }
}

void Matroska::Chapters::clear()
{
  d->editions.clear();
  setNeedsRender(true);
}

const Matroska::Chapters::ChapterEditionList &Matroska::Chapters::chapterEditionList() const
{
  return d->editions;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////
ByteVector Matroska::Chapters::renderInternal()
{
  if(d->editions.isEmpty()) {
    // Avoid writing a Chapters element without ChapterEdition element.
    return {};
  }

  EBML::MkChapters chapters;
  for(const auto &chapterEdition : std::as_const(d->editions)) {
    auto chapterEditionElement = EBML::make_unique_element<EBML::Element::Id::MkEditionEntry>();

    if(const auto uid = chapterEdition.uid()) {
      auto uidElement = EBML::make_unique_element<EBML::Element::Id::MkEditionUID>();
      uidElement->setValue(uid);
      chapterEditionElement->appendElement(std::move(uidElement));
    }
    auto defaultElement = EBML::make_unique_element<EBML::Element::Id::MkEditionFlagDefault>();
    defaultElement->setValue(chapterEdition.isDefault());
    chapterEditionElement->appendElement(std::move(defaultElement));
    auto orderedElement = EBML::make_unique_element<EBML::Element::Id::MkEditionFlagOrdered>();
    orderedElement->setValue(chapterEdition.isOrdered());
    chapterEditionElement->appendElement(std::move(orderedElement));

    for(const auto &chapter : chapterEdition.chapterList()) {
      auto chapterElement = EBML::make_unique_element<EBML::Element::Id::MkChapterAtom>();

      auto cuidElement = EBML::make_unique_element<EBML::Element::Id::MkChapterUID>();
      const auto cuid = chapter.uid();
      cuidElement->setValue(cuid ? cuid : EBML::randomUID());
      chapterElement->appendElement(std::move(cuidElement));
      auto timeStartElement = EBML::make_unique_element<EBML::Element::Id::MkChapterTimeStart>();
      timeStartElement->setValue(chapter.timeStart());
      chapterElement->appendElement(std::move(timeStartElement));
      auto timeEndElement = EBML::make_unique_element<EBML::Element::Id::MkChapterTimeEnd>();
      timeEndElement->setValue(chapter.timeEnd());
      chapterElement->appendElement(std::move(timeEndElement));
      auto hiddenElement = EBML::make_unique_element<EBML::Element::Id::MkChapterFlagHidden>();
      hiddenElement->setValue(chapter.isHidden());
      chapterElement->appendElement(std::move(hiddenElement));

      for(const auto &display : chapter.displayList()) {
        auto displayElement = EBML::make_unique_element<EBML::Element::Id::MkChapterDisplay>();

        auto stringElement = EBML::make_unique_element<EBML::Element::Id::MkChapString>();
        stringElement->setValue(display.string());
        displayElement->appendElement(std::move(stringElement));
        auto languageElement = EBML::make_unique_element<EBML::Element::Id::MkChapLanguage>();
        languageElement->setValue(display.language());
        displayElement->appendElement(std::move(languageElement));

        chapterElement->appendElement(std::move(displayElement));
      }

      chapterEditionElement->appendElement(std::move(chapterElement));
    }

    chapters.appendElement(std::move(chapterEditionElement));
  }
  return chapters.render();
}
