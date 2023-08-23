/***************************************************************************
    copyright           : (C) 2009 by Lukas Lalinsky
    email               : lukas@oxygene.sk
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

#include "mp4coverart.h"
#include "tag.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(MP4CoverArt, testSimple)
{
  MP4::CoverArt c(MP4::CoverArt::PNG, "foo");
  ASSERT_EQ(MP4::CoverArt::PNG, c.format());
  ASSERT_EQ(ByteVector("foo"), c.data());

  MP4::CoverArt c2(c);
  ASSERT_EQ(MP4::CoverArt::PNG, c2.format());
  ASSERT_EQ(ByteVector("foo"), c2.data());

  MP4::CoverArt c3 = c;
  ASSERT_EQ(MP4::CoverArt::PNG, c3.format());
  ASSERT_EQ(ByteVector("foo"), c3.data());
}

TEST(MP4CoverArt, testList)
{
  MP4::CoverArtList l;
  l.append(MP4::CoverArt(MP4::CoverArt::PNG, "foo"));
  l.append(MP4::CoverArt(MP4::CoverArt::JPEG, "bar"));

  ASSERT_EQ(MP4::CoverArt::PNG, l[0].format());
  ASSERT_EQ(ByteVector("foo"), l[0].data());
  ASSERT_EQ(MP4::CoverArt::JPEG, l[1].format());
  ASSERT_EQ(ByteVector("bar"), l[1].data());
}
