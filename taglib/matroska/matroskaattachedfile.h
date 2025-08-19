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

#ifndef TAGLIB_MATROSKAATTACHEDFILE_H
#define TAGLIB_MATROSKAATTACHEDFILE_H

#include "taglib_export.h"

namespace TagLib {
  class String;
  class ByteVector;
  namespace Matroska {
    class TAGLIB_EXPORT AttachedFile
    {
    public:
      using UID = unsigned long long;
      AttachedFile();
      ~AttachedFile();

      void setFileName(const String &fileName);
      const String &fileName() const;
      void setDescription(const String &description);
      const String &description() const;
      void setMediaType(const String &mediaType);
      const String &mediaType() const;
      void setData(const ByteVector &data);
      const ByteVector &data() const;
      void setUID(UID uid);
      UID uid() const;

    private:
      class AttachedFilePrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<AttachedFilePrivate> d;
    };
  }
}

#endif
