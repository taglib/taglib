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

#ifndef TAGLIB_XMPROPERTIES_H
#define TAGLIB_XMPROPERTIES_H

#include <stdint.h>

#include "tstring.h"
#include "audioproperties.h"

namespace TagLib {
	namespace XM {
		class Properties : public AudioProperties {
			friend class File;
		public:
			Properties(AudioProperties::ReadStyle propertiesStyle) :
				AudioProperties(propertiesStyle),
				m_sampleLength(0),
				m_channels(0),
				m_version(0),
				m_restartPosition(0),
				m_patternCount(0),
				m_instrumentCount(0),
				m_flags(0),
				m_tempo(0),
				m_bpmSpeed(0) {}
			
			int length()     const { return 0; }
			int bitrate()    const { return 0; }
			int sampleRate() const { return 0; }
			int channels()   const { return m_channels; }

			uint16_t sampleLength()    const { return m_sampleLength; }
			uint16_t version()         const { return m_version; }
			uint16_t restartPosition() const { return m_restartPosition; }
			uint16_t patternCount()    const { return m_patternCount; }
			uint16_t instrumentCount() const { return m_instrumentCount; }
			uint16_t flags()           const { return m_flags; }
			uint16_t tempo()           const { return m_tempo; }
			uint16_t bpmSpeed()        const { return m_bpmSpeed; }

		protected:
			void setSampleLength(int sampleLength) { m_sampleLength = sampleLength; }
			void setChannels(int channels) { m_channels = channels; }

			void setVersion(uint16_t version) { m_version = version; }
			void setRestartPosition(uint16_t restartPosition) {
				m_restartPosition = restartPosition;
			}
			void setPatternCount(uint16_t patternCount) { m_patternCount = patternCount; }
			void setInstrumentCount(uint16_t instrumentCount) {
				m_instrumentCount = instrumentCount;
			}
			void setFlags   (uint16_t flags)    { m_flags = flags; }
			void setTempo   (uint16_t tempo)    { m_tempo = tempo; }
			void setBpmSpeed(uint16_t bpmSpeed) { m_bpmSpeed = bpmSpeed; }

		private:
			uint16_t m_sampleLength;
			int      m_channels;
			uint16_t m_version;
			uint16_t m_restartPosition;
			uint16_t m_patternCount;
			uint16_t m_instrumentCount;
			uint16_t m_flags;
			uint16_t m_tempo;
			uint16_t m_bpmSpeed;
		};
	}
}

#endif
