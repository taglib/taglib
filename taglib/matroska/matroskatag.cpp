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
  const String *getTag(const String &key) const;

  template <typename T>
  int removeSimpleTags(T &&p)
  {
    auto &list = tags;
    int numRemoved = 0;
    for(auto it = list.begin(); it != list.end();) {
      it = std::find_if(it, list.end(), std::forward<T>(p));
      if(it != list.end()) {
        delete *it;
        *it = nullptr;
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

  template <typename T>
  const SimpleTag *findSimpleTag(T &&p) const
  {
    auto &list = tags;
    auto it = std::find_if(list.begin(), list.end(), std::forward<T>(p));
    return it != list.end() ? *it : nullptr;
  }

  template <typename T>
  SimpleTag *findSimpleTag(T &&p)
  {
    return const_cast<SimpleTag *>(
      const_cast<const TagPrivate *>(this)->findSimpleTag(std::forward<T>(p))
    );
  }

  List<SimpleTag *> tags;
  ByteVector data;
};

Matroska::Tag::Tag() :
  Element(static_cast<ID>(EBML::Element::Id::MkTags)),
  d(std::make_unique<TagPrivate>())
{
  d->tags.setAutoDelete(true);
}
Matroska::Tag::~Tag() = default;

void Matroska::Tag::addSimpleTag(SimpleTag *tag)
{
  d->tags.append(tag);
}

void Matroska::Tag::removeSimpleTag(SimpleTag *tag)
{
  auto it = d->tags.find(tag);
  if(it != d->tags.end()) {
    delete *it;
    d->tags.erase(it);
  }
}

void Matroska::Tag::clearSimpleTags()
{
  d->tags.clear();
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
  d->setTag("DATE", String::number(i));
}

void Matroska::Tag::setTrack(unsigned int i)
{
  d->setTag("TRACKNUMBER", String::number(i));
}

String Matroska::Tag::title() const
{
  const auto value = d->getTag("TITLE");
  return value ? *value : String();
}

String Matroska::Tag::artist() const
{
  const auto value = d->getTag("ARTIST");
  return value ? *value : String();
}

String Matroska::Tag::album() const
{
  const auto value = d->getTag("ALBUM");
  return value ? *value : String();
}

String Matroska::Tag::comment() const
{
  const auto value = d->getTag("COMMENT");
  return value ? *value : String();
}

String Matroska::Tag::genre() const
{
  const auto value = d->getTag("GENRE");
  return value ? *value : String();
}

unsigned int Matroska::Tag::year() const
{
  auto value = d->getTag("DATE");
  if(!value)
    return 0;
  auto list = value->split("-");
  return static_cast<unsigned int>(list.front().toInt());
}

unsigned int Matroska::Tag::track() const
{
  auto value = d->getTag("TRACKNUMBER");
  if(!value)
    return 0;
  auto list = value->split("-");
  return static_cast<unsigned int>(list.front().toInt());
}

bool Matroska::Tag::isEmpty() const
{
  return d->tags.isEmpty();
}

bool Matroska::Tag::render()
{
  EBML::MkTags tags;
  List<List<SimpleTag *> *> targetList;
  targetList.setAutoDelete(true);

  // Build target-based list
  for(auto tag : d->tags) {
    auto targetTypeValue = tag->targetTypeValue();
    auto it = std::find_if(targetList.begin(),
      targetList.end(),
      [&](auto list) {
        const auto *simpleTag = list->front();
        return simpleTag->targetTypeValue() == targetTypeValue;
      }
    );
    if(it == targetList.end()) {
      auto list = new List<SimpleTag *>();
      list->append(tag);
      targetList.append(list);
    }
    else
      (*it)->append(tag);
  }
  for(auto list : targetList) {
    auto frontTag = list->front();
    auto targetTypeValue = frontTag->targetTypeValue();
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
    for(auto simpleTag : *list) {
      auto t = EBML::make_unique_element<EBML::Element::Id::MkSimpleTag>();
      auto tagName = EBML::make_unique_element<EBML::Element::Id::MkTagName>();
      tagName->setValue(simpleTag->name());
      t->appendElement(std::move(tagName));

      // Tag Value
      SimpleTagString *tStr = nullptr;
      SimpleTagBinary *tBin = nullptr;
      if((tStr = dynamic_cast<SimpleTagString *>(simpleTag))) {
        auto tagValue = EBML::make_unique_element<EBML::Element::Id::MkTagString>();
        tagValue->setValue(tStr->value());
        t->appendElement(std::move(tagValue));
      }
      else if((tBin = dynamic_cast<SimpleTagBinary *>(simpleTag))) {
        auto tagValue = EBML::make_unique_element<EBML::Element::Id::MkTagBinary>();
        tagValue->setValue(tBin->value());
        t->appendElement(std::move(tagValue));
      }

      // Language
      auto language = EBML::make_unique_element<EBML::Element::Id::MkTagsTagLanguage>();
      const String &lang = simpleTag->language();
      language->setValue(!lang.isEmpty() ? lang : "und");
      t->appendElement(std::move(language));

      // Default language flag
      auto dlf = EBML::make_unique_element<EBML::Element::Id::MkTagsLanguageDefault>();
      dlf->setValue(simpleTag->defaultLanguageFlag() ? 1 : 0);
      t->appendElement(std::move(dlf));

      tag->appendElement(std::move(t));
    }
    tags.appendElement(std::move(tag));
  }

  auto data = tags.render();
  auto beforeSize = size();
  auto afterSize = data.size();
  if(afterSize != beforeSize) {
    if(!emitSizeChanged(afterSize - beforeSize))
      return false;
  }
  setData(data);
  return true;
}

namespace
{
  // PropertyMap key, Tag name, Target type value
  // If the key is the same as the name and the target type value is Track,
  // no translation is needed because this is the default mapping.
  // Therefore, keys like TITLE, ARTIST, GENRE, COMMENT, etc. are omitted.
  // For offical tags, see https://www.matroska.org/technical/tagging.html
  constexpr std::array simpleTagsTranslation {
    std::tuple("ALBUM", "TITLE", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("ALBUMARTIST", "ARTIST", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("TRACKNUMBER", "PART_NUMBER", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("TRACKTOTAL", "TOTAL_PARTS", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("DISCNUMBER", "PART_NUMBER", Matroska::SimpleTag::TargetTypeValue::Part),
    std::tuple("DISCTOTAL", "TOTAL_PARTS", Matroska::SimpleTag::TargetTypeValue::Part),
    std::tuple("DATE", "DATE_RELEASED", Matroska::SimpleTag::TargetTypeValue::Album),
    // Todo - original date
    std::tuple("ALBUMSORT", "TITLESORT", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("ALBUMARTISTSORT", "ARTISTSORT", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("ENCODEDBY", "ENCODED_BY", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("MEDIA", "ORIGINAL_MEDIA_TYPE", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("LABEL", "LABEL_CODE", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("CATALOGNUMBER", "CATALOG_NUMBER", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("DJMIXER", "MIXED_BY", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("REMIXER", "REMIXED_BY", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("INITIALKEY", "INITIAL_KEY", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("RELEASEDATE", "DATE_RELEASED", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("ENCODINGTIME", "DATE_ENCODED", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("TAGGINGDATE", "DATE_TAGGED", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("ENCODEDBY", "ENCODER", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("ENCODING", "ENCODER_SETTINGS", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("OWNER", "PURCHASE_OWNER", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("MUSICBRAINZ_ALBUMARTISTID", "MUSICBRAINZ_ALBUMARTISTID", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("MUSICBRAINZ_ALBUMID", "MUSICBRAINZ_ALBUMID", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("MUSICBRAINZ_RELEASEGROUPID", "MUSICBRAINZ_RELEASEGROUPID", Matroska::SimpleTag::TargetTypeValue::Album),
  };

  std::pair<String, Matroska::SimpleTag::TargetTypeValue> translateKey(const String &key)
  {
    auto it = std::find_if(simpleTagsTranslation.cbegin(),
      simpleTagsTranslation.cend(),
      [&key](const auto &t) { return key == std::get<0>(t); }
    );
    if(it != simpleTagsTranslation.end())
      return { std::get<1>(*it), std::get<2>(*it) };
    if (!key.isEmpty() && !key.startsWith("_"))
      return { key, Matroska::SimpleTag::TargetTypeValue::Track };
    return { String(), Matroska::SimpleTag::TargetTypeValue::None };
  }

  String translateTag(const String &name, Matroska::SimpleTag::TargetTypeValue targetTypeValue)
  {
    auto it = std::find_if(simpleTagsTranslation.cbegin(),
      simpleTagsTranslation.cend(),
      [&name, targetTypeValue](const auto &t) {
        return name == std::get<1>(t)
               && targetTypeValue == std::get<2>(t);
      }
    );
    return it != simpleTagsTranslation.end()
      ? String(std::get<0>(*it), String::UTF8)
      : targetTypeValue == Matroska::SimpleTag::TargetTypeValue::Track && !name.startsWith("_")
      ? name
        : String();
  }
}

bool Matroska::Tag::TagPrivate::setTag(const String &key, const String &value)
{
  const auto pair = translateKey(key);
  // Workaround Clang issue - no lambda capture of structured bindings
  const String &name = pair.first;
  auto targetTypeValue = pair.second;
  if(name.isEmpty())
    return false;
  removeSimpleTags(
    [&name, targetTypeValue] (auto t) {
      return t->name() == name
             && t->targetTypeValue() == targetTypeValue;
    }
  );
  if(!value.isEmpty()) {
    auto t = new SimpleTagString();
    t->setTargetTypeValue(targetTypeValue);
    t->setName(name);
    t->setValue(value);
    tags.append(t);
  }
  return true;
}

const String *Matroska::Tag::TagPrivate::getTag(const String &key) const
{
  const auto pair = translateKey(key);
  // Workaround Clang issue - no lambda capture of structured bindings
  const String &name = pair.first;
  auto targetTypeValue = pair.second;
  if(name.isEmpty())
    return nullptr;
  auto tag = dynamic_cast<const SimpleTagString *>(
    findSimpleTag(
      [&name, targetTypeValue] (auto t) {
        return t->name() == name
               && t->targetTypeValue() == targetTypeValue;
      }
    )
  );
  return tag ? &tag->value() : nullptr;
}

PropertyMap Matroska::Tag::setProperties(const PropertyMap &propertyMap)
{
  PropertyMap unsupportedProperties;
  for(const auto &[key, values] : propertyMap) {
    for(const auto &value : values) {
      if(auto [name, targetTypeValue] = translateKey(key);
         !name.isEmpty()) {
        auto t = new SimpleTagString();
        t->setTargetTypeValue(targetTypeValue);
        t->setName(name);
        t->setValue(value);
        d->tags.append(t);
      }
      else {
        unsupportedProperties[key] = values;
      }
    }
  }
  return unsupportedProperties;
}

PropertyMap Matroska::Tag::properties() const
{
  PropertyMap properties;
  SimpleTagString *tStr = nullptr;
  for(auto simpleTag : std::as_const(d->tags)) {
    if((tStr = dynamic_cast<SimpleTagString *>(simpleTag))) {
      String key = translateTag(tStr->name(), tStr->targetTypeValue());
      if(!key.isEmpty())
        properties[key].append(tStr->value());
    }
  }
  return properties;
}
