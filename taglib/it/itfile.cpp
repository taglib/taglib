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

#include "tstringlist.h"
#include "itfile.h"
#include "modfileprivate.h"

using namespace TagLib;
using namespace IT;

// Just copied this array from some example code.
// I think this might be unneccesarry and only needed if
// you convert IT to XM to keep your mod player more simple.
static const uchar AUTOVIB_IT_TO_XM[] = {0, 3, 1, 4, 2, 0, 0, 0};

class IT::File::FilePrivate
{
public:
	FilePrivate(AudioProperties::ReadStyle propertiesStyle)
		: tag(), properties(propertiesStyle)
	{
	}

	Mod::Tag       tag;
	IT::Properties properties;
};

IT::File::File(FileName file, bool readProperties,
               AudioProperties::ReadStyle propertiesStyle) :
		Mod::File(file),
		d(new FilePrivate(propertiesStyle))
{
	read(readProperties);
}

IT::File::File(IOStream *stream, bool readProperties,
               AudioProperties::ReadStyle propertiesStyle) :
		Mod::File(stream),
		d(new FilePrivate(propertiesStyle))
{
	read(readProperties);
}

IT::File::~File()
{
	delete d;
}

Mod::Tag *IT::File::tag() const
{
    return &d->tag;
}

IT::Properties *IT::File::audioProperties() const
{
    return &d->properties;
}

bool IT::File::save()
{
	seek(4);
	writeString(d->tag.title(), 26);
	// TODO: write comment as instrument and sample names
    return true;
}

void IT::File::read(bool)
{
	if(!isOpen())
		return;

	seek(0);
	READ_ASSERT(readBlock(4) == "IMPM");
	READ_STRING(d->tag.setTitle, 26);

	seek(2, Current);

	READ_U16L_AS(length);
	READ_U16L_AS(instrumentCount);
	READ_U16L_AS(sampleCount);
		
	d->properties.setSampleLength(length);
	d->properties.setInstrumentCount(instrumentCount);
	d->properties.setSampleCount(sampleCount);
	READ_U16L(d->properties.setPatternCount);
	READ_U16L(d->properties.setVersion);
	READ_U16L(d->properties.setCmwt);
	READ_U16L(d->properties.setFlags);

	READ_U16L_AS(special);

	d->properties.setSpecial(special);
	READ_U16L(d->properties.setBaseVolume);

	seek(1, Current);

	READ_BYTE(d->properties.setTempo);
	READ_BYTE(d->properties.setBpmSpeed);

	StringList comment;

	for(ushort i = 0; i < instrumentCount; ++ i)
	{
		seek(192 + length + (i << 2));
		READ_U32L_AS(instrumentOffset);
		seek(instrumentOffset);

		ByteVector instrumentMagic = readBlock(4);
		// TODO: find out if it can really be both here and not just IMPI
		READ_ASSERT(instrumentMagic == "IMPS" || instrumentMagic == "IMPI");

		READ_STRING_AS(dosFileName, 13);

		seek(15, Current);

		READ_STRING_AS(instrumentName, 26);
		comment.append(instrumentName);
	}
		
	for(ushort i = 0; i < sampleCount; ++ i)
	{
		seek(192 + length + (instrumentCount << 2) + (i << 2));
		READ_U32L_AS(sampleOffset);
		
		seek(sampleOffset);

		ByteVector sampleMagic = readBlock(4);
		// TODO: find out if it can really be both here and not just IMPS
		READ_ASSERT(sampleMagic == "IMPS" || sampleMagic == "IMPI");

		READ_STRING_AS(dosFileName, 13);
		READ_BYTE_AS(globalVolume);
		READ_BYTE_AS(sampleFlags);
		READ_BYTE_AS(sampleValume);
		READ_STRING_AS(sampleName, 26);
		READ_BYTE_AS(sampleCvt);
		READ_BYTE_AS(samplePanning);
		READ_U32L_AS(sampleLength);
		READ_U32L_AS(repeatStart);
		READ_U32L_AS(repeatStop);
		READ_U32L_AS(c4speed);
		READ_U32L_AS(sustainLoopStart);
		READ_U32L_AS(sustainLoopEnd);
		READ_U32L_AS(sampleDataOffset);
		READ_BYTE_AS(vibratoRate);
		READ_BYTE_AS(vibratoDepth);
		READ_BYTE_AS(vibratoSweep);
		READ_BYTE_AS(vibratoType);

		if(c4speed == 0)
		{
			c4speed = 8363;
		}
		else if(c4speed < 256)
		{
			c4speed = 256;
		}
			
		vibratoDepth = vibratoDepth & 0x7F;
		vibratoSweep = (vibratoSweep + 3) >> 2;
		vibratoType  = AUTOVIB_IT_TO_XM[vibratoType & 0x07];

		comment.append(sampleName);
	}

	d->tag.setComment(comment.toString("\n"));
}
