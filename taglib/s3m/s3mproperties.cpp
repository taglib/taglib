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

#include "s3mproperties.h"

using namespace TagLib;
using namespace S3M;

class S3M::Properties::PropertiesPrivate
{
public:
	PropertiesPrivate() :
		sampleLength(0),
		channels(0),
		stereo(0),
		sampleCount(0),
		patternCount(0),
		flags(0),
		version(0),
		samplesType(0),
		baseVolume(0),
		tempo(0),
		bpmSpeed(0),
		ultraClick(0),
		usePanningValues(false) {}
	
	ushort sampleLength;
	int    channels;
	bool   stereo;
	ushort sampleCount;
	ushort patternCount;
	ushort flags;
	ushort version;
	ushort samplesType;
	int    baseVolume;
	uchar  tempo;
	uchar  bpmSpeed;
	uchar  ultraClick;
	bool   usePanningValues;
};

S3M::Properties::Properties(AudioProperties::ReadStyle propertiesStyle) :
		AudioProperties(propertiesStyle),
		d(new PropertiesPrivate)
{
}

S3M::Properties::~Properties()
{
	delete d;
}

int S3M::Properties::length() const
{
	return 0;
}

int S3M::Properties::bitrate() const
{
	return 0;
}

int S3M::Properties::sampleRate() const
{
	return 0;
}

int S3M::Properties::channels() const
{
	return d->channels;
}

ushort S3M::Properties::sampleLength() const
{
	return d->sampleLength;
}

bool S3M::Properties::stereo() const
{
	return d->stereo;
}

ushort S3M::Properties::sampleCount() const
{
	return d->sampleCount;
}

ushort S3M::Properties::patternCount() const
{
	return d->patternCount;
}

ushort S3M::Properties::flags() const
{
	return d->flags;
}

ushort S3M::Properties::version() const
{
	return d->version;
}

ushort S3M::Properties::samplesType() const
{
	return d->samplesType;
}

int S3M::Properties::baseVolume() const
{
	return d->baseVolume;
}

uchar S3M::Properties::tempo() const
{
	return d->tempo;
}

uchar S3M::Properties::bpmSpeed() const
{
	return d->bpmSpeed;
}

uchar S3M::Properties::ultraClick() const
{
	return d->ultraClick;
}

bool S3M::Properties::usePanningValues() const
{
	return d->usePanningValues;
}

void S3M::Properties::setSampleLength(ushort sampleLength)
{
	d->sampleLength = sampleLength;
}

void S3M::Properties::setChannels(int channels)
{
	d->channels = channels;
}

void S3M::Properties::setStereo(bool stereo)
{
	d->stereo = stereo;
}

void S3M::Properties::setSampleCount(ushort sampleCount)
{
	d->sampleCount = sampleCount;
}

void S3M::Properties::setPatternCount(ushort patternCount)
{
	d->patternCount = patternCount;
}

void S3M::Properties::setFlags(ushort flags)
{
	d->flags = flags;
}

void S3M::Properties::setVersion(ushort version)
{
	d->version = version;
}

void S3M::Properties::setSamplesType(ushort samplesType)
{
	d->samplesType = samplesType;
}

void S3M::Properties::setBaseVolume(int baseVolume)
{
	d->baseVolume = baseVolume;
}

void S3M::Properties::setTempo(uchar tempo)
{
	d->tempo = tempo;
}

void S3M::Properties::setBpmSpeed(uchar bpmSpeed)
{
	d->bpmSpeed = bpmSpeed;
}

void S3M::Properties::setUltraClick(uchar ultraClick)
{
	d->ultraClick = ultraClick;
}

void S3M::Properties::setUsePanningValues(bool usePanningValues)
{
	d->usePanningValues = usePanningValues;
}
