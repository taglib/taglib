/***************************************************************************
    copyright            : (C) 2003 by Scott Wheeler
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

#ifdef HAVE_CONFIG_H
# include  "config.h"
#endif

#include <cstdlib>
#include "fileref.h"
#include "tfile.h"
#include "asffile.h"
#include "vorbisfile.h"
#include "mpegfile.h"
#include "flacfile.h"
#include "oggflacfile.h"
#include "mpcfile.h"
#include "wavpackfile.h"
#include "speexfile.h"
#include "trueaudiofile.h"
#include "mp4file.h"
#include "tag.h"
#include <cstring>
#include "id3v2framefactory.h"
#include "tpropertymap.h"

#include "tag_c.h"

using namespace TagLib;

namespace
{
  List<char *> strings;
  bool unicodeStrings = true;
  bool stringManagementEnabled = true;

  char *stringToCharArray(const String &s)
  {
    const std::string str = s.to8Bit(unicodeStrings);

#ifdef HAVE_ISO_STRDUP

    return ::_strdup(str.c_str());

#else

    return ::strdup(str.c_str());

#endif
  }

  String charArrayToString(const char *s)
  {
    return String(s, unicodeStrings ? String::UTF8 : String::Latin1);
  }
}  // namespace

void taglib_set_strings_unicode(BOOL unicode)
{
  unicodeStrings = (unicode != 0);
}

void taglib_set_string_management_enabled(BOOL management)
{
  stringManagementEnabled = (management != 0);
}

void taglib_free(void* pointer)
{
  free(pointer);
}

////////////////////////////////////////////////////////////////////////////////
// TagLib::File wrapper
////////////////////////////////////////////////////////////////////////////////

TagLib_File *taglib_file_new(const char *filename)
{
  return reinterpret_cast<TagLib_File *>(FileRef::create(filename));
}

TagLib_File *taglib_file_new_type(const char *filename, TagLib_File_Type type)
{
  switch(type) {
  case TagLib_File_MPEG:
    return reinterpret_cast<TagLib_File *>(new MPEG::File(filename));
  case TagLib_File_OggVorbis:
    return reinterpret_cast<TagLib_File *>(new Ogg::Vorbis::File(filename));
  case TagLib_File_FLAC:
    return reinterpret_cast<TagLib_File *>(new FLAC::File(filename));
  case TagLib_File_MPC:
    return reinterpret_cast<TagLib_File *>(new MPC::File(filename));
  case TagLib_File_OggFlac:
    return reinterpret_cast<TagLib_File *>(new Ogg::FLAC::File(filename));
  case TagLib_File_WavPack:
    return reinterpret_cast<TagLib_File *>(new WavPack::File(filename));
  case TagLib_File_Speex:
    return reinterpret_cast<TagLib_File *>(new Ogg::Speex::File(filename));
  case TagLib_File_TrueAudio:
    return reinterpret_cast<TagLib_File *>(new TrueAudio::File(filename));
  case TagLib_File_MP4:
    return reinterpret_cast<TagLib_File *>(new MP4::File(filename));
  case TagLib_File_ASF:
    return reinterpret_cast<TagLib_File *>(new ASF::File(filename));
  default:
    return 0;
  }
}

void taglib_file_free(TagLib_File *file)
{
  delete reinterpret_cast<File *>(file);
}

BOOL taglib_file_is_valid(const TagLib_File *file)
{
  return reinterpret_cast<const File *>(file)->isValid();
}

TagLib_Tag *taglib_file_tag(const TagLib_File *file)
{
  auto f = reinterpret_cast<const File *>(file);
  return reinterpret_cast<TagLib_Tag *>(f->tag());
}

const TagLib_AudioProperties *taglib_file_audioproperties(const TagLib_File *file)
{
  auto f = reinterpret_cast<const File *>(file);
  return reinterpret_cast<const TagLib_AudioProperties *>(f->audioProperties());
}

BOOL taglib_file_save(TagLib_File *file)
{
  return reinterpret_cast<File *>(file)->save();
}

////////////////////////////////////////////////////////////////////////////////
// TagLib::Tag wrapper
////////////////////////////////////////////////////////////////////////////////

char *taglib_tag_title(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->title());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_artist(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->artist());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_album(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->album());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_comment(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->comment());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_genre(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->genre());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

unsigned int taglib_tag_year(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  return t->year();
}

unsigned int taglib_tag_track(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  return t->track();
}

void taglib_tag_set_title(TagLib_Tag *tag, const char *title)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setTitle(charArrayToString(title));
}

void taglib_tag_set_artist(TagLib_Tag *tag, const char *artist)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setArtist(charArrayToString(artist));
}

void taglib_tag_set_album(TagLib_Tag *tag, const char *album)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setAlbum(charArrayToString(album));
}

void taglib_tag_set_comment(TagLib_Tag *tag, const char *comment)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setComment(charArrayToString(comment));
}

void taglib_tag_set_genre(TagLib_Tag *tag, const char *genre)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setGenre(charArrayToString(genre));
}

void taglib_tag_set_year(TagLib_Tag *tag, unsigned int year)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setYear(year);
}

void taglib_tag_set_track(TagLib_Tag *tag, unsigned int track)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setTrack(track);
}

void taglib_tag_free_strings()
{
  if(!stringManagementEnabled)
    return;

  for(auto it = strings.cbegin(); it != strings.cend(); ++it)
    free(*it);
  strings.clear();
}

////////////////////////////////////////////////////////////////////////////////
// TagLib::AudioProperties wrapper
////////////////////////////////////////////////////////////////////////////////

int taglib_audioproperties_length(const TagLib_AudioProperties *audioProperties)
{
  auto p = reinterpret_cast<const AudioProperties *>(audioProperties);
  return p->lengthInSeconds();
}

int taglib_audioproperties_bitrate(const TagLib_AudioProperties *audioProperties)
{
  auto p = reinterpret_cast<const AudioProperties *>(audioProperties);
  return p->bitrate();
}

int taglib_audioproperties_samplerate(const TagLib_AudioProperties *audioProperties)
{
  auto p = reinterpret_cast<const AudioProperties *>(audioProperties);
  return p->sampleRate();
}

int taglib_audioproperties_channels(const TagLib_AudioProperties *audioProperties)
{
  auto p = reinterpret_cast<const AudioProperties *>(audioProperties);
  return p->channels();
}

void taglib_id3v2_set_default_text_encoding(TagLib_ID3v2_Encoding encoding)
{
  String::Type type = String::Latin1;

  switch(encoding)
  {
  case TagLib_ID3v2_Latin1:
    type = String::Latin1;
    break;
  case TagLib_ID3v2_UTF16:
    type = String::UTF16;
    break;
  case TagLib_ID3v2_UTF16BE:
    type = String::UTF16BE;
    break;
  case TagLib_ID3v2_UTF8:
    type = String::UTF8;
    break;
  }

  ID3v2::FrameFactory::instance()->setDefaultTextEncoding(type);
}


/******************************************************************************
 * Properties API
 ******************************************************************************/
