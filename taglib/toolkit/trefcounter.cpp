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

#include "trefcounter.h"

#include <atomic>

namespace TagLib
{

  class RefCounter::RefCounterPrivate
  {
  public:
    RefCounterPrivate() :
      refCount(1)
    {
    }

    std::atomic_int refCount;
  };

  RefCounter::RefCounter() :
    d(new RefCounterPrivate())
  {
  }

  RefCounter::~RefCounter()
  {
    delete d;
  }

  void RefCounter::ref()
  {
    d->refCount.fetch_add(1);
  }

  bool RefCounter::deref()
  {
    return d->refCount.fetch_sub(1) == 1;
  }

  int RefCounter::count() const
  {
    return d->refCount.load();
  }
}  // namespace TagLib
