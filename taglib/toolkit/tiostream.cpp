/***************************************************************************
    copyright            : (C) 2011 by Lukas Lalinsky
    email                : lalinsky@gmail.com
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

#include "tiostream.h"

using namespace TagLib;

#ifdef _WIN32

// MSVC 2008 or later can't produce the binary for Win9x.
#if !defined(_MSC_VER) || (_MSC_VER < 1500)

namespace 
{

  // Determines whether or not the running system is WinNT.
  // In other words, whether the system supports Unicode.

  bool isWinNT() 
  {
    OSVERSIONINFOA ver = {};
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    if(GetVersionExA(&ver)) {
      return (ver.dwPlatformId == VER_PLATFORM_WIN32_NT);
    }
    else {
      return false;
    }
  }
  
  const bool IsWinNT = isWinNT();

  // Converts a UTF-16 string into a local encoding.

  std::string unicodeToAnsi(const std::wstring &wstr)
  {
    const int len = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
    if(len == 0)
      return std::string();

    std::string str(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, &str[0], len, NULL, NULL);

    return str;
  }
}

// If WinNT, stores a Unicode string into m_wname directly.
// If Win9x, converts and stores it into m_name to avoid calling Unicode version functions.

FileName::FileName(const wchar_t *name) 
  : m_wname(IsWinNT ? name : L"")
  , m_name(IsWinNT ? "" : unicodeToAnsi(name))
{
}

#else

FileName::FileName(const wchar_t *name) 
  : m_wname(name)
{
}

#endif 

FileName::FileName(const char *name) 
  : m_name(name) 
{
}

FileName::FileName(const FileName &name) 
  : m_wname(name.m_wname)
  , m_name(name.m_name) 
{
}

FileName::operator const wchar_t *() const 
{ 
  return m_wname.c_str(); 
}

FileName::operator const char *() const 
{ 
  return m_name.c_str(); 
}

const std::wstring &FileName::wstr() const 
{ 
  return m_wname; 
}

const std::string &FileName::str() const 
{ 
  return m_name; 
}  

#endif

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

IOStream::IOStream()
{
}

IOStream::~IOStream()
{
}

void IOStream::clear()
{
}

