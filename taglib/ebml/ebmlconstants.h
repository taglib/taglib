/***************************************************************************
    copyright            : (C) 2013 by Sebastian Rachuj
    email                : rachus@web.de
 ***************************************************************************/

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

#ifndef TAGLIB_EBML_CONSTANTS
#define TAGLIB_EBML_CONSTANTS

namespace TagLib {

  namespace EBML {
    //! Shorter representation of the type.
    typedef unsigned long long int ulli;
    
    //! The id of an EBML Void element that is just a placeholder.
    const ulli Void = 0xecL;
    
    //! The id of an EBML CRC32 element that contains a crc32 value.
    const ulli CRC32 = 0xc3L;
    
    //! A namespace containing the ids of the EBML header's elements.
    namespace Header {
      const ulli EBML = 0x1a45dfa3L;
      const ulli EBMLVersion = 0x4286L;
      const ulli EBMLReadVersion = 0x42f7L;
      const ulli EBMLMaxIDWidth = 0x42f2L;
      const ulli EBMLMaxSizeWidth = 0x42f3L;
      const ulli DocType = 0x4282L;
      const ulli DocTypeVersion = 0x4287L;
      const ulli DocTypeReadVersion = 0x4285L;
    }
    
  }

}


#endif
