/***************************************************************************
    copyright            : (C) 2013 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
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

#include "tdebuglistener.h"

#include <iostream>

#ifdef _WIN32
# include <windows.h>
#endif

using namespace TagLib;

namespace
{
  class DefaultListener : public DebugListener
  {
  public:
    void printMessage(const String &msg) override
    {
#ifdef _WIN32

      const std::wstring wstr = msg.toWString();
      const int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
      if(len != 0) {
        std::vector<char> buf(len);
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buf.data(), len, nullptr, nullptr);

        std::cerr << std::string(buf.begin(), buf.end());
      }

#else

      std::cerr << msg;

#endif
    }
  };

  DefaultListener defaultListener;
}  // namespace

namespace TagLib
{
  class DebugListener::DebugListenerPrivate
  {
  };

  DebugListener *debugListener = &defaultListener;

  DebugListener::DebugListener() = default;

  DebugListener::~DebugListener() = default;

  void setDebugListener(DebugListener *listener)
  {
    if(listener)
      debugListener = listener;
    else
      debugListener = &defaultListener;
  }
}  // namespace TagLib
