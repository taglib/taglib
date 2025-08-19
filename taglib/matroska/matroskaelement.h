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

#ifndef HAS_MATROSKAELEMENT_H
#define HAS_MATROSKAELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include <memory>
#include "taglib_export.h"
#include "taglib.h"
#include "tbytevector.h"
#include "tlist.h"

namespace TagLib {
  class File;
  namespace Matroska {
    class TAGLIB_EXPORT Element
    {
    public:
      using ID = unsigned int;
      explicit Element(ID id);
      virtual ~Element();

      offset_t size() const;
      offset_t offset() const;
      ID id() const;
      void setOffset(offset_t offset);
      void adjustOffset(offset_t delta);
      void setSize(offset_t size);
      void setID(ID id);
      //virtual ByteVector render() = 0;
      virtual bool render() = 0;
      void setData(const ByteVector &data);
      const ByteVector &data() const;
      virtual void write(TagLib::File &file);
      void addSizeListener(Element *element);
      void addSizeListeners(const List<Element *> &elements);
      void addOffsetListener(Element *element);
      void addOffsetListeners(const List<Element *> &elements);
      //virtual void updatePosition(Element &caller, offset_t delta) = 0;
      bool emitSizeChanged(offset_t delta);
      bool emitOffsetChanged(offset_t delta);
      virtual bool offsetChanged(Element &caller, offset_t delta);
      virtual bool sizeChanged(Element &caller, offset_t delta);

    private:
      class ElementPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<ElementPrivate> e;
    };
  }
}

#endif
#endif
