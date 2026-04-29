/***************************************************************************
    copyright            : (C) 2026 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#ifndef TAGLIB_MATROSKAWRITESTYLE_H
#define TAGLIB_MATROSKAWRITESTYLE_H

namespace TagLib::Matroska {
  /*!
   * Controls the trade-off between file size and write speed when saving.
   * Mode of writing tags, attachments and chapters to the file.
   * For very large files and/or slow (network) filesystems, using
   * \c AvoidInsert will reduce write time significantly.
   */
  enum class WriteStyle {
    //! Write tags, attachments and chapters as compact as possible (default).
    Compact,
    //! Do not shrink elements; add void padding when content gets smaller.
    //! Allow inserts when content gets larger.
    DoNotShrink,
    //! Like \c DoNotShrink but also avoid inserts for non-last elements:
    //! replace a growing non-last element with a void of the old size and
    //! append the new element at the end of the segment.
    AvoidInsert
  };
}

#endif //TAGLIB_MATROSKAWRITESTYLE_H
