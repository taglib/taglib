/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
 ***************************************************************************/

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

#include "ebmlmkinfo.h"
#include "ebmlstringelement.h"
#include "ebmluintelement.h"
#include "ebmlfloatelement.h"
#include "matroskaproperties.h"

using namespace TagLib;

void EBML::MkInfo::parse(Matroska::Properties *properties)
{
  if(!properties)
    return;

  unsigned long long timestampScale = 1000000;
  double duration = 0.0;
  String title;
  for(const auto &element : elements) {
    Id id = element->getId();
    if (id == Id::MkTimestampScale) {
      timestampScale = element_cast<Id::MkTimestampScale>(element)->getValue();
    }
    else if (id == Id::MkDuration) {
      duration = element_cast<Id::MkDuration>(element)->getValueAsDouble();
    }
    else if (id == Id::MkTitle) {
      title = element_cast<Id::MkTitle>(element)->getValue();
    }
  }

  properties->setLengthInMilliseconds(
    static_cast<int>(duration * static_cast<double>(timestampScale) / 1000000.0));
  properties->setTitle(title);
}
