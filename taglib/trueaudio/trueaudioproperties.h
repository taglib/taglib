/***************************************************************************
    copyright            : (C) 2006 by Lukáš Lalinský
    email                : lalinsky@gmail.com

    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
                           (original MPC implementation)
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

#ifndef TAGLIB_TRUEAUDIOPROPERTIES_H
#define TAGLIB_TRUEAUDIOPROPERTIES_H

#include "audioproperties.h"

namespace TagLib {

  namespace TrueAudio {

    class File;

    //! An implementation of audio property reading for TrueAudio

    /*!
     * This reads the data from an TrueAudio stream found in the AudioProperties
     * API.
     */

    class TAGLIB_EXPORT AudioProperties : public TagLib::AudioProperties
    {
    public:
      /*!
       * Create an instance of TrueAudio::AudioProperties with the data read from 
       * the ByteVector \a data.
       */
      AudioProperties(File *file, offset_t streamLength, ReadStyle style = Average);

      /*!
       * Destroys this TrueAudio::AudioProperties instance.
       */
      virtual ~AudioProperties();

      // Reimplementations.

      virtual int length() const;
      virtual int bitrate() const;
      virtual int sampleRate() const;
      virtual int channels() const;

      /*!
       * Returns number of bits per sample.
       */
      int bitsPerSample() const;

      /*!
       * Returns the total number of sample frames
       */
      uint sampleFrames() const;

      /*!
       * Returns the major version number.
       */
      int ttaVersion() const;

    private:
      void read(File *file, offset_t streamLength);

      class PropertiesPrivate;
      NonRefCountPtr<PropertiesPrivate> d;
    };
  }
}

#endif
