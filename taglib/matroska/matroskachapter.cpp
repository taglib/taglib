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
#include "tstring.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::Chapter::Display::DisplayPrivate
{
public:
  DisplayPrivate() = default;
  ~DisplayPrivate() = default;
  String string;
  String language;
};

class Matroska::Chapter::ChapterPrivate
{
public:
  ChapterPrivate() = default;
  ~ChapterPrivate() = default;
  UID uid = 0;
  Time timeStart = 0;
  Time timeEnd = 0;
  List<Display> displayList;
  bool hidden = false;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::Chapter::Chapter(Time timeStart, Time timeEnd,
  const List<Display> &displayList, UID uid, bool hidden) :
  d(std::make_unique<ChapterPrivate>())
{
  d->uid = uid;
  d->timeStart = timeStart;
  d->timeEnd = timeEnd;
  d->displayList = displayList;
  d->hidden = hidden;
}

Matroska::Chapter::Chapter(const Chapter &other) :
  d(std::make_unique<ChapterPrivate>(*other.d))
{
}

Matroska::Chapter::Chapter(Chapter &&other) noexcept = default;

Matroska::Chapter::~Chapter() = default;

Matroska::Chapter &Matroska::Chapter::operator=(Chapter &&other) noexcept = default;

Matroska::Chapter &Matroska::Chapter::operator=(const Chapter &other)
{
  Chapter(other).swap(*this);
  return *this;
}

void Matroska::Chapter::swap(Chapter &other) noexcept
{
  using std::swap;

  swap(d, other.d);
}

Matroska::Chapter::UID Matroska::Chapter::uid() const
{
  return d->uid;
}

Matroska::Chapter::Time Matroska::Chapter::timeStart() const
{
  return d->timeStart;
}

Matroska::Chapter::Time Matroska::Chapter::timeEnd() const
{
  return d->timeEnd;
}

bool Matroska::Chapter::isHidden() const
{
  return d->hidden;
}

const List<Matroska::Chapter::Display> &Matroska::Chapter::displayList() const
{
  return d->displayList;
}

Matroska::Chapter::Display::Display(const String &string, const String &language) :
  d(std::make_unique<DisplayPrivate>())
{
  d->string = string;
  d->language = language;
}

Matroska::Chapter::Display::Display(const Display &other) :
  d(std::make_unique<DisplayPrivate>(*other.d))
{
}

Matroska::Chapter::Display::Display(Display &&other) noexcept = default;

Matroska::Chapter::Display::~Display() = default;

Matroska::Chapter::Display &Matroska::Chapter::Display::operator=(const Display &other)
{
  Display(other).swap(*this);
  return *this;
}

Matroska::Chapter::Display &Matroska::Chapter::Display::operator=(
  Display &&other) noexcept = default;

void Matroska::Chapter::Display::swap(Display &other) noexcept
{
  using std::swap;

  swap(d, other.d);
}

const String &Matroska::Chapter::Display::string() const
{
  return d->string;
}

const String &Matroska::Chapter::Display::language() const
{
  return d->language;
}
