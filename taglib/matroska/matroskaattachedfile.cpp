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
#include "tbytevector.h"

using namespace TagLib;

class Matroska::AttachedFile::AttachedFilePrivate
{
public:
  AttachedFilePrivate(const ByteVector &data, const String &fileName,
    const String &mediaType, UID uid, const String &description) :
    fileName(fileName), description(description), mediaType(mediaType),
    data(data), uid(uid) {}
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

Matroska::AttachedFile::AttachedFile(const ByteVector &data,
  const String &fileName, const String &mediaType, UID uid,
  const String &description) :
  d(std::make_unique<AttachedFilePrivate>(data, fileName, mediaType, uid, description))
{
}

Matroska::AttachedFile::AttachedFile(const AttachedFile &other) :
  d(std::make_unique<AttachedFilePrivate>(*other.d))
{
}

Matroska::AttachedFile::AttachedFile(AttachedFile &&other) noexcept = default;

Matroska::AttachedFile::~AttachedFile() = default;

Matroska::AttachedFile &Matroska::AttachedFile::operator=(AttachedFile &&other) noexcept = default;

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

const String &Matroska::AttachedFile::fileName() const
{
  return d->fileName;
}

const String &Matroska::AttachedFile::description() const
{
  return d->description;
}

const String &Matroska::AttachedFile::mediaType() const
{
  return d->mediaType;
}

const ByteVector &Matroska::AttachedFile::data() const
{
  return d->data;
}

Matroska::AttachedFile::UID Matroska::AttachedFile::uid() const
{
  return d->uid;
}
