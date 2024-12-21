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

#include "tag_c.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <utility>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "tstringlist.h"
#include "tbytevectorstream.h"
#include "tiostream.h"
#include "tfile.h"
#include "tpropertymap.h"
#include "fileref.h"
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
#include "aifffile.h"
#include "wavfile.h"
#include "apefile.h"
#include "itfile.h"
#include "modfile.h"
#include "s3mfile.h"
#include "xmfile.h"
#include "opusfile.h"
#include "dsffile.h"
#include "dsdifffile.h"
#include "shortenfile.h"
#include "tag.h"
#include "id3v2framefactory.h"

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
// TagLib::IOStream wrapper
////////////////////////////////////////////////////////////////////////////////

TagLib_IOStream *taglib_memory_iostream_new(const char *data, unsigned int size)
{
  return reinterpret_cast<TagLib_IOStream *>(
    new ByteVectorStream(ByteVector(data, size)));
}

void taglib_iostream_free(TagLib_IOStream *stream)
{
  delete reinterpret_cast<IOStream *>(stream);
}

////////////////////////////////////////////////////////////////////////////////
// TagLib::FileRef wrapper
////////////////////////////////////////////////////////////////////////////////

TagLib_File *taglib_file_new(const char *filename)
{
  return reinterpret_cast<TagLib_File *>(new FileRef(filename));
}

TagLib_File *taglib_file_new_type(const char *filename, TagLib_File_Type type)
{
  File *file = NULL;
  switch(type) {
  case TagLib_File_MPEG:
    file = new MPEG::File(filename);
      break;
  case TagLib_File_OggVorbis:
    file = new Ogg::Vorbis::File(filename);
    break;
  case TagLib_File_FLAC:
    file = new FLAC::File(filename);
    break;
  case TagLib_File_MPC:
    file = new MPC::File(filename);
    break;
  case TagLib_File_OggFlac:
    file = new Ogg::FLAC::File(filename);
    break;
  case TagLib_File_WavPack:
    file = new WavPack::File(filename);
    break;
  case TagLib_File_Speex:
    file = new Ogg::Speex::File(filename);
    break;
  case TagLib_File_TrueAudio:
    file = new TrueAudio::File(filename);
    break;
  case TagLib_File_MP4:
    file = new MP4::File(filename);
    break;
  case TagLib_File_ASF:
    file = new ASF::File(filename);
    break;
  case TagLib_File_AIFF:
    file = new RIFF::AIFF::File(filename);
    break;
  case TagLib_File_WAV:
    file = new RIFF::WAV::File(filename);
    break;
  case TagLib_File_APE:
    file = new APE::File(filename);
    break;
  case TagLib_File_IT:
    file = new IT::File(filename);
    break;
  case TagLib_File_Mod:
    file = new Mod::File(filename);
    break;
  case TagLib_File_S3M:
    file = new S3M::File(filename);
    break;
  case TagLib_File_XM:
    file = new XM::File(filename);
    break;
  case TagLib_File_Opus:
    file = new Ogg::Opus::File(filename);
    break;
  case TagLib_File_DSF:
    file = new DSF::File(filename);
    break;
  case TagLib_File_DSDIFF:
    file = new DSDIFF::File(filename);
    break;
  case TagLib_File_SHORTEN:
    file = new Shorten::File(filename);
    break;
  default:
    break;
  }
  return file ? reinterpret_cast<TagLib_File *>(new FileRef(file)) : NULL;
}

TagLib_File *taglib_file_new_iostream(TagLib_IOStream *stream)
{
  return reinterpret_cast<TagLib_File *>(
    new FileRef(reinterpret_cast<IOStream *>(stream)));
}

void taglib_file_free(TagLib_File *file)
{
  delete reinterpret_cast<FileRef *>(file);
}

BOOL taglib_file_is_valid(const TagLib_File *file)
{
  return !reinterpret_cast<const FileRef *>(file)->isNull();
}

TagLib_Tag *taglib_file_tag(const TagLib_File *file)
{
  auto f = reinterpret_cast<const FileRef *>(file);
  return reinterpret_cast<TagLib_Tag *>(f->tag());
}

const TagLib_AudioProperties *taglib_file_audioproperties(const TagLib_File *file)
{
  auto f = reinterpret_cast<const FileRef *>(file);
  return reinterpret_cast<const TagLib_AudioProperties *>(f->audioProperties());
}

BOOL taglib_file_save(TagLib_File *file)
{
  return reinterpret_cast<FileRef *>(file)->save();
}

