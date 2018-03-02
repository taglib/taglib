/***************************************************************************
    copyright           : (C) 2018 inMusic brands, inc.
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

#ifndef TAGLIB_ADTSHEADER_H
#define TAGLIB_ADTSHEADER_H

#include <memory>

#include "taglib.h"
#include "taglib_export.h"
#include "mpegheader.h"

namespace TagLib {

  class ByteVector;
  class File;

  namespace MPEG {

    //! An implementation of ADTS frame headers

    /*!
     * This is an implementation of ADTS headers used for .aac files.
     * I've used <a href="https://wiki.multimedia.cx/index.php/ADTS">this</a>
     * document as a reference.
     */

    class TAGLIB_EXPORT ADTSHeader
    {
    public:
      /*!
       * Parses an ADTS header based on \a file and \a offset.
       */
      ADTSHeader(TagLib::File *file, offset_t offset);

      /*!
       * Does a shallow copy of \a h.
       */
      ADTSHeader(const ADTSHeader &h);

      /*!
       * Destroys this Header instance.
       */
      virtual ~ADTSHeader();

      /*!
       * Returns true if the frame is at least an appropriate size and has
       * legal values.
       */
      bool isValid() const;

      /*!
       * Returns the MPEG Version of the header.
       */
      Header::Version version() const;

      /*!
       * Returns true if the MPEG protection bit is enabled.
       */
      bool protectionEnabled() const;

      /*!
       * Returns the approximate bitrate.
       */
      int bitrate() const;

      /*!
       * Returns the sample rate in Hz.
       */
      int sampleRate() const;

      /*!
       * Channel configurations.
       */
      enum ChannelMode {
        Custom = 0,
        FrontCenter = 1,
        FrontLeftRight = 2,
        FrontCenterLeftRight = 3,
        FrontCenterLeftRightBackCenter = 4,
        FrontCenterLeftRightBackLeftRight = 5,
        FrontCenterLeftRightBackLeftRightLFE = 6,
        FrontCenterLeftRightSideLeftRightBackLeftRightLFE = 7,
      };

      /*!
       * Returns the channel mode for this frame.
       */
      ChannelMode channelMode() const;

      /*!
       * Returns true if the copyrighted bit is set.
       */
      bool isCopyrighted() const;

      /*!
       * Returns true if the "original" bit is set.
       */
      bool isOriginal() const;

      /*!
       * Returns the frame length in bytes.
       */
      int frameLength() const;

      /*!
       * Returns the number of frames per sample.
       */
      int samplesPerFrame() const;

      /*!
       * Makes a shallow copy of the header.
       */
      ADTSHeader &operator=(const ADTSHeader &h);

    private:
      void parse(File *file, offset_t offset);

      class HeaderPrivate;
      std::shared_ptr<HeaderPrivate> d;
    };
  }  // namespace MPEG
}  // namespace TagLib

#endif
