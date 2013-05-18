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

#include "taglib_config.h"

#if !defined(TAGLIB_USE_STD_SHARED_PTR) \
  && !defined(TAGLIB_USE_TR1_SHARED_PTR) \
  && !defined(TAGLIB_USE_BOOST_SHARED_PTR)

#include "config.h"
#include "tsmartptr.h"

#if defined(HAVE_STD_ATOMIC)
# include <atomic>
# define ATOMIC_INT std::atomic<unsigned int>
# define ATOMIC_INC(x) x.fetch_add(1)
# define ATOMIC_DEC(x) (x.fetch_sub(1) - 1)
#elif defined(HAVE_BOOST_ATOMIC)
# include <boost/atomic.hpp>
# define ATOMIC_INT boost::atomic<unsigned int>
# define ATOMIC_INC(x) x.fetch_add(1)
# define ATOMIC_DEC(x) (x.fetch_sub(1) - 1)
#elif defined(HAVE_GCC_ATOMIC)
# define ATOMIC_INT int
# define ATOMIC_INC(x) __sync_add_and_fetch(&x, 1)
# define ATOMIC_DEC(x) __sync_sub_and_fetch(&x, 1)
#elif defined(HAVE_WIN_ATOMIC)
# if !defined(NOMINMAX)
#   define NOMINMAX
# endif
# include <windows.h>
# define ATOMIC_INT long
# define ATOMIC_INC(x) InterlockedIncrement(&x)
# define ATOMIC_DEC(x) InterlockedDecrement(&x)
#elif defined(HAVE_MAC_ATOMIC)
# include <libkern/OSAtomic.h>
# define ATOMIC_INT int32_t
# define ATOMIC_INC(x) OSAtomicIncrement32Barrier(&x)
# define ATOMIC_DEC(x) OSAtomicDecrement32Barrier(&x)
#elif defined(HAVE_IA64_ATOMIC)
# include <ia64intrin.h>
# define ATOMIC_INT int
# define ATOMIC_INC(x) __sync_add_and_fetch(&x, 1)
# define ATOMIC_DEC(x) __sync_sub_and_fetch(&x, 1)
#else
# define ATOMIC_INT int
# define ATOMIC_INC(x) (++x)
# define ATOMIC_DEC(x) (--x)
#endif

namespace TagLib
{
  class CounterBase::CounterBasePrivate
  {
  public:
    CounterBasePrivate() 
      : refCount(1) 
    {
    }

    volatile ATOMIC_INT refCount;
  };

  CounterBase::CounterBase()
    : d(new CounterBasePrivate())
  {
  }

  CounterBase::~CounterBase()
  {
    delete d;
  }

  void CounterBase::addref() 
  { 
    ATOMIC_INC(d->refCount); 
  }

  void CounterBase::release()
  {
    if(ATOMIC_DEC(d->refCount) == 0) {
      dispose();
      delete this;
    }
  }

  long CounterBase::use_count() const
  {
    return static_cast<long>(d->refCount);
  }
}

#endif
