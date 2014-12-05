/***************************************************************************
    copyright            : (C) 2006 by Lukáš Lalinský
    email                : lalinsky@gmail.com

    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
                           (original Vorbis implementation)
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

#include <tstring.h>
#include <tdebug.h>

#include <oggpageheader.h>

#include "speexproperties.h"
#include "speexfile.h"

using namespace TagLib;
using namespace TagLib::Ogg;

class Speex::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate(File *f, ReadStyle s) :
    file(f),
    style(s),
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    speexVersion(0),
    vbr(false),
    mode(0) {}

  File *file;
  ReadStyle style;
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int speexVersion;
  bool vbr;
  int mode;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Speex::Properties::Properties(File *file, ReadStyle style) : AudioProperties(style)
{
  d = new PropertiesPrivate(file, style);
  read();
}

Speex::Properties::~Properties()
{
  delete d;
}

int Speex::Properties::length() const
{
  return d->length;
}

int Speex::Properties::bitrate() const
{
  return int(float(d->bitrate) / float(1000) + 0.5);
}

int Speex::Properties::sampleRate() const
{
  return d->sampleRate;
}

int Speex::Properties::channels() const
{
  return d->channels;
}

int Speex::Properties::speexVersion() const
{
  return d->speexVersion;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void Speex::Properties::read()
{
  // Get the identification header from the Ogg implementation.

  ByteVector data = d->file->packet(0);

  // speex_version_id;       /**< Version for Speex (for checking compatibility) */
  d->speexVersion = data.toUInt(28, false);

  // rate;                   /**< Sampling rate used */
  d->sampleRate = data.toUInt(36, false);

  // mode;                   /**< Mode used (0 for narrowband, 1 for wideband) */
  d->mode = data.toUInt(40, false);

  // nb_channels;            /**< Number of channels encoded */
  d->channels = data.toUInt(48, false);

  // bitrate;                /**< Bit-rate used */
  d->bitrate = data.toUInt(52, false);

  // vbr;                    /**< 1 for a VBR encoding, 0 otherwise */
  d->vbr = data.toUInt(60, false) == 1;

  const Ogg::PageHeader *first = d->file->firstPageHeader();
  const Ogg::PageHeader *last = d->file->lastPageHeader();

  if(first && last) {
    long long start = first->absoluteGranularPosition();
    long long end = last->absoluteGranularPosition();

    if(start >= 0 && end >= 0 && d->sampleRate > 0)
      d->length = (int) ((end - start) / (long long) d->sampleRate);
    else
      debug("Speex::Properties::read() -- Either the PCM values for the start or "
            "end of this file was incorrect or the sample rate is zero.");
  }
  else
    debug("Speex::Properties::read() -- Could not find valid first and last Ogg pages.");
}
