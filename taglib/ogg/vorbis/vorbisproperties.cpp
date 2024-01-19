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

#include "vorbisproperties.h"

#include "tstring.h"
#include "tdebug.h"
#include "oggpageheader.h"
#include "vorbisfile.h"

using namespace TagLib;

class Vorbis::Properties::PropertiesPrivate
{
public:
  int length { 0 };
  int bitrate { 0 };
  int sampleRate { 0 };
  int channels { 0 };
  int vorbisVersion { 0 };
  int bitrateMaximum { 0 };
  int bitrateNominal { 0 };
  int bitrateMinimum { 0 };
};

namespace TagLib {
  /*!
   * Vorbis headers can be found with one type ID byte and the string "vorbis" in
   * an Ogg stream.  0x01 indicates the setup header.
   */
  static constexpr char vorbisSetupHeaderID[] = { 0x01, 'v', 'o', 'r', 'b', 'i', 's', 0 };
} // namespace TagLib

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Vorbis::Properties::Properties(File *file, ReadStyle style) :
  AudioProperties(style),
  d(std::make_unique<PropertiesPrivate>())
{
  read(file);
}

Vorbis::Properties::~Properties() = default;

int Vorbis::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int Vorbis::Properties::bitrate() const
{
  return d->bitrate;
}

int Vorbis::Properties::sampleRate() const
{
  return d->sampleRate;
}

int Vorbis::Properties::channels() const
{
  return d->channels;
}

int Vorbis::Properties::vorbisVersion() const
{
  return d->vorbisVersion;
}

int Vorbis::Properties::bitrateMaximum() const
{
  return d->bitrateMaximum;
}

int Vorbis::Properties::bitrateNominal() const
{
  return d->bitrateNominal;
}

int Vorbis::Properties::bitrateMinimum() const
{
  return d->bitrateMinimum;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void Vorbis::Properties::read(File *file)
{
  // Get the identification header from the Ogg implementation.

  const ByteVector data = file->packet(0);
  if(data.size() < 28) {
    debug("Vorbis::Properties::read() -- data is too short.");
    return;
  }

  unsigned int pos = 0;

  if(data.mid(pos, 7) != vorbisSetupHeaderID) {
    debug("Vorbis::Properties::read() -- invalid Vorbis identification header");
    return;
  }

  pos += 7;

  d->vorbisVersion = data.toUInt(pos, false);
  pos += 4;

  d->channels = static_cast<unsigned char>(data[pos]);
  pos += 1;

  d->sampleRate = data.toUInt(pos, false);
  pos += 4;

  d->bitrateMaximum = data.toUInt(pos, false);
  pos += 4;

  d->bitrateNominal = data.toUInt(pos, false);
  pos += 4;

  d->bitrateMinimum = data.toUInt(pos, false);

  // Find the length of the file.  See http://wiki.xiph.org/VorbisStreamLength/
  // for my notes on the topic.

  const Ogg::PageHeader *first = file->firstPageHeader();
  const Ogg::PageHeader *last  = file->lastPageHeader();

  if(first && last) {
    const long long start = first->absoluteGranularPosition();
    const long long end   = last->absoluteGranularPosition();

    if(start >= 0 && end >= 0 && d->sampleRate > 0) {
      if(const long long frameCount = end - start; frameCount > 0) {
        const auto length = static_cast<double>(frameCount) * 1000.0 / d->sampleRate;
        offset_t fileLengthWithoutOverhead = file->length();
        // Ignore the three initial header packets, see "1.3.1. Decode Setup" in
        // https://xiph.org/vorbis/doc/Vorbis_I_spec.html
        for (unsigned int i = 0; i < 3; ++i) {
          fileLengthWithoutOverhead -= file->packet(i).size();
        }
        d->length  = static_cast<int>(length + 0.5);
        d->bitrate = static_cast<int>(fileLengthWithoutOverhead * 8.0 / length + 0.5);
      }
    }
    else {
      debug("Vorbis::Properties::read() -- Either the PCM values for the start or "
            "end of this file was incorrect or the sample rate is zero.");
    }
  }
  else
    debug("Vorbis::Properties::read() -- Could not find valid first and last Ogg pages.");

  // Alternative to the actual average bitrate.

  if(d->bitrate == 0 && d->bitrateNominal > 0)
    d->bitrate = static_cast<int>(d->bitrateNominal / 1000.0 + 0.5);
}
