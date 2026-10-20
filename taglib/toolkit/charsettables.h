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
 *   License along with this library; if not, write to the Free Software    *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef TAGLIB_CHARSETTABLES_H
#define TAGLIB_CHARSETTABLES_H

// THIS FILE IS NOT A PART OF THE TAGLIB API

#ifndef DO_NOT_DOCUMENT  // tell Doxygen not to document this header

#include "tcharset.h"

namespace TagLib {
  namespace Charset {
    namespace detail {

      /*!
       * One row of the generated code page table, sorted by code page.  UTF8
       * has no row because it is not a single byte code page.
       */
      struct CodePageEntry
      {
        Type type;
        unsigned int codePage;
        const unsigned short *toUnicode;
      };

      /*!
       * Returns the row for \a codePage, or null if there is no table for it.
       * A Type and the code page number a CSET chunk states are the same value,
       * so they name the same row.  UTF8 has no row and is never found here.
       */
      const CodePageEntry *entryForCodePage(unsigned int codePage);

    }  // namespace detail
  }  // namespace Charset
}  // namespace TagLib

#endif  // DO_NOT_DOCUMENT

#endif
