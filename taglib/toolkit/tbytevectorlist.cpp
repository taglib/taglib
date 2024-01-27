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

#include "tbytevectorlist.h"

using namespace TagLib;

class ByteVectorList::ByteVectorListPrivate
{
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

ByteVectorList ByteVectorList::split(const ByteVector &v, const ByteVector &pattern,
                                     int byteAlign, int max)
{
  ByteVectorList l;

  unsigned int previousOffset = 0;
  for(int offset = v.find(pattern, 0, byteAlign);
      offset != -1 && (max == 0 || max > static_cast<int>(l.size()) + 1);
      offset = v.find(pattern, offset + pattern.size(), byteAlign))
  {
    if(offset - previousOffset >= 1)
      l.append(v.mid(previousOffset, offset - previousOffset));
    else
      l.append(ByteVector());

    previousOffset = offset + pattern.size();
  }

  if(previousOffset < v.size())
    l.append(v.mid(previousOffset, v.size() - previousOffset));

  return l;
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

ByteVectorList::ByteVectorList() = default;

ByteVectorList::~ByteVectorList() = default;

ByteVectorList::ByteVectorList(const ByteVectorList &l) :
  List<ByteVector>(l)
{
  // Uncomment if d is used, d.get() is nullptr and *d behavior undefined
  // *d = *l.d;
}

ByteVectorList::ByteVectorList(std::initializer_list<ByteVector> init) :
  List<ByteVector>(init)
{
}

ByteVectorList &ByteVectorList::operator=(const ByteVectorList &l)
{
  if(this == &l)
    return *this;

  List<ByteVector>::operator=(l);
  // Uncomment if d is used, d.get() is nullptr and *d behavior undefined
  // *d = *l.d;
  return *this;
}

ByteVectorList &ByteVectorList::operator=(std::initializer_list<ByteVector> init)
{
  List<ByteVector>::operator=(init);
  return *this;
}

ByteVector ByteVectorList::toByteVector(const ByteVector &separator) const
{
  ByteVector v;

  for(auto it = begin(); it != end(); ++it) {
    v.append(*it);
    if(std::next(it) != end())
      v.append(separator);
  }

  return v;
}

////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &s, const ByteVectorList &l)
{
  for(auto it = l.begin(); it != l.end(); ++it) {
    if(it != l.begin()) {
      s << ' ';
    }
    s << *it;
  }
  return s;
}
