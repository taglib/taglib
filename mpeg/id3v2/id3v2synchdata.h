/***************************************************************************
    copyright            : (C) 2002, 2003 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#ifndef TAGLIB_ID3V2SYNCHDATA_H
#define TAGLIB_ID3V2SYNCHDATA_H

#include <tbytevector.h>
#include <taglib.h>

namespace TagLib {

  namespace ID3v2 {

    //! A few functions for ID3v2 synch safe integer conversion

    /*!
     * In the ID3v2.4 standard most integer values are encoded as "synch safe"
     * integers which are encoded in such a way that they will not give false
     * MPEG syncs and confuse MPEG decoders.  This namespace provides some
     * methods for converting to and from these values to ByteVectors for
     * things rendering and parsing ID3v2 data.
     */

    namespace SynchData
    {
      /*!
       * This returns the unsigned integer value of \a data where \a data is a
       * ByteVector that contains a \e synchsafe integer (Structure,
       * <a href="id3v2-structure.html#6.2">6.2</a>).  The default \a length of
       * 4 is used if another value is not specified.
       */
      uint toUInt(const ByteVector &data);

      /*!
       * Returns a 4 byte (32 bit) synchsafe integer based on \a value.
       */
      ByteVector fromUInt(uint value);
    }

  }
}

#endif
