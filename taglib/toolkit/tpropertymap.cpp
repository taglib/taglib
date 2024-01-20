/***************************************************************************
    copyright           : (C) 2012 by Michael Helmling
    email               : helmling@mathematik.uni-kl.de
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

#include "tpropertymap.h"

#include <utility>

using namespace TagLib;

class PropertyMap::PropertyMapPrivate
{
public:
    StringList unsupported;
};

PropertyMap::PropertyMap() :
  d(std::make_unique<PropertyMapPrivate>())
{

}

PropertyMap::PropertyMap(const PropertyMap &m) :
  SimplePropertyMap(m),
  d(std::make_unique<PropertyMapPrivate>())
{
  *d = *m.d;
}

PropertyMap::PropertyMap(const SimplePropertyMap &m) :
  d(std::make_unique<PropertyMapPrivate>())
{
  for(const auto &[key, val] : m) {
    if(!key.isEmpty())
      insert(key.upper(), val);
    else
      d->unsupported.append(key.upper());
  }
}

PropertyMap::~PropertyMap() = default;

bool PropertyMap::insert(const String &key, const StringList &values)
{
  String realKey = key.upper();
  auto result = SimplePropertyMap::find(realKey);
  if(result == end())
    SimplePropertyMap::insert(realKey, values);
  else
    SimplePropertyMap::operator[](realKey).append(values);
  return true;
}

bool PropertyMap::replace(const String &key, const StringList &values)
{
  String realKey = key.upper();
  SimplePropertyMap::erase(realKey);
  SimplePropertyMap::insert(realKey, values);
  return true;
}

PropertyMap::Iterator PropertyMap::find(const String &key)
{
  return SimplePropertyMap::find(key.upper());
}

PropertyMap::ConstIterator PropertyMap::find(const String &key) const
{
  return SimplePropertyMap::find(key.upper());
}

bool PropertyMap::contains(const String &key) const
{
  return SimplePropertyMap::contains(key.upper());
}

bool PropertyMap::contains(const PropertyMap &other) const
{
  return std::all_of(other.begin(), other.end(),
    [this](const auto &o) { return contains(o.first) && (*this)[o.first] == o.second; });
}

PropertyMap &PropertyMap::erase(const String &key)
{
  SimplePropertyMap::erase(key.upper());
  return *this;
}

PropertyMap &PropertyMap::erase(const PropertyMap &other)
{
  for(const auto &[property, _] : other)
    erase(property);
  return *this;
}

PropertyMap &PropertyMap::merge(const PropertyMap &other)
{
  for(const auto &[property, val] : other)
    insert(property, val);
  d->unsupported.append(other.d->unsupported);
  return *this;
}

StringList PropertyMap::value(const String &key,
                                    const StringList &defaultValue) const
{
  return SimplePropertyMap::value(key.upper(), defaultValue);
}

const StringList &PropertyMap::operator[](const String &key) const
{
  return SimplePropertyMap::operator[](key.upper());
}

StringList &PropertyMap::operator[](const String &key)
{
  return SimplePropertyMap::operator[](key.upper());
}

bool PropertyMap::operator==(const PropertyMap &other) const
{
  for(const auto &[property, val] : other) {
    if(auto thisFind = find(property);
       thisFind == end() || thisFind->second != val)
      return false;
  }
  for(const auto &[property, val] : *this) {
    if(auto otherFind = other.find(property);
       otherFind == other.end() || otherFind->second != val)
      return false;
  }
  return d->unsupported == other.d->unsupported;
}

bool PropertyMap::operator!=(const PropertyMap &other) const
{
  return !(*this == other);
}

String PropertyMap::toString() const
{
  String ret;

  for(const auto &[property, val] : *this)
    ret += property + "=" + val.toString(", ") + "\n";
  if(!d->unsupported.isEmpty())
    ret += "Unsupported Data: " + d->unsupported.toString(", ") + "\n";
  return ret;
}

void PropertyMap::removeEmpty()
{
  PropertyMap m;
  for(const auto &[property, val] : std::as_const(*this)) {
    if(!val.isEmpty())
      m.insert(property, val);
  }
  *this = m;
}

const StringList &PropertyMap::unsupportedData() const
{
  return d->unsupported;
}

void PropertyMap::addUnsupportedData(const String &key)
{
  d->unsupported.append(key);
}

PropertyMap &PropertyMap::operator=(const PropertyMap &other)
{
  if(this == &other)
    return *this;

  SimplePropertyMap::operator=(other);
  *d = *other.d;
  return *this;
}

#ifdef _MSC_VER
// When building with shared libraries and tests, MSVC will fail with
// "already defined in test_opus.obj" as soon as operator[] of
// Ogg::FieldListMap is used because this will instantiate the same template
// Map<String, StringList>. Therefore this template is instantiated here
// and declared extern in the headers using it.
// Suppress warning "needs to have dll-interface to be used by clients",
// in the template, TAGLIB_EXPORT cannot be used on the members, therefore
// the private implemenation is exported too, but not accessible by clients.
#pragma warning(suppress: 4251)
template class TAGLIB_EXPORT TagLib::Map<TagLib::String, TagLib::StringList>;
#endif
