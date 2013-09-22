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

#ifndef TAGLIB_SMARTPTR_H
#define TAGLIB_SMARTPTR_H

// This file is not a part of TagLib public interface. This is not installed.

#include "config.h"

#ifndef DO_NOT_DOCUMENT // Tell Doxygen to skip this class.
/*!
 * \warning This <b>is not</b> part of the TagLib public API!
 */

#if defined(HAVE_STD_SHARED_PTR) 

# include <memory>
# define SHARED_PTR std::shared_ptr

#elif defined(HAVE_TR1_SHARED_PTR) 

# include <tr1/memory>
# define SHARED_PTR std::tr1::shared_ptr

#elif defined(HAVE_BOOST_SHARED_PTR)

# include <boost/shared_ptr.hpp>
# define SHARED_PTR boost::shared_ptr

#else   //  HAVE_STD_SHARED_PTR

# include <algorithm>

# if defined(HAVE_GCC_ATOMIC)
#   define ATOMIC_INT int
#   define ATOMIC_INC(x) __sync_add_and_fetch(&x, 1)
#   define ATOMIC_DEC(x) __sync_sub_and_fetch(&x, 1)
# elif defined(HAVE_WIN_ATOMIC)
#   if !defined(NOMINMAX)
#     define NOMINMAX
#   endif
#   include <windows.h>
#   define ATOMIC_INT long
#   define ATOMIC_INC(x) InterlockedIncrement(&x)
#   define ATOMIC_DEC(x) InterlockedDecrement(&x)
# elif defined(HAVE_MAC_ATOMIC)
#   include <libkern/OSAtomic.h>
#   define ATOMIC_INT int32_t
#   define ATOMIC_INC(x) OSAtomicIncrement32Barrier(&x)
#   define ATOMIC_DEC(x) OSAtomicDecrement32Barrier(&x)
# elif defined(HAVE_IA64_ATOMIC)
#   include <ia64intrin.h>
#   define ATOMIC_INT int
#   define ATOMIC_INC(x) __sync_add_and_fetch(&x, 1)
#   define ATOMIC_DEC(x) __sync_sub_and_fetch(&x, 1)
# else
#   define ATOMIC_INT int
#   define ATOMIC_INC(x) (++x)
#   define ATOMIC_DEC(x) (--x)
# endif

namespace TagLib 
{
  // Self-implements RefCountPtr<T> if shared_ptr<T> is not available.
  // I STRONGLY RECOMMEND using standard shared_ptr<T> rather than this class.
  
  // Counter base class. Provides a reference counter.

  class CounterBase
  {
  public:
    CounterBase()
      : refCount(1)
    {
    }

    virtual ~CounterBase()
    {
    }

    void addref() 
    {    
      ATOMIC_INC(refCount);
    }

    void release()
    {
      if(ATOMIC_DEC(refCount) == 0) {
        dispose();
        delete this;
      }
    }

    long use_count() const
    {
      return static_cast<long>(refCount);
    }

    virtual void dispose() = 0;

  private:
    volatile ATOMIC_INT refCount;
  };

  // Counter impl class. Provides a dynamic deleter.

  template <typename T>
  class CounterImpl : public CounterBase
  {
  public:
    CounterImpl(T *p)
      : p(p)
    {
    }

    virtual void dispose() 
    { 
      delete p; 
    }

    T *get() const
    {
      return p;
    }

  private:
    T *p;
  };

  template<typename T>
  class RefCountPtr
  {
  public:
    RefCountPtr()
      : px(0)
      , counter(0)
    {
    }

    template <typename U>
    explicit RefCountPtr(U *p)
      : px(p)
      , counter(new CounterImpl<U>(p))
    {
    }

    RefCountPtr(const RefCountPtr<T> &x)
      : px(x.px)
      , counter(x.counter)
    {
      if(counter)
        counter->addref();
    }

    template <typename U>
    RefCountPtr(const RefCountPtr<U> &x)
      : px(x.px)
      , counter(x.counter)
    {
      if(counter)
        counter->addref();
    }

    ~RefCountPtr()
    {
      if(counter)
        counter->release();
    }

    T *get() const
    {
      return px;
    }

