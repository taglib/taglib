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


PropertyMap::PropertyMap() : SimplePropertyMap()
{
}

PropertyMap::PropertyMap(const PropertyMap &m) : SimplePropertyMap(m), unsupported(m.unsupported)
{
}

PropertyMap::PropertyMap(const SimplePropertyMap &m)
{
  for(SimplePropertyMap::ConstIterator it = m.begin(); it != m.end(); ++it){
    String key = prepareKey(it->first);
    if(!key.isNull())
      insert(it->first, it->second);
    else
      unsupported.append(it->first);
  }
}

PropertyMap::~PropertyMap()
{
}

bool PropertyMap::insert(const String &key, const StringList &values)
{
  String realKey = prepareKey(key);
  if(realKey.isNull())
    return false;

  Iterator result = SimplePropertyMap::find(realKey);
  if(result == end())
    SimplePropertyMap::insert(realKey, values);
  else
    SimplePropertyMap::operator[](realKey).append(values);
  return true;
}

bool PropertyMap::replace(const String &key, const StringList &values)
{
  String realKey = prepareKey(key);
  if(realKey.isNull())
    return false;
  SimplePropertyMap::erase(realKey);
  SimplePropertyMap::insert(realKey, values);
  return true;
}

PropertyMap::Iterator PropertyMap::find(const String &key)
{
  String realKey = prepareKey(key);
  if(realKey.isNull())
    return end();
  return SimplePropertyMap::find(realKey);
}

PropertyMap::ConstIterator PropertyMap::find(const String &key) const
{
  String realKey = prepareKey(key);
  if(realKey.isNull())
    return end();
  return SimplePropertyMap::find(realKey);
}

bool PropertyMap::contains(const String &key) const
{
  String realKey = prepareKey(key);
  // we consider keys with empty value list as not present
  if(realKey.isNull() || SimplePropertyMap::operator[](realKey).isEmpty())
    return false;
  return SimplePropertyMap::contains(realKey);
}

PropertyMap &PropertyMap::erase(const String &key)
{
  String realKey = prepareKey(key);
  if(realKey.isNull())
    return *this;
  SimplePropertyMap::erase(realKey);
  return *this;
}

PropertyMap &PropertyMap::merge(const PropertyMap &other)
{
  for(PropertyMap::ConstIterator it = other.begin(); it != other.end(); ++it) {
    insert(it->first, it->second);
  }
  unsupported.append(other.unsupported);
  return *this;
}

const StringList &PropertyMap::operator[](const String &key) const
{
  String realKey = prepareKey(key);
  return SimplePropertyMap::operator[](realKey);
}

StringList &PropertyMap::operator[](const String &key)
{
  String realKey = prepareKey(key);
  return SimplePropertyMap::operator[](realKey);
}

void PropertyMap::removeEmpty()
{
  StringList emptyKeys;
  for(Iterator it = begin(); it != end(); ++it)
    if(it->second.isEmpty())
      emptyKeys.append(it->first);
  for(StringList::Iterator emptyIt = emptyKeys.begin(); emptyIt != emptyKeys.end(); emptyIt++ )
    erase(*emptyIt);
}

StringList &PropertyMap::unsupportedData()
{
  return unsupported;
}

const StringList &PropertyMap::unsupportedData() const
{
  return unsupported;
}

String PropertyMap::prepareKey(const String &proposed) {
  if(proposed.isEmpty())
    return String::null;
  for (String::ConstIterator it = proposed.begin(); it != proposed.end(); it++)
    // forbid non-printable, non-ascii, '=' (#61) and '~' (#126)
    if (*it < 32 || *it >= 128 || *it == 61 || *it == 126)
      return String::null;
  return proposed.upper();
}
