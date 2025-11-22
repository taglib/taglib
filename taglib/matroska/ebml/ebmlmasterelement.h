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

#include <list>

#include "ebmlelement.h"
#include "taglib.h"

namespace TagLib
{
  class ByteVector;

  namespace EBML {
    class MasterElement : public Element
    {
    public:
      MasterElement(Id id, int sizeLength, offset_t dataSize, offset_t offset);
      explicit MasterElement(Id id);
      ~MasterElement() override;

      offset_t getOffset() const;
      bool read(File &file) override;
      ByteVector render() override;
      void appendElement(std::unique_ptr<Element> &&element);
      std::list<std::unique_ptr<Element>>::iterator begin();
      std::list<std::unique_ptr<Element>>::iterator end();
      std::list<std::unique_ptr<Element>>::const_iterator begin() const;
      std::list<std::unique_ptr<Element>>::const_iterator end() const;
      std::list<std::unique_ptr<Element>>::const_iterator cbegin() const;
      std::list<std::unique_ptr<Element>>::const_iterator cend() const;
      offset_t getPadding() const;
      void setPadding(offset_t numBytes);
      offset_t getMinRenderSize() const;
      void setMinRenderSize(offset_t minimumSize);

    protected:
      offset_t offset;
      offset_t padding = 0;
      offset_t minRenderSize = 0;
      std::list<std::unique_ptr<Element>> elements;
    };

  }
}

#endif
#endif
