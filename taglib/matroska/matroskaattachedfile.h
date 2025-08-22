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

#include <memory>
#include "taglib_export.h"

namespace TagLib {
  class String;
  class ByteVector;
  namespace Matroska {
    //! Attached file embedded into a Matroska file.
    class TAGLIB_EXPORT AttachedFile
    {
    public:
      using UID = unsigned long long;
      AttachedFile();

      /*!
       * Construct an attached file as a copy of \a other.
       */
      AttachedFile(const AttachedFile &other);

      /*!
       * Construct an attached file moving from \a other.
       */
      AttachedFile(AttachedFile &&other) noexcept;

      /*!
       * Destroys this attached file.
       */
      ~AttachedFile();

      /*!
       * Copies the contents of \a other into this object.
       */
      AttachedFile &operator=(const AttachedFile &other);

      /*!
       * Moves the contents of \a other into this object.
       */
      AttachedFile &operator=(AttachedFile &&other);

      /*!
       * Exchanges the content of the object with the content of \a other.
       */
      void swap(AttachedFile &other) noexcept;

      /*!
       * Set the \a fileName of the attached file.
       */
      void setFileName(const String &fileName);

      /*!
       * Returns the filename of the attached file.
       */
      const String &fileName() const;

      /*!
       * Set a human-friendly \a description for the attached file.
       */
      void setDescription(const String &description);

      /*!
       * Returns the human-friendly description for the attached file.
       */
      const String &description() const;

      /*!
       * Set the \a mediaType of the attached file.
       */
      void setMediaType(const String &mediaType);

      /*!
       * Returns the media type of the attached file.
       */
      const String &mediaType() const;

      /*!
       * Set the data of the attached file.
       */
      void setData(const ByteVector &data);

      /*!
       * Returns the data of the attached file.
       */
      const ByteVector &data() const;

      /*!
       * Set the \a uid representing the file, as random as possible.
       */
      void setUID(UID uid);

      /*!
       * Returns the UID of the attached file.
       */
      UID uid() const;

    private:
      class AttachedFilePrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<AttachedFilePrivate> d;
    };
  }
}

#endif
