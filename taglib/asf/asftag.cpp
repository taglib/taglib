/**************************************************************************
    copyright            : (C) 2005-2007 by Lukáš Lalinský
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

#include "asftag.h"

#include <array>
#include <utility>

#include "tpropertymap.h"
#include "asfattribute.h"
#include "asfpicture.h"

using namespace TagLib;

namespace
{
  StringList attributeListToStringList(const ASF::AttributeList &attributes)
  {
    StringList strs;
    for(const auto &attribute : attributes) {
      strs.append(attribute.toString());
    }
    return strs;
  }
}  // namespace

class ASF::Tag::TagPrivate
{
public:
  String title;
  String artist;
  String copyright;
  String comment;
  String rating;
  AttributeListMap attributeListMap;
};

ASF::Tag::Tag() :
  d(std::make_unique<TagPrivate>())
{
}

ASF::Tag::~Tag() = default;

String ASF::Tag::title() const
{
  return d->title;
}

String ASF::Tag::artist() const
{
  return d->artist;
}

String ASF::Tag::album() const
{
  if(d->attributeListMap.contains("WM/AlbumTitle"))
    return joinTagValues(
      attributeListToStringList(d->attributeListMap.value("WM/AlbumTitle")));
  return String();
}

String ASF::Tag::copyright() const
{
  return d->copyright;
}

String ASF::Tag::comment() const
{
  return d->comment;
}

String ASF::Tag::rating() const
{
  return d->rating;
}

unsigned int ASF::Tag::year() const
{
  if(d->attributeListMap.contains("WM/Year"))
    return d->attributeListMap["WM/Year"][0].toString().toInt();
  return 0;
}

unsigned int ASF::Tag::track() const
{
  if(d->attributeListMap.contains("WM/TrackNumber")) {
    const ASF::Attribute attr = d->attributeListMap["WM/TrackNumber"][0];
    if(attr.type() == ASF::Attribute::DWordType)
      return attr.toUInt();
    return attr.toString().toInt();
  }
  if(d->attributeListMap.contains("WM/Track"))
    return d->attributeListMap["WM/Track"][0].toUInt();
  return 0;
}

String ASF::Tag::genre() const
{
  if(d->attributeListMap.contains("WM/Genre"))
    return joinTagValues(
      attributeListToStringList(d->attributeListMap.value("WM/Genre")));
  return String();
}

void ASF::Tag::setTitle(const String &value)
{
  d->title = value;
}

void ASF::Tag::setArtist(const String &value)
{
  d->artist = value;
}

void ASF::Tag::setCopyright(const String &value)
{
  d->copyright = value;
}

void ASF::Tag::setComment(const String &value)
{
  d->comment = value;
}

void ASF::Tag::setRating(const String &value)
{
  d->rating = value;
}

void ASF::Tag::setAlbum(const String &value)
{
  setAttribute("WM/AlbumTitle", value);
}

void ASF::Tag::setGenre(const String &value)
{
  setAttribute("WM/Genre", value);
}

void ASF::Tag::setYear(unsigned int value)
{
  setAttribute("WM/Year", String::number(value));
}

void ASF::Tag::setTrack(unsigned int value)
{
  setAttribute("WM/TrackNumber", String::number(value));
}

ASF::AttributeListMap& ASF::Tag::attributeListMap()
{
  return d->attributeListMap;
}

const ASF::AttributeListMap &ASF::Tag::attributeListMap() const
{
  return d->attributeListMap;
}

bool ASF::Tag::contains(const String &key) const
{
  return d->attributeListMap.contains(key);
}

void ASF::Tag::removeItem(const String &key)
{
  d->attributeListMap.erase(key);
}

ASF::AttributeList ASF::Tag::attribute(const String &name) const
{
  return d->attributeListMap[name];
}

void ASF::Tag::setAttribute(const String &name, const Attribute &attribute)
{
  AttributeList val;
  val.append(attribute);
  d->attributeListMap.insert(name, val);
}

void ASF::Tag::setAttribute(const String &name, const AttributeList &values)
{
  d->attributeListMap.insert(name, values);
}

void ASF::Tag::addAttribute(const String &name, const Attribute &attribute)
{
  if(d->attributeListMap.contains(name)) {
    d->attributeListMap[name].append(attribute);
  }
  else {
    setAttribute(name, attribute);
  }
}

bool ASF::Tag::isEmpty() const
{
  return TagLib::Tag::isEmpty() &&
         copyright().isEmpty() &&
         rating().isEmpty() &&
         d->attributeListMap.isEmpty();
}

namespace
{
  constexpr std::array keyTranslation {
    std::pair("WM/AlbumTitle", "ALBUM"),
    std::pair("WM/AlbumArtist", "ALBUMARTIST"),
    std::pair("WM/AuthorURL", "ARTISTWEBPAGE"),
    std::pair("WM/Composer", "COMPOSER"),
    std::pair("WM/Writer", "LYRICIST"),
    std::pair("WM/Conductor", "CONDUCTOR"),
    std::pair("WM/ModifiedBy", "REMIXER"),
    std::pair("WM/Year", "DATE"),
    std::pair("WM/OriginalAlbumTitle", "ORIGINALALBUM"),
    std::pair("WM/OriginalArtist", "ORIGINALARTIST"),
    std::pair("WM/OriginalFilename", "ORIGINALFILENAME"),
    std::pair("WM/OriginalLyricist", "ORIGINALLYRICIST"),
    std::pair("WM/OriginalReleaseYear", "ORIGINALDATE"),
    std::pair("WM/Producer", "PRODUCER"),
    std::pair("WM/ContentGroupDescription", "WORK"),
    std::pair("WM/SubTitle", "SUBTITLE"),
    std::pair("WM/SetSubTitle", "DISCSUBTITLE"),
    std::pair("WM/TrackNumber", "TRACKNUMBER"),
    std::pair("WM/PartOfSet", "DISCNUMBER"),
    std::pair("WM/Genre", "GENRE"),
    std::pair("WM/BeatsPerMinute", "BPM"),
    std::pair("WM/Mood", "MOOD"),
    std::pair("WM/InitialKey", "INITIALKEY"),
    std::pair("WM/ISRC", "ISRC"),
    std::pair("WM/Lyrics", "LYRICS"),
    std::pair("WM/Media", "MEDIA"),
    std::pair("WM/Publisher", "LABEL"),
    std::pair("WM/CatalogNo", "CATALOGNUMBER"),
    std::pair("WM/Barcode", "BARCODE"),
    std::pair("WM/EncodedBy", "ENCODEDBY"),
    std::pair("WM/EncodingSettings", "ENCODING"),
    std::pair("WM/EncodingTime", "ENCODINGTIME"),
    std::pair("WM/AudioFileURL", "FILEWEBPAGE"),
    std::pair("WM/AlbumSortOrder", "ALBUMSORT"),
    std::pair("WM/AlbumArtistSortOrder", "ALBUMARTISTSORT"),
    std::pair("WM/ArtistSortOrder", "ARTISTSORT"),
    std::pair("WM/TitleSortOrder", "TITLESORT"),
    std::pair("WM/Script", "SCRIPT"),
    std::pair("WM/Language", "LANGUAGE"),
    std::pair("WM/ARTISTS", "ARTISTS"),
    std::pair("ASIN", "ASIN"),
    std::pair("MusicBrainz/Track Id", "MUSICBRAINZ_TRACKID"),
    std::pair("MusicBrainz/Artist Id", "MUSICBRAINZ_ARTISTID"),
    std::pair("MusicBrainz/Album Id", "MUSICBRAINZ_ALBUMID"),
    std::pair("MusicBrainz/Album Artist Id", "MUSICBRAINZ_ALBUMARTISTID"),
    std::pair("MusicBrainz/Album Release Country", "RELEASECOUNTRY"),
    std::pair("MusicBrainz/Album Status", "RELEASESTATUS"),
    std::pair("MusicBrainz/Album Type", "RELEASETYPE"),
    std::pair("MusicBrainz/Release Group Id", "MUSICBRAINZ_RELEASEGROUPID"),
    std::pair("MusicBrainz/Release Track Id", "MUSICBRAINZ_RELEASETRACKID"),
    std::pair("MusicBrainz/Work Id", "MUSICBRAINZ_WORKID"),
    std::pair("MusicIP/PUID", "MUSICIP_PUID"),
    std::pair("Acoustid/Id", "ACOUSTID_ID"),
    std::pair("Acoustid/Fingerprint", "ACOUSTID_FINGERPRINT"),
  };

  String translateKey(const String &key)
  {
    for(const auto &[k, t] : keyTranslation) {
      if(key == k)
        return t;
    }

    return String();
  }
}  // namespace

PropertyMap ASF::Tag::properties() const
{
  PropertyMap props;

  if(!d->title.isEmpty()) {
    props["TITLE"] = d->title;
  }
  if(!d->artist.isEmpty()) {
    props["ARTIST"] = d->artist;
  }
  if(!d->copyright.isEmpty()) {
    props["COPYRIGHT"] = d->copyright;
  }
  if(!d->comment.isEmpty()) {
    props["COMMENT"] = d->comment;
  }

  for(const auto &[k, attributes] : std::as_const(d->attributeListMap)) {
    if(const String key = translateKey(k); !key.isEmpty()) {
      for(const auto &attr : attributes) {
        if(key == "TRACKNUMBER") {
          if(attr.type() == ASF::Attribute::DWordType)
            props.insert(key, String::number(attr.toUInt()));
          else
            props.insert(key, attr.toString());
        }
        else {
          props.insert(key, attr.toString());
        }
      }
    }
    else {
      props.addUnsupportedData(k);
    }
  }
  return props;
}

void ASF::Tag::removeUnsupportedProperties(const StringList &props)
{
  for(const auto &prop : props)
    d->attributeListMap.erase(prop);
}

PropertyMap ASF::Tag::setProperties(const PropertyMap &props)
{
  static Map<String, String> reverseKeyMap;
  if(reverseKeyMap.isEmpty()) {
    for(const auto &[k, t] : keyTranslation) {
      reverseKeyMap[t] = k;
    }
  }

  const PropertyMap origProps = properties();
  for(const auto &[prop, _] : origProps) {
    if(!props.contains(prop) || props[prop].isEmpty()) {
      if(prop == "TITLE") {
        d->title.clear();
      }
      else if(prop == "ARTIST") {
        d->artist.clear();
      }
      else if(prop == "COMMENT") {
        d->comment.clear();
      }
      else if(prop == "COPYRIGHT") {
        d->copyright.clear();
      }
      else {
        d->attributeListMap.erase(reverseKeyMap[prop]);
      }
    }
  }

  PropertyMap ignoredProps;
  for(const auto &[prop, attributes] : props) {
    if(reverseKeyMap.contains(prop)) {
      String name = reverseKeyMap[prop];
      removeItem(name);
      for(const auto &attr : attributes) {
        addAttribute(name, attr);
      }
    }
    else if(prop == "TITLE") {
      d->title = attributes.toString();
    }
    else if(prop == "ARTIST") {
      d->artist = attributes.toString();
    }
    else if(prop == "COMMENT") {
      d->comment = attributes.toString();
    }
    else if(prop == "COPYRIGHT") {
      d->copyright = attributes.toString();
    }
    else {
      ignoredProps.insert(prop, attributes);
    }
  }

  return ignoredProps;
}

StringList ASF::Tag::complexPropertyKeys() const
{
  StringList keys;
  if(d->attributeListMap.contains("WM/Picture")) {
    keys.append("PICTURE");
  }
  return keys;
}

List<VariantMap> ASF::Tag::complexProperties(const String &key) const
{
  List<VariantMap> props;
  if(const String uppercaseKey = key.upper(); uppercaseKey == "PICTURE") {
    const AttributeList pictures = d->attributeListMap.value("WM/Picture");
    for(const Attribute &attr : pictures) {
      ASF::Picture picture = attr.toPicture();
      VariantMap property;
      property.insert("data", picture.picture());
      property.insert("mimeType", picture.mimeType());
      property.insert("description", picture.description());
      property.insert("pictureType",
        ASF::Picture::typeToString(picture.type()));
      props.append(property);
    }
  }
  return props;
}

bool ASF::Tag::setComplexProperties(const String &key, const List<VariantMap> &value)
{
  if(const String uppercaseKey = key.upper(); uppercaseKey == "PICTURE") {
    removeItem("WM/Picture");

    for(const auto &property : value) {
      ASF::Picture picture;
      picture.setPicture(property.value("data").value<ByteVector>());
      picture.setMimeType(property.value("mimeType").value<String>());
      picture.setDescription(property.value("description").value<String>());
      picture.setType(ASF::Picture::typeFromString(
        property.value("pictureType").value<String>()));
      addAttribute("WM/Picture", Attribute(picture));
    }
  }
  else {
    return false;
  }
  return true;
}