////////////////////////////////////////////////////////////////////////////////
// TagLib::Tag wrapper
////////////////////////////////////////////////////////////////////////////////

char *taglib_tag_title(const TagLib_Tag *tag)
{
  auto t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->title());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_artist(const TagLib_Tag *tag)
{
  auto t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->artist());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_album(const TagLib_Tag *tag)
{
  auto t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->album());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_comment(const TagLib_Tag *tag)
{
  auto t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->comment());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_genre(const TagLib_Tag *tag)
{
  auto t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->genre());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

unsigned int taglib_tag_year(const TagLib_Tag *tag)
{
  auto t = reinterpret_cast<const Tag *>(tag);
  return t->year();
}

unsigned int taglib_tag_track(const TagLib_Tag *tag)
{
  auto t = reinterpret_cast<const Tag *>(tag);
  return t->track();
}

void taglib_tag_set_title(TagLib_Tag *tag, const char *title)
{
  auto t = reinterpret_cast<Tag *>(tag);
  t->setTitle(charArrayToString(title));
}

void taglib_tag_set_artist(TagLib_Tag *tag, const char *artist)
{
  auto t = reinterpret_cast<Tag *>(tag);
  t->setArtist(charArrayToString(artist));
}

void taglib_tag_set_album(TagLib_Tag *tag, const char *album)
{
  auto t = reinterpret_cast<Tag *>(tag);
  t->setAlbum(charArrayToString(album));
}

void taglib_tag_set_comment(TagLib_Tag *tag, const char *comment)
{
  auto t = reinterpret_cast<Tag *>(tag);
  t->setComment(charArrayToString(comment));
}

void taglib_tag_set_genre(TagLib_Tag *tag, const char *genre)
{
  auto t = reinterpret_cast<Tag *>(tag);
  t->setGenre(charArrayToString(genre));
}

void taglib_tag_set_year(TagLib_Tag *tag, unsigned int year)
{
  auto t = reinterpret_cast<Tag *>(tag);
  t->setYear(year);
}

void taglib_tag_set_track(TagLib_Tag *tag, unsigned int track)
{
  auto t = reinterpret_cast<Tag *>(tag);
  t->setTrack(track);
}

void taglib_tag_free_strings()
{
  if(!stringManagementEnabled)
    return;

  for(auto &string : std::as_const(strings))
    free(string);
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

  auto tfile = reinterpret_cast<FileRef *>(file);
  PropertyMap map = tfile->tag()->properties();

  if(value) {
    auto property = map.find(prop);
    if(property == map.end()) {
      map.insert(prop, StringList(charArrayToString(value)));
    }
    else {
      if(append) {
        property->second.append(charArrayToString(value));
      }
      else {
        property->second = StringList(charArrayToString(value));
      }
    }
  }
  else {
    map.erase(prop);
  }

  tfile->setProperties(map);
}

}  // namespace

void taglib_property_set(TagLib_File *file, const char *prop, const char *value)
{
  _taglib_property_set(file, prop, value, false);
}

void taglib_property_set_append(TagLib_File *file, const char *prop, const char *value)
{
  _taglib_property_set(file, prop, value, true);
}

