/***************************************************************************
    copyright           : (C) 2011 by Lukas Lalinsky
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

#include "tbytevectorstream.h"
#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

TEST(ByteVectorStream, testInitialData)
{
  ByteVector v("abcd");
  ByteVectorStream stream(v);

  ASSERT_EQ(ByteVector("abcd"), *stream.data());
}

TEST(ByteVectorStream, testWriteBlock)
{
  ByteVector v("abcd");
  ByteVectorStream stream(v);

  stream.seek(1);
  stream.writeBlock(ByteVector("xx"));
  ASSERT_EQ(ByteVector("axxd"), *stream.data());
}

TEST(ByteVectorStream, testWriteBlockResize)
{
  ByteVector v("abcd");
  ByteVectorStream stream(v);

  stream.seek(3);
  stream.writeBlock(ByteVector("xx"));
  ASSERT_EQ(ByteVector("abcxx"), *stream.data());
  stream.seek(5);
  stream.writeBlock(ByteVector("yy"));
  ASSERT_EQ(ByteVector("abcxxyy"), *stream.data());
}

TEST(ByteVectorStream, testReadBlock)
{
  ByteVector v("abcd");
  ByteVectorStream stream(v);

  ASSERT_EQ(ByteVector("a"), stream.readBlock(1));
  ASSERT_EQ(ByteVector("bc"), stream.readBlock(2));
  ASSERT_EQ(ByteVector("d"), stream.readBlock(3));
  ASSERT_EQ(ByteVector(""), stream.readBlock(3));
}

TEST(ByteVectorStream, testRemoveBlock)
{
  ByteVector v("abcd");
  ByteVectorStream stream(v);

  stream.removeBlock(1, 1);
  ASSERT_EQ(ByteVector("acd"), *stream.data());
  stream.removeBlock(0, 2);
  ASSERT_EQ(ByteVector("d"), *stream.data());
  stream.removeBlock(0, 2);
  ASSERT_EQ(ByteVector(""), *stream.data());
}

TEST(ByteVectorStream, testInsert)
{
  ByteVector v("abcd");
  ByteVectorStream stream(v);

  stream.insert(ByteVector("xx"), 1, 1);
  ASSERT_EQ(ByteVector("axxcd"), *stream.data());
  stream.insert(ByteVector("yy"), 0, 2);
  ASSERT_EQ(ByteVector("yyxcd"), *stream.data());
  stream.insert(ByteVector("foa"), 3, 2);
  ASSERT_EQ(ByteVector("yyxfoa"), *stream.data());
  stream.insert(ByteVector("123"), 3, 0);
  ASSERT_EQ(ByteVector("yyx123foa"), *stream.data());
}

TEST(ByteVectorStream, testSeekEnd)
{
  ByteVector v("abcdefghijklmnopqrstuvwxyz");
  ByteVectorStream stream(v);
  ASSERT_EQ(static_cast<offset_t>(26), stream.length());

  stream.seek(-4, IOStream::End);
  ASSERT_EQ(ByteVector("w"), stream.readBlock(1));

  stream.seek(-25, IOStream::End);
  ASSERT_EQ(ByteVector("b"), stream.readBlock(1));
}