    long use_count() const
    {
      if(counter)
        return counter->use_count();
      else
        return 0;
    }

    bool unique() const 
    { 
      return (use_count() == 1);
    }

    template <typename U>
    void reset(U *p)
    {
      if(px != p)
        RefCountPtr<T>(p).swap(*this);
    }

    void reset()
    {
      RefCountPtr<T>().swap(*this);
    }

    void swap(RefCountPtr<T> &x)
    {
      std::swap(px, x.px);
      std::swap(counter, x.counter);
    }

    RefCountPtr<T> &operator=(const RefCountPtr<T> &x)
    {
      if(px != x.px) {
        if(counter)
          counter->release();

        px = x.px;
        counter = x.counter;

        if(counter)
          counter->addref();
      }
      return *this;
    }

    template <typename U>
    RefCountPtr<T> &operator=(const RefCountPtr<U> &x)
    {
      if(px != x.px) {
        if(counter)
          counter->release();

        px = x.px;
        counter = x.counter;

        if(counter)
          counter->addref();
      }
      return *this;
    }

    T& operator*() const
    {
      return *px;
    }

    T* operator->() const
    {
      return px;
    }

    operator bool() const
    {
      return (px != 0);
    }

    bool operator!() const
    {
      return (px == 0);
    }

  private:
    T *px;
    CounterBase *counter;

    template <typename U> friend class RefCountPtr;
  };

  template <typename T, typename U>
  bool operator==(const RefCountPtr<T> &a, const RefCountPtr<U> &b)
  {
    return (a.get() == b.get());
  }

  template <typename T, typename U>
  bool operator!=(const RefCountPtr<T> &a, const RefCountPtr<U> &b)
  {
    return (a.get() != b.get());
  }

  template <typename T>
  void swap(RefCountPtr<T> &a, RefCountPtr<T> &b)
  {
    a.swap(b);
  }
}

# define SHARED_PTR TagLib::RefCountPtr

#endif  // HAVE_STD_SHARED_PTR etc.

#if defined(HAVE_STD_UNIQUE_PTR) 

# include <memory>
# define SCOPED_PTR std::unique_ptr

#elif defined(HAVE_BOOST_SCOPED_PTR)

# include <boost/scoped_ptr.hpp>
# define SCOPED_PTR boost::scoped_ptr

#else // HAVE_STD_UNIQUE_PTR
 
# include <algorithm>

namespace TagLib
{
  // Self-implements NonRefCountPtr<T> if unique_ptr<T> is not available.
  // I STRONGLY RECOMMEND using standard unique_ptr<T> rather than this class.

  template<typename T> 
  class NonRefCountPtr
  {
  public:
    explicit NonRefCountPtr(T *p = 0)
      : px(p) 
    {
    }

    ~NonRefCountPtr()
    {
      delete px;
    }

    void reset(T *p = 0)
    {
      NonRefCountPtr<T>(p).swap(*this);
    }

    T &operator*() const
    {
      return *px;
    }

    T *operator->() const 
    {
      return px;
    }

    T *get() const
    {
      return px;
    }

    operator bool() const
    {
      return (px != 0);
    }

    bool operator!() const
    {
      return (px == 0);
    }


    void swap(NonRefCountPtr &x) 
    {
      std::swap(px, x.px);
    }

  private:

    // Noncopyable
    NonRefCountPtr(const NonRefCountPtr &);
    NonRefCountPtr &operator=(const NonRefCountPtr &);

    void operator==(const NonRefCountPtr &) const;
    void operator!=(const NonRefCountPtr &) const;

    T *px;
  };

  template <typename T, typename U>
  bool operator==(const NonRefCountPtr<T> &a, U *b)
  {
    return (a.get() == b);
  }

  template <typename T, typename U>
  bool operator!=(const NonRefCountPtr<T> &a, U *b)
  {
    return (a.get() != b);
  }

  template <typename T>
    void swap(NonRefCountPtr<T> &a, NonRefCountPtr<T> &b)
  {
    a.swap(b);
  }
}

# define SCOPED_PTR TagLib::NonRefCountPtr

#endif  // HAVE_STD_UNIQUE_PTR etc.


#endif // DO_NOT_DOCUMENT
#endif
