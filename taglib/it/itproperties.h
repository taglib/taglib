/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#ifndef TAGLIB_ITPROPERTIES_H
#define TAGLIB_ITPROPERTIES_H

#include "taglib.h"
#include "audioproperties.h"

namespace TagLib {
  namespace IT {
    class TAGLIB_EXPORT Properties : public AudioProperties {
      friend class File;
    public:
      /*! Flag bits. */
      enum {
        F_STEREO             =   1,
        F_VOL0_MIX_OPT       =   2,
        F_INSTRUMENTS        =   4,
        F_LINEAR_SLIDES      =   8,
        F_OLD_EFFECTS        =  16,
        F_LINK_EFFECT        =  32,
        F_MIDI_PITCH_CTRL    =  64,
        F_EMBEDDED_MIDI_CONF = 128
      };

      /*! Special bits. */
      enum {
        S_MESSAGE            = 1,
        S_EMBEDDED_MIDI_CONF = 8
      };

      Properties(AudioProperties::ReadStyle propertiesStyle);
      virtual ~Properties();
      
      int length()     const;
      int bitrate()    const;
      int sampleRate() const;
      int channels()   const;

      ushort lengthInPatterns()  const;
      bool   stereo()            const;
      ushort instrumentCount()   const;
      ushort sampleCount()       const;
      ushort patternCount()      const;
      ushort version()           const;
      ushort compatibleVersion() const;
      ushort flags()             const;
      ushort special()           const;
      uchar  globalVolume()      const;
      uchar  mixVolume()         const;
      uchar  tempo()             const;
      uchar  bpmSpeed()          const;
      uchar  panningSeparation() const;
      uchar  pitchWheelDepth()   const;

    protected:
      void setChannels(int channels);

      void setLengthInPatterns(ushort lengthInPatterns);
      void setInstrumentCount(ushort instrumentCount);
      void setSampleCount (ushort sampleCount);
      void setPatternCount(ushort patternCount);
      void setVersion     (ushort version);
      void setCompatibleVersion(ushort compatibleVersion);
      void setFlags       (ushort flags);
      void setSpecial     (ushort special);
      void setGlobalVolume(uchar globalVolume);
      void setMixVolume   (uchar mixVolume);
      void setTempo       (uchar tempo);
      void setBpmSpeed    (uchar bpmSpeed);
      void setPanningSeparation(uchar panningSeparation);
      void setPitchWheelDepth  (uchar pitchWheelDepth);

    private:
      Properties(const Properties&);
      Properties &operator=(const Properties&);

      class PropertiesPrivate;
      PropertiesPrivate *d;
    };
  }
}

#endif
