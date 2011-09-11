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
#include <utility>

namespace TagLib {
  namespace ID3v2 {
    /*!
     * This file contains methods used by the unified dictionary interface for ID3v2 tags
     * (tag name conversion, handling of un-translatable frameIDs, ...).
     */

    typedef Map<ByteVector, ByteVector> FrameIDMap;
    typedef std::pair<String, StringList> KeyValuePair;

    // forward declaration
    class Frame;
    /*!
     * Returns an appropriate ID3 frame ID for the given free-form tag name. This method
     * will return TXXX if no specialized translation is found.
     */
    ByteVector TAGLIB_EXPORT tagNameToFrameID(const String &);

    /*!
     * Returns a free-form tag name for the given ID3 frame ID. Note that this does not work
     * for general frame IDs such as TXXX or WXXX.
     */
    String TAGLIB_EXPORT frameIDToTagName(const ByteVector &);

    /*!
     * Tell if the given frame ID is ignored by the unified dictionary subsystem. This is true
     * for frames that don't admit a textual representation, such as pictures or other binary
     * information.
     */
    bool TAGLIB_EXPORT isIgnored(const ByteVector &);

    bool TAGLIB_EXPORT isDeprecated(const ByteVector&);

    /*!
     * Parse the ID3v2::Frame *Frame* to a pair of a human-readable key (e.g. ARTIST) and
     * a StringList containing the values.
     */
    KeyValuePair parseFrame(const Frame*);

    /*!
     * Create an appropriate ID3v2::Frame for the given tag name and values.
     */
    Frame *createFrame(const String &tag, const StringList &values);
    /*!
     * prepare the given tag name for use in a unified dictionary: make it uppercase and
     * removes prefixes set by the ExFalso/QuodLibet package.
     */
    String prepareTagName(const String &);


  }
}


#endif /* ID3V2DICTTOOLS_H_ */
