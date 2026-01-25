/**************************************************************************
    copyright            : (C) 2026 by Antoine Colombier
    email                : antoine@mixxx.org
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

#include "mp4stem.h"

using namespace TagLib;

MP4::Stem::Stem(const ByteVector &data) :
  d(std::make_shared<StemPrivate>())
{
  d->data = data;
}

MP4::Stem::Stem() = default;
MP4::Stem::Stem(const Stem &) = default;
MP4::Stem &MP4::Stem::operator=(const Stem &) = default;

void
MP4::Stem::swap(Stem &item) noexcept
{
  using std::swap;

  swap(d, item.d);
}

MP4::Stem::~Stem() = default;

ByteVector
MP4::Stem::data() const
{
  return d->data;
}

bool MP4::Stem::operator==(const Stem &other) const
{
  return data() == other.data();
}

bool MP4::Stem::operator!=(const Stem &other) const
{
  return !(*this == other);
}
