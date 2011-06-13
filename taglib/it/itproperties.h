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

#include <stdint.h>

#include "audioproperties.h"

namespace TagLib {
	namespace IT {
		class TAGLIB_EXPORT Properties : public AudioProperties {
			friend class File;
		public:
			Properties(AudioProperties::ReadStyle propertiesStyle) :
				AudioProperties(propertiesStyle),
				m_sampleLength(0),
				m_stereo(false),
				m_instrumentCount(0),
				m_sampleCount(0),
				m_patternCount(0),
				m_version(0),
				m_cmwt(0),
				m_flags(0),
				m_special(0),
				m_baseVolume(0),
				m_tempo(0),
				m_bpmSpeed(0) {}
			
			int length()     const { return 0; }
			int bitrate()    const { return 0; }
			int sampleRate() const { return 0; }
			int channels()   const { return m_stereo ? 2 : 1; }

			uint16_t sampleLength()    const { return m_sampleLength; }
			bool     stereo()          const { return m_stereo; }
			uint16_t instrumentCount() const { return m_instrumentCount; }
			uint16_t sampleCount()     const { return m_sampleCount; }
			uint16_t patternCount()    const { return m_patternCount; }
			uint16_t version()         const { return m_version; }
			uint16_t cmwt()            const { return m_cmwt; }
			uint16_t flags()           const { return m_flags; }
			uint16_t special()         const { return m_special; }
			int      baseVolume()      const { return m_baseVolume; }
			uint8_t  tempo()           const { return m_tempo; }
			uint8_t  bpmSpeed()        const { return m_bpmSpeed; }

		protected:
			void setSampleLength(uint16_t sampleLength) { m_sampleLength = sampleLength; }
			void setStereo(bool stereo) { m_stereo = stereo; }

			void setInstrumentCount (uint16_t instrumentCount) {
				m_instrumentCount = instrumentCount;
			}
			void setSampleCount (uint16_t sampleCount)  { m_sampleCount = sampleCount; }
			void setPatternCount(uint16_t patternCount) { m_patternCount = patternCount; }
			void setFlags       (uint16_t flags)        { m_flags = flags; }
			void setSpecial     (uint16_t special)      { m_special = special; }
			void setCmwt        (uint16_t cmwt)         { m_cmwt = cmwt; }
			void setVersion     (uint16_t version)      { m_version = version; }
			void setBaseVolume  (int baseVolume)        { m_baseVolume = baseVolume; }
			void setTempo       (uint8_t tempo)         { m_tempo = tempo; }
			void setBpmSpeed    (uint8_t bpmSpeed)      { m_bpmSpeed = bpmSpeed; }

		private:
			uint16_t m_sampleLength;
			bool     m_stereo;
			uint16_t m_instrumentCount;
			uint16_t m_sampleCount;
			uint16_t m_patternCount;
			uint16_t m_version;
			uint16_t m_cmwt;
			uint16_t m_flags;
			uint16_t m_special;
			int      m_baseVolume;
			uint8_t  m_tempo;
			uint8_t  m_bpmSpeed;
		};
	}
}

#endif