namespace {

void _taglib_property_set(TagLib_File *file, const char* prop, const char* value, bool append)
{
  if(file == NULL || prop == NULL)
    return;

  auto tfile = reinterpret_cast<File *>(file);
  PropertyMap map = tfile->tag()->properties();

  if(value) {
    auto property = map.find(prop);
    if(property == map.end()) {
      map.insert(prop, StringList(value));
    }
    else {
      if(append) {
        property->second.append(value);
      }
      else {
        property->second = StringList(value);
      }
    }
  }
  else {
    map.erase(prop);
  }

  tfile->setProperties(map);
}

}  // namespace

void taglib_property_set(TagLib_File *f, const char *prop, const char *value)
{
  _taglib_property_set(f, prop, value, false);
}

void taglib_property_set_append(TagLib_File *f, const char *prop, const char *value)
{
  _taglib_property_set(f, prop, value, true);
}

char** taglib_property_keys(TagLib_File *file)
{
  if(file == NULL)
    return NULL;

  const PropertyMap map = reinterpret_cast<const File *>(file)->properties();
  if(map.isEmpty())
    return NULL;

  auto props = static_cast<char **>(malloc(sizeof(char *) * (map.size() + 1)));
  char **pp = props;

  for(const auto &i : map) {
    *pp++ = stringToCharArray(i.first);
  }
  *pp = NULL;

  return props;
}

char **taglib_property_get(TagLib_File *file, const char *prop)
{
  if(file == NULL || prop == NULL)
    return NULL;

  const PropertyMap map = reinterpret_cast<const File *>(file)->properties();

  auto property = map.find(prop);
  if(property == map.end())
    return NULL;

  auto props = static_cast<char **>(malloc(sizeof(char *) * (property->second.size() + 1)));
  char **pp = props;

  for(const auto &i : property->second) {
    *pp++ = stringToCharArray(i);
  }
  *pp = NULL;

  return props;
}

void taglib_property_free(char **props)
{
  if(props == NULL)
    return;

  char **p = props;
  while(*p) {
      free(*p++);
  }
  free(props);
}
