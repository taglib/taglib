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

#include "matroskatag.h"
#include <algorithm>
#include <array>
#include <tuple>
#include "matroskasimpletag.h"
#include "ebmlmasterelement.h"
#include "ebmlstringelement.h"
#include "ebmlbinaryelement.h"
#include "ebmlmktags.h"
#include "ebmluintelement.h"
#include "ebmlutils.h"
#include "tpropertymap.h"
#include "tlist.h"

using namespace TagLib;

class Matroska::Tag::TagPrivate
{
public:
  TagPrivate() = default;
  ~TagPrivate() = default;

  bool setTag(const String &key, const String &value);
  String getTag(const String &key) const;

  template <typename T>
  int removeSimpleTags(T &&p)
  {
    auto &list = tags;
    int numRemoved = 0;
    for(auto it = list.begin(); it != list.end();) {
      it = std::find_if(it, list.end(), std::forward<T>(p));
      if(it != list.end()) {
        it = list.erase(it);
        numRemoved++;
      }
    }
    return numRemoved;
  }

  template <typename T>
  SimpleTagsList findSimpleTags(T &&p)
  {
    auto &list = tags;
    for(auto it = list.begin(); it != list.end();) {
      it = std::find_if(it, list.end(), std::forward<T>(p));
      if(it != list.end()) {
        list.append(*it);
        ++it;
      }
    }
    return list;
  }

  SimpleTagsList tags;
  ByteVector data;
};

Matroska::Tag::Tag() :
  Element(static_cast<ID>(EBML::Element::Id::MkTags)),
  d(std::make_unique<TagPrivate>())
{
}

Matroska::Tag::~Tag() = default;

void Matroska::Tag::addSimpleTag(const SimpleTag &tag)
{
  d->tags.append(tag);
  setNeedsRender(true);
}

void Matroska::Tag::removeSimpleTag(const String &name,
  SimpleTag::TargetTypeValue targetTypeValue)
{
  auto it = std::find_if(d->tags.begin(), d->tags.end(),
    [&name, targetTypeValue](const SimpleTag &t) {
        return t.name() == name && t.targetTypeValue() == targetTypeValue;
    }
  );
  if(it != d->tags.end()) {
    d->tags.erase(it);
    setNeedsRender(true);
  }
}

void Matroska::Tag::clearSimpleTags()
{
  d->tags.clear();
  setNeedsRender(true);
}

const Matroska::SimpleTagsList &Matroska::Tag::simpleTagsList() const
{
  return d->tags;
}

void Matroska::Tag::setTitle(const String &s)
{
  d->setTag("TITLE", s);
}

void Matroska::Tag::setArtist(const String &s)
{
  d->setTag("ARTIST", s);
}

void Matroska::Tag::setAlbum(const String &s)
{
  d->setTag("ALBUM", s);
}

void Matroska::Tag::setComment(const String &s)
{
  d->setTag("COMMENT", s);
}

void Matroska::Tag::setGenre(const String &s)
{
  d->setTag("GENRE", s);
}

void Matroska::Tag::setYear(unsigned int i)
{
  d->setTag("DATE", i != 0 ? String::number(i) : String());
}

void Matroska::Tag::setTrack(unsigned int i)
{
  d->setTag("TRACKNUMBER", i != 0 ? String::number(i) : String());
}

String Matroska::Tag::title() const
{
  return d->getTag("TITLE");
}

String Matroska::Tag::artist() const
{
  return d->getTag("ARTIST");
}

String Matroska::Tag::album() const
{
  return d->getTag("ALBUM");
}

String Matroska::Tag::comment() const
{
  return d->getTag("COMMENT");
}

String Matroska::Tag::genre() const
{
  return d->getTag("GENRE");
}

unsigned int Matroska::Tag::year() const
{
  auto value = d->getTag("DATE");
  if(value.isEmpty())
    return 0;
  auto list = value.split("-");
  return static_cast<unsigned int>(list.front().toInt());
}

unsigned int Matroska::Tag::track() const
{
  auto value = d->getTag("TRACKNUMBER");
  if(value.isEmpty())
    return 0;
  auto list = value.split("-");
  return static_cast<unsigned int>(list.front().toInt());
}

bool Matroska::Tag::isEmpty() const
{
  return d->tags.isEmpty();
}

