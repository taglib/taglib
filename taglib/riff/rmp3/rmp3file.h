/***************************************************************************
    copyright            : (C) 2013 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
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

#ifndef TAGLIB_RMP3FILE_H
#define TAGLIB_RMP3FILE_H

#include "taglib_export.h"
#include "rifffile.h"
#include "infotag.h"
#include "rmp3properties.h"

namespace TagLib {

  namespace RIFF {

    //! An implementation of RMP3 metadata

    /*!
     * This is implementation of RMP3 metadata.
     */

    namespace RMP3 {

      //! An implementation of TagLib::File with RIFF/RMP3 specific methods

      /*!
       * This implements and provides an interface for RIFF/RMP3 files to the
       * TagLib::Tag and TagLib::AudioProperties interfaces by way of implementing
       * the abstract TagLib::File API as well as providing some additional
       * information specific to RIFF/RMP3 files.
       *
       * The technical description of RMP3 format (in Japanese) is found at:
       * http://hp.vector.co.jp/authors/VA005308/help/wav2mp3/rmpinfo.html
       */

      class TAGLIB_EXPORT File : public TagLib::RIFF::File
      {
      public:
        /*!
         * Constructs a RIFF/RMP3 file from \a file.  If \a readProperties is true 
         * the file's audio properties will also be read.
         *
         * \note In the current implementation, \a propertiesStyle is ignored.
         */
        File(FileName file, bool readProperties = true,
             AudioProperties::ReadStyle propertiesStyle = AudioProperties::Average);

        /*!
         * Constructs a RIFF/RMP3 file from \a stream.  If \a readProperties is true 
         * the file's audio properties will also be read.
         *
         * \note TagLib will *not* take ownership of the stream, the caller is
         * responsible for deleting it after the File object.
         *
         * \note In the current implementation, \a propertiesStyle is ignored.
         */
        File(IOStream *stream, bool readProperties = true,
             AudioProperties::ReadStyle propertiesStyle = AudioProperties::Average);

        /*!
         * Destroys this instance of the File.
         */
        virtual ~File();

        /*!
         * Returns the RIFF INFO Tag for this file.
         */
        virtual Info::Tag *tag() const;

        /*!
         * Implements the unified property interface -- export function.
         * This method forwards to ID3v2::Tag::properties().
         */
        PropertyMap properties() const;

        void removeUnsupportedProperties(const StringList &properties);

        /*!
         * Implements the unified property interface -- import function.
         * This method forwards to ID3v2::Tag::setProperties().
         */
        PropertyMap setProperties(const PropertyMap &);

        /*!
         * Returns the WAV::Properties for this file.  If no audio properties
         * were read then this will return a null pointer.
         */
        virtual AudioProperties *audioProperties() const;

        /*!
         * Saves the file.
         */
        virtual bool save();

        /*!
         * Returns whether or not the file on disk actually has an RIFF INFO tag
         * (aka RIFF/SIF).
         */
        bool hasInfoTag() const;

        /*!
         * Returns the position in the file of the first MPEG frame.
         */
        offset_t firstFrameOffset();

        /*!
         * Returns the position in the file of the next MPEG frame,
         * using the current position as start
         */
        offset_t nextFrameOffset(offset_t position);

        /*!
         * Returns the position in the file of the previous MPEG frame,
         * using the current position as start
         */
        offset_t previousFrameOffset(offset_t position);

        /*!
         * Returns the position in the file of the last MPEG frame.
         */
        offset_t lastFrameOffset();

      private:
        void read(bool readProperties, AudioProperties::ReadStyle propertiesStyle);

        class FilePrivate;
        FilePrivate *d;
      };
    }
  }
}

#endif
