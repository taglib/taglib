/***************************************************************************
    copyright            : (C) 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#ifndef TAGLIB_AIFFPROPERTIES_H
#define TAGLIB_AIFFPROPERTIES_H

#include "audioproperties.h"

namespace TagLib {

  namespace RIFF {

    namespace AIFF {

      class File;

      //! An implementation of audio property reading for AIFF or AIFF-C

      /*!
       * This reads the data from an AIFF stream found in the AudioProperties
       * API.
       */

      class TAGLIB_EXPORT AudioProperties : public TagLib::AudioProperties
      {
      public:
        /*!
         * Creates an instance of AIFF::AudioProperties with the data read from
         * the ByteVector \a data.
         */
        AudioProperties(const ByteVector &data, ReadStyle style);

        /*!
         * Destroys this AIFF::AudioProperties instance.
         */
        virtual ~AudioProperties();

        virtual bool isNull() const;

        // Reimplementations.

        virtual int length() const;
        virtual int bitrate() const;
        virtual int sampleRate() const;
        virtual int channels() const;

        int sampleWidth() const;
        uint sampleFrames() const;

        /*!
         * Returns true if the file is in AIFF-C format, false if AIFF format.
         */
        bool isAiffC() const;

        /*!
         * Returns the compression type of the AIFF-C file.  For example, "NONE" for
         * not compressed, "ACE2" for ACE 2-to-1.
         *
         * If the file is in AIFF format, always returns an empty vector.
         *
         * \see isAiffC()
         */
        ByteVector compressionType() const;

        /*!
         * Returns the concrete compression name of the AIFF-C file.
         *
         * If the file is in AIFF format, always returns an empty string.
         *
         * \see isAiffC()
         */
        String compressionName() const;

      private:
        void read(const ByteVector &data);

        class PropertiesPrivate;
        PropertiesPrivate *d;
      };
    }
  }
}

#endif
