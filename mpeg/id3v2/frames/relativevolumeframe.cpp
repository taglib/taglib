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

#include "relativevolumeframe.h"

using namespace TagLib;
using namespace ID3v2;

class RelativeVolumeFrame::RelativeVolumeFramePrivate
{
public:
  RelativeVolumeFramePrivate() : channelType(Other), volumeAdjustment(0) {}

  String identification;
  ChannelType channelType;
  short volumeAdjustment;
  PeakVolume peakVolume;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

RelativeVolumeFrame::RelativeVolumeFrame(const ByteVector &data) : Frame(data)
{
  d = new RelativeVolumeFramePrivate;
  setData(data);
}

RelativeVolumeFrame::~RelativeVolumeFrame()
{
  delete d;
}

String RelativeVolumeFrame::toString() const
{
  return d->identification;
}

RelativeVolumeFrame::ChannelType RelativeVolumeFrame::channelType() const
{
  return d->channelType;
}

void RelativeVolumeFrame::setChannelType(ChannelType t)
{
  d->channelType = t;
}

short RelativeVolumeFrame::volumeAdjustmentIndex() const
{
  return d->volumeAdjustment;
}

void RelativeVolumeFrame::setVolumeAdjustmentIndex(short index)
{
  d->volumeAdjustment = index;
}

float RelativeVolumeFrame::volumeAdjustment() const
{
  return float(d->volumeAdjustment) / float(512);
}

void RelativeVolumeFrame::setVolumeAdjustment(float adjustment)
{
  d->volumeAdjustment = short(adjustment / float(512));
}

RelativeVolumeFrame::PeakVolume RelativeVolumeFrame::peakVolume() const
{
  return d->peakVolume;
}

void RelativeVolumeFrame::setPeakVolume(const PeakVolume &peak)
{
  d->peakVolume = peak;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void RelativeVolumeFrame::parseFields(const ByteVector &data)
{
  int pos = data.find(textDelimiter(String::Latin1));
  d->identification = String(data.mid(0, pos), String::Latin1);

  d->volumeAdjustment = data.mid(pos, 2).toShort();
  pos += 2;

  d->peakVolume.bitsRepresentingPeak = data[pos];
  pos += 1;

  d->peakVolume.peakVolume = data.mid(pos, d->peakVolume.bitsRepresentingPeak);
}

ByteVector RelativeVolumeFrame::renderFields() const
{
  ByteVector data;

  data.append(d->identification.data(String::Latin1));
  data.append(textDelimiter(String::Latin1));
  data.append(ByteVector::fromShort(d->volumeAdjustment));
  data.append(char(d->peakVolume.bitsRepresentingPeak));
  data.append(d->peakVolume.peakVolume);

  return data;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

RelativeVolumeFrame::RelativeVolumeFrame(const ByteVector &data, Header *h) : Frame(h)
{
  d = new RelativeVolumeFramePrivate;
  parseFields(fieldData(data));
}
