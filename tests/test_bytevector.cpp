/***************************************************************************
    copyright           : (C) 2007 by Lukas Lalinsky
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

#define _USE_MATH_DEFINES
#include "tbytevector.h"
#include "tbytevectorlist.h"
#include <cmath>
#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

TEST(ByteVector, testByteVector)
{
  ByteVector s1("foo");
  ASSERT_EQ(1U, ByteVectorList::split(s1, " ").size());

  ByteVector s2("f");
  ASSERT_EQ(1U, ByteVectorList::split(s2, " ").size());

  ASSERT_TRUE(ByteVector().isEmpty());
  ASSERT_EQ(0U, ByteVector().size());
  ASSERT_TRUE(ByteVector("asdf").clear().isEmpty());
  ASSERT_EQ(0U, ByteVector("asdf").clear().size());
  ASSERT_EQ(ByteVector(), ByteVector("asdf").clear());

  ByteVector i("blah blah");
  ByteVector j("blah");
  ASSERT_TRUE(i.containsAt(j, 5, 0));
  ASSERT_TRUE(i.containsAt(j, 6, 1));
  ASSERT_TRUE(i.containsAt(j, 6, 1, 3));

  i.clear();
  ASSERT_TRUE(i.isEmpty());
}

TEST(ByteVector, testFind1)
{
  ASSERT_EQ(4, ByteVector("....SggO.").find("SggO"));
  ASSERT_EQ(4, ByteVector("....SggO.").find("SggO", 0));
  ASSERT_EQ(4, ByteVector("....SggO.").find("SggO", 1));
  ASSERT_EQ(4, ByteVector("....SggO.").find("SggO", 2));
  ASSERT_EQ(4, ByteVector("....SggO.").find("SggO", 3));
  ASSERT_EQ(4, ByteVector("....SggO.").find("SggO", 4));
  ASSERT_EQ(-1, ByteVector("....SggO.").find("SggO", 5));
  ASSERT_EQ(-1, ByteVector("....SggO.").find("SggO", 6));
  ASSERT_EQ(-1, ByteVector("....SggO.").find("SggO", 7));
  ASSERT_EQ(-1, ByteVector("....SggO.").find("SggO", 8));

  // Intentional out-of-bounds access.
  ByteVector v("0123456789x");
  v.resize(10);
  v.data()[10] = 'x';
  ASSERT_EQ(-1, v.find("789x", 7));
}

TEST(ByteVector, testFind2)
{
  ASSERT_EQ(0, ByteVector("\x01", 1).find("\x01"));
  ASSERT_EQ(0, ByteVector("\x01\x02", 2).find("\x01\x02"));
  ASSERT_EQ(-1, ByteVector("\x01", 1).find("\x02"));
  ASSERT_EQ(-1, ByteVector("\x01\x02", 2).find("\x01\x03"));
}

TEST(ByteVector, testFind3)
{
  ASSERT_EQ(4, ByteVector("....SggO.").find('S'));
  ASSERT_EQ(4, ByteVector("....SggO.").find('S', 0));
  ASSERT_EQ(4, ByteVector("....SggO.").find('S', 1));
  ASSERT_EQ(4, ByteVector("....SggO.").find('S', 2));
  ASSERT_EQ(4, ByteVector("....SggO.").find('S', 3));
  ASSERT_EQ(4, ByteVector("....SggO.").find('S', 4));
  ASSERT_EQ(-1, ByteVector("....SggO.").find('S', 5));
  ASSERT_EQ(-1, ByteVector("....SggO.").find('S', 6));
  ASSERT_EQ(-1, ByteVector("....SggO.").find('S', 7));
  ASSERT_EQ(-1, ByteVector("....SggO.").find('S', 8));
}

TEST(ByteVector, testRfind1)
{
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 0));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 1));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 2));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 3));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 4));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 5));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 6));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 7));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS", 8));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind("OggS"));
}

TEST(ByteVector, testRfind2)
{
  ByteVector r0("**************");
  ByteVector r1("OggS**********");
  ByteVector r2("**********OggS");
  ByteVector r3("OggS******OggS");
  ByteVector r4("OggS*OggS*OggS");

  ASSERT_EQ(-1, r0.find("OggS"));
  ASSERT_EQ(-1, r0.rfind("OggS"));
  ASSERT_EQ(0, r1.find("OggS"));
  ASSERT_EQ(0, r1.rfind("OggS"));
  ASSERT_EQ(10, r2.find("OggS"));
  ASSERT_EQ(10, r2.rfind("OggS"));
  ASSERT_EQ(0, r3.find("OggS"));
  ASSERT_EQ(10, r3.rfind("OggS"));
  ASSERT_EQ(10, r4.rfind("OggS"));
  ASSERT_EQ(10, r4.rfind("OggS", 0));
  ASSERT_EQ(5, r4.rfind("OggS", 7));
  ASSERT_EQ(10, r4.rfind("OggS", 12));
}

TEST(ByteVector, testRfind3)
{
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 0));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 1));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 2));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 3));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 4));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 5));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 6));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 7));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O', 8));
  ASSERT_EQ(1, ByteVector(".OggS....").rfind('O'));
}

TEST(ByteVector, testToHex)
{
  ByteVector v("\xf0\xe1\xd2\xc3\xb4\xa5\x96\x87\x78\x69\x5a\x4b\x3c\x2d\x1e\x0f", 16);

  ASSERT_EQ(ByteVector("f0e1d2c3b4a5968778695a4b3c2d1e0f"), v.toHex());
}

TEST(ByteVector, testIntegerConversion)
{
  const ByteVector data("\x00\xff\x01\xff\x00\xff\x01\xff\x00\xff\x01\xff\x00\xff", 14);

  ASSERT_EQ(static_cast<short>(0x00ff), data.toShort());
  ASSERT_EQ(static_cast<short>(0xff00), data.toShort(false));
  ASSERT_EQ(static_cast<short>(0xff01), data.toShort(5U));
  ASSERT_EQ(static_cast<short>(0x01ff), data.toShort(5U, false));
  ASSERT_EQ(static_cast<short>(0xff), data.toShort(13U));
  ASSERT_EQ(static_cast<short>(0xff), data.toShort(13U, false));

  ASSERT_EQ(static_cast<unsigned short>(0x00ff), data.toUShort());
  ASSERT_EQ(static_cast<unsigned short>(0xff00), data.toUShort(false));
  ASSERT_EQ(static_cast<unsigned short>(0xff01), data.toUShort(5U));
  ASSERT_EQ(static_cast<unsigned short>(0x01ff), data.toUShort(5U, false));
  ASSERT_EQ(static_cast<unsigned short>(0xff), data.toUShort(13U));
  ASSERT_EQ(static_cast<unsigned short>(0xff), data.toUShort(13U, false));

  ASSERT_EQ(0x00ff01ffU, data.toUInt());
  ASSERT_EQ(0xff01ff00U, data.toUInt(false));
  ASSERT_EQ(0xff01ff00U, data.toUInt(5U));
  ASSERT_EQ(0x00ff01ffU, data.toUInt(5U, false));
  ASSERT_EQ(0x00ffU, data.toUInt(12U));
  ASSERT_EQ(0xff00U, data.toUInt(12U, false));

  ASSERT_EQ(0x00ff01U, data.toUInt(0U, 3U));
  ASSERT_EQ(0x01ff00U, data.toUInt(0U, 3U, false));
  ASSERT_EQ(0xff01ffU, data.toUInt(5U, 3U));
  ASSERT_EQ(0xff01ffU, data.toUInt(5U, 3U, false));
  ASSERT_EQ(0x00ffU, data.toUInt(12U, 3U));
  ASSERT_EQ(0xff00U, data.toUInt(12U, 3U, false));

  ASSERT_EQ(static_cast<long long>(0x00ff01ff00ff01ffULL), data.toLongLong());
  ASSERT_EQ(static_cast<long long>(0xff01ff00ff01ff00ULL), data.toLongLong(false));
  ASSERT_EQ(static_cast<long long>(0xff01ff00ff01ff00ULL), data.toLongLong(5U));
  ASSERT_EQ(static_cast<long long>(0x00ff01ff00ff01ffULL), data.toLongLong(5U, false));
  ASSERT_EQ(static_cast<long long>(0x00ffU), data.toLongLong(12U));
  ASSERT_EQ(static_cast<long long>(0xff00U), data.toLongLong(12U, false));
}

TEST(ByteVector, testFloatingPointConversion)
{
  const double Tolerance = 1.0e-7;

  const ByteVector pi32le("\xdb\x0f\x49\x40", 4);
  ASSERT_TRUE(std::abs(pi32le.toFloat32LE(0) - M_PI) < Tolerance);
  ASSERT_EQ(pi32le, ByteVector::fromFloat32LE(pi32le.toFloat32LE(0)));

  const ByteVector pi32be("\x40\x49\x0f\xdb", 4);
  ASSERT_TRUE(std::abs(pi32be.toFloat32BE(0) - M_PI) < Tolerance);
  ASSERT_EQ(pi32be, ByteVector::fromFloat32BE(pi32be.toFloat32BE(0)));

  const ByteVector pi64le("\x18\x2d\x44\x54\xfb\x21\x09\x40", 8);
  ASSERT_TRUE(std::abs(pi64le.toFloat64LE(0) - M_PI) < Tolerance);
  ASSERT_EQ(pi64le, ByteVector::fromFloat64LE(pi64le.toFloat64LE(0)));

  const ByteVector pi64be("\x40\x09\x21\xfb\x54\x44\x2d\x18", 8);
  ASSERT_TRUE(std::abs(pi64be.toFloat64BE(0) - M_PI) < Tolerance);
  ASSERT_EQ(pi64be, ByteVector::fromFloat64BE(pi64be.toFloat64BE(0)));

  const ByteVector pi80le("\x00\xc0\x68\x21\xa2\xda\x0f\xc9\x00\x40", 10);
  ASSERT_TRUE(std::abs(pi80le.toFloat80LE(0) - M_PI) < Tolerance);

  const ByteVector pi80be("\x40\x00\xc9\x0f\xda\xa2\x21\x68\xc0\x00", 10);
  ASSERT_TRUE(std::abs(pi80be.toFloat80BE(0) - M_PI) < Tolerance);
}

TEST(ByteVector, testReplace)
{
  {
    ByteVector a("abcdabf");
    a.replace(ByteVector(""), ByteVector("<a>"));
    ASSERT_EQ(ByteVector("abcdabf"), a);
  }
  {
    ByteVector a("abcdabf");
    a.replace(ByteVector("foobartoolong"), ByteVector("<a>"));
    ASSERT_EQ(ByteVector("abcdabf"), a);
  }
  {
    ByteVector a("abcdabf");
    a.replace(ByteVector("xx"), ByteVector("yy"));
    ASSERT_EQ(ByteVector("abcdabf"), a);
  }
  {
    ByteVector a("abcdabf");
    a.replace(ByteVector("a"), ByteVector("x"));
    ASSERT_EQ(ByteVector("xbcdxbf"), a);
    a.replace(ByteVector("x"), ByteVector("a"));
    ASSERT_EQ(ByteVector("abcdabf"), a);
  }
  {
    ByteVector a("abcdabf");
    a.replace('a', 'x');
    ASSERT_EQ(ByteVector("xbcdxbf"), a);
    a.replace('x', 'a');
    ASSERT_EQ(ByteVector("abcdabf"), a);
  }
  {
    ByteVector a("abcdabf");
    a.replace(ByteVector("ab"), ByteVector("xy"));
    ASSERT_EQ(ByteVector("xycdxyf"), a);
    a.replace(ByteVector("xy"), ByteVector("ab"));
    ASSERT_EQ(ByteVector("abcdabf"), a);
  }
  {
    ByteVector a("abcdabf");
    a.replace(ByteVector("a"), ByteVector("<a>"));
    ASSERT_EQ(ByteVector("<a>bcd<a>bf"), a);
    a.replace(ByteVector("<a>"), ByteVector("a"));
    ASSERT_EQ(ByteVector("abcdabf"), a);
  }
  {
    ByteVector a("abcdabf");
    a.replace(ByteVector("b"), ByteVector("<b>"));
    ASSERT_EQ(ByteVector("a<b>cda<b>f"), a);
    a.replace(ByteVector("<b>"), ByteVector("b"));
    ASSERT_EQ(ByteVector("abcdabf"), a);
  }
  {
    ByteVector a("abcdabc");
    a.replace(ByteVector("c"), ByteVector("<c>"));
    ASSERT_EQ(ByteVector("ab<c>dab<c>"), a);
    a.replace(ByteVector("<c>"), ByteVector("c"));
    ASSERT_EQ(ByteVector("abcdabc"), a);
  }
  {
    ByteVector a("abcdaba");
    a.replace(ByteVector("a"), ByteVector("<a>"));
    ASSERT_EQ(ByteVector("<a>bcd<a>b<a>"), a);
    a.replace(ByteVector("<a>"), ByteVector("a"));
    ASSERT_EQ(ByteVector("abcdaba"), a);
  }
}

TEST(ByteVector, testReplaceAndDetach)
{
  {
    ByteVector a("abcdabf");
    ByteVector b = a;
    a.replace(ByteVector("a"), ByteVector("x"));
    ASSERT_EQ(ByteVector("xbcdxbf"), a);
    ASSERT_EQ(ByteVector("abcdabf"), b);
  }
  {
    ByteVector a("abcdabf");
    ByteVector b = a;
    a.replace('a', 'x');
    ASSERT_EQ(ByteVector("xbcdxbf"), a);
    ASSERT_EQ(ByteVector("abcdabf"), b);
  }
  {
    ByteVector a("abcdabf");
    ByteVector b = a;
    a.replace(ByteVector("ab"), ByteVector("xy"));
    ASSERT_EQ(ByteVector("xycdxyf"), a);
    ASSERT_EQ(ByteVector("abcdabf"), b);
  }
  {
    ByteVector a("abcdabf");
    ByteVector b = a;
    a.replace(ByteVector("a"), ByteVector("<a>"));
    ASSERT_EQ(ByteVector("<a>bcd<a>bf"), a);
    ASSERT_EQ(ByteVector("abcdabf"), b);
  }
  {
    ByteVector a("ab<c>dab<c>");
    ByteVector b = a;
    a.replace(ByteVector("<c>"), ByteVector("c"));
    ASSERT_EQ(ByteVector("abcdabc"), a);
    ASSERT_EQ(ByteVector("ab<c>dab<c>"), b);
  }
}

TEST(ByteVector, testIterator)
{
  ByteVector v1("taglib");
  ByteVector v2 = v1;

  auto it1      = v1.begin();
  auto it2      = v2.begin();

  ASSERT_EQ('t', *it1);
  ASSERT_EQ('t', *it2);

  std::advance(it1, 4);
  std::advance(it2, 4);
  *it2 = 'I';
  ASSERT_EQ('i', *it1);
  ASSERT_EQ('I', *it2);
  ASSERT_EQ(ByteVector("taglib"), v1);
  ASSERT_EQ(ByteVector("taglIb"), v2);

  auto it3 = v1.rbegin();
  auto it4 = v2.rbegin();

  ASSERT_EQ('b', *it3);
  ASSERT_EQ('b', *it4);

  std::advance(it3, 4);
  std::advance(it4, 4);
  *it4 = 'A';
  ASSERT_EQ('a', *it3);
  ASSERT_EQ('A', *it4);
  ASSERT_EQ(ByteVector("taglib"), v1);
  ASSERT_EQ(ByteVector("tAglIb"), v2);

  ByteVector v3;
  v3  = ByteVector("0123456789").mid(3, 4);

  it1 = v3.begin();
  it2 = v3.end() - 1;
  ASSERT_EQ('3', *it1);
  ASSERT_EQ('6', *it2);

  it3 = v3.rbegin();
  it4 = v3.rend() - 1;
  ASSERT_EQ('6', *it3);
  ASSERT_EQ('3', *it4);
}

TEST(ByteVector, testResize)
{
  ByteVector a = ByteVector("0123456789");
  ByteVector b = a.mid(3, 4);
  b.resize(6, 'A');
  ASSERT_EQ(static_cast<unsigned int>(6), b.size());
  ASSERT_EQ('6', b[3]);
  ASSERT_EQ('A', b[4]);
  ASSERT_EQ('A', b[5]);
  b.resize(10, 'B');
  ASSERT_EQ(static_cast<unsigned int>(10), b.size());
  ASSERT_EQ('6', b[3]);
  ASSERT_EQ('B', b[6]);
  ASSERT_EQ('B', b[9]);
  b.resize(3, 'C');
  ASSERT_EQ(static_cast<unsigned int>(3), b.size());
  ASSERT_EQ(-1, b.find('C'));
  b.resize(3);
  ASSERT_EQ(static_cast<unsigned int>(3), b.size());

  // Check if a and b were properly detached.

  ASSERT_EQ(static_cast<unsigned int>(10), a.size());
  ASSERT_EQ('3', a[3]);
  ASSERT_EQ('5', a[5]);

  // Special case that refCount == 1 and d->offset != 0.

  ByteVector c = ByteVector("0123456789").mid(3, 4);
  c.resize(6, 'A');
  ASSERT_EQ(static_cast<unsigned int>(6), c.size());
  ASSERT_EQ('6', c[3]);
  ASSERT_EQ('A', c[4]);
  ASSERT_EQ('A', c[5]);
  c.resize(10, 'B');
  ASSERT_EQ(static_cast<unsigned int>(10), c.size());
  ASSERT_EQ('6', c[3]);
  ASSERT_EQ('B', c[6]);
  ASSERT_EQ('B', c[9]);
  c.resize(3, 'C');
  ASSERT_EQ(static_cast<unsigned int>(3), c.size());
  ASSERT_EQ(-1, c.find('C'));
}

TEST(ByteVector, testAppend1)
{
  ByteVector v1("foo");
  v1.append("bar");
  ASSERT_EQ(ByteVector("foobar"), v1);

  ByteVector v2("foo");
  v2.append("b");
  ASSERT_EQ(ByteVector("foob"), v2);

  ByteVector v3;
  v3.append("b");
  ASSERT_EQ(ByteVector("b"), v3);

  ByteVector v4("foo");
  v4.append(v1);
  ASSERT_EQ(ByteVector("foofoobar"), v4);

  ByteVector v5("foo");
  v5.append('b');
  ASSERT_EQ(ByteVector("foob"), v5);

  ByteVector v6;
  v6.append('b');
  ASSERT_EQ(ByteVector("b"), v6);

  ByteVector v7("taglib");
  ByteVector v8 = v7;

  v7.append("ABC");
  ASSERT_EQ(ByteVector("taglibABC"), v7);
  v7.append('1');
  v7.append('2');
  v7.append('3');
  ASSERT_EQ(ByteVector("taglibABC123"), v7);
  ASSERT_EQ(ByteVector("taglib"), v8);
}

TEST(ByteVector, testAppend2)
{
  ByteVector a("1234");
  a.append(a);
  ASSERT_EQ(ByteVector("12341234"), a);
}

TEST(ByteVector, testBase64)
{
  ByteVector sempty;
  ByteVector t0("a"); // test 1 byte
  ByteVector t1("any carnal pleasure.");
  ByteVector t2("any carnal pleasure");
  ByteVector t3("any carnal pleasur");
  ByteVector s0("a"); // test 1 byte
  ByteVector s1("any carnal pleasure.");
  ByteVector s2("any carnal pleasure");
  ByteVector s3("any carnal pleasur");
  ByteVector eempty;
  ByteVector e0("YQ==");
  ByteVector e1("YW55IGNhcm5hbCBwbGVhc3VyZS4=");
  ByteVector e2("YW55IGNhcm5hbCBwbGVhc3VyZQ==");
  ByteVector e3("YW55IGNhcm5hbCBwbGVhc3Vy");

  // Encode
  ASSERT_EQ(eempty, sempty.toBase64());
  ASSERT_EQ(e0, s0.toBase64());
  ASSERT_EQ(e1, s1.toBase64());
  ASSERT_EQ(e2, s2.toBase64());
  ASSERT_EQ(e3, s3.toBase64());

  // Decode
  ASSERT_EQ(sempty, ByteVector::fromBase64(eempty));
  ASSERT_EQ(s0, ByteVector::fromBase64(e0));
  ASSERT_EQ(s1, ByteVector::fromBase64(e1));
  ASSERT_EQ(s2, ByteVector::fromBase64(e2));
  ASSERT_EQ(s3, ByteVector::fromBase64(e3));

  ASSERT_EQ(t0, ByteVector::fromBase64(s0.toBase64()));
  ASSERT_EQ(t1, ByteVector::fromBase64(s1.toBase64()));
  ASSERT_EQ(t2, ByteVector::fromBase64(s2.toBase64()));
  ASSERT_EQ(t3, ByteVector::fromBase64(s3.toBase64()));

  ByteVector all(static_cast<unsigned int>(256));

  // in order
  {
    for(int i = 0; i < 256; i++) {
      all[i] = static_cast<unsigned char>(i);
    }
    ByteVector b64      = all.toBase64();
    ByteVector original = ByteVector::fromBase64(b64);
    ASSERT_EQ(all, original);
  }

  // reverse
  {
    for(int i = 0; i < 256; i++) {
      all[i] = static_cast<unsigned char>(255) - i;
    }
    ByteVector b64      = all.toBase64();
    ByteVector original = ByteVector::fromBase64(b64);
    ASSERT_EQ(all, original);
  }

  // all zeroes
  {
    for(int i = 0; i < 256; i++) {
      all[i] = 0;
    }
    ByteVector b64      = all.toBase64();
    ByteVector original = ByteVector::fromBase64(b64);
    ASSERT_EQ(all, original);
  }

  // all ones
  {
    for(int i = 0; i < 256; i++) {
      all[i] = static_cast<unsigned char>(0xff);
    }
    ByteVector b64      = all.toBase64();
    ByteVector original = ByteVector::fromBase64(b64);
    ASSERT_EQ(all, original);
  }

  // Missing end bytes
  {
    // No missing bytes
    ByteVector m0("YW55IGNhcm5hbCBwbGVhc3VyZQ==");
    ASSERT_EQ(s2, ByteVector::fromBase64(m0));

    // 1 missing byte
    ByteVector m1("YW55IGNhcm5hbCBwbGVhc3VyZQ=");
    ASSERT_EQ(sempty, ByteVector::fromBase64(m1));

    // 2 missing bytes
    ByteVector m2("YW55IGNhcm5hbCBwbGVhc3VyZQ");
    ASSERT_EQ(sempty, ByteVector::fromBase64(m2));

    // 3 missing bytes
    ByteVector m3("YW55IGNhcm5hbCBwbGVhc3VyZ");
    ASSERT_EQ(sempty, ByteVector::fromBase64(m3));
  }

  // Grok invalid characters
  {
    ByteVector invalid("abd\x00\x01\x02\x03\x04");
    ASSERT_EQ(sempty, ByteVector::fromBase64(invalid));
  }
}
