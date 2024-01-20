/**************************************************************************
    copyright            : (C) 2007 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

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

#include "mp4properties.h"

#include "tdebug.h"
#include "tstring.h"
#include "mp4file.h"
#include "mp4atom.h"

using namespace TagLib;

namespace
{
  // Calculate the total bytes used by audio data, used to calculate the bitrate
  long long calculateMdatLength(const MP4::AtomList &list)
  {
    long long totalLength = 0;
    for(const auto &atom : list) {
      offset_t length = atom->length();
      if(length == 0)
        return 0; // for safety, see checkValid() in mp4file.cpp

      if(atom->name() == "mdat")
        totalLength += length;

      totalLength += calculateMdatLength(atom->children());
    }

    return totalLength;
  }
}  // namespace

class MP4::Properties::PropertiesPrivate
{
public:
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int channels { 0 };
  int bitsPerSample { 0 };
  bool encrypted { false };
  Codec codec { MP4::Properties::Unknown };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MP4::Properties::Properties(File *file, const MP4::Atoms *atoms, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(file, atoms);
}

MP4::Properties::~Properties() = default;

int
MP4::Properties::channels() const
{
  return d->channels;
}

int
MP4::Properties::sampleRate() const
{
  return d->sampleRate;
}

int
MP4::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int
MP4::Properties::bitrate() const
{
  return d->bitrate;
}

int
MP4::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

bool
MP4::Properties::isEncrypted() const
{
  return d->encrypted;
}

MP4::Properties::Codec
MP4::Properties::codec() const
{
  return d->codec;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void
MP4::Properties::read(File *file, const Atoms *atoms)
{
  MP4::Atom *moov = atoms->find("moov");
  if(!moov) {
    debug("MP4: Atom 'moov' not found");
    return;
  }

  MP4::Atom *trak = nullptr;
  ByteVector data;

  const MP4::AtomList trakList = moov->findall("trak");
  for(const auto &track : trakList) {
    const MP4::Atom *hdlr = track->find("mdia", "hdlr");
    if(!hdlr) {
      debug("MP4: Atom 'trak.mdia.hdlr' not found");
      return;
    }
    trak = track;
    file->seek(hdlr->offset());
    data = file->readBlock(hdlr->length());
    if(data.containsAt("soun", 16)) {
      break;
    }
    trak = nullptr;
  }
  if(!trak) {
    debug("MP4: No audio tracks");
    return;
  }

  const MP4::Atom *mdhd = trak->find("mdia", "mdhd");
  if(!mdhd) {
    debug("MP4: Atom 'trak.mdia.mdhd' not found");
    return;
  }

  file->seek(mdhd->offset());
  data = file->readBlock(mdhd->length());

  const unsigned int version = data.at(8);
  long long unit;
  long long length;
  if(version == 1) {
    if(data.size() < 36 + 8) {
      debug("MP4: Atom 'trak.mdia.mdhd' is smaller than expected");
      return;
    }
    unit   = data.toUInt(28U);
    length = data.toLongLong(32U);
  }
  else {
    if(data.size() < 24 + 8) {
      debug("MP4: Atom 'trak.mdia.mdhd' is smaller than expected");
      return;
    }
    unit   = data.toUInt(20U);
    length = data.toUInt(24U);
  }
  if(length == 0) {
    // No length found in the media header (mdhd), try the movie header (mvhd)
    if(const MP4::Atom *mvhd = moov->find("mvhd")) {
      file->seek(mvhd->offset());
      data = file->readBlock(mvhd->length());
      if(data.size() >= 24 + 4) {
        unit   = data.toUInt(20U);
        length = data.toUInt(24U);
      }
    }
  }
  if(unit > 0 && length > 0)
    d->length = static_cast<int>(length * 1000.0 / unit + 0.5);

  MP4::Atom *atom = trak->find("mdia", "minf", "stbl", "stsd");
  if(!atom) {
    return;
  }

  file->seek(atom->offset());
  data = file->readBlock(atom->length());
  if(data.containsAt("mp4a", 20)) {
    d->codec         = AAC;
    d->channels      = data.toShort(40U);
    d->bitsPerSample = data.toShort(42U);
    d->sampleRate    = data.toUInt(46U);
    if(data.containsAt("esds", 56) && data.at(64) == 0x03) {
      unsigned int pos = 65;
      if(data.containsAt("\x80\x80\x80", pos)) {
        pos += 3;
      }
      pos += 4;
      if(data.at(pos) == 0x04) {
        pos += 1;
        if(data.containsAt("\x80\x80\x80", pos)) {
          pos += 3;
        }
        pos += 10;
        if(const unsigned int bitrateValue = data.toUInt(pos);
           bitrateValue != 0 || d->length <= 0) {
          d->bitrate = static_cast<int>((bitrateValue + 500) / 1000.0 + 0.5);
        }
        else {
          d->bitrate = static_cast<int>(
                calculateMdatLength(atoms->atoms()) * 8 / d->length);
        }
      }
    }
  }
  else if(data.containsAt("alac", 20)) {
    if(atom->length() == 88 && data.containsAt("alac", 56)) {
      d->codec         = ALAC;
      d->bitsPerSample = data.at(69);
      d->channels      = data.at(73);
      d->bitrate       = static_cast<int>(data.toUInt(80U) / 1000.0 + 0.5);
      d->sampleRate    = data.toUInt(84U);

      if(d->bitrate == 0 && d->length > 0) {
        // There are files which do not contain a nominal bitrate, e.g. those
        // generated by refalac64.exe. Calculate the bitrate from the audio
        // data size (mdat atoms) and the duration.
        d->bitrate = static_cast<int>(calculateMdatLength(atoms->atoms()) * 8 / d->length);
      }
    }
  }

  if(atom->find("drms")) {
    d->encrypted = true;
  }
}
