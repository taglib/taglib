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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tdebug.h>
#include <tstring.h>
#include "mp4file.h"
#include "mp4atom.h"
#include "mp4properties.h"

using namespace TagLib;

class MP4::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() : length(0), bitrate(0), sampleRate(0), channels(0), bitsPerSample(0), encrypted(false), format(Unknown) {}

  enum Format {
    Unknown = 0,
    AAC = 1,
    ALAC = 2,
  };

  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int bitsPerSample;
  bool encrypted;
  Format format;
};

MP4::Properties::Properties(File *file, MP4::Atoms *atoms, ReadStyle style)
  : AudioProperties(style)
{
  d = new PropertiesPrivate;

  MP4::Atom *moov = atoms->find("moov");
  if(!moov) {
    debug("MP4: Atom 'moov' not found");
    return;
  }

  MP4::Atom *trak = 0;
  ByteVector data;

  MP4::AtomList trakList = moov->findall("trak");
  for (unsigned int i = 0; i < trakList.size(); i++) {
    trak = trakList[i];
    MP4::Atom *hdlr = trak->find("mdia", "hdlr");
    if(!hdlr) {
      debug("MP4: Atom 'trak.mdia.hdlr' not found");
      return;
    }
    file->seek(hdlr->offset);
    data = file->readBlock(hdlr->length);
    if(data.mid(16, 4) == "soun") {
      break;
    }
    trak = 0;
  }
  if (!trak) {
    debug("MP4: No audio tracks");
    return;
  }

  MP4::Atom *mdhd = trak->find("mdia", "mdhd");
  if(!mdhd) {
    debug("MP4: Atom 'trak.mdia.mdhd' not found");
    return;
  }

  file->seek(mdhd->offset);
  data = file->readBlock(mdhd->length);
  uint version = data[8];
  if(version == 1) {
    if (data.size() < 36 + 8) {
      debug("MP4: Atom 'trak.mdia.mdhd' is smaller than expected");
      return;
    }
    long long unit = data.mid(28, 8).toInt64();
    long long length = data.mid(36, 8).toInt64();
    d->length = unit ? int(length / unit) : 0;
  }
  else {
    if (data.size() < 24 + 4) {
      debug("MP4: Atom 'trak.mdia.mdhd' is smaller than expected");
      return;
    }
    unsigned int unit = data.mid(20, 4).toUInt32();
    unsigned int length = data.mid(24, 4).toUInt32();
    d->length = unit ? length / unit : 0;
  }

  MP4::Atom *atom = trak->find("mdia", "minf", "stbl", "stsd");
  if(!atom) {
    return;
  }

  file->seek(atom->offset);
  data = file->readBlock(atom->length);
  if(data.mid(20, 4) == "mp4a") {
    d->format = PropertiesPrivate::AAC;
    d->channels = data.mid(40, 2).toInt16();
    d->bitsPerSample = data.mid(42, 2).toInt16();
    d->sampleRate = data.mid(46, 4).toUInt32();
    if(data.mid(56, 4) == "esds" && data[64] == 0x03) {
      long pos = 65;
      if(data.mid(pos, 3) == "\x80\x80\x80") {
        pos += 3;
      }
      pos += 4;
      if(data[pos] == 0x04) {
        pos += 1;
        if(data.mid(pos, 3) == "\x80\x80\x80") {
          pos += 3;
        }
        pos += 10;
        d->bitrate = (data.mid(pos, 4).toUInt32() + 500) / 1000;
      }
    }
  }
  else if (data.mid(20, 4) == "alac") {
    d->format = PropertiesPrivate::ALAC;
    if (atom->length == 88 && data.mid(56, 4) == "alac") {
      d->bitsPerSample = data.at(69);
      d->channels = data.at(73);
      d->bitrate = data.mid(80, 4).toUInt32() / 1000;
      d->sampleRate = data.mid(84, 4).toUInt32();
    }
  }

  MP4::Atom *drms = atom->find("drms");
  if(drms) {
    d->encrypted = true;
  }
}

MP4::Properties::~Properties()
{
  delete d;
}

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
MP4::Properties::length() const
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

String
MP4::Properties::toString() const
{
  String format;
  if(d->format == PropertiesPrivate::AAC) {
    format = "AAC";
  }
  else if(d->format == PropertiesPrivate::ALAC) {
    format = "ALAC";
  }
  else {
    format = "Unknown";
  }
  StringList desc;
  desc.append("MPEG-4 audio (" + format + ")");
  desc.append(String::number(length()) + " seconds");
  desc.append(String::number(bitrate()) + " kbps");
  return desc.toString(", ");
}

