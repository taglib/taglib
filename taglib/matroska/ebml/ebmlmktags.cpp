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
#include "ebmluintelement.h"
#include "ebmlstringelement.h"
#include "ebmlutils.h"
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskasimpletag.h"
#include "tlist.h"
#include "tdebug.h"
#include "tutils.h"

using namespace TagLib;

Matroska::Tag* EBML::MkTags::parse()
{
  auto mTag = new Matroska::Tag();

  // Loop through each <Tag> element
  for (auto tagsChild : elements) {
    if (tagsChild->getId() != ElementIDs::MkTag)
      continue;
    auto tag = static_cast<MasterElement*>(tagsChild);
    List<MasterElement*> simpleTags;
    MasterElement *targets = nullptr;

    // Identify the <Targets> element and the <SimpleTag> elements
    for (auto tagChild : *tag) {
      Id tagChildId = tagChild->getId();
      if (!targets && tagChildId == ElementIDs::MkTagTargets)
        targets = static_cast<MasterElement*>(tagChild);
      else if (tagChildId == ElementIDs::MkSimpleTag)
        simpleTags.append(static_cast<MasterElement*>(tagChild));
    }

    // Parse the <Targets> element
    Matroska::SimpleTag::TargetTypeValue targetTypeValue = Matroska::SimpleTag::TargetTypeValue::None;
    if (targets) {
      for (auto targetsChild : *targets) {
        Id id = targetsChild->getId();
        if (id == ElementIDs::MkTagTargetTypeValue
            && targetTypeValue == Matroska::SimpleTag::TargetTypeValue::None) {
          targetTypeValue = static_cast<Matroska::SimpleTag::TargetTypeValue>(
            static_cast<UIntElement*>(targetsChild)->getValue()
          );            
        }
      }
    }

    // Parse each <SimpleTag>
    for (auto simpleTag : simpleTags) {
      const String *tagName = nullptr;
      const String *tagValueString = nullptr;
      const ByteVector *tagValueBinary = nullptr;
      const String *language = nullptr;
      bool defaultLanguageFlag = true;

      for (auto simpleTagChild : *simpleTag) {
        Id id = simpleTagChild->getId();
        if (id == ElementIDs::MkTagName && !tagName)
          tagName = &(static_cast<UTF8StringElement*>(simpleTagChild)->getValue());
        else if (id == ElementIDs::MkTagString && !tagValueString)
          tagValueString = &(static_cast<UTF8StringElement*>(simpleTagChild)->getValue());
        else if (id == ElementIDs::MkTagsTagLanguage && !language)
          language = &(static_cast<Latin1StringElement*>(simpleTagChild)->getValue());
        else if (id == ElementIDs::MkTagsLanguageDefault)
          defaultLanguageFlag = static_cast<UIntElement*>(simpleTagChild)->getValue() ? true : false;
      }
      if (!tagName || (tagValueString && tagValueBinary) || (!tagValueString && !tagValueBinary))
        continue;

      // Create a Simple Tag object and add it to the Tag object
      Matroska::SimpleTag *sTag = nullptr;
      if (tagValueString) {
        auto sTagString = new Matroska::SimpleTagString();
        sTagString->setTargetTypeValue(targetTypeValue);
        sTagString->setValue(*tagValueString);
        sTag = sTagString;
      }
      else if (tagValueBinary) {
        auto sTagBinary = new Matroska::SimpleTagBinary();
        sTagBinary->setTargetTypeValue(targetTypeValue);
        sTagBinary->setValue(*tagValueBinary);
        sTag = sTagBinary;
      }
      sTag->setName(*tagName);
      if (language)
        sTag->setLanguage(*language);
      sTag->setDefaultLanguageFlag(defaultLanguageFlag);
      mTag->addSimpleTag(sTag);
    }
  }
  return mTag;
}
