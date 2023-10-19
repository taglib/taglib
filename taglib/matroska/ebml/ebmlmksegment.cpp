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

#include "ebmlmksegment.h"
#include "ebmlmktags.h"
#include "ebmlmkattachments.h"
#include "ebmlutils.h"
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskaattachments.h"
#include "tutils.h"
#include "tbytevector.h"
#include "tdebug.h"

using namespace TagLib;

EBML::MkSegment::~MkSegment()
{
  delete tags;
  delete attachments;
}

bool EBML::MkSegment::read(File &file)
{
  offset_t maxOffset = file.tell() + dataSize;
  EBML::Element *element = nullptr;

  while((element = findNextElement(file, maxOffset))) {
    Id id = element->getId();
    if(id == ElementIDs::MkTags) {
      tags = static_cast<MkTags*>(element);
      if(!tags->read(file))
        return false;
    }
    else if(id == ElementIDs::MkAttachments) {
      attachments = static_cast<MkAttachments*>(element);
      if(!attachments->read(file))
        return false;
    }
    else {
      element->skipData(file);
      delete element;
    }
  }
  return true;
}

Matroska::Tag* EBML::MkSegment::parseTag()
{
  return tags ? tags->parse() : nullptr;
}

Matroska::Attachments* EBML::MkSegment::parseAttachments()
{
  return attachments ? attachments->parse() : nullptr;
}