ByteVector Matroska::Tag::renderInternal()
{
  if(d->tags.isEmpty()) {
    // Avoid writing a Tags element without Tag element.
    return {};
  }

  EBML::MkTags tags;
  List<SimpleTagsList> targetList;

  // Build target-based list
  for(const auto &tag : std::as_const(d->tags)) {
    auto targetTypeValue = tag.targetTypeValue();
    auto it = std::find_if(targetList.begin(),
      targetList.end(),
      [&](const auto &list) {
        const auto &simpleTag = list.front();
        return simpleTag.targetTypeValue() == targetTypeValue;
      }
    );
    if(it == targetList.end()) {
      SimpleTagsList list;
      list.append(tag);
      targetList.append(list);
    }
    else
      it->append(tag);
  }
  for(const auto &list : targetList) {
    const auto &frontTag = list.front();
    auto targetTypeValue = frontTag.targetTypeValue();
    auto tag = EBML::make_unique_element<EBML::Element::Id::MkTag>();

    // Build <Tag Targets> element
    auto targets = EBML::make_unique_element<EBML::Element::Id::MkTagTargets>();
    if(targetTypeValue != SimpleTag::TargetTypeValue::None) {
      auto element = EBML::make_unique_element<EBML::Element::Id::MkTagTargetTypeValue>();
      element->setValue(static_cast<unsigned int>(targetTypeValue));
      targets->appendElement(std::move(element));
    }
    tag->appendElement(std::move(targets));

    // Build <Simple Tag> element
    for(const auto &simpleTag : list) {
      auto t = EBML::make_unique_element<EBML::Element::Id::MkSimpleTag>();
      auto tagName = EBML::make_unique_element<EBML::Element::Id::MkTagName>();
      tagName->setValue(simpleTag.name());
      t->appendElement(std::move(tagName));

      // Tag Value
      if(simpleTag.type() == SimpleTag::StringType) {
        auto tagValue = EBML::make_unique_element<EBML::Element::Id::MkTagString>();
        tagValue->setValue(simpleTag.toString());
        t->appendElement(std::move(tagValue));
      }
      else if(simpleTag.type() == SimpleTag::BinaryType) {
        auto tagValue = EBML::make_unique_element<EBML::Element::Id::MkTagBinary>();
        tagValue->setValue(simpleTag.toByteVector());
        t->appendElement(std::move(tagValue));
      }

      // Language
      auto language = EBML::make_unique_element<EBML::Element::Id::MkTagsTagLanguage>();
      const String &lang = simpleTag.language();
      language->setValue(!lang.isEmpty() ? lang : "und");
      t->appendElement(std::move(language));

      // Default language flag
      auto dlf = EBML::make_unique_element<EBML::Element::Id::MkTagsLanguageDefault>();
      dlf->setValue(simpleTag.defaultLanguageFlag() ? 1 : 0);
      t->appendElement(std::move(dlf));

      tag->appendElement(std::move(t));
    }
    tags.appendElement(std::move(tag));
  }
  return tags.render();
}

