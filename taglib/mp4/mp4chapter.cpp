/**************************************************************************
    copyright            : (C) 2026 by Ryan Francesconi
 **************************************************************************/

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

#include "mp4chapter.h"
#include "tstring.h"

using namespace TagLib;

class MP4::Chapter::ChapterPrivate
{
public:
  ChapterPrivate() = default;
  ~ChapterPrivate() = default;
  String title;
  long long startTime {0};
};

MP4::Chapter::Chapter(const String &title, long long startTime) :
  d(std::make_unique<ChapterPrivate>())
{
  d->title = title;
  d->startTime = startTime;
}

MP4::Chapter::Chapter(const Chapter &other) :
  d(std::make_unique<ChapterPrivate>(*other.d))
{
}

MP4::Chapter::Chapter(Chapter &&other) noexcept = default;

MP4::Chapter::Chapter::~Chapter() = default;

MP4::Chapter &MP4::Chapter::Chapter::operator=(const Chapter &other)
{
  Chapter(other).swap(*this);
  return *this;
}

MP4::Chapter &MP4::Chapter::Chapter::operator=(
  Chapter &&other) noexcept = default;

bool MP4::Chapter::operator==(const Chapter &other) const
{
  return title() == other.title() && startTime() == other.startTime();
}

bool MP4::Chapter::operator!=(const Chapter &other) const
{
  return !(*this == other);
}

void MP4::Chapter::swap(Chapter &other) noexcept
{
  using std::swap;

  swap(d, other.d);
}

const String &MP4::Chapter::title() const
{
  return d->title;
}

long long MP4::Chapter::startTime() const
{
  return d->startTime;
}
