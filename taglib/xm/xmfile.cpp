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
#include "xmfile.h"
#include "modfileprivate.h"

#include <algorithm>

using namespace TagLib;
using namespace XM;

class XM::File::FilePrivate
{
public:
	FilePrivate(AudioProperties::ReadStyle propertiesStyle)
		: tag(), properties(propertiesStyle)
	{
	}

	XM::Tag        tag;
	XM::Properties properties;
};

XM::File::File(FileName file, bool readProperties,
               AudioProperties::ReadStyle propertiesStyle) :
               Mod::File(file),
			   d(new FilePrivate(propertiesStyle))
{
	read(readProperties);
}

XM::File::File(IOStream *stream, bool readProperties,
               AudioProperties::ReadStyle propertiesStyle) :
               Mod::File(stream),
			   d(new FilePrivate(propertiesStyle))
{
	read(readProperties);
}

XM::File::~File()
{
	delete d;
}

XM::Tag *XM::File::tag() const
{
    return &d->tag;
}

XM::Properties *XM::File::audioProperties() const
{
    return &d->properties;
}

bool XM::File::save()
{
	seek(17);
	writeString(d->tag.title(), 20);
	seek(1, Current);
	writeString(d->tag.trackerName(), 20);
	// TODO: write comment as instrument and sample names
    return true;
}

void XM::File::read(bool)
{
	if(!isOpen())
		return;

	READ_ASSERT(readBlock(17) == "Extended Module: ");

	READ_STRING(d->tag.setTitle, 20);
	READ_BYTE_AS(mark);
	READ_ASSERT(mark == 0x1A);
	
	READ_STRING(d->tag.setTrackerName, 20);
	READ_U16L(d->properties.setVersion);
	READ_U32L_AS(headerSize);
	READ_U16L(d->properties.setSampleLength);
	READ_U16L(d->properties.setRestartPosition);
	READ_U16L(d->properties.setChannels);
	READ_U16L_AS(patternCount);
	d->properties.setPatternCount(patternCount);
	READ_U16L_AS(instrumentCount);
	d->properties.setInstrumentCount(instrumentCount);
	READ_U16L(d->properties.setFlags);
	READ_U16L(d->properties.setTempo);
	READ_U16L(d->properties.setBpmSpeed);

	seek(60 + headerSize);
		
	for(ushort i = 0; i < patternCount; ++ i)
	{
		READ_U32L_AS(patternHeaderLength);
		READ_BYTE_AS(patternType);
		READ_U16L_AS(rowCount);
		READ_U16L_AS(patternDataSize);

		seek(patternHeaderLength - (4+1+2+2) + patternDataSize, Current);
	}
		
	StringList intrumentNames;
	StringList sampleNames;
	for(ushort i = 0; i < instrumentCount; ++ i)
	{
		long pos = tell();
		READ_U32L_AS(instrumentSize);

		String instrumentName;
		uchar instrumentType = 0;
		ushort sampleCount = 0;
			
		if(instrumentSize > 4)
		{
			if(!readString(instrumentName, std::min(22UL, instrumentSize-4)))
			{
				setValid(false);
				return;
			}

			if(instrumentSize >= (4+22+1))
			{
				if(!readByte(instrumentType))
				{
					setValid(false);
					return;
				}

				if (instrumentSize >= (4+22+1+2))
				{
					if(!readU16L(sampleCount))
					{
						setValid(false);
						return;
					}
				}
			}
		}

		ulong sumSampleLength = 0;
		ulong sampleHeaderSize = 0;
		if (sampleCount > 0)
		{
			if(!readU32L(sampleHeaderSize))
			{
				setValid(false);
				return;
			}

			seek(pos + instrumentSize);

			long sampleheaderPos = tell();
			for (ushort j = 0; j < sampleCount; ++ j)
			{
				seek(sampleheaderPos + sampleHeaderSize * j);
				READ_U32L_AS(length);
				READ_U32L_AS(loopStart);
				READ_U32L_AS(loopLength);
				READ_BYTE_AS(volume);
				READ_BYTE_AS(finetune);
				READ_BYTE_AS(sampleType);
				READ_BYTE_AS(panning);
				READ_BYTE_AS(noteNumber);
				READ_BYTE_AS(compression);
				READ_STRING_AS(sampleName, 22);
				
				sumSampleLength += length;
				sampleNames.append(sampleName);
			}
		}
		intrumentNames.append(instrumentName);
		seek(pos + instrumentSize + sampleHeaderSize * sampleCount + sumSampleLength);
	}

	d->tag.setComment(intrumentNames.toString("\n") + "\n" + sampleNames.toString("\n"));
}
