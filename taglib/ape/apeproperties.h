/***************************************************************************
    copyright            : (C) 2010 by Alex Novichkov
    email                : novichko@atnet.ru

    copyright            : (C) 2006 by Lukáš Lalinský
    email                : lalinsky@gmail.com
                           (original WavPack implementation)
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

#ifndef TAGLIB_APEPROPERTIES_H
#define TAGLIB_APEPROPERTIES_H

#include "taglib_export.h"
#include "audioproperties.h"

namespace TagLib {

  namespace APE {

    class File;

    //! An implementation of audio property reading for APE

    /*!
     * This reads the data from an APE stream found in the AudioProperties
     * API.
     */

    class TAGLIB_EXPORT AudioProperties : public TagLib::AudioProperties
    {
    public:
      /*!
       * Creates an instance of APE::AudioProperties with the data read from
       * the ByteVector \a data.
       */
      AudioProperties(File *file, ReadStyle style = Average);

      /*!
       * Destroys this APE::AudioProperties instance.
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
      uint sampleFrames() const;

      /*!
       * Returns APE version.
       */
      int version() const;

    private:
      void read(File *file);

      offset_t findDescriptor(File *file);
      offset_t findID3v2(File *file);

      void analyzeCurrent(File *file);
      void analyzeOld(File *file);

      class PropertiesPrivate;
      PropertiesPrivate *d;
    };
  }
}

#endif
