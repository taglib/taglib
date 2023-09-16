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

#ifdef _WIN32
# include <windows.h>
# include "tstring.h"
#endif

using namespace TagLib;

#ifdef _WIN32

namespace
{
  std::wstring ansiToUnicode(const char *str)
  {
    const int len = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
    if(len == 0)
      return std::wstring();

    std::wstring wstr(len - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, str, -1, wstr.data(), len);

    return wstr;
  }
} // namespace

FileName::FileName(const wchar_t *name) :
  m_wname(name)
{
}

FileName::FileName(const char *name) :
  m_wname(ansiToUnicode(name))
{
}

FileName::FileName(const FileName &) = default;

FileName::operator const wchar_t *() const
{
  return m_wname.c_str();
}

const std::wstring &FileName::wstr() const
{
  return m_wname;
}

String FileName::toString() const
{
  return String(m_wname.c_str());
}

#endif  // _WIN32

class IOStream::IOStreamPrivate
{
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

IOStream::IOStream() = default;

IOStream::~IOStream() = default;

void IOStream::clear()
{
}