char** taglib_property_keys(const TagLib_File *file)
{
  if(file == NULL)
    return NULL;

  const PropertyMap map = reinterpret_cast<const FileRef *>(file)->properties();
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

char **taglib_property_get(const TagLib_File *file, const char *prop)
{
  if(file == NULL || prop == NULL)
    return NULL;

  const PropertyMap map = reinterpret_cast<const FileRef *>(file)->properties();

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


/******************************************************************************
 * Complex Properties API
 ******************************************************************************/

namespace {

bool _taglib_complex_property_set(
  TagLib_File *file, const char *key,
  const TagLib_Complex_Property_Attribute **value, bool append)
{
  if(file == NULL || key == NULL)
    return false;

  auto tfile = reinterpret_cast<FileRef *>(file);

  if(value == NULL) {
    return tfile->setComplexProperties(key, {});
  }

  VariantMap map;
  const TagLib_Complex_Property_Attribute** attrPtr = value;
  while(*attrPtr) {
    const TagLib_Complex_Property_Attribute *attr = *attrPtr;
    String attrKey(attr->key);
    switch(attr->value.type) {
    case TagLib_Variant_Void:
      map.insert(attrKey, Variant());
      break;
    case TagLib_Variant_Bool:
      map.insert(attrKey, attr->value.value.boolValue != 0);
      break;
    case TagLib_Variant_Int:
      map.insert(attrKey, attr->value.value.intValue);
      break;
    case TagLib_Variant_UInt:
      map.insert(attrKey, attr->value.value.uIntValue);
      break;
    case TagLib_Variant_LongLong:
      map.insert(attrKey, attr->value.value.longLongValue);
      break;
    case TagLib_Variant_ULongLong:
      map.insert(attrKey, attr->value.value.uLongLongValue);
      break;
    case TagLib_Variant_Double:
      map.insert(attrKey, attr->value.value.doubleValue);
      break;
    case TagLib_Variant_String:
      map.insert(attrKey, charArrayToString(attr->value.value.stringValue));
      break;
    case TagLib_Variant_StringList: {
      StringList strs;
      if(attr->value.value.stringListValue) {
        char **s = attr->value.value.stringListValue;;
        while(*s) {
          strs.append(charArrayToString(*s++));
        }
      }
      map.insert(attrKey, strs);
      break;
    }
    case TagLib_Variant_ByteVector:
      map.insert(attrKey, ByteVector(attr->value.value.byteVectorValue,
                                     attr->value.size));
      break;
    }
    ++attrPtr;
  }

  return append ? tfile->setComplexProperties(key, tfile->complexProperties(key).append(map))
                : tfile->setComplexProperties(key, {map});
}

}  // namespace

BOOL taglib_complex_property_set(
  TagLib_File *file, const char *key,
  const TagLib_Complex_Property_Attribute **value)
{
  return _taglib_complex_property_set(file, key, value, false);
}

BOOL taglib_complex_property_set_append(
  TagLib_File *file, const char *key,
  const TagLib_Complex_Property_Attribute **value)
{
  return _taglib_complex_property_set(file, key, value, true);
}

char** taglib_complex_property_keys(const TagLib_File *file)
{
  if(file == NULL) {
    return NULL;
  }

  const StringList strs = reinterpret_cast<const FileRef *>(file)->complexPropertyKeys();
  if(strs.isEmpty()) {
    return NULL;
  }

  auto keys = static_cast<char **>(malloc(sizeof(char *) * (strs.size() + 1)));
  char **keyPtr = keys;

  for(const auto &str : strs) {
    *keyPtr++ = stringToCharArray(str);
  }
  *keyPtr = NULL;

  return keys;
}

TagLib_Complex_Property_Attribute*** taglib_complex_property_get(
  const TagLib_File *file, const char *key)
{
  if(file == NULL || key == NULL) {
    return NULL;
  }

  const auto variantMaps = reinterpret_cast<const FileRef *>(file)->complexProperties(key);
  if(variantMaps.isEmpty()) {
    return NULL;
  }

  auto props = static_cast<TagLib_Complex_Property_Attribute ***>(
    malloc(sizeof(TagLib_Complex_Property_Attribute **) * (variantMaps.size() + 1)));
  TagLib_Complex_Property_Attribute ***propPtr = props;

  for(const auto &variantMap : variantMaps) {
    if(!variantMap.isEmpty()) {
      auto attrs = static_cast<TagLib_Complex_Property_Attribute **>(
        malloc(sizeof(TagLib_Complex_Property_Attribute *) * (variantMap.size() + 1)));
      auto attr = static_cast<TagLib_Complex_Property_Attribute *>(
        malloc(sizeof(TagLib_Complex_Property_Attribute) * variantMap.size()));
      TagLib_Complex_Property_Attribute **attrPtr = attrs;
      // The next assignment is redundant to silence the clang analyzer,
      // it is done at the end of the loop, which must be entered because
      // variantMap is not empty.
      *attrPtr = attr;
      for(const auto &[k, v] : variantMap) {
        attr->key = stringToCharArray(k);
        attr->value.size = 0;
        switch(v.type()) {
        case Variant::Void:
          attr->value.type = TagLib_Variant_Void;
          attr->value.value.stringValue = NULL;
          break;
        case Variant::Bool:
          attr->value.type = TagLib_Variant_Bool;
          attr->value.value.boolValue = v.value<bool>();
          break;
        case Variant::Int:
          attr->value.type = TagLib_Variant_Int;
          attr->value.value.intValue = v.value<int>();
          break;
        case Variant::UInt:
          attr->value.type = TagLib_Variant_UInt;
          attr->value.value.uIntValue = v.value<unsigned int>();
          break;
        case Variant::LongLong:
          attr->value.type = TagLib_Variant_LongLong;
          attr->value.value.longLongValue = v.value<long long>();
          break;
        case Variant::ULongLong:
          attr->value.type = TagLib_Variant_ULongLong;
          attr->value.value.uLongLongValue = v.value<unsigned long long>();
          break;
        case Variant::Double:
          attr->value.type = TagLib_Variant_Double;
          attr->value.value.doubleValue = v.value<double>();
          break;
        case Variant::String: {
          attr->value.type = TagLib_Variant_String;
          auto str = v.value<String>();
          attr->value.value.stringValue = stringToCharArray(str);
          attr->value.size = str.size();
          break;
        }
        case Variant::StringList: {
          attr->value.type = TagLib_Variant_StringList;
          auto strs = v.value<StringList>();
          auto strPtr = static_cast<char **>(malloc(sizeof(char *) * (strs.size() + 1)));
          attr->value.value.stringListValue = strPtr;
          attr->value.size = strs.size();
          for(const auto &str : strs) {
            *strPtr++ = stringToCharArray(str);
          }
          *strPtr = NULL;
          break;
        }
        case Variant::ByteVector: {
          attr->value.type = TagLib_Variant_ByteVector;
          const ByteVector data = v.value<ByteVector>();
          auto bytePtr = static_cast<char *>(malloc(data.size()));
          attr->value.value.byteVectorValue = bytePtr;
          attr->value.size = data.size();
          ::memcpy(bytePtr, data.data(), data.size());
          break;
        }
        case Variant::ByteVectorList:
        case Variant::VariantList:
        case Variant::VariantMap: {
          attr->value.type = TagLib_Variant_String;
          std::stringstream ss;
          ss << v;
          attr->value.value.stringValue = stringToCharArray(ss.str());
          break;
        }
        }
        *attrPtr++ = attr++;
      }
      *attrPtr = NULL;
      *propPtr++ = attrs;
    }
  }
  *propPtr = NULL;
  return props;
}

void taglib_picture_from_complex_property(
  TagLib_Complex_Property_Attribute*** properties,
  TagLib_Complex_Property_Picture_Data *picture)
{
  if(!properties || !picture) {
    return;
  }
  std::memset(picture, 0, sizeof(*picture));
  TagLib_Complex_Property_Attribute*** propPtr = properties;
  while(!picture->data && *propPtr) {
    TagLib_Complex_Property_Attribute** attrPtr = *propPtr;
    while(*attrPtr) {
      TagLib_Complex_Property_Attribute *attr = *attrPtr;
      switch(attr->value.type) {
      case TagLib_Variant_String:
        if(strcmp("mimeType", attr->key) == 0) {
          picture->mimeType = attr->value.value.stringValue;
        }
        else if(strcmp("description", attr->key) == 0) {
          picture->description = attr->value.value.stringValue;
        }
        else if(strcmp("pictureType", attr->key) == 0) {
          picture->pictureType = attr->value.value.stringValue;
        }
        break;
      case TagLib_Variant_ByteVector:
        if(strcmp("data", attr->key) == 0) {
          picture->data = attr->value.value.byteVectorValue;
          picture->size = attr->value.size;
        }
        break;
      default:
        break;
      }
      ++attrPtr;
    }
    ++propPtr;
  }
}

void taglib_complex_property_free_keys(char **keys)
{
  if(keys == NULL) {
    return;
  }

  char **k = keys;
  while(*k) {
    free(*k++);
  }
  free(keys);
}

void taglib_complex_property_free(
  TagLib_Complex_Property_Attribute ***props)
{
  if(props == NULL) {
    return;
  }
  TagLib_Complex_Property_Attribute*** propPtr = props;
  while(*propPtr) {
    TagLib_Complex_Property_Attribute** attrPtr = *propPtr;
    while(*attrPtr) {
      TagLib_Complex_Property_Attribute *attr = *attrPtr;
      switch(attr->value.type) {
      case TagLib_Variant_String:
        free(attr->value.value.stringValue);
        break;
      case TagLib_Variant_StringList:
        if(attr->value.value.stringListValue) {
          char **s = attr->value.value.stringListValue;
          while(*s) {
            free(*s++);
          }
          free(attr->value.value.stringListValue);
        }
        break;
      case TagLib_Variant_ByteVector:
        free(attr->value.value.byteVectorValue);
        break;
      case TagLib_Variant_Void:
      case TagLib_Variant_Bool:
      case TagLib_Variant_Int:
      case TagLib_Variant_UInt:
      case TagLib_Variant_LongLong:
      case TagLib_Variant_ULongLong:
      case TagLib_Variant_Double:
        break;
      }
      free(attr->key);
      ++attrPtr;
    }
    free(**propPtr);
    free(*propPtr++);
  }
  free(props);
}
