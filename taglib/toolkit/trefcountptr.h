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

#ifndef TAGLIB_REFCOUNTPTR_H
#define TAGLIB_REFCOUNTPTR_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(HAVE_STD_SHARED_PTR)
# include <memory>
#elif defined(HAVE_TR1_SHARED_PTR) 
# include <tr1/memory>
#elif defined(HAVE_BOOST_SHARED_PTR) 
# include <boost/shared_ptr.hpp>
#else
# ifdef __APPLE__
#   include <libkern/OSAtomic.h>
#   define TAGLIB_ATOMIC_MAC
# elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#   define TAGLIB_ATOMIC_WIN
# elif defined (__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 401)    \
  && (defined(__i386__) || defined(__i486__) || defined(__i586__) || \
  defined(__i686__) || defined(__x86_64) || defined(__ia64)) \
  && !defined(__INTEL_COMPILER)
#   define TAGLIB_ATOMIC_GCC
# elif defined(__ia64) && defined(__INTEL_COMPILER)
#   include <ia64intrin.h>
#   define TAGLIB_ATOMIC_GCC
# endif
#endif

#ifndef DO_NOT_DOCUMENT // Tell Doxygen to skip this class.
/*!
 * \internal
 * This is just used as a smart pointer for shared classes in TagLib.
 *
 * \warning This <b>is not</b> part of the TagLib public API!
 */

namespace TagLib {

#if defined(HAVE_STD_SHARED_PTR) || defined(HAVE_TR1_SHARED_PTR) || defined(HAVE_BOOST_SHARED_PTR)

# if defined(SUPPORT_TEMPLATE_ALIAS)

  // Defines RefCountPtr<T> as an alias of shared_ptr<T>
  // if shared_ptr<T> and the template alias are both available.

#   if defined(HAVE_STD_SHARED_PTR) || defined(HAVE_TR1_SHARED_PTR)

  template <typename T> 
  using RefCountPtr = std::tr1::shared_ptr<T>;

#   else

  template <typename T> 
  using RefCountPtr = boost::shared_ptr<T>;

#   endif

# else

  // Defines RefCountPtr<T> as a derived class of shared_ptr<T>.
  // if shared_ptr<T> is available but the template alias is not.

#   if defined(HAVE_STD_SHARED_PTR) || defined(HAVE_TR1_SHARED_PTR)

  template <typename T>
  class RefCountPtr : public std::tr1::shared_ptr<T>
  {
  public:
    explicit RefCountPtr(T *p) : std::tr1::shared_ptr<T>(p) {}
  };

#   else

  template <typename T>
  class RefCountPtr : public boost::shared_ptr<T>
  {
  public:
    explicit RefCountPtr(T *p) : boost::shared_ptr<T>(p) {}
  };

#   endif

# endif

#else // HAVE_*_SHARED_PTR

  // Implements RefCountPtr<T> if shared_ptr<T> is not available.

  template<typename T>
  class RefCountPtr
  {
  private:

    // Counter base class. Provides a reference counter.

    class CounterBase
    {
    public:
      CounterBase() 
        : count(1) 
      {
      }

      virtual ~CounterBase()
      {
      }

      void addref() 
      { 
        increment(&count); 
      }

      void release()
      {
        if(decrement(&count) == 0) {
          dispose();
          delete this;
        }
      }

      long use_count() const
      {
        return static_cast<long>(count);
      }

      virtual void dispose() = 0;

    private:
# if defined(TAGLIB_ATOMIC_MAC)
      typedef volatile int32_t counter_t;

      inline static void increment(counter_t *c) { OSAtomicIncrement32Barrier(c); }
      inline static counter_t decrement(counter_t *c) { return OSAtomicDecrement32Barrier(c); }

# elif defined(TAGLIB_ATOMIC_WIN)
      typedef volatile long counter_t;

      inline static void increment(counter_t *c) { InterlockedIncrement(c); }
      inline static counter_t decrement(counter_t *c) { return InterlockedDecrement(c); }

# elif defined(TAGLIB_ATOMIC_GCC)
      typedef volatile int counter_t;

      inline static void increment(counter_t *c) { __sync_add_and_fetch(c, 1); }
      inline static counter_t decrement(counter_t *c) { return __sync_sub_and_fetch(c, 1); }

# else
      typedef uint counter_t;

      inline static void increment(counter_t *c) { ++(*c) }
      inline static counter_t decrement(counter_t *c) { return --(*c); }

# endif

      counter_t count;
    };

    // Counter impl class. Provides a dynamic deleter.

    template <typename U>
    class CounterImpl : public CounterBase
    {
    public:
      CounterImpl(U *p)
        : p(p)
      {
      }

      virtual void dispose() 
      { 
        delete p; 
      }

      U *get() const
      {
        return p;
      }

    private:
      U *p;
    };

  public:
    template <typename U>
    explicit RefCountPtr(U *p)
      : counter(new CounterImpl<U>(p))
    {
    }

    RefCountPtr(const RefCountPtr &x)
    {
      counter = x.counter;
      counter->addref();
    }

    ~RefCountPtr()
    {
      counter->release();
    }

    T *get() const
    {
      return static_cast<CounterImpl<T>*>(counter)->get();
    }

    long use_count() const
    {
      return counter->use_count();
    }

    bool unique() const 
    { 
      return (use_count() == 1);
    }

    template <typename U>
    void reset(U *p)
    {
      if(get() != p)
      {
        counter->release();
        counter = new CounterImpl<U>(p);
      }
    }

    RefCountPtr<T> &operator=(const RefCountPtr<T> &x)
    {
      if(get() != x.get())
      {
        counter->release();

        counter = x.counter;
        counter->addref();
      }
      return *this;
    }

    T& operator*() const
    {
      return *get();
    }

    T* operator->() const
    {
      return get();
    }

    bool operator==(const RefCountPtr<T> &x) const 
    {
      return (get() == x.get());
    }

    bool operator!=(const RefCountPtr<T> &x) const
    {
      return !operator==(x);
    }

    operator bool() const
    {
      return (get() != 0);
    }

  private:
    CounterBase *counter;
  };

#endif // HAVE_*_SHARED_PTR
}
#endif // DO_NOT_DOCUMENT

#endif
