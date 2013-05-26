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

# include "tstring.h"
# include "tdebug.h"
# include <windows.h>

namespace 
{
  // Check if the running system has CreateFileW() function.
  // Windows9x systems don't have CreateFileW() or can't accept Unicode file names. 

  bool supportsUnicode()
  {
    const FARPROC p = GetProcAddress(GetModuleHandleA("kernel32"), "CreateFileW");
    return (p != NULL);
  }

  // Indicates whether the system supports Unicode file names.
  
  const bool SystemSupportsUnicode = supportsUnicode(); 

  // Converts a UTF-16 string into a local encoding.
  // This function should only be used in Windows9x systems which don't support 
  // Unicode file names.

  std::string unicodeToAnsi(const wchar_t *wstr)
  {
    if(SystemSupportsUnicode) {
      debug("unicodeToAnsi() - Should not be used on WinNT systems.");
    }

    const int len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    if(len == 0)
      return std::string();

    std::string str(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, &str[0], len, NULL, NULL);

    return str;
  }
}

class FileName::FileNamePrivate
{
public:
  std::wstring wname;
  std::string  name;
};

FileName::FileName(const wchar_t *name) 
  : d(new FileNamePrivate())
{
  // If Windows NT, stores a Unicode string directly.
  // If Windows 9x, stores it converting into an ANSI string.
  if(SystemSupportsUnicode)
    d->wname = name;
  else
    d->name = unicodeToAnsi(name);
}

FileName::FileName(const char *name) 
  : d(new FileNamePrivate())
{
  d->name = name;
}

FileName::FileName(const FileName &name) 
  : d(new FileNamePrivate(*name.d))
{
}

FileName &FileName::operator==(const FileName &name)
{
  *d = *name.d;
  return *this;
}

FileName::operator const wchar_t *() const 
{ 
  return d->wname.c_str(); 
}

FileName::operator const char *() const 
{ 
  return d->name.c_str(); 
}

const std::wstring &FileName::wstr() const 
{ 
  return d->wname; 
}

const std::string &FileName::str() const 
{ 
  return d->name; 
}  

#endif  // _WIN32

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

