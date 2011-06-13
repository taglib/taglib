/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#ifndef TAGLIB_MODFILE_H
#define TAGLIB_MODFILE_H

#include "taglib.h"
#include "tfile.h"
#include "tstring.h"
#include "taglib_export.h"

namespace TagLib {
  namespace Mod {
    class TAGLIB_EXPORT File : public TagLib::File {
      protected:
        File(FileName file);
        File(IOStream *stream);

        void writeString(const String &s, ulong size);
        bool readString(String &s, ulong size);
        bool readByte(uchar &byte);
        bool readU16L(ushort &number);
        bool readU32L(ulong &number);
    };
  }
}

#endif
