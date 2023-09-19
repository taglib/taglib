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
    if (tagsChild->getId() != EBML_ID_MK_TAG)
      continue;
    auto tag = static_cast<MasterElement*>(tagsChild);
    List<MasterElement*> simpleTags;
    MasterElement *targets = nullptr;

    // Identify the <Targets> element and the <SimpleTag> elements
    for (auto tagChild : *tag) {
      Id tagChildId = tagChild->getId();
      if (!targets && tagChildId == EBML_ID_MK_TAG_TARGETS)
        targets = static_cast<MasterElement*>(tagChild);
      else if (tagChildId == EBML_ID_MK_SIMPLE_TAG)
        simpleTags.append(static_cast<MasterElement*>(tagChild));
    }

    // Parse the <Targets> element
    Matroska::Tag::TargetTypeValue targetTypeValue = Matroska::Tag::TargetTypeValue::None;
    if (targets) {
      for (auto targetsChild : *targets) {
        Id id = targetsChild->getId();
        if (id == EBML_ID_MK_TARGET_TYPE_VALUE
            && targetTypeValue == Matroska::Tag::TargetTypeValue::None) {
          targetTypeValue = static_cast<Matroska::Tag::TargetTypeValue>(
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

      for (auto simpleTagChild : *simpleTag) {
        Id id = simpleTagChild->getId();
        if (id == EBML_ID_MK_TAG_NAME && !tagName)
          tagName = &(static_cast<UTF8StringElement*>(simpleTagChild)->getValue());
        else if (id == EBML_ID_MK_TAG_STRING && !tagValueString)
          tagValueString = &(static_cast<UTF8StringElement*>(simpleTagChild)->getValue());
      }
      if (!tagName || (tagValueString && tagValueBinary) || (!tagValueString && !tagValueBinary))
        continue;

      // Create a Simple Tag object and add it to the Tag object
      Matroska::SimpleTag *sTag = nullptr;
      if (tagValueString) {
        auto sTagString = new Matroska::SimpleTagString(targetTypeValue);
        sTagString->setValue(*tagValueString);
        sTag = sTagString;
      }
      else if (tagValueBinary) {
        auto sTagBinary = new Matroska::SimpleTagBinary(targetTypeValue);
        sTagBinary->setValue(*tagValueBinary);
        sTag = sTagBinary;
      }
      sTag->setName(*tagName);
      mTag->addSimpleTag(sTag);
    }
  }
  return mTag;
}
