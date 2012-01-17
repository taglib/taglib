/***************************************************************************
    copyright           : (C) 2012 by Michael Helmling
    email               : helmling@mathematik.uni-kl.de
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include "tpropertymap.h"

using namespace TagLib;

typedef Map<String,StringList> supertype;

PropertyMap::PropertyMap() : Map<String,StringList>()
{
}

PropertyMap::PropertyMap(const PropertyMap &m) : Map<String,StringList>(m)
{
}

PropertyMap::~PropertyMap()
{
}

bool PropertyMap::insert(const String &key, const StringList &values)
{
  String realKey = prepareKey(key);
  if (realKey.isNull())
    return false;

  Iterator result = supertype::find(realKey);
  if (result == end())
    supertype::insert(realKey, values);
  else
    supertype::operator[](realKey).append(values);
  return true;
}

bool PropertyMap::replace(const String &key, const StringList &values)
{
  String realKey = prepareKey(key);
  if (realKey.isNull())
    return false;
  supertype::erase(realKey);
  supertype::insert(realKey, values);
  return true;
}

PropertyMap::Iterator PropertyMap::find(const String &key)
{
  String realKey = prepareKey(key);
  if (realKey.isNull())
    return end();
  return supertype::find(realKey);
}

PropertyMap::ConstIterator PropertyMap::find(const String &key) const
{
  String realKey = prepareKey(key);
  if (realKey.isNull())
    return end();
  return supertype::find(realKey);
}

bool PropertyMap::contains(const String &key) const
{
  String realKey = prepareKey(key);
  if (realKey.isNull())
    return false;
  return supertype::contains(realKey);
}

/*!
 * Erase the \a key and its values from the map.
 */
PropertyMap &PropertyMap::erase(const String &key)
{
  String realKey = prepareKey(key);
  if (realKey.isNull())
    return *this;
  supertype::erase(realKey);
  return *this;
}

const StringList &PropertyMap::operator[](const String &key) const
{
  String realKey = prepareKey(key);
  return supertype::operator[](realKey);
}

StringList &PropertyMap::operator[](const String &key)
{
  String realKey = prepareKey(key);
  return supertype::operator[](realKey);
}

StringList &PropertyMap::unsupportedData()
{
  return unsupported;
}

String PropertyMap::prepareKey(const String &proposed) const {
  if (proposed.isEmpty())
    return String::null;
  for (String::ConstIterator it = proposed.begin(); it != proposed.end(); it++)
    // forbid non-printable, non-ascii, '=' (#61) and '~' (#126)
    if (*it < 32 || *it >= 128 || *it == 61 || *it == 126)
      return String::null;
  return proposed.upper();
}
