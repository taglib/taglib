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
#include "taglib.h"
#include "tstring.h"
#include "taglib_export.h"

namespace TagLib {
  class String;
  class ByteVector;

  namespace EBML {
    class MkAttachments;
  }

  namespace Matroska {
    class File;

    //! Attached file embedded into a Matroska file.
    class TAGLIB_EXPORT AttachedFile
    {
    public:
      //! Unique identifier.
      using UID = unsigned long long;

      /*!
       * Construct an attached file.
       */
      AttachedFile(const ByteVector &data, const String &fileName,
                   const String &mediaType, UID uid = 0,
                   const String &description = String());

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
      AttachedFile &operator=(AttachedFile &&other) noexcept;

      /*!
       * Exchanges the content of the object with the content of \a other.
       */
      void swap(AttachedFile &other) noexcept;

      /*!
       * Returns the filename of the attached file.
       */
      const String &fileName() const;

      /*!
       * Returns the human-friendly description for the attached file.
       */
      const String &description() const;

      /*!
       * Returns the media type of the attached file.
       */
      const String &mediaType() const;

      /*!
       * Returns the data of the attached file.
       *
       * \note When the attached file was read from a file, its data is only
       * loaded from the file when the attachments are requested using
       * Matroska::File::attachments().  Objects obtained from there therefore
       * always have their data available.
       */
      const ByteVector &data() const;

      /*!
       * Returns the UID of the attached file.
       */
      UID uid() const;

    private:
      friend class EBML::MkAttachments;
      friend class File;
      class AttachedFilePrivate;

      /*!
       * Construct an attached file whose data is not loaded yet.  The data is
       * at \a dataOffset in the file and \a dataSize bytes long, it will be
       * loaded by Matroska::File when the attachments are requested.
       */
      AttachedFile(offset_t dataOffset, offset_t dataSize,
                   const String &fileName, const String &mediaType, UID uid,
                   const String &description);

      //! Returns \c true if the data still has to be loaded from the file.
      bool isDataDeferred() const;

      //! Returns the offset of the not yet loaded data inside the file.
      offset_t deferredDataOffset() const;

      //! Returns the size of the not yet loaded data.
      offset_t deferredDataSize() const;

      //! Sets the data which has been loaded from the file.
      void setLoadedData(const ByteVector &data);

      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<AttachedFilePrivate> d;
    };
  }
}

#endif
