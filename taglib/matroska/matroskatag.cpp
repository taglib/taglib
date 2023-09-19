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

#include "matroskatag.h"
#include "matroskasimpletag.h"
#include "tlist.h"
#include "tdebug.h"

using namespace TagLib;

class Matroska::Tag::TagPrivate 
{
  public:
    TagPrivate() = default;
    ~TagPrivate() {
      for (auto tag : tags)
        delete tag;
    }
    List<SimpleTag*> tags;

};

Matroska::Tag::Tag()
: TagLib::Tag(),
  d(std::make_unique<TagPrivate>())
{

}
Matroska::Tag::~Tag() = default;

void Matroska::Tag::addSimpleTag(SimpleTag *tag)
{
  d->tags.append(tag);
}

void Matroska::Tag::removeSimpleTag(SimpleTag *tag)
{
  auto it = d->tags.find(tag);
  if (it != d->tags.end())
    d->tags.erase(it);
}

const Matroska::SimpleTagsList& Matroska::Tag::simpleTagsList() const
{
  return d->tags;
}
