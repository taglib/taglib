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
#include <string>
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
    std::tuple("WM/AlbumTitle", "ALBUM", ASF::Attribute::UnicodeType),
    std::tuple("WM/AlbumArtist", "ALBUMARTIST", ASF::Attribute::UnicodeType),
    std::tuple("WM/AuthorURL", "ARTISTWEBPAGE", ASF::Attribute::UnicodeType),
    std::tuple("WM/Composer", "COMPOSER", ASF::Attribute::UnicodeType),
    std::tuple("WM/Writer", "LYRICIST", ASF::Attribute::UnicodeType),
    std::tuple("WM/Conductor", "CONDUCTOR", ASF::Attribute::UnicodeType),
    std::tuple("WM/ModifiedBy", "REMIXER", ASF::Attribute::UnicodeType),
    std::tuple("WM/Year", "DATE", ASF::Attribute::UnicodeType),
    std::tuple("WM/OriginalAlbumTitle", "ORIGINALALBUM", ASF::Attribute::UnicodeType),
    std::tuple("WM/OriginalArtist", "ORIGINALARTIST", ASF::Attribute::UnicodeType),
    std::tuple("WM/OriginalFilename", "ORIGINALFILENAME", ASF::Attribute::UnicodeType),
    std::tuple("WM/OriginalLyricist", "ORIGINALLYRICIST", ASF::Attribute::UnicodeType),
    std::tuple("WM/OriginalReleaseYear", "ORIGINALDATE", ASF::Attribute::UnicodeType),
    std::tuple("WM/Producer", "PRODUCER", ASF::Attribute::UnicodeType),
    std::tuple("WM/ContentGroupDescription", "WORK", ASF::Attribute::UnicodeType),
    std::tuple("WM/SubTitle", "SUBTITLE", ASF::Attribute::UnicodeType),
    std::tuple("WM/SetSubTitle", "DISCSUBTITLE", ASF::Attribute::UnicodeType),
    std::tuple("WM/TrackNumber", "TRACKNUMBER", ASF::Attribute::UnicodeType),
    std::tuple("WM/PartOfSet", "DISCNUMBER", ASF::Attribute::UnicodeType),
    std::tuple("WM/Genre", "GENRE", ASF::Attribute::UnicodeType),
    std::tuple("WM/BeatsPerMinute", "BPM", ASF::Attribute::UnicodeType),
    std::tuple("WM/Mood", "MOOD", ASF::Attribute::UnicodeType),
    std::tuple("WM/InitialKey", "INITIALKEY", ASF::Attribute::UnicodeType),
    std::tuple("WM/ISRC", "ISRC", ASF::Attribute::UnicodeType),
    std::tuple("WM/Lyrics", "LYRICS", ASF::Attribute::UnicodeType),
    std::tuple("WM/Media", "MEDIA", ASF::Attribute::UnicodeType),
    std::tuple("WM/Publisher", "LABEL", ASF::Attribute::UnicodeType),
    std::tuple("WM/CatalogNo", "CATALOGNUMBER", ASF::Attribute::UnicodeType),
    std::tuple("WM/Barcode", "BARCODE", ASF::Attribute::UnicodeType),
    std::tuple("WM/EncodedBy", "ENCODEDBY", ASF::Attribute::UnicodeType),
    std::tuple("WM/EncodingSettings", "ENCODING", ASF::Attribute::UnicodeType),
    std::tuple("WM/EncodingTime", "ENCODINGTIME", ASF::Attribute::QWordType),
    std::tuple("WM/AudioFileURL", "FILEWEBPAGE", ASF::Attribute::UnicodeType),
    std::tuple("WM/AlbumSortOrder", "ALBUMSORT", ASF::Attribute::UnicodeType),
    std::tuple("WM/AlbumArtistSortOrder", "ALBUMARTISTSORT", ASF::Attribute::UnicodeType),
    std::tuple("WM/ArtistSortOrder", "ARTISTSORT", ASF::Attribute::UnicodeType),
    std::tuple("WM/TitleSortOrder", "TITLESORT", ASF::Attribute::UnicodeType),
    std::tuple("WM/Script", "SCRIPT", ASF::Attribute::UnicodeType),
    std::tuple("WM/Language", "LANGUAGE", ASF::Attribute::UnicodeType),
    std::tuple("WM/ARTISTS", "ARTISTS", ASF::Attribute::UnicodeType),
    std::tuple("ASIN", "ASIN", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Track Id", "MUSICBRAINZ_TRACKID", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Artist Id", "MUSICBRAINZ_ARTISTID", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Album Id", "MUSICBRAINZ_ALBUMID", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Album Artist Id", "MUSICBRAINZ_ALBUMARTISTID", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Album Release Country", "RELEASECOUNTRY", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Album Status", "RELEASESTATUS", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Album Type", "RELEASETYPE", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Release Group Id", "MUSICBRAINZ_RELEASEGROUPID", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Release Track Id", "MUSICBRAINZ_RELEASETRACKID", ASF::Attribute::UnicodeType),
    std::tuple("MusicBrainz/Work Id", "MUSICBRAINZ_WORKID", ASF::Attribute::UnicodeType),
    std::tuple("MusicIP/PUID", "MUSICIP_PUID", ASF::Attribute::UnicodeType),
    std::tuple("Acoustid/Id", "ACOUSTID_ID", ASF::Attribute::UnicodeType),
    std::tuple("Acoustid/Fingerprint", "ACOUSTID_FINGERPRINT", ASF::Attribute::UnicodeType),
    std::tuple("replaygain_track_gain", "REPLAYGAIN_TRACK_GAIN", ASF::Attribute::UnicodeType),
    std::tuple("replaygain_track_peak", "REPLAYGAIN_TRACK_PEAK", ASF::Attribute::UnicodeType),
    std::tuple("replaygain_album_gain", "REPLAYGAIN_ALBUM_GAIN", ASF::Attribute::UnicodeType),
    std::tuple("replaygain_album_peak", "REPLAYGAIN_ALBUM_PEAK", ASF::Attribute::UnicodeType),
    std::tuple("WM/MediaClassPrimaryID", "MEDIACLASSPRIMARYID", ASF::Attribute::GuidType),
    std::tuple("WM/MediaClassSecondaryID", "MEDIACLASSSECONDARYID", ASF::Attribute::GuidType),
    std::tuple("WM/WMCollectionGroupID", "COLLECTIONGROUPID", ASF::Attribute::GuidType),
    std::tuple("WM/WMCollectionID", "COLLECTIONID", ASF::Attribute::GuidType),
    std::tuple("WM/WMContentID", "CONTENTID", ASF::Attribute::GuidType),
    std::tuple("WM/ContentDistributor", "CONTENTDISTRIBUTOR", ASF::Attribute::UnicodeType),
    std::tuple("WM/ParentalRating", "PARENTALRATING", ASF::Attribute::UnicodeType),
    std::tuple("WM/Period", "PERIOD", ASF::Attribute::UnicodeType),
    std::tuple("WM/PromotionURL", "PROMOTIONURL", ASF::Attribute::UnicodeType),
    std::tuple("WM/ToolName", "TOOLNAME", ASF::Attribute::UnicodeType),
    std::tuple("WM/ToolVersion", "TOOLVERSION", ASF::Attribute::UnicodeType),
    std::tuple("WM/Provider", "PROVIDER", ASF::Attribute::UnicodeType),
    std::tuple("WM/UniqueFileIdentifier", "UNIQUEFILEIDENTIFIER", ASF::Attribute::UnicodeType),
    std::tuple("WMFSDKVersion", "WMFSDKVERSION", ASF::Attribute::UnicodeType),
    std::tuple("WMFSDKNeeded", "WMFSDKNEEDED", ASF::Attribute::UnicodeType),
    std::tuple("DeviceConformanceTemplate", "DEVICECONFORMANCETEMPLATE", ASF::Attribute::UnicodeType),
    std::tuple("MediaFoundationVersion", "MEDIAFOUNDATIONVERSION", ASF::Attribute::UnicodeType),
    std::tuple("IsVBR", "ISVBR", ASF::Attribute::BoolType),
    std::tuple("PeakValue", "PEAKVALUE", ASF::Attribute::DWordType),
    std::tuple("AverageLevel", "AVERAGELEVEL", ASF::Attribute::DWordType),
  };

  // Attribute names are matched case-insensitively; taggers disagree on the
  // casing of names not defined by Windows Media (e.g. replaygain_track_gain).
  String translateKey(const String &key)
  {
    const String upperKey = key.upper();
    for(const auto &[k, v, t] : keyTranslation) {
      if(upperKey == String(k).upper())
        return v;
    }

    return String();
  }

  void eraseAttribute(ASF::AttributeListMap &attributeListMap, const String &name)
  {
    const String upperName = name.upper();
    StringList keys;
    for(const auto &[k, attributes] : std::as_const(attributeListMap)) {
      if(k.upper() == upperName)
        keys.append(k);
    }
    for(const auto &k : keys)
      attributeListMap.erase(k);
  }

  String attributeToString(const ASF::Attribute &attr)
  {
    switch(attr.type()) {
    case ASF::Attribute::WordType:
    case ASF::Attribute::DWordType:
    case ASF::Attribute::QWordType:
    case ASF::Attribute::BoolType:
      return String(std::to_string(attr.toULongLong()));
    case ASF::Attribute::GuidType:
      if(const ByteVector data = attr.toByteVector(); data.size() == 16) {
        String str;
        for(int i = 0; i < 16; ++i) {
          if(i == 4 || i == 6 || i == 8 || i == 10) {
            str += '-';
          }
          const auto c = static_cast<unsigned char>(data[i]);
          unsigned char d = c >> 4;
          str += static_cast<char>(d >= 10 ? d - 10 + 'A' : d + '0');
          d = c & 0x0f;
          str += static_cast<char>(d >= 10 ? d - 10 + 'A' : d + '0');
        }
        return str;
      }
      return {};
    default:
      return attr.toString();
    }
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
        // The same attribute can occur in both the extended content
        // description object and the metadata (library) object, skip exact
        // duplicates (e.g. a second identical IsVBR).
        if(const String value = attributeToString(attr);
           !props.value(key).contains(value)) {
          props.insert(key, value);
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
  static const Map<String, std::pair<String, Attribute::AttributeTypes>> reverseKeyMap = [] {
    Map<String, std::pair<String, Attribute::AttributeTypes>> map;
    for(const auto &[k, v, t] : keyTranslation) {
      map[v] = {k, t};
    }
    return map;
  }();

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
        eraseAttribute(d->attributeListMap, reverseKeyMap[prop].first);
      }
    }
  }

  PropertyMap ignoredProps;
  for(const auto &[prop, attributes] : props) {
    if(reverseKeyMap.contains(prop)) {
      const auto &[name, type] = reverseKeyMap[prop];
      eraseAttribute(d->attributeListMap, name);
      for(const auto &str : attributes) {
        switch(type) {
        case Attribute::WordType:
          addAttribute(name, static_cast<unsigned short>(str.toULongLong()));
          break;
        case Attribute::DWordType:
          addAttribute(name, static_cast<unsigned int>(str.toULongLong()));
          break;
        case Attribute::QWordType:
          addAttribute(name, str.toULongLong());
          break;
        case Attribute::BoolType:
          addAttribute(name, !str.isEmpty() && str != "0" && str.upper() != "TRUE");
          break;
        case Attribute::GuidType: {
          ByteVector data;
          String hexStr;
          for(wchar_t c : str) {
            if(c >= 'a' && c <= 'f') {
              hexStr += static_cast<wchar_t>(c + 'A' - 'a');
            } else if((c >= 'A' && c <= 'F') || (c >= '0' && c <= '9')) {
              hexStr += c;
            }
          }
          if(hexStr.length() == 32) {
            unsigned char buf[16];
            unsigned char* bufPtr = buf;
            for(int i = 0; i < 32;) {
              auto h = static_cast<unsigned char>(hexStr[i++]);
              auto l = static_cast<unsigned char>(hexStr[i++]);
              *bufPtr++ = static_cast<unsigned char>(
                ((h >= 'A' ? h + 10 - 'A' : h - '0') << 4) |
                 (l >= 'A' ? l + 10 - 'A' : l - '0'));
            }
            data = ByteVector(reinterpret_cast<char*>(buf), 16);
          }
          addAttribute(name, Attribute::fromGuid(data));
          break;
        }
        default:
          addAttribute(name, str);
          break;
        }
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
