/**************************************************************************
    copyright            : (C) 2005-2007 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

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

#include <taglib.h>
#include <tdebug.h>
#include <tsmartptr.h>

#include "asfattribute.h"
#include "asffile.h"
#include "asfutils.h"

using namespace TagLib;

namespace
{
  struct AttributeData
  {
    ASF::Attribute::AttributeTypes type;
    String stringValue;
    ByteVector byteVectorValue;
    ASF::Picture pictureValue;
    union {
      unsigned int intValue;
      unsigned short shortValue;
      unsigned long long longLongValue;
      bool boolValue;
    };
    int stream;
    int language;
  };
}

class ASF::Attribute::AttributePrivate
{
public:
  AttributePrivate() :
    data(new AttributeData())
  {
    data->pictureValue = ASF::Picture::fromInvalid();
    data->stream       = 0;
    data->language     = 0;
  }

  SHARED_PTR<AttributeData> data;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ASF::Attribute::Attribute() :
  d(new AttributePrivate())
{
  d->data->type = UnicodeType;
}

ASF::Attribute::Attribute(const ASF::Attribute &other) :
  d(new AttributePrivate(*other.d))
{
}

ASF::Attribute::Attribute(const String &value) :
  d(new AttributePrivate())
{
  d->data->type = UnicodeType;
  d->data->stringValue = value;
}

ASF::Attribute::Attribute(const ByteVector &value) :
  d(new AttributePrivate())
{
  d->data->type = BytesType;
  d->data->byteVectorValue = value;
}

ASF::Attribute::Attribute(const ASF::Picture &value) :
  d(new AttributePrivate())
{
  d->data->type = BytesType;
  d->data->pictureValue = value;
}

ASF::Attribute::Attribute(unsigned int value) :
  d(new AttributePrivate())
{
  d->data->type = DWordType;
  d->data->intValue = value;
}

ASF::Attribute::Attribute(unsigned long long value) :
  d(new AttributePrivate())
{
  d->data->type = QWordType;
  d->data->longLongValue = value;
}

ASF::Attribute::Attribute(unsigned short value) :
  d(new AttributePrivate())
{
  d->data->type = WordType;
  d->data->shortValue = value;
}

ASF::Attribute::Attribute(bool value) :
  d(new AttributePrivate())
{
  d->data->type = BoolType;
  d->data->boolValue = value;
}

ASF::Attribute::~Attribute()
{
  delete d;
}

ASF::Attribute &ASF::Attribute::operator=(const ASF::Attribute &other)
{
  Attribute(other).swap(*this);
  return *this;
}

void ASF::Attribute::swap(Attribute &other)
{
  using std::swap;

  swap(d, other.d);
}

ASF::Attribute::AttributeTypes ASF::Attribute::type() const
{
  return d->data->type;
}

String ASF::Attribute::toString() const
{
  return d->data->stringValue;
}

ByteVector ASF::Attribute::toByteVector() const
{
  if(d->data->pictureValue.isValid())
    return d->data->pictureValue.render();

  return d->data->byteVectorValue;
}

unsigned short ASF::Attribute::toBool() const
{
  return d->data->shortValue;
}

unsigned short ASF::Attribute::toUShort() const
{
  return d->data->shortValue;
}

unsigned int ASF::Attribute::toUInt() const
{
  return d->data->intValue;
}

unsigned long long ASF::Attribute::toULongLong() const
{
  return d->data->longLongValue;
}

ASF::Picture ASF::Attribute::toPicture() const
{
  return d->data->pictureValue;
}

String ASF::Attribute::parse(ASF::File &f, int kind)
{
  unsigned int size, nameLength;
  String name;
  d->data->pictureValue = Picture::fromInvalid();
  // extended content descriptor
  if(kind == 0) {
    nameLength = readWORD(&f);
    name = readString(&f, nameLength);
    d->data->type = ASF::Attribute::AttributeTypes(readWORD(&f));
    size = readWORD(&f);
  }
  // metadata & metadata library
  else {
    int temp = readWORD(&f);
    // metadata library
    if(kind == 2) {
      d->data->language = temp;
    }
    d->data->stream = readWORD(&f);
    nameLength = readWORD(&f);
    d->data->type = ASF::Attribute::AttributeTypes(readWORD(&f));
    size = readDWORD(&f);
    name = readString(&f, nameLength);
  }

  if(kind != 2 && size > 65535) {
    debug("ASF::Attribute::parse() -- Value larger than 64kB");
  }

  switch(d->data->type) {
  case WordType:
    d->data->shortValue = readWORD(&f);
    break;

  case BoolType:
    if(kind == 0) {
      d->data->boolValue = (readDWORD(&f) == 1);
    }
    else {
      d->data->boolValue = (readWORD(&f) == 1);
    }
    break;

  case DWordType:
    d->data->intValue = readDWORD(&f);
    break;

  case QWordType:
    d->data->longLongValue = readQWORD(&f);
    break;

  case UnicodeType:
    d->data->stringValue = readString(&f, size);
    break;

  case BytesType:
  case GuidType:
    d->data->byteVectorValue = f.readBlock(size);
    break;
  }

  if(d->data->type == BytesType && name == "WM/Picture") {
    d->data->pictureValue.parse(d->data->byteVectorValue);
    if(d->data->pictureValue.isValid()) {
      d->data->byteVectorValue.clear();
    }
  }

  return name;
}

int ASF::Attribute::dataSize() const
{
  switch (d->data->type) {
  case WordType:
    return 2;
  case BoolType:
    return 4;
  case DWordType:
    return 4;
  case QWordType:
    return 5;
  case UnicodeType:
    return static_cast<int>(d->data->stringValue.size() * 2 + 2);
  case BytesType:
    if(d->data->pictureValue.isValid())
      return d->data->pictureValue.dataSize();
  case GuidType:
    return static_cast<int>(d->data->byteVectorValue.size());
  }
  return 0;
}

ByteVector ASF::Attribute::render(const String &name, int kind) const
{
  ByteVector data;

  switch (d->data->type) {
  case WordType:
    data.append(ByteVector::fromUInt16LE(d->data->shortValue));
    break;

  case BoolType:
    if(kind == 0) {
      data.append(ByteVector::fromUInt32LE(d->data->boolValue ? 1 : 0));
    }
    else {
      data.append(ByteVector::fromUInt16LE(d->data->boolValue ? 1 : 0));
    }
    break;

  case DWordType:
    data.append(ByteVector::fromUInt32LE(d->data->intValue));
    break;

  case QWordType:
    data.append(ByteVector::fromUInt64LE(d->data->longLongValue));
    break;

  case UnicodeType:
    data.append(renderString(d->data->stringValue));
    break;

  case BytesType:
    if(d->data->pictureValue.isValid()) {
      data.append(d->data->pictureValue.render());
      break;
    }
  case GuidType:
    data.append(d->data->byteVectorValue);
    break;
  }

  if(kind == 0) {
    data = renderString(name, true) +
           ByteVector::fromUInt16LE((int)d->data->type) +
           ByteVector::fromUInt16LE(data.size()) +
           data;
  }
  else {
    ByteVector nameData = renderString(name);
    data = ByteVector::fromUInt16LE(kind == 2 ? d->data->language : 0) +
           ByteVector::fromUInt16LE(d->data->stream) +
           ByteVector::fromUInt16LE(nameData.size()) +
           ByteVector::fromUInt16LE((int)d->data->type) +
           ByteVector::fromUInt32LE(data.size()) +
           nameData +
           data;
  }

  return data;
}

int ASF::Attribute::language() const
{
  return d->data->language;
}

void ASF::Attribute::setLanguage(int value)
{
  d->data->language = value;
}

int ASF::Attribute::stream() const
{
  return d->data->stream;
}

void ASF::Attribute::setStream(int value)
{
  d->data->stream = value;
}
