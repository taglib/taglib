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

#ifdef _WIN32
# include <windows.h>
#endif

#include "tdebug.h"
#include "tstring.h"

using namespace TagLib;

#ifdef _WIN32

namespace 
{
  // Check if the running system has OutputDebugStringW() function.
  // Windows9x systems don't have OutputDebugStringW() or can't accept Unicode messages. 

  bool supportsUnicode()
  {
    const FARPROC p = GetProcAddress(GetModuleHandleA("kernel32"), "OutputDebugStringW");
    return (p != NULL);
  }

  // Indicates whether the system supports Unicode debug messages.

  const bool SystemSupportsUnicode = supportsUnicode(); 

  // Converts a UTF-16 string into a local encoding.
  // This function should only be used in Windows9x systems which don't support 
  // Unicode file names.

  std::string unicodeToAnsi(const String &s)
  {
    const wchar_t *wstr = s.toWString().c_str();
    const int len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    if(len == 0)
      return std::string();

    std::string str(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, &str[0], len, NULL, NULL);

    return str;
  }
}

#endif  // _WIN32

void TagLib::debug(const String &s)
{
  String msg = "TagLib: " + s + "\n";

#ifdef _WIN32

  if(SystemSupportsUnicode) 
    OutputDebugStringW(msg.toWString().c_str());
  else
    OutputDebugStringA(unicodeToAnsi(s).c_str());

#else

  std::cerr << msg;

#endif
}

#endif
