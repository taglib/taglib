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

    class RelativeVolumeFrame : public Frame
    {
      friend class FrameFactory;

    public:

      enum ChannelType {
        Other        = 0x00,
        MasterVolume = 0x01,
        FrontRight   = 0x02,
        FrontLeft    = 0x03,
        BackRight    = 0x04,
        BackLeft     = 0x05,
        FrontCentre  = 0x06,
        BackCentre   = 0x07,
        Subwoofer    = 0x08
      };

      struct PeakVolume
      {
        PeakVolume() : bitsRepresentingPeak(0) {}
        unsigned char bitsRepresentingPeak;
        ByteVector peakVolume;
      };

      RelativeVolumeFrame(const ByteVector &data);
      virtual ~RelativeVolumeFrame();

      virtual String toString() const;

      ChannelType channelType() const;
      void setChannelType(ChannelType t);
      short volumeAdjustmentIndex() const;
      void setVolumeAdjustmentIndex(short index);
      float volumeAdjustment() const;
      void setVolumeAdjustment(float adjustment);
      PeakVolume peakVolume() const;
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
