/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
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

#ifndef TAGLIB_VORBISPROPERTIES_H
#define TAGLIB_VORBISPROPERTIES_H

#include "taglib_export.h"
#include "audioproperties.h"

namespace TagLib {

  namespace Ogg {

  namespace Vorbis {

    class File;

    //! An implementation of audio property reading for Ogg Vorbis

    /*!
     * This reads the data from an Ogg Vorbis stream found in the AudioProperties
     * API.
     */

    class TAGLIB_EXPORT AudioProperties : public TagLib::AudioProperties
    {
    public:
      /*!
       * Creates an instance of Vorbis::Properties with the data read from the
       * Vorbis::File \a file.
       */
      AudioProperties(File *file, ReadStyle style = Average);

      /*!
       * Destroys this VorbisProperties instance.
       */
      virtual ~AudioProperties();

      /*!
       * Returns the length of the file in seconds.  The length is rounded down to
       * the nearest whole second.
       *
       * \note This method is just an alias of lengthInSeconds().
       *
       * \deprecated
       */
      virtual int length() const;

      /*!
       * Returns the length of the file in seconds.  The length is rounded down to
       * the nearest whole second.
       *
       * \see lengthInMilliseconds()
       */
      virtual int lengthInSeconds() const;

      /*!
       * Returns the length of the file in milliseconds.
       *
       * \see lengthInSeconds()
       */
      virtual int lengthInMilliseconds() const;

      /*!
       * Returns the average bit rate of the file in kb/s.
       */
      virtual int bitrate() const;

      /*!
       * Returns the sample rate in Hz.
       */
      virtual int sampleRate() const;

      /*!
       * Returns the number of audio channels.
       */
      virtual int channels() const;
      virtual String toString() const;

      /*!
       * Returns the Vorbis version, currently "0" (as specified by the spec).
       */
      int vorbisVersion() const;

      /*!
       * Returns the maximum bitrate as read from the Vorbis identification
       * header.
       *
       * \note The value is in bits per second unlike bitrate().
       */
      int bitrateMaximum() const;

      /*!
       * Returns the nominal bitrate as read from the Vorbis identification
       * header.
       *
       * \note The value is in bits per second unlike bitrate().
       */
      int bitrateNominal() const;

      /*!
       * Returns the minimum bitrate as read from the Vorbis identification
       * header.
       *
       * \note The value is in bits per second unlike bitrate().
       */
      int bitrateMinimum() const;

    private:
      void read(File *file);

      class PropertiesPrivate;
      PropertiesPrivate *d;
    };
  }

  }

}

#endif
