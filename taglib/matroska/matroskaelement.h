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

#include <memory>
#include "taglib_export.h"
#include "tutils.h"
#include "tbytevector.h"


namespace TagLib {
  namespace Matroska {
    class TAGLIB_EXPORT Element
    {
    public:
      Element();
      virtual ~Element();
      virtual ByteVector render() = 0;
      offset_t size() const;
      offset_t offset() const;
      void setOffset(offset_t offset);
      void setSize(offset_t size);

    private:
      class ElementPrivate;
      std::unique_ptr<ElementPrivate> e;

    };
  }
}

 #endif