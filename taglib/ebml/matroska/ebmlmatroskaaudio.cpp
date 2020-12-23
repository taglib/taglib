/***************************************************************************
    copyright            : (C) 2013 by Sebastian Rachuj
    email                : rachus@web.de
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

#include "ebmlmatroskaconstants.h"
#include "ebmlmatroskaaudio.h"

using namespace TagLib;

class EBML::Matroska::AudioProperties::AudioPropertiesPrivate
{
public:
  // Constructor
  AudioPropertiesPrivate(File *p_document) :
    document(p_document),
    length(0),
    bitrate(0),
    channels(1),
    samplerate(8000)
  {
    Element *elem = document->getDocumentRoot()->getChild(Constants::Segment);
    Element *info = elem->getChild(Constants::SegmentInfo);
    Element *value;
    
    if(info && (value = info->getChild(Constants::Duration))) {
      length = static_cast<int>(value->getAsFloat());
      if((value = info->getChild(Constants::TimecodeScale))){
        length /= (value->getAsUnsigned() / 1000);}
    }
    
    info = elem->getChild(Constants::Tracks);
    if(!info || !(info = info->getChild(Constants::TrackEntry)) ||
      !(info = info->getChild(Constants::Audio))) {
      
      return;
    }
    
    // Dirty bitrate:
    document->seek(0, File::End);
    bitrate = ((8 * document->tell()) / length) / 1000;
    
    if((value = info->getChild(Constants::Channels)))
      channels = value->getAsUnsigned();
    
    if((value = info->getChild(Constants::SamplingFrequency)))
      samplerate = static_cast<int>(value->getAsFloat());
  }
  
  // The corresponding file
  File *document;
  
  // The length of the file
  int length;
  
  // The bitrate
  int bitrate;
  
  // The amount of channels
  int channels;
  
  // The sample rate
  int samplerate;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

EBML::Matroska::AudioProperties::AudioProperties(File *document) :
  TagLib::AudioProperties(TagLib::AudioProperties::Fast),
  d(new AudioPropertiesPrivate(document))
{
}

EBML::Matroska::AudioProperties::~AudioProperties()
{
  delete d;
}

int EBML::Matroska::AudioProperties::length() const
{
  return d->length;
}

int EBML::Matroska::AudioProperties::bitrate() const
{
  return d->bitrate;
}

int EBML::Matroska::AudioProperties::channels() const
{
  return d->channels;
}

int EBML::Matroska::AudioProperties::sampleRate() const
{
  return d->samplerate;
}
