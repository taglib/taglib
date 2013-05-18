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

#include "taglib_config.h"
#include <algorithm>

#if defined(TAGLIB_USE_STD_SHARED_PTR)
# include <memory>
#elif defined(TAGLIB_USE_TR1_SHARED_PTR) 
# include <tr1/memory>
#elif defined(TAGLIB_USE_BOOST_SHARED_PTR)
# include <boost/shared_ptr.hpp>
#endif

#if defined(TAGLIB_USE_STD_UNIQUE_PTR)
# include <memory>
#elif defined(TAGLIB_USE_BOOST_SCOPED_PTR)
# include <boost/scoped_ptr.hpp>
#endif

#ifndef DO_NOT_DOCUMENT // Tell Doxygen to skip this class.
/*!
 * \warning This <b>is not</b> part of the TagLib public API!
 */

namespace TagLib 
{
#if defined(TAGLIB_USE_STD_SHARED_PTR) \
  || defined(TAGLIB_USE_TR1_SHARED_PTR) \
  || defined(TAGLIB_USE_BOOST_SHARED_PTR)

  // RefCountPtr<T> is just a thin wrapper of shared_ptr<T>.
  // It will be optimized out by compilers and performs equivalent to them.

  template <typename T>
  class RefCountPtr
  {
  public:
    RefCountPtr()
      : sp()
    {
    }

    template <typename U>
    explicit RefCountPtr(U *p)
      : sp(p)
    {
    }

    RefCountPtr(const RefCountPtr<T> &x)
      : sp(x.sp)
    {
    }

    template <typename U>
    RefCountPtr(const RefCountPtr<U> &x)
      : sp(x.sp)
    {
    }

# ifdef TAGLIB_USE_MOVE_SEMANTICS

    RefCountPtr(RefCountPtr<T> &&x)
      : sp(std::move(x.sp))
    {
    }

    template <typename U>
    RefCountPtr(RefCountPtr<U> &&x)
      : sp(std::move(x.sp))
    {
    }

# endif

    T *get() const
    {
      return sp.get();
    }

    long use_count() const
    {
      return sp.use_count();
    }

    bool unique() const 
    { 
      return sp.unique();
    }

    template <typename U>
    void reset(U *p)
    {
      sp.reset(p);
    }

    void reset()
    {
      sp.reset();
    }

    void swap(RefCountPtr<T> &x)
    {
      sp.swap(x.sp);
    }

    RefCountPtr<T> &operator=(const RefCountPtr<T> &x)
    {
      sp = x.sp;
      return *this;
    }

    template <typename U>
    RefCountPtr<T> &operator=(const RefCountPtr<U> &x)
    {
      sp = x.sp;
      return *this;
    }

# ifdef TAGLIB_USE_MOVE_SEMANTICS

    RefCountPtr<T> &operator=(RefCountPtr<T> &&x)
    {
      sp = std::move(x.sp);
      return *this;
    }

    template <typename U>
    RefCountPtr<T> &operator=(RefCountPtr<U> &&x)
    {
      sp = std::move(x.sp);
      return *this;
    }

# endif

    T& operator*() const
    {
      return sp.operator*();
    }

    T* operator->() const
    {
      return sp.operator->();
    }

    operator bool() const
    {
      return static_cast<bool>(sp);
    }

    bool operator!() const
    {
      return !static_cast<bool>(sp);
    }

  private:
    template <typename U> friend class RefCountPtr;

# if defined(TAGLIB_USE_STD_SHARED_PTR) 

    std::shared_ptr<T> sp;

# elif defined(TAGLIB_USE_TR1_SHARED_PTR) 

    std::tr1::shared_ptr<T> sp;

# else

    boost::shared_ptr<T> sp;

# endif
  };

#else   // TAGLIB_USE_STD_SHARED_PTR etc.

  // Self-implements RefCountPtr<T> if shared_ptr<T> is not available.
  // I STRONGLY RECOMMEND using standard shared_ptr<T> rather than this class.
  
  // Counter base class. Provides a reference counter.

  class CounterBase
  {
  public:
    CounterBase();
    virtual ~CounterBase();

    void addref();
    void release();
    long use_count() const;

    virtual void dispose() = 0;

  private:
    class CounterBasePrivate;
    CounterBasePrivate *d;
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

#endif  // TAGLIB_USE_STD_SHARED_PTR etc.

#if defined(TAGLIB_USE_STD_UNIQUE_PTR) || defined(TAGLIB_USE_BOOST_SCOPED_PTR)

  // NonRefCountPtr<T> is just a thin wrapper of unique_ptr<T> or scoped_ptr<T>.
  // It will be optimized out by compilers and performs equivalent to them.

 template<typename T> 
  class NonRefCountPtr
  {
  public:
    explicit NonRefCountPtr(T *p = 0)
      : up(p) 
    {
    }

    ~NonRefCountPtr()
    {
    }

    void reset(T *p = 0)
    {
      NonRefCountPtr<T>(p).swap(*this);
    }

    T &operator*() const
    {
      return up.operator*();
    }

    T *operator->() const 
    {
      return up.operator->();
    }

    T *get() const
    {
      return up.get();
    }

    operator bool() const
    {
      return static_cast<bool>(up);
    }

    bool operator!() const
    {
      return !static_cast<bool>(up);
    }

    void swap(NonRefCountPtr &x) 
    {
      up.swap(x.up);
    }

  private:

    // Noncopyable
    NonRefCountPtr(const NonRefCountPtr &);
    NonRefCountPtr &operator=(const NonRefCountPtr &);

    void operator==(const NonRefCountPtr &) const;
    void operator!=(const NonRefCountPtr &) const;

# if defined(TAGLIB_USE_STD_UNIQUE_PTR) 

    std::unique_ptr<T> up;

# else

    boost::scoped_ptr<T> up;

# endif
  };

#else // TAGLIB_USE_STD_UNIQUE_PTR

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

#endif  // TAGLIB_USE_STD_UNIQUE_PTR

  // Comparison operators for smart pointers.

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

  template <typename T, typename U>
  bool operator==(const RefCountPtr<T> &a, U *b)
  {
    return (a.get() == b);
  }

  template <typename T, typename U>
  bool operator!=(const RefCountPtr<T> &a, U *b)
  {
    return (a.get() != b);
  }

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

  template <typename T, typename U>
  bool operator==(T *a, const RefCountPtr<U> &b)
  {
    return (a == b.get());
  }

  template <typename T, typename U>
  bool operator!=(T *a, const RefCountPtr<U> &b)
  {
    return (a != b.get());
  }

  template <typename T, typename U>
  bool operator==(T *a, const NonRefCountPtr<U> &b)
  {
    return (a == b.get());
  }

  template <typename T, typename U>
  bool operator!=(T *a, const NonRefCountPtr<U> &b)
  {
    return (a != b.get());
  }

  template <typename T>
  void swap(RefCountPtr<T> &a, RefCountPtr<T> &b)
  {
    a.swap(b);
  }

  template <typename T>
  void swap(NonRefCountPtr<T> &a, NonRefCountPtr<T> &b)
  {
    a.swap(b);
  }
}

#endif // DO_NOT_DOCUMENT
#endif
