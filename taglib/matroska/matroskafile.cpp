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
#include "ebmlutils.h"
#include "ebmlelement.h"
#include "ebmlmksegment.h"
#include "tlist.h"
#include "tdebug.h"
#include "tutils.h"

#include <memory>

using namespace TagLib;

class Matroska::File::FilePrivate
{
public:
  FilePrivate() {}

  ~FilePrivate()
  {
    delete tag;
  }

  FilePrivate(const FilePrivate &) = delete;
  FilePrivate &operator=(const FilePrivate &) = delete;
  Matroska::Tag *tag = nullptr;

  //Properties *properties = nullptr;
};

Matroska::File::File(FileName file, bool readProperties)
: TagLib::File(file),
  d(std::make_unique<FilePrivate>())
{
  if (!isOpen()) {
    debug("Failed to open matroska file");
    setValid(false);
    return;
  }
  read(readProperties);
}
Matroska::File::~File() = default;

TagLib::Tag* Matroska::File::tag() const
{
  return d->tag;
}

void Matroska::File::read(bool readProperties)
{
  offset_t fileLength = length();

  // Find the EBML Header
  std::unique_ptr<EBML::Element> head(EBML::Element::factory(*this));
  if (!head || head->getId() != EBML_ID_HEAD) {
    debug("Failed to find EBML head");
    setValid(false);
    return;
  }
  head->skipData(*this);

  // Find the Matroska segment
  std::unique_ptr<EBML::MkSegment> segment(
    static_cast<EBML::MkSegment*>(EBML::findElement(*this, EBML_ID_MK_SEGMENT, fileLength - tell()))
  );
  if (!segment) {
    debug("Failed to find Matroska segment");
    setValid(false);
    return;
  }
  if (!segment->read(*this)) {
    debug("Failed to read segment");
    setValid(false);
    return;
  }
  d->tag = segment->parseTag();

}
