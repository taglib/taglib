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

#include "matroskaattachedfile.h"
#include "tstring.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::AttachedFile::AttachedFilePrivate
{
public:
  AttachedFilePrivate() = default;
  ~AttachedFilePrivate() = default;
  String fileName;
  String description;
  String mediaType;
  ByteVector data;
  UID uid = 0;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Matroska::AttachedFile::AttachedFile() :
  d(std::make_unique<AttachedFilePrivate>())
{
}

Matroska::AttachedFile::AttachedFile(const AttachedFile &other) :
  d(std::make_unique<AttachedFilePrivate>(*other.d))
{
}

Matroska::AttachedFile::AttachedFile(AttachedFile&& other) noexcept = default;

Matroska::AttachedFile::~AttachedFile() = default;

Matroska::AttachedFile &Matroska::AttachedFile::operator=(AttachedFile &&other) = default;

Matroska::AttachedFile &Matroska::AttachedFile::operator=(const AttachedFile &other)
{
  AttachedFile(other).swap(*this);
  return *this;
}

void Matroska::AttachedFile::swap(AttachedFile &other) noexcept
{
  using std::swap;

  swap(d, other.d);
}

void Matroska::AttachedFile::setFileName(const String &fileName)
{
  d->fileName = fileName;
}

const String &Matroska::AttachedFile::fileName() const
{
  return d->fileName;
}

void Matroska::AttachedFile::setDescription(const String &description)
{
  d->description = description;
}

const String &Matroska::AttachedFile::description() const
{
  return d->description;
}

void Matroska::AttachedFile::setMediaType(const String &mediaType)
{
  d->mediaType = mediaType;
}

const String &Matroska::AttachedFile::mediaType() const
{
  return d->mediaType;
}

void Matroska::AttachedFile::setData(const ByteVector &data)
{
  d->data = data;
}

const ByteVector &Matroska::AttachedFile::data() const
{
  return d->data;
}

void Matroska::AttachedFile::setUID(UID uid)
{
  d->uid = uid;
}

Matroska::AttachedFile::UID Matroska::AttachedFile::uid() const
{
  return d->uid;
}
