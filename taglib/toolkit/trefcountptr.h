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

#include <algorithm>

#ifndef DO_NOT_DOCUMENT // Tell Doxygen to skip this class.
/*!
 * \internal
 * This is just used as a smart pointer for shared classes in TagLib.
 *
 * \warning This <b>is not</b> part of the TagLib public API!
 */

namespace TagLib {

#if defined(TAGLIB_USE_STD_SHARED_PTR) || defined(TAGLIB_USE_TR1_SHARED_PTR) 
  
#define TAGLIB_SHARED_PTR std::tr1::shared_ptr

#elif defined(TAGLIB_USE_BOOST_SHARED_PTR)

#define TAGLIB_SHARED_PTR boost::shared_ptr

#else // TAGLIB_USE_*_SHARED_PTR

  // Self-implements RefCountPtr<T> if shared_ptr<T> is not available.
  // I STRONGLY RECOMMEND using standard shared_ptr<T> rather than this class.
  
  class RefCounter
  {
  public:
    RefCounter();
    ~RefCounter();

    size_t ref();
    size_t deref();
    size_t count() const;

  private:
    class RefCounterPrivate;
    RefCounterPrivate *d;
  };
  
  template<typename T>
  class RefCountPtr
  {
  private:

    // Counter base class. Provides a reference counter.

    class CounterBase
    {
    public:
      virtual ~CounterBase()
      {
      }

      void addref() 
      { 
        count.ref();
      }

      void release()
      {
        if(count.deref() == 0) {
          dispose();
          delete this;
        }
      }

      long use_count() const
      {
        return static_cast<long>(count.count());
      }

      virtual void dispose() = 0;

    private:
      RefCounter count;
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
    explicit RefCountPtr()
      : counter(0)
    {
    }

    template <typename U>
    explicit RefCountPtr(U *p)
      : counter(new CounterImpl<U>(p))
    {
    }

    RefCountPtr(const RefCountPtr<T> &x)
      : counter(x.counter)
    {
      if(counter)
        counter->addref();
    }

    template <typename U>
    RefCountPtr(const RefCountPtr<U> &x)
      : counter(reinterpret_cast<CounterBase*>(x.counter))
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
      if(counter)
        return static_cast<CounterImpl<T>*>(counter)->get();
      else
        return 0;
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
      if(get() != p)
        RefCountPtr<T>(p).swap(*this);
    }

    void reset()
    {
      RefCountPtr<T>().swap(*this);
    }

    void swap(RefCountPtr<T> &x)
    {
      std::swap(counter, x.counter);
    }

    RefCountPtr<T> &operator=(const RefCountPtr<T> &x)
    {
      if(get() != x.get()) {
        if(counter)
          counter->release();

        counter = x.counter;

        if(counter)
          counter->addref();
      }
      return *this;
    }

    template <typename U>
    RefCountPtr<T> &operator=(const RefCountPtr<U> &x)
    {
      if(get() != x.get()) {
        if(counter)
          counter->release();

        counter = reinterpret_cast<CounterBase*>(x.counter);

        if(counter)
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
    mutable CounterBase *counter;

    template <typename U> friend class RefCountPtr;
  };

  template <typename T>
  void swap(RefCountPtr<T> &a, RefCountPtr<T> &b)
  {
    a.swap(b);
  }

#endif // TAGLIB_USE_*_SHARED_PTR
}
#endif // DO_NOT_DOCUMENT

#endif