namespace
{
  // PropertyMap key, Tag name, Target type value, strict
  // If the key is the same as the name and the target type value is Track,
  // no translation is needed because this is the default mapping.
  // Therefore, keys like TITLE, ARTIST, GENRE, COMMENT, etc. are omitted
  // unless they shall have priority over higher level tags with the same name
  // when no target type value is given. The strict boolean marks
  // entries which shall not be mapped without correct target type value.
  // For offical tags, see https://www.matroska.org/technical/tagging.html
  constexpr std::array simpleTagsTranslation {
    std::tuple("TITLE", "TITLE", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("ALBUM", "TITLE", Matroska::SimpleTag::TargetTypeValue::Album, true),
    std::tuple("ARTIST", "ARTIST", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("ALBUMARTIST", "ARTIST", Matroska::SimpleTag::TargetTypeValue::Album, true),
    std::tuple("TRACKNUMBER", "PART_NUMBER", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("DISCNUMBER", "PART_NUMBER", Matroska::SimpleTag::TargetTypeValue::Album, true),
    std::tuple("TRACKTOTAL", "TOTAL_PARTS", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("DISCTOTAL", "TOTAL_PARTS", Matroska::SimpleTag::TargetTypeValue::Album, true),
    std::tuple("DATE", "DATE_RELEASED", Matroska::SimpleTag::TargetTypeValue::Album, false),
    // Todo - original date
    std::tuple("TITLESORT", "TITLESORT", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("ALBUMSORT", "TITLESORT", Matroska::SimpleTag::TargetTypeValue::Album, true),
    std::tuple("ARTISTSORT", "ARTISTSORT", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("ALBUMARTISTSORT", "ARTISTSORT", Matroska::SimpleTag::TargetTypeValue::Album, true),
    std::tuple("ENCODEDBY", "ENCODED_BY", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("MEDIA", "ORIGINAL_MEDIA_TYPE", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("LABEL", "LABEL_CODE", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("CATALOGNUMBER", "CATALOG_NUMBER", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("DJMIXER", "MIXED_BY", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("REMIXER", "REMIXED_BY", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("INITIALKEY", "INITIAL_KEY", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("RELEASEDATE", "DATE_RELEASED", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("ENCODINGTIME", "DATE_ENCODED", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("TAGGINGDATE", "DATE_TAGGED", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("ENCODEDBY", "ENCODER", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("ENCODING", "ENCODER_SETTINGS", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("OWNER", "PURCHASE_OWNER", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("REPLAYGAIN_TRACK_GAIN", "REPLAYGAIN_GAIN", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("REPLAYGAIN_ALBUM_GAIN", "REPLAYGAIN_GAIN", Matroska::SimpleTag::TargetTypeValue::Album, true),
    std::tuple("REPLAYGAIN_TRACK_PEAK", "REPLAYGAIN_PEAK", Matroska::SimpleTag::TargetTypeValue::Track, false),
    std::tuple("REPLAYGAIN_ALBUM_PEAK", "REPLAYGAIN_PEAK", Matroska::SimpleTag::TargetTypeValue::Album, true),
    std::tuple("MUSICBRAINZ_ALBUMARTISTID", "MUSICBRAINZ_ALBUMARTISTID", Matroska::SimpleTag::TargetTypeValue::Album, false),
    std::tuple("MUSICBRAINZ_ALBUMID", "MUSICBRAINZ_ALBUMID", Matroska::SimpleTag::TargetTypeValue::Album, false),
    std::tuple("MUSICBRAINZ_RELEASEGROUPID", "MUSICBRAINZ_RELEASEGROUPID", Matroska::SimpleTag::TargetTypeValue::Album, false),
  };

  std::tuple<String, Matroska::SimpleTag::TargetTypeValue, bool> translateKey(const String &key)
  {
    auto it = std::find_if(simpleTagsTranslation.cbegin(),
      simpleTagsTranslation.cend(),
      [&key](const auto &t) { return key == std::get<0>(t); }
    );
    if(it != simpleTagsTranslation.end())
      return { std::get<1>(*it), std::get<2>(*it), std::get<3>(*it) };
    if (!key.isEmpty())
      return { key, Matroska::SimpleTag::TargetTypeValue::Track, false };
    return { String(), Matroska::SimpleTag::TargetTypeValue::None, false };
  }

  String translateTag(const String &name, Matroska::SimpleTag::TargetTypeValue targetTypeValue)
  {
    auto it = std::find_if(simpleTagsTranslation.cbegin(),
      simpleTagsTranslation.cend(),
      [&name, targetTypeValue](const auto &t) {
        return name == std::get<1>(t)
               && (targetTypeValue == std::get<2>(t) ||
                   (targetTypeValue == Matroska::SimpleTag::TargetTypeValue::None
                     && !std::get<3>(t)));
      }
    );
    return it != simpleTagsTranslation.end()
      ? String(std::get<0>(*it), String::UTF8)
      : (targetTypeValue == Matroska::SimpleTag::TargetTypeValue::Track ||
         targetTypeValue == Matroska::SimpleTag::TargetTypeValue::None)
      ? name
        : String();
  }
}

bool Matroska::Tag::TagPrivate::setTag(const String &key, const String &value)
{
  const auto tpl = translateKey(key);
  // Workaround Clang issue - no lambda capture of structured bindings
  const String &name = std::get<0>(tpl);
  auto targetTypeValue = std::get<1>(tpl);
  if(name.isEmpty())
    return false;
  removeSimpleTags(
    [&name, targetTypeValue] (const auto &t) {
      return t.name() == name
             && t.targetTypeValue() == targetTypeValue;
    }
  );
  if(!value.isEmpty()) {
    tags.append(SimpleTag(name, value, targetTypeValue));
  }
  return true;
}

String Matroska::Tag::TagPrivate::getTag(const String &key) const
{
  const auto tpl = translateKey(key);
  // Workaround Clang issue - no lambda capture of structured bindings
  const String &name = std::get<0>(tpl);
  auto targetTypeValue = std::get<1>(tpl);
  bool strict = std::get<2>(tpl);
  if(name.isEmpty())
    return {};
  auto it = std::find_if(tags.begin(), tags.end(),
    [&name, targetTypeValue, strict] (const SimpleTag &t) {
      return t.name() == name
        && t.type() == SimpleTag::StringType
        && (t.targetTypeValue() == targetTypeValue ||
            (t.targetTypeValue() == SimpleTag::TargetTypeValue::None && !strict));
    }
  );
  return it != tags.end() ? it->toString() : String();
}

PropertyMap Matroska::Tag::properties() const
{
  PropertyMap properties;
  for(const auto &simpleTag : std::as_const(d->tags)) {
    if(simpleTag.type() == SimpleTag::StringType) {
      String key = translateTag(simpleTag.name(), simpleTag.targetTypeValue());
      if(!key.isEmpty())
        properties[key].append(simpleTag.toString());
      else
        properties.addUnsupportedData(simpleTag.name());
    }
  }
  return properties;
}

PropertyMap Matroska::Tag::setProperties(const PropertyMap &propertyMap)
{
  // Remove all simple tags which would be returned in properties()
  for(auto it = d->tags.begin(); it != d->tags.end();) {
    String key;
    if(it->type() == SimpleTag::StringType &&
       !(key = translateTag(it->name(), it->targetTypeValue())).isEmpty()) {
      it = d->tags.erase(it);
      setNeedsRender(true);
    }
    else {
      ++it;
    }
  }

  // Add the new properties
  PropertyMap unsupportedProperties;
  for(const auto &[key, values] : propertyMap) {
    for(const auto &value : values) {
      if(auto [name, targetTypeValue, _] = translateKey(key);
         !name.isEmpty()) {
        d->tags.append(SimpleTag(name, value, targetTypeValue));
        setNeedsRender(true);
      }
      else {
        unsupportedProperties[key] = values;
      }
    }
  }
  return unsupportedProperties;
}

void Matroska::Tag::removeUnsupportedProperties(const StringList& properties)
{
  d->removeSimpleTags(
    [&properties](const SimpleTag &t) {
      return properties.contains(t.name());
    }
  );
}

StringList Matroska::Tag::complexPropertyKeys() const
{
  StringList keys;
  for(const SimpleTag &t : std::as_const(d->tags)) {
    if(t.type() != SimpleTag::StringType ||
       translateTag(t.name(), t.targetTypeValue()).isEmpty()) {
      keys.append(t.name());
    }
  }
  return keys;
}

List<VariantMap> Matroska::Tag::complexProperties(const String& key) const
{
  List<VariantMap> props;
  if(key.upper() != "PICTURE") { // Pictures are handled at the file level
    for(const SimpleTag &t : std::as_const(d->tags)) {
      if(t.name() == key &&
         (t.type() != SimpleTag::StringType ||
          translateTag(t.name(), t.targetTypeValue()).isEmpty())) {
        VariantMap property;
        if(t.type() != SimpleTag::StringType) {
          property.insert("data", t.toByteVector());
        }
        else {
          property.insert("value", t.toString());
        }
        property.insert("name", t.name());
        property.insert("targetTypeValue", t.targetTypeValue());
        property.insert("language", t.language());
        property.insert("defaultLanguage", t.defaultLanguageFlag());
        props.append(property);
      }
    }
  }
  return props;
}

bool Matroska::Tag::setComplexProperties(const String& key, const List<VariantMap>& value)
{
  if(key.upper() == "PICTURE") {
    // Pictures are handled at the file level
    return false;
  }
  d->removeSimpleTags(
    [&key](const SimpleTag &t) {
      return t.name() == key &&
        (t.type() != SimpleTag::StringType ||
         translateTag(t.name(), t.targetTypeValue()).isEmpty());
    }
  );
  bool result = false;
  for(const auto &property : value) {
    if(property.value("name").value<String>() == key &&
       (property.contains("data") || property.contains("value") )) {
      SimpleTag::TargetTypeValue targetTypeValue;
      Variant targetTypeValueVar = property.value("targetTypeValue", 0);
      switch(targetTypeValueVar.type()) {
      case Variant::UInt:
        targetTypeValue = static_cast<SimpleTag::TargetTypeValue>(targetTypeValueVar.value<unsigned int>());
        break;
      case Variant::LongLong:
        targetTypeValue = static_cast<SimpleTag::TargetTypeValue>(targetTypeValueVar.value<long long>());
        break;
      case Variant::ULongLong:
        targetTypeValue = static_cast<SimpleTag::TargetTypeValue>(targetTypeValueVar.value<unsigned long long>());
        break;
      default:
        targetTypeValue = static_cast<SimpleTag::TargetTypeValue>(targetTypeValueVar.value<int>());
      }
      auto language = property.value("language").value<String>();
      bool defaultLanguage = property.value("defaultLanguage", true).value<bool>();
      d->tags.append(property.contains("data")
        ? SimpleTag(key, property.value("data").value<ByteVector>(),
                    targetTypeValue, language, defaultLanguage)
        : SimpleTag(key, property.value("value").value<String>(),
                    targetTypeValue, language, defaultLanguage));
      setNeedsRender(true);
      result = true;
    }
  }
  return result;
}
