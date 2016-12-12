/***************************************************************************
    copyright            : (C) 2013 by Sebastian Rachuj
    email                : rachus@web.de
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

#ifndef TAGLIB_EBMLMATROSKAAUDIO_H
#define TAGLIB_EBMLMATROSKAAUDIO_H

#include "ebmlmatroskafile.h"

#include "audioproperties.h"

namespace TagLib {

  namespace EBML {

    namespace Matroska {

      /*!
       * This class represents the audio properties of a matroska file.
       * Currently all information are read from the container format and
       * could be inexact.
       */
      class TAGLIB_EXPORT AudioProperties : public TagLib::AudioProperties
      {
      public:
        //! Destructor
        virtual ~AudioProperties();

        /*!
         * Constructs an instance from a file.
         */
        explicit AudioProperties(File *document);

        /*!
         * Returns the length of the file.
         */
        virtual int length() const;
        virtual int lengthInSeconds() const;
        virtual int lengthInMilliseconds() const;

        /*!
         * Returns the bit rate of the file. Since the container format does not
         * offer a proper value, it ist currently calculated by dividing the
         * file size by the length.
         */
        virtual int bitrate() const;

        /*!
         * Returns the amount of channels of the file.
         */
        virtual int channels() const;

        /*!
         * Returns the sample rate of the file.
         */
        virtual int sampleRate() const;

      private:
        void read(File *document);

        class AudioPropertiesPrivate;
        AudioPropertiesPrivate *d;
      };

    }

  }

}

#endif
