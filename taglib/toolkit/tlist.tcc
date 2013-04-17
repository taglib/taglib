/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#include <algorithm>

namespace TagLib {

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

// The functionality of List<T>::setAutoDelete() is implemented here partial
// template specialization.  This is implemented in such a way that calling
// setAutoDelete() on non-pointer types will simply have no effect.

// A base for the generic and specialized private class types.  New
// non-templatized members should be added here.

class ListPrivateBase
{
public:
  ListPrivateBase() : autoDelete(false) {}
  bool autoDelete;
};

// A generic implementation

template <class T>
template <class TP> class List<T>::ListPrivate  : public ListPrivateBase
{
public:
  ListPrivate() : ListPrivateBase() {}
  ListPrivate(const std::list<TP> &l) : ListPrivateBase(), list(l) {}

#ifdef SUPPORT_MOVE_SEMANTICS

  ListPrivate(std::list<TP> &&l) : ListPrivateBase(), list(l) {}

#endif

  void clear() {
    std::list<TP>().swap(list);
  }
  std::list<TP> list;
};

// A partial specialization for all pointer types that implements the
// setAutoDelete() functionality.

template <class T>
template <class TP> class List<T>::ListPrivate<TP *>  : public ListPrivateBase
{
public:
  ListPrivate() : ListPrivateBase() {}
  ListPrivate(const std::list<TP *> &l) : ListPrivateBase(), list(l) {}

#ifdef SUPPORT_MOVE_SEMANTICS

  ListPrivate(std::list<TP *> &&l) : ListPrivateBase(), list(l) {}

#endif

  ~ListPrivate() {
    deletePointers();
  }

  void clear() {
    deletePointers();
    std::list<TP *>().swap(list);
  }

  std::list<TP *> list;

private:
  void deletePointers() {
    if(!autoDelete)
      return;
    
    typename std::list<TP *>::const_iterator it = list.begin();
    for(; it != list.end(); ++it)
    delete *it;
  }
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

template <class T>
List<T>::List()
    : d(new ListPrivate<T>())
{
}

template <class T>
List<T>::List(const List<T> &l) 
: d(l.d)
{
}

#ifdef SUPPORT_MOVE_SEMANTICS

template <class T>
List<T>::List(List<T> &&l) 
  : d(std::move(l.d))
{
}

#endif 

template <class T>
List<T>::~List()
{
}

template <class T>
typename List<T>::Iterator List<T>::begin()
{
  detach();
  return d->list.begin();
}

template <class T>
typename List<T>::ConstIterator List<T>::begin() const
{
  return d->list.begin();
}

template <class T>
typename List<T>::Iterator List<T>::end()
{
  detach();
  return d->list.end();
}

template <class T>
typename List<T>::ConstIterator List<T>::end() const
{
  return d->list.end();
}

template <class T>
typename List<T>::Iterator List<T>::insert(Iterator it, const T &item)
{
  detach();
  return d->list.insert(it, item);
}

template <class T>
List<T> &List<T>::sortedInsert(const T &value, bool unique)
{
  detach();
  Iterator it = begin();
  while(it != end() && *it < value)
    ++it;
  if(unique && it != end() && *it == value)
    return *this;
  insert(it, value);
  return *this;
}

template <class T>
List<T> &List<T>::append(const T &item)
{
  detach();
  d->list.push_back(item);
  return *this;
}

template <class T>
List<T> &List<T>::append(const List<T> &l)
{
  detach();
  d->list.insert(d->list.end(), l.begin(), l.end());
  return *this;
}

#ifdef SUPPORT_MOVE_SEMANTICS

template <class T>
List<T> &List<T>::append(T &&item)
{
  detach();
  d->list.push_back(item);
  return *this;
}

template <class T>
List<T> &List<T>::append(List<T> &&l)
{
  detach();
  
  for(Iterator it = l.begin(); it != l.end(); ++it)
    d->list.push_back(std::move(*it));
  
  return *this;
}

#endif

template <class T>
List<T> &List<T>::prepend(const T &item)
{
  detach();
  d->list.push_front(item);
  return *this;
}

template <class T>
List<T> &List<T>::prepend(const List<T> &l)
{
  detach();
  d->list.insert(d->list.begin(), l.d->list.begin(), l.d->list.end());
  return *this;
}

#ifdef SUPPORT_MOVE_SEMANTICS

template <class T>
List<T> &List<T>::prepend(T &&item)
{
  detach();
  d->list.push_front(item);
  return *this;
}

template <class T>
List<T> &List<T>::prepend(List<T> &&l)
{
  detach();
  
  for(typename std::list<T>::reverse_iterator it = l.d->list.rbegin(); it != l.d->list.rend(); ++it)
    d->list.push_front(std::move(*it));
  
  return *this;
}

#endif

template <class T>
List<T> &List<T>::clear()
{
  detach();
  d->clear();
  return *this;
}

template <class T>
size_t List<T>::size() const
{
  return d->list.size();
}

template <class T>
bool List<T>::isEmpty() const
{
  return d->list.empty();
}

template <class T>
typename List<T>::Iterator List<T>::find(const T &value)
{
  return std::find(d->list.begin(), d->list.end(), value);
}

template <class T>
typename List<T>::ConstIterator List<T>::find(const T &value) const
{
  return std::find(d->list.begin(), d->list.end(), value);
}

template <class T>
bool List<T>::contains(const T &value) const
{
  return std::find(d->list.begin(), d->list.end(), value) != d->list.end();
}

template <class T>
typename List<T>::Iterator List<T>::erase(Iterator it)
{
  return d->list.erase(it);
}

template <class T>
const T &List<T>::front() const
{
  return d->list.front();
}

template <class T>
T &List<T>::front()
{
  detach();
  return d->list.front();
}

template <class T>
const T &List<T>::back() const
{
  return d->list.back();
}

template <class T>
void List<T>::setAutoDelete(bool autoDelete)
{
  d->autoDelete = autoDelete;
}

template <class T>
T &List<T>::back()
{
  detach();
  return d->list.back();
}

template <class T>
T &List<T>::operator[](size_t i)
{
  Iterator it = d->list.begin();

  for(uint j = 0; j < i; j++)
    ++it;

  return *it;
}

template <class T>
const T &List<T>::operator[](size_t i) const
{
  ConstIterator it = d->list.begin();

  for(uint j = 0; j < i; j++)
    ++it;

  return *it;
}

template <class T>
List<T> &List<T>::operator=(const List<T> &l)
{
  d = l.d;
  return *this;
}

#ifdef SUPPORT_MOVE_SEMANTICS

template <class T>
List<T> &List<T>::operator=(List<T> &&l)
{
  d = std::move(l.d);
  return *this;
}

#endif

template <class T>
bool List<T>::operator==(const List<T> &l) const
{
  return d->list == l.d->list;
}

template <class T>
bool List<T>::operator!=(const List<T> &l) const
{
  return d->list != l.d->list;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

template <class T>
void List<T>::detach()
{
  if(!d.unique())
    d.reset(new ListPrivate<T>(d->list));
}

} // namespace TagLib
