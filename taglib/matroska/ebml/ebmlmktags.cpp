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

#include "ebmlmktags.h"
#include "ebmlbinaryelement.h"
#include "ebmluintelement.h"
#include "ebmlstringelement.h"
#include "matroskatag.h"
#include "matroskasimpletag.h"
#include "tlist.h"

using namespace TagLib;

std::unique_ptr<Matroska::Tag> EBML::MkTags::parse()
{
  auto mTag = std::make_unique<Matroska::Tag>();
  mTag->setOffset(offset);
  mTag->setSize(getSize());
  mTag->setID(static_cast<Matroska::Element::ID>(id));

  // Loop through each <Tag> element
  for(const auto &tagsChild : elements) {
    if(tagsChild->getId() != Id::MkTag)
      continue;
    auto tag = element_cast<Id::MkTag>(tagsChild);
    List<const MasterElement *> simpleTags;
    const MasterElement *targets = nullptr;

    // Identify the <Targets> element and the <SimpleTag> elements
    for(const auto &tagChild : *tag) {
      Id tagChildId = tagChild->getId();
      if(!targets && tagChildId == Id::MkTagTargets)
        targets = element_cast<Id::MkTagTargets>(tagChild);
      else if(tagChildId == Id::MkSimpleTag)
        simpleTags.append(element_cast<Id::MkSimpleTag>(tagChild));
    }

    // Parse the <Targets> element
    Matroska::SimpleTag::TargetTypeValue targetTypeValue = Matroska::SimpleTag::TargetTypeValue::None;
    if(targets) {
      for(const auto &targetsChild : *targets) {
        Id id = targetsChild->getId();
        if(id == Id::MkTagTargetTypeValue
            && targetTypeValue == Matroska::SimpleTag::TargetTypeValue::None) {
          targetTypeValue = static_cast<Matroska::SimpleTag::TargetTypeValue>(
            element_cast<Id::MkTagTargetTypeValue>(targetsChild)->getValue()
          );
        }
      }
    }

    // Parse each <SimpleTag>
    for(auto simpleTag : simpleTags) {
      const String *tagName = nullptr;
      const String *tagValueString = nullptr;
      const ByteVector *tagValueBinary = nullptr;
      const String *language = nullptr;
      bool defaultLanguageFlag = true;

      for(const auto &simpleTagChild : *simpleTag) {
        Id id = simpleTagChild->getId();
        if(id == Id::MkTagName && !tagName)
          tagName = &(element_cast<Id::MkTagName>(simpleTagChild)->getValue());
        else if(id == Id::MkTagString && !tagValueString)
          tagValueString = &(element_cast<Id::MkTagString>(simpleTagChild)->getValue());
        else if(id == Id::MkTagBinary && !tagValueBinary)
          tagValueBinary = &(element_cast<Id::MkTagBinary>(simpleTagChild)->getValue());
        else if(id == Id::MkTagsTagLanguage && !language)
          language = &(element_cast<Id::MkTagsTagLanguage>(simpleTagChild)->getValue());
        else if(id == Id::MkTagsLanguageDefault)
          defaultLanguageFlag = element_cast<Id::MkTagsLanguageDefault>(simpleTagChild)->getValue() ? true : false;
      }
      if(!tagName || (tagValueString && tagValueBinary) || (!tagValueString && !tagValueBinary))
        continue;

      // Create a Simple Tag object and add it to the Tag object
      Matroska::SimpleTag *sTag = nullptr;
      if(tagValueString) {
        auto sTagString = new Matroska::SimpleTagString();
        sTagString->setTargetTypeValue(targetTypeValue);
        sTagString->setValue(*tagValueString);
        sTag = sTagString;
      }
      else { // tagValueBinary must be non null
        auto sTagBinary = new Matroska::SimpleTagBinary();
        sTagBinary->setTargetTypeValue(targetTypeValue);
        sTagBinary->setValue(*tagValueBinary);
        sTag = sTagBinary;
      }
      sTag->setName(*tagName);
      if(language)
        sTag->setLanguage(*language);
      sTag->setDefaultLanguageFlag(defaultLanguageFlag);
      mTag->addSimpleTag(sTag);
    }
  }
  return mTag;
}
