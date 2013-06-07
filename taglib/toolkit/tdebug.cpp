/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#ifndef NDEBUG
#include <iostream>
#include <bitset>

#include "tdebug.h"
#include "tstring.h"

#ifdef _WIN32
# include <windows.h>
#endif

using namespace TagLib;

#ifdef _WIN32

namespace
{
  std::string unicodeToAnsi(const String &s) 
  {
    const wstring wstr = s.toWString();
    const int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if(len == 0)
      return std::string();

    std::string str(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], len, NULL, NULL);

    return str;
  }
}

#endif

void TagLib::debug(const String &s)
{
  std::cerr << "TagLib: ";
  
#ifdef _WIN32

  std::cerr << unicodeToAnsi(s);

#else

  std::cerr << s;

#endif

  std::cerr << std::endl;
}

void TagLib::debugData(const ByteVector &v)
{
  for(uint i = 0; i < v.size(); i++) {

    std::cout << "*** [" << i << "] - '" << char(v[i]) << "' - int " << int(v[i])
              << std::endl;

    std::bitset<8> b(v[i]);

    for(int j = 0; j < 8; j++)
      std::cout << i << ":" << j << " " << b.test(j) << std::endl;

    std::cout << std::endl;
  }
}
#endif
