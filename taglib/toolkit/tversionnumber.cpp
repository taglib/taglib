/***************************************************************************
    copyright            : (C) 2020 by Kevin Andre
    email                : hyperquantum@gmail.com

    copyright            : (C) 2023 by Urs Fleisch
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

#include "tversionnumber.h"

#include "tstring.h"
#include "taglib.h"

using namespace TagLib;

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

String VersionNumber::toString() const
{
  return String::number(majorVersion()) + '.' +
         String::number(minorVersion()) + '.' +
         String::number(patchVersion());
}

////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

VersionNumber TagLib::runtimeVersion()
{
  return VersionNumber(TAGLIB_MAJOR_VERSION, TAGLIB_MINOR_VERSION, TAGLIB_PATCH_VERSION);
}
