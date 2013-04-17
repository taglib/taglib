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

// TAGLIB_USE_CXX11 determines whether or not to enable C++11 features.

#ifdef TAGLIB_USE_CXX11
# include <memory>

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

#ifdef TAGLIB_USE_CXX11

  // RefCountPtr<T> is just an alias of std::shared_ptr<T> if C++11 is available.
# define RefCountPtr std::shared_ptr

  // Workaround for the fact that some compilers don't support the template aliases.

#else

namespace TagLib {

  // RefCountPtr<T> mimics std::shared_ptr<T> if C++11 is not available.

  template<typename T>
  class RefCountPtr
  {
  private:

    // Counter base class. Provides a reference counter.

    class counter_base
    {
    public:
      counter_base() 
        : count(1) 
      {
      }

      virtual ~counter_base()
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
    class counter_impl : public counter_base
    {
    public:
      counter_impl(U *p)
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
      : counter(new counter_impl<U>(p))
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
      return static_cast<counter_impl<T>*>(counter)->get();
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
        counter = new counter_impl<U>(p);
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
    counter_base *counter;
  };
}
#endif // TAGLIB_USE_CXX11

#endif // DO_NOT_DOCUMENT

#endif