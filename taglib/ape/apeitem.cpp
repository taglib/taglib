/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.com
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

#include "apeitem.h"

#include <utility>
#include <numeric>

#include "tdebug.h"

using namespace TagLib;
using namespace APE;

class APE::Item::ItemPrivate
{
public:
  Item::ItemTypes type { Text };
  String key;
  ByteVector value;
  StringList text;
  bool readOnly { false };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

APE::Item::Item() :
  d(std::make_unique<ItemPrivate>())
{
}

APE::Item::Item(const String &key, const StringList &values) :
  d(std::make_unique<ItemPrivate>())
{
  d->key = key;
  d->text = values;
}

APE::Item::Item(const String &key, const ByteVector &value, bool binary) :
  d(std::make_unique<ItemPrivate>())
{
  d->key = key;
  if(binary) {
    d->type = Binary;
    d->value = value;
  }
  else {
    d->text.append(value);
  }
}

APE::Item::Item(const Item &item) :
  d(std::make_unique<ItemPrivate>(*item.d))
{
}

APE::Item::~Item() = default;

Item &APE::Item::operator=(const Item &item)
{
  Item(item).swap(*this);
  return *this;
}

void APE::Item::swap(Item &item) noexcept
{
  using std::swap;

  swap(d, item.d);
}

void APE::Item::setReadOnly(bool readOnly)
{
  d->readOnly = readOnly;
}

bool APE::Item::isReadOnly() const
{
  return d->readOnly;
}

void APE::Item::setType(APE::Item::ItemTypes val)
{
  d->type = val;
}

APE::Item::ItemTypes APE::Item::type() const
{
  return d->type;
}

String APE::Item::key() const
{
  return d->key;
}

ByteVector APE::Item::binaryData() const
{
  return d->value;
}

void APE::Item::setBinaryData(const ByteVector &value)
{
  d->type = Binary;
  d->value = value;
  d->text.clear();
}

void APE::Item::setKey(const String &key)
{
  d->key = key;
}

void APE::Item::setValue(const String &value)
{
  d->type = Text;
  d->text = value;
  d->value.clear();
}

void APE::Item::setValues(const StringList &values)
{
  d->type = Text;
  d->text = values;
  d->value.clear();
}

void APE::Item::appendValue(const String &value)
{
  d->type = Text;
  d->text.append(value);
  d->value.clear();
}

void APE::Item::appendValues(const StringList &values)
{
  d->type = Text;
  d->text.append(values);
  d->value.clear();
}

int APE::Item::size() const
{
  int result = 8 + d->key.size() + 1;
  switch(d->type) {
    case Text:
      if(!d->text.isEmpty()) {
        result = std::accumulate(d->text.cbegin(), d->text.cend(), result,
            [](int sz, const String &t) {
          return sz + 1 + t.data(String::UTF8).size();
        }) - 1;
      }
      break;

    case Binary:
    case Locator:
      result += d->value.size();
      break;
  }
  return result;
}

StringList APE::Item::values() const
{
  return d->text;
}

String APE::Item::toString() const
{
  if(d->type == Text && !isEmpty())
    return d->text.front();
  return String();
}

bool APE::Item::isEmpty() const
{
  switch(d->type) {
    case Text:
      if(d->text.isEmpty())
        return true;
      return d->text.size() == 1 && d->text.front().isEmpty();
    case Binary:
    case Locator:
      return d->value.isEmpty();
    default:
      return false;
  }
}

void APE::Item::parse(const ByteVector &data)
{
  // 11 bytes is the minimum size for an APE item

  if(data.size() < 11) {
    debug("APE::Item::parse() -- no data in item");
    return;
  }

  const unsigned int valueLength  = data.toUInt(0, false);
  const unsigned int flags        = data.toUInt(4, false);

  // An item key can contain ASCII characters from 0x20 up to 0x7E, not UTF-8.
  // We assume that the validity of the given key has been checked.

  d->key = String(&data[8], String::Latin1);

  const ByteVector val = data.mid(8 + d->key.size() + 1, valueLength);

  setReadOnly(flags & 1);
  setType(static_cast<ItemTypes>((flags >> 1) & 3));

  if(Text == d->type)
    d->text = StringList(ByteVectorList::split(val, '\0'), String::UTF8);
  else
    d->value = val;
}

ByteVector APE::Item::render() const
{
  ByteVector data;
  unsigned int flags = (d->readOnly ? 1 : 0) | (d->type << 1);
  ByteVector val;

  if(isEmpty())
    return data;

  if(d->type == Text) {
    auto it = d->text.cbegin();

    val.append(it->data(String::UTF8));
    for(it = std::next(it); it != d->text.cend(); ++it) {
      val.append('\0');
      val.append(it->data(String::UTF8));
    }
    d->value = val;
  }
  else
    val.append(d->value);

  data.append(ByteVector::fromUInt(val.size(), false));
  data.append(ByteVector::fromUInt(flags, false));
  data.append(d->key.data(String::Latin1));
  data.append(ByteVector('\0'));
  data.append(val);

  return data;
}
