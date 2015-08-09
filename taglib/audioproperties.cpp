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

#include <tbytevector.h>

#include "aiffproperties.h"
#include "apeproperties.h"
#include "asfproperties.h"
#include "flacproperties.h"
#include "mp4properties.h"
#include "mpcproperties.h"
#include "mpegproperties.h"
#include "opusproperties.h"
#include "speexproperties.h"
#include "trueaudioproperties.h"
#include "vorbisproperties.h"
#include "wavproperties.h"
#include "wavpackproperties.h"

#include "audioproperties.h"
#include "tstringlist.h"

using namespace TagLib;

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

AudioProperties::~AudioProperties()
{
}

String AudioProperties::toString() const
{
  StringList desc;
  desc.append("Audio");
  desc.append(String::number(length()) + " seconds");
  desc.append(String::number(bitrate()) + " kbps");
  return desc.toString(", ");
}

int TagLib::AudioProperties::lengthInSeconds() const
{
  // This is an ugly workaround but we can't add a virtual function.
  // Should be virtual in taglib2.

  if(dynamic_cast<const APE::AudioProperties*>(this))
    return dynamic_cast<const APE::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const ASF::AudioProperties*>(this))
    return dynamic_cast<const ASF::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const FLAC::AudioProperties*>(this))
    return dynamic_cast<const FLAC::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const MP4::AudioProperties*>(this))
    return dynamic_cast<const MP4::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const MPC::AudioProperties*>(this))
    return dynamic_cast<const MPC::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const MPEG::AudioProperties*>(this))
    return dynamic_cast<const MPEG::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const Ogg::Opus::AudioProperties*>(this))
    return dynamic_cast<const Ogg::Opus::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const Ogg::Speex::AudioProperties*>(this))
    return dynamic_cast<const Ogg::Speex::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const TrueAudio::AudioProperties*>(this))
    return dynamic_cast<const TrueAudio::AudioProperties*>(this)->lengthInSeconds();

  else if (dynamic_cast<const RIFF::AIFF::AudioProperties*>(this))
    return dynamic_cast<const RIFF::AIFF::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const RIFF::WAV::AudioProperties*>(this))
    return dynamic_cast<const RIFF::WAV::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const Ogg::Vorbis::AudioProperties*>(this))
    return dynamic_cast<const Ogg::Vorbis::AudioProperties*>(this)->lengthInSeconds();

  else if(dynamic_cast<const WavPack::AudioProperties*>(this))
    return dynamic_cast<const WavPack::AudioProperties*>(this)->lengthInSeconds();

  else
    return 0;
}

int TagLib::AudioProperties::lengthInMilliseconds() const
{
  // This is an ugly workaround but we can't add a virtual function.
  // Should be virtual in taglib2.

  if(dynamic_cast<const APE::AudioProperties*>(this))
    return dynamic_cast<const APE::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const ASF::AudioProperties*>(this))
    return dynamic_cast<const ASF::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const FLAC::AudioProperties*>(this))
    return dynamic_cast<const FLAC::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const MP4::AudioProperties*>(this))
    return dynamic_cast<const MP4::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const MPC::AudioProperties*>(this))
    return dynamic_cast<const MPC::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const MPEG::AudioProperties*>(this))
    return dynamic_cast<const MPEG::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const Ogg::Opus::AudioProperties*>(this))
    return dynamic_cast<const Ogg::Opus::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const Ogg::Speex::AudioProperties*>(this))
    return dynamic_cast<const Ogg::Speex::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const TrueAudio::AudioProperties*>(this))
    return dynamic_cast<const TrueAudio::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const RIFF::AIFF::AudioProperties*>(this))
    return dynamic_cast<const RIFF::AIFF::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const RIFF::WAV::AudioProperties*>(this))
    return dynamic_cast<const RIFF::WAV::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const Ogg::Vorbis::AudioProperties*>(this))
    return dynamic_cast<const Ogg::Vorbis::AudioProperties*>(this)->lengthInMilliseconds();

  else if(dynamic_cast<const WavPack::AudioProperties*>(this))
    return dynamic_cast<const WavPack::AudioProperties*>(this)->lengthInMilliseconds();

  else
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

AudioProperties::AudioProperties() :
  d(0)
{
}
