/***************************************************************************
    copyright            : (C) 2004 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#ifndef TAGLIB_RELATIVEVOLUMEFRAME_H
#define TAGLIB_RELATIVEVOLUMEFRAME_H

#include <id3v2frame.h>

namespace TagLib {

  namespace ID3v2 {

    //! An ID3v2 relative volume adjustment frame implementation

    /*!
     * This is an implementation of ID3v2 relative volume adjustment.  The
     * presense of this frame makes it possible to specify an increase in volume
     * for an audio file or specific audio tracks in that file.
     *
     * Multiple relative volume adjustment frames may be present in the tag
     * each with a unique identification and describing volume adjustment for
     * different channel types.
     */

    class RelativeVolumeFrame : public Frame
    {
      friend class FrameFactory;

    public:

      /*!
       * This indicates the type of volume adjustment that should be applied.
       */
      enum ChannelType {
        //! A type not enumerated below
        Other        = 0x00,
        //! The master volume for the track
        MasterVolume = 0x01,
        //! The front right audio channel
        FrontRight   = 0x02,
        //! The front left audio channel
        FrontLeft    = 0x03,
        //! The back right audio channel
        BackRight    = 0x04,
        //! The back left audio channel
        BackLeft     = 0x05,
        //! The front center audio channel
        FrontCentre  = 0x06,
        //! The back center audio channel
        BackCentre   = 0x07,
        //! The subwoofer audio channel
        Subwoofer    = 0x08
      };

      //! Struct that stores the relevant values for ID3v2 peak volume

      /*!
       * The peak volume is described as a series of bits that is padded to fill
       * a block of bytes.  These two values should always be updated in tandem.
       */
      struct PeakVolume
      {
        /*!
         * Constructs an empty peak volume description.
         */
        PeakVolume() : bitsRepresentingPeak(0) {}
        /*!
         * The number of bits (in the range of 0 to 255) used to describe the
         * peak volume.
         */
        unsigned char bitsRepresentingPeak;
        /*!
         * The array of bits (represented as a series of bytes) used to describe
         * the peak volume.
         */
        ByteVector peakVolume;
      };

      /*!
       * Constructs a RelativeVolumeFrame.  The relevant data should be set
       * manually.
       */
      RelativeVolumeFrame();

      /*!
       * Constructs a RelativeVolumeFrame based on the contents of \a data.
       */
      RelativeVolumeFrame(const ByteVector &data);

      /*!
       * Destroys the RelativeVolumeFrame instance.
       */
      virtual ~RelativeVolumeFrame();

      /*!
       * Returns the frame's identification.
       *
       * \see identification()
       */
      virtual String toString() const;

      /*!
       * Returns the channel type that this frame refers to.
       *
       * \see setChannelType()
       */
      ChannelType channelType() const;

      /*!
       * Sets the channel type that this frame refers to.
       *
       * \see channelType()
       */
      void setChannelType(ChannelType t);

      /*!
       * Returns the relative volume adjustment "index".  As indicated by the
       * ID3v2 standard this is a 16-bit signed integer that reflects the
       * decibils of adjustment when divided by 512.
       *
       * \see setVolumeAdjustmentIndex()
       * \see volumeAjustment()
       */
      short volumeAdjustmentIndex() const;

      /*!
       * Set the volume adjustment to \a index.  As indicated by the ID3v2
       * standard this is a 16-bit signed integer that reflects the decibils of
       * adjustment when divided by 512.
       *
       * \see volumeAdjustmentIndex()
       * \see setVolumeAjustment()
       */
      void setVolumeAdjustmentIndex(short index);

      /*!
       * Returns the relative volume adjustment in decibels.
       *
       * \note Because this is actually stored internally as an "index" to this
       * value the value returned by this method may not be identical to the
       * value set using setVolumeAdjustment().
       *
       * \see setVolumeAdjustment()
       * \see volumeAdjustmentIndex()
       */
      float volumeAdjustment() const;

      /*!
       * Set the relative volume adjustment in decibels to \a adjustment.
       *
       * \note Because this is actually stored internally as an "index" to this
       * value the value set by this method may not be identical to the one
       * returned by volumeAdjustment().
       *
       * \see setVolumeAdjustment()
       * \see volumeAdjustmentIndex()
       */
      void setVolumeAdjustment(float adjustment);

      /*!
       * Returns the peak volume (represented as a length and a string of bits).
       *
       * \see setPeakVolume()
       */
      PeakVolume peakVolume() const;

      /*!
       * Sets the peak volume to \a peak.
       *
       * \see peakVolume()
       */
      void setPeakVolume(const PeakVolume &peak);

    protected:
      virtual void parseFields(const ByteVector &data);
      virtual ByteVector renderFields() const;

    private:
      RelativeVolumeFrame(const ByteVector &data, Header *h);
      RelativeVolumeFrame(const RelativeVolumeFrame &);
      RelativeVolumeFrame &operator=(const RelativeVolumeFrame &);

      class RelativeVolumeFramePrivate;
      RelativeVolumeFramePrivate *d;
    };

  }
}
#endif
