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

#ifndef TAGLIB_MATROSKAPROPERTIES_H
#define TAGLIB_MATROSKAPROPERTIES_H

#include "taglib_export.h"
#include "audioproperties.h"


namespace TagLib {

  namespace MPEG {

    class File;

    //! An implementation of audio property reading for MP3

    /*!
     * This reads the data from an MPEG Layer III stream found in the
     * AudioProperties API.
     */

    class TAGLIB_EXPORT Properties : public AudioProperties
    {
    public:

      /*!
       * Destroys this MPEG Properties instance.
       */
      ~Properties() override {}

      Properties(const Properties &) = delete;
      Properties &operator=(const Properties &) = delete;

      /*!
       * Returns the length of the file in milliseconds.
       *
       * \see lengthInSeconds()
       */
      int lengthInMilliseconds() const override {}

      /*!
       * Returns the average bit rate of the file in kb/s.
       */
      int bitrate() const override {}

      /*!
       * Returns the sample rate in Hz.
       */
      int sampleRate() const override {}

      /*!
       * Returns the number of audio channels.
       */
      int channels() const override {}

    private:

      class PropertiesPrivate;
      std::unique_ptr<PropertiesPrivate> d;
    };
  }  // namespace MPEG
}  // namespace TagLib

#endif
