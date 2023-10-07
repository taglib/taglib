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
#include "matroskasimpletag.h"
#include "ebmlmasterelement.h"
#include "ebmlstringelement.h"
#include "ebmlmktags.h"
#include "ebmluintelement.h"
#include "ebmlutils.h"
#include "tpropertymap.h"
#include "tlist.h"
#include "tdebug.h"

#include <algorithm>
#include <array>
#include <tuple>

using namespace TagLib;

namespace TagLib {
  namespace Matroska {
    namespace Utils {
      std::pair<String, Matroska::SimpleTag::TargetTypeValue> translateKey(const String &key);
      String translateTag(const String &name, Matroska::SimpleTag::TargetTypeValue targetTypeValue);
    }
  }
}

class Matroska::Tag::TagPrivate 
{
  public:
    TagPrivate() = default;
    ~TagPrivate() = default;
    List<SimpleTag*> tags;

};

Matroska::Tag::Tag()
: TagLib::Tag(),
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
  if (it != d->tags.end())
    d->tags.erase(it);
}

void Matroska::Tag::clearSimpleTags()
{
  d->tags.clear();
}

const Matroska::SimpleTagsList& Matroska::Tag::simpleTagsList() const
{
  return d->tags;
}

Matroska::SimpleTagsList& Matroska::Tag::simpleTagsListPrivate()
{
  return d->tags;
}

const Matroska::SimpleTagsList& Matroska::Tag::simpleTagsListPrivate() const
{
  return d->tags;
}

void Matroska::Tag::setTitle(const String &s)
{
  setTag("TITLE", s);
}

void Matroska::Tag::setArtist(const String &s)
{
  setTag("ARTIST", s);
}

void Matroska::Tag::setAlbum(const String &s)
{
  setTag("ALBUM", s);
}

void Matroska::Tag::setComment(const String &s)
{
  setTag("COMMENT", s);
}

void Matroska::Tag::setGenre(const String &s)
{
  setTag("GENRE", s);
}

void Matroska::Tag::setYear(unsigned int i)
{
  setTag("DATE", String::number(i));
}

void Matroska::Tag::setTrack(unsigned int i)
{
  setTag("TRACKNUMBER", String::number(i));
}

String Matroska::Tag::title() const
{
  const auto value = getTag("TITLE");
  return value ? *value : String(); 
}

String Matroska::Tag::artist() const
{
  const auto value = getTag("ARTIST");
  return value ? *value : String(); 
}

String Matroska::Tag::album() const
{
  const auto value = getTag("ALBUM");
  return value ? *value : String(); 
}

String Matroska::Tag::comment() const
{ 
  const auto value = getTag("COMMENT");
  return value ? *value : String(); 
}

String Matroska::Tag::genre() const
{
  const auto value = getTag("GENRE");
  return value ? *value : String(); 
}

unsigned int Matroska::Tag::year() const
{
  auto value = getTag("DATE");
  if (!value)
    return 0;
  auto list = value->split("-");
  return static_cast<unsigned int>(list.front().toInt());
}

unsigned int Matroska::Tag::track() const
{
  auto value = getTag("TRACKNUMBER");
  if (!value)
    return 0;
  auto list = value->split("-");
  return static_cast<unsigned int>(list.front().toInt());
}

bool Matroska::Tag::isEmpty() const
{
  return d->tags.isEmpty();
}

ByteVector Matroska::Tag::render()
{
  EBML::MkTags tags;
  List<List<SimpleTag*>*> targetList;
  targetList.setAutoDelete(true);

  // Build target-based list
  for (auto tag : d->tags) {
    auto targetTypeValue = tag->targetTypeValue();
    auto it = std::find_if(targetList.begin(),
      targetList.end(),
      [&](auto list) {
        const auto *simpleTag = list->front();
        return simpleTag->targetTypeValue() == targetTypeValue;
      }
    );
    if (it == targetList.end()) {
      auto list = new List<SimpleTag*>();
      list->append(tag);
      targetList.append(list);
    }
    else
      (*it)->append(tag);
  }
  for (auto list : targetList) {
    auto frontTag = list->front();
    auto targetTypeValue = frontTag->targetTypeValue();
    auto tag = new EBML::MasterElement(EBML::ElementIDs::MkTag);

    // Build <Tag Targets element>
    auto targets = new EBML::MasterElement(EBML::ElementIDs::MkTagTargets);
    if (targetTypeValue != Matroska::SimpleTag::TargetTypeValue::None) {
      auto element = new EBML::UIntElement(EBML::ElementIDs::MkTagTargetTypeValue);
      element->setValue(static_cast<unsigned int>(targetTypeValue));
      targets->appendElement(element);
    }
    tag->appendElement(targets);

    // Build <Simple Tag> element
    for (auto simpleTag : *list) {
      auto t = new EBML::MasterElement(EBML::ElementIDs::MkSimpleTag);
      auto tagName = new EBML::UTF8StringElement(EBML::ElementIDs::MkTagName);
      tagName->setValue(simpleTag->name());
      t->appendElement(tagName);

      Matroska::SimpleTagString *tStr = nullptr;
      Matroska::SimpleTagBinary *tBin = nullptr;
      if((tStr = dynamic_cast<Matroska::SimpleTagString*>(simpleTag))) {
        auto tagValue = new EBML::UTF8StringElement(EBML::ElementIDs::MkTagString);
        tagValue->setValue(tStr->value());
        t->appendElement(tagValue);
      }
      else if((tBin = dynamic_cast<Matroska::SimpleTagBinary*>(simpleTag))) {
        // Todo
      }

      // Todo: language
      tag->appendElement(t);
    }
    tags.appendElement(tag);
  }

  return tags.render();
}

