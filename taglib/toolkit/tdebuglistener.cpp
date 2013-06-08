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

#ifndef NDEBUG
# include <iostream>
# include <bitset>
# ifdef _WIN32
#   include <windows.h>
# endif
#endif

using namespace TagLib;

namespace
{
  class DefaultListener : public DebugListener
  {
  public:
    virtual void printMessage(const String &msg)
    {
#ifndef NDEBUG
# ifdef _WIN32

      std::string s;
      const wstring wstr = msg.toWString();
      const int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
      if(len != 0) {
        s.resize(len);
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &s[0], len, NULL, NULL);
      }

      std::cerr << "TagLib: " << s << std::endl;

# else

      std::cerr << "TagLib: " << msg << std::endl;

# endif 
#endif
    }

    virtual void printData(const ByteVector &v)
    {
#ifndef NDEBUG

      for(size_t i = 0; i < v.size(); i++) 
      {
        std::cout << "*** [" << i << "] - '" << char(v[i]) << "' - int " << int(v[i])
          << std::endl;

        std::bitset<8> b(v[i]);

        for(int j = 0; j < 8; j++)
          std::cout << i << ":" << j << " " << b.test(j) << std::endl;

        std::cout << std::endl;
      }

#endif
    }
  };

  DefaultListener defaultListener;
  DebugListener *debugListener = &defaultListener;
}

TagLib::DebugListener::DebugListener()
{
}

TagLib::DebugListener::~DebugListener()
{
}

void TagLib::setDebugListener(DebugListener *listener)
{
  if(listener)
    debugListener = listener;
  else
    debugListener = &defaultListener;
}

DebugListener *TagLib::getDebugListener()
{
  return debugListener;
}
