/***************************************************************************
    copyright            : (C) 2011 by Michael Helmling
    email                : supermihi@web.de
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

#ifndef ID3V2DICTTOOLS_H_
#define ID3V2DICTTOOLS_H_

#include "tstringlist.h"
#include "taglib_export.h"
#include "tmap.h"

namespace TagLib {
  namespace ID3v2 {
    /*!
     * This file contains methods used by the unified dictionary interface for ID3v2 tags
     * (tag name conversion, handling of un-translatable frameIDs, ...).
     */
    typedef Map<ByteVector, ByteVector> FrameIDMap;

    String TAGLIB_EXPORT frameIDToTagName(const ByteVector &id);

    bool TAGLIB_EXPORT isIgnored(const ByteVector &);

    FrameIDMap TAGLIB_EXPORT deprecationMap();

    bool TAGLIB_EXPORT isDeprecated(const ByteVector&);


  }
}


#endif /* ID3V2DICTTOOLS_H_ */
