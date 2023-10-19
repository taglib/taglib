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

#ifndef TAGLIB_EBMLMASTERELEMENT_H
#define TAGLIB_EBMLMASTERELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include "ebmlutils.h"
#include "ebmlelement.h"
#include "tbytevector.h"
#include "tlist.h"
#include "taglib.h"

namespace TagLib {
  namespace EBML {
    class MasterElement : public Element
    {
    public:
      MasterElement(Id id, int sizeLength, offset_t dataSize, offset_t offset)
      : Element(id, sizeLength, dataSize), offset(offset)
      {}
      MasterElement(Id id)
      : Element(id, 0, 0), offset(0)
      {}
      ~MasterElement() override;
      offset_t getOffset() const { return offset; }
      offset_t getSize() const { return headSize() + dataSize; }
      bool read(File &file) override;
      ByteVector render() override;
      void appendElement(Element *element) { elements.append(element); }
      List<Element*>::Iterator begin () { return elements.begin(); }
      List<Element*>::Iterator end () { return elements.end(); }
      List<Element*>::ConstIterator cbegin () const { return elements.cbegin(); }
      List<Element*>::ConstIterator cend () const { return elements.cend(); }

    protected:
      offset_t offset;
      List<Element*> elements;
    };
    
  }
}


#endif
#endif