namespace
{
  // PropertyMap key, Tag name, Target type value
  constexpr std::array simpleTagsTranslation {
    std::tuple("TITLE", "TITLE", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("ALBUM", "TITLE", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("ARTIST", "ARTIST", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("ALBUMARTIST", "ARTIST", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("SUBTITLE", "SUBTITLE", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("TRACKNUMBER", "PART_NUMBER", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("DISCNUMBER", "PART_NUMBER", Matroska::SimpleTag::TargetTypeValue::Part),
    std::tuple("DATE", "DATE_RELEASED", Matroska::SimpleTag::TargetTypeValue::Album),
    // Todo - original date
    std::tuple("GENRE", "GENRE", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("COMMENT", "COMMENT", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("TITLESORT", "TITLESORT", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("ALBUMSORT", "TITLESORT", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("ARTISTSORT", "ARTISTSORT", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("ARTISTSORT", "ARTISTSORT", Matroska::SimpleTag::TargetTypeValue::Album),
    std::tuple("COMPOSERSORT", "COMPOSERSORT", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("COMPOSER", "COMPOSER", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("LYRICIST", "LYRICIST", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("CONDUCTOR", "CONDUCTOR", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("REMIXER", "REMIXER", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("BPM", "BPM", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("COPYRIGHT", "COPYRIGHT", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("ENCODEDBY", "ENCODED_BY", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("MOOD", "MOOD", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("MEDIA", "ORIGINAL_MEDIA_TYPE", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("LABEL", "LABEL_CODE", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("CATALOGNUMBER", "CATALOG_NUMBER", Matroska::SimpleTag::TargetTypeValue::Track),
    std::tuple("BARCODE", "BARCODE", Matroska::SimpleTag::TargetTypeValue::Track),
    // Todo MusicBrainz tags
  };
}

bool Matroska::Tag::setTag(const String &key, const String &value)
{
  const auto pair = Matroska::Utils::translateKey(key);
  // Workaround Clang issue - no lambda capture of structured bindings
  const String &name = pair.first;
  auto targetTypeValue = pair.second;
  if (name.isEmpty())
    return false;
  removeSimpleTags(
    [&name, targetTypeValue] (auto t) { 
      return t->name() == name 
             && t->targetTypeValue() == targetTypeValue;
    }
  );
  if (!value.isEmpty()) {
    auto t = new Matroska::SimpleTagString();
    t->setTargetTypeValue(targetTypeValue);
    t->setName(name);
    t->setValue(value);
    addSimpleTag(t);
  }
  return true;
}

const String* Matroska::Tag::getTag(const String &key) const
{
  const auto pair = Matroska::Utils::translateKey(key);
  // Workaround Clang issue - no lambda capture of structured bindings
  const String &name = pair.first;
  auto targetTypeValue = pair.second;
  if (name.isEmpty())
    return nullptr;
  auto tag = dynamic_cast<const Matroska::SimpleTagString*>(
    findSimpleTag(
      [&name, targetTypeValue] (auto t) {
        return t->name() == name
               && t->targetTypeValue() == targetTypeValue;
      }
    )
  );
  return tag ? &tag->value() : nullptr;
}

std::pair<String, Matroska::SimpleTag::TargetTypeValue> Matroska::Utils::translateKey(const String &key)
{
  auto it = std::find_if(simpleTagsTranslation.cbegin(),
    simpleTagsTranslation.cend(), 
    [&key](const auto &t) { return key == std::get<0>(t); }
  );
  if (it != simpleTagsTranslation.end())
    return { std::get<1>(*it), std::get<2>(*it) };
  else
    return { String(), Matroska::SimpleTag::TargetTypeValue::None };
}

String Matroska::Utils::translateTag(const String &name, Matroska::SimpleTag::TargetTypeValue targetTypeValue)
{
  auto it = std::find_if(simpleTagsTranslation.cbegin(),
    simpleTagsTranslation.cend(),
    [&name, targetTypeValue](const auto &t) {
      return name == std::get<1>(t)
             && targetTypeValue == std::get<2>(t);
    }
  );
  return it != simpleTagsTranslation.end() ? std::get<0>(*it) : String();
}

PropertyMap Matroska::Tag::setProperties(const PropertyMap &propertyMap)
{
  PropertyMap unsupportedProperties;
  for (const auto& [key, value] : propertyMap) {
    if (!setTag(key, value.toString()))
      unsupportedProperties[key] = value;
  }
  return unsupportedProperties;
}

PropertyMap Matroska::Tag::properties() const
{
  PropertyMap properties;
  Matroska::SimpleTagString *tStr = nullptr;
  for (auto simpleTag : d->tags) {
    if ((tStr = dynamic_cast<Matroska::SimpleTagString*>(simpleTag))) {
      String key = Matroska::Utils::translateTag(tStr->name(), tStr->targetTypeValue());
      if (!key.isEmpty() && !properties.contains(key))
        properties[key] = tStr->value();
    }
  }
  return properties;
}
