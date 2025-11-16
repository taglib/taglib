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

#include "matroskachapter.h"
#include "matroskachapteredition.h"
#include "tstring.h"
#include "tbytevector.h"
#include "tlist.h"

using namespace TagLib;

class Matroska::ChapterEdition::ChapterEditionPrivate
{
public:
  ChapterEditionPrivate() = default;
  ~ChapterEditionPrivate() = default;
  List<Chapter> chapters;
  UID uid = 0;
  bool flagDefault = false;
  bool flagOrdered = false;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::ChapterEdition::ChapterEdition(const List<Chapter> &chapterList,
        bool isDefault, bool isOrdered, UID uid) :
  d(std::make_unique<ChapterEditionPrivate>())
{
  d->chapters = chapterList;
  d->uid = uid;
  d->flagDefault = isDefault;
  d->flagOrdered = isOrdered;
}

Matroska::ChapterEdition::ChapterEdition(const ChapterEdition &other) :
  d(std::make_unique<ChapterEditionPrivate>(*other.d))
{
}

Matroska::ChapterEdition::ChapterEdition(ChapterEdition &&other) noexcept = default;

Matroska::ChapterEdition::~ChapterEdition() = default;

Matroska::ChapterEdition &Matroska::ChapterEdition::operator=(
  ChapterEdition &&other) noexcept = default;

Matroska::ChapterEdition &Matroska::ChapterEdition::operator=(const ChapterEdition &other)
{
  ChapterEdition(other).swap(*this);
  return *this;
}

void Matroska::ChapterEdition::swap(ChapterEdition &other) noexcept
{
  using std::swap;

  swap(d, other.d);
}

Matroska::ChapterEdition::UID Matroska::ChapterEdition::uid() const
{
  return d->uid;
}

bool Matroska::ChapterEdition::isDefault() const
{
  return d->flagDefault;
}

bool Matroska::ChapterEdition::isOrdered() const
{
  return d->flagOrdered;
}

const List<Matroska::Chapter> &Matroska::ChapterEdition::chapterList() const
{
  return d->chapters;
}
