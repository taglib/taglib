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

#include "tsmartptr.h"

namespace TagLib {

template <class Key, class T>
class Map<Key, T>::MapPrivate
{
public:
  MapPrivate() : 
    map(new MapType()) {}

  SHARED_PTR<MapType> map;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
Map<Key, T>::Map() : 
  d(new MapPrivate())
{
}

template <class Key, class T>
Map<Key, T>::Map(const Map<Key, T> &m) : 
  d(new MapPrivate(*m.d))
{
}

template <class Key, class T>
Map<Key, T>::~Map()
{
  delete d;
}

template <class Key, class T>
typename Map<Key, T>::Iterator Map<Key, T>::begin()
{
  detach();
  return d->map->begin();
}

template <class Key, class T>
typename Map<Key, T>::ConstIterator Map<Key, T>::begin() const
{
  return d->map->begin();
}

template <class Key, class T>
typename Map<Key, T>::Iterator Map<Key, T>::end()
{
  detach();
  return d->map->end();
}

template <class Key, class T>
typename Map<Key, T>::ConstIterator Map<Key, T>::end() const
{
  return d->map->end();
}

template <class Key, class T>
Map<Key, T> &Map<Key, T>::insert(const Key &key, const T &value)
{
  detach();
  (*d->map)[key] = value;
  return *this;
}

template <class Key, class T>
Map<Key, T> &Map<Key, T>::clear()
{
  detach();
  d->map->clear();
  return *this;
}

template <class Key, class T>
bool Map<Key, T>::isEmpty() const
{
  return d->map->empty();
}

template <class Key, class T>
typename Map<Key, T>::Iterator Map<Key, T>::find(const Key &key)
{
  detach();
  return d->map->find(key);
}

template <class Key, class T>
typename Map<Key,T>::ConstIterator Map<Key, T>::find(const Key &key) const
{
  return d->map->find(key);
}

template <class Key, class T>
bool Map<Key, T>::contains(const Key &key) const
{
  return d->map->find(key) != d->map->end();
}

template <class Key, class T>
Map<Key, T> &Map<Key,T>::erase(Iterator it)
{
  detach();
  d->map->erase(it);
  return *this;
}

template <class Key, class T>
Map<Key, T> &Map<Key,T>::erase(const Key &key)
{
  detach();
  d->map->erase(key);
  return *this;
}

template <class Key, class T>
size_t Map<Key, T>::size() const
{
  return d->map->size();
}

template <class Key, class T>
const T &Map<Key, T>::operator[](const Key &key) const
{
  return (*d->map)[key];
}

template <class Key, class T>
T &Map<Key, T>::operator[](const Key &key)
{
  detach();
  return (*d->map)[key];
}

template <class Key, class T>
Map<Key, T> &Map<Key, T>::operator=(const Map<Key, T> &m)
{
  *d = *m.d;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

template <class Key, class T>
void Map<Key, T>::detach()
{
  if(!d->map.unique())
    d->map.reset(new MapType(*d->map));
}

} // namespace TagLib
