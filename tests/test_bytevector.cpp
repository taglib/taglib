/* Copyright (C) 2003 Scott Wheeler <wheeler@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstring>
#include <tbytevector.h>
#include <tbytevectorlist.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace TagLib;

class TestByteVector : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestByteVector);
  CPPUNIT_TEST(testByteVector);
  CPPUNIT_TEST(testFind1);
  CPPUNIT_TEST(testFind2);
  CPPUNIT_TEST(testRfind1);
  CPPUNIT_TEST(testRfind2);
  CPPUNIT_TEST(testToHex);
  CPPUNIT_TEST(testNumericCoversion);
  CPPUNIT_TEST(testReplace);
  CPPUNIT_TEST(testIterator);
  CPPUNIT_TEST_SUITE_END();

public:

  void testConversion(unsigned int i, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
  {
    ByteVector v(4, 0);

    v[3] = a;
    v[2] = b;
    v[1] = c;
    v[0] = d;
    CPPUNIT_ASSERT(v.toUInt(false) == i);

    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;
    CPPUNIT_ASSERT(v.toUInt() == i);
  }

  void testByteVector()
  {
    ByteVector v("foobar");

    CPPUNIT_ASSERT(v.find("ob") == 2);
    CPPUNIT_ASSERT(v.find('b') == 3);

    ByteVector n(4, 0);
    n[0] = 1;
    CPPUNIT_ASSERT(n.toUInt(true) == 16777216);
    CPPUNIT_ASSERT(n.toUInt(false) == 1);
    CPPUNIT_ASSERT(ByteVector::fromUInt(16777216, true) == n);
    CPPUNIT_ASSERT(ByteVector::fromUInt(1, false) == n);

    CPPUNIT_ASSERT(ByteVector::fromUInt(0xa0).toUInt() == 0xa0);

    testConversion(0x000000a0, 0x00, 0x00, 0x00, 0xa0);
    testConversion(0xd50bf072, 0xd5, 0x0b, 0xf0, 0x72);

    ByteVector intVector(2, 0);
    intVector[0] = char(0xfc);
    intVector[1] = char(0x00);
    CPPUNIT_ASSERT(intVector.toShort() == -1024);
    intVector[0] = char(0x04);
    intVector[1] = char(0x00);
    CPPUNIT_ASSERT(intVector.toShort() == 1024);

    CPPUNIT_ASSERT(ByteVector::fromLongLong(1).toLongLong() == 1);
    CPPUNIT_ASSERT(ByteVector::fromLongLong(0).toLongLong() == 0);
    CPPUNIT_ASSERT(ByteVector::fromLongLong(0xffffffffffffffffLL).toLongLong() == -1);
    CPPUNIT_ASSERT(ByteVector::fromLongLong(0xfffffffffffffffeLL).toLongLong() == -2);
    CPPUNIT_ASSERT(ByteVector::fromLongLong(1024).toLongLong() == 1024);

    ByteVector a1("foo");
    a1.append("bar");
    CPPUNIT_ASSERT(a1 == "foobar");

    ByteVector a2("foo");
    a2.append("b");
    CPPUNIT_ASSERT(a2 == "foob");

    ByteVector a3;
    a3.append("b");
    CPPUNIT_ASSERT(a3 == "b");

    ByteVector s1("foo");
    CPPUNIT_ASSERT(ByteVectorList::split(s1, " ").size() == 1);

    ByteVector s2("f");
    CPPUNIT_ASSERT(ByteVectorList::split(s2, " ").size() == 1);

    CPPUNIT_ASSERT(ByteVector().size() == 0);
    CPPUNIT_ASSERT(ByteVector("asdf").clear().size() == 0);
    CPPUNIT_ASSERT(ByteVector("asdf").clear() == ByteVector());

    ByteVector i("blah blah");
    ByteVector j("blah");
    CPPUNIT_ASSERT(i.containsAt(j, 5, 0));
    CPPUNIT_ASSERT(i.containsAt(j, 6, 1));
    CPPUNIT_ASSERT(i.containsAt(j, 6, 1, 3));
  }

  void testFind1()
  {
    CPPUNIT_ASSERT_EQUAL(4, ByteVector("....SggO."). find("SggO"));
    CPPUNIT_ASSERT_EQUAL(4, ByteVector("....SggO."). find("SggO", 0));
    CPPUNIT_ASSERT_EQUAL(4, ByteVector("....SggO."). find("SggO", 1));
    CPPUNIT_ASSERT_EQUAL(4, ByteVector("....SggO."). find("SggO", 2));
    CPPUNIT_ASSERT_EQUAL(4, ByteVector("....SggO."). find("SggO", 3));
    CPPUNIT_ASSERT_EQUAL(4, ByteVector("....SggO."). find("SggO", 4));
    CPPUNIT_ASSERT_EQUAL(-1, ByteVector("....SggO."). find("SggO", 5));
    CPPUNIT_ASSERT_EQUAL(-1, ByteVector("....SggO."). find("SggO", 6));
    CPPUNIT_ASSERT_EQUAL(-1, ByteVector("....SggO."). find("SggO", 7));
    CPPUNIT_ASSERT_EQUAL(-1, ByteVector("....SggO."). find("SggO", 8));

    // Intentional out-of-bounds access.
    ByteVector v("0123456789x");
    v.resize(10);
    v.data()[10] = 'x';
    CPPUNIT_ASSERT_EQUAL(-1, v.find("789x", 7));
  }

  void testFind2()
  {
    CPPUNIT_ASSERT_EQUAL(0, ByteVector("\x01", 1).find("\x01"));
    CPPUNIT_ASSERT_EQUAL(0, ByteVector("\x01\x02", 2).find("\x01\x02"));
    CPPUNIT_ASSERT_EQUAL(-1, ByteVector("\x01", 1).find("\x02"));
    CPPUNIT_ASSERT_EQUAL(-1, ByteVector("\x01\x02", 2).find("\x01\x03"));
  }

  void testRfind1()
  {
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 0));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 1));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 2));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 3));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 4));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 5));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 6));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 7));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS", 8));
    CPPUNIT_ASSERT_EQUAL(1, ByteVector(".OggS....").rfind("OggS"));
  }

  void testRfind2()
  {
    ByteVector r0("**************");
    ByteVector r1("OggS**********");
    ByteVector r2("**********OggS");
    ByteVector r3("OggS******OggS");
    ByteVector r4("OggS*OggS*OggS");

    CPPUNIT_ASSERT_EQUAL(-1, r0.find("OggS"));
    CPPUNIT_ASSERT_EQUAL(-1, r0.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL(0, r1.find("OggS"));
    CPPUNIT_ASSERT_EQUAL(0, r1.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL(10, r2.find("OggS"));
    CPPUNIT_ASSERT_EQUAL(10, r2.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL(0, r3.find("OggS"));
    CPPUNIT_ASSERT_EQUAL(10, r3.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL(10, r4.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL(10, r4.rfind("OggS", 0));
    CPPUNIT_ASSERT_EQUAL(5, r4.rfind("OggS", 7));
    CPPUNIT_ASSERT_EQUAL(10, r4.rfind("OggS", 12));
  }

  void testToHex()
  {
    ByteVector v("\xf0\xe1\xd2\xc3\xb4\xa5\x96\x87\x78\x69\x5a\x4b\x3c\x2d\x1e\x0f", 16);

    CPPUNIT_ASSERT_EQUAL(ByteVector("f0e1d2c3b4a5968778695a4b3c2d1e0f"), v.toHex());
  }

  void testNumericCoversion()
  {
    CPPUNIT_ASSERT_EQUAL((unsigned short)0xFFFF, ByteVector("\xff\xff", 2).toUShort());
    CPPUNIT_ASSERT_EQUAL((unsigned short)0x0001, ByteVector("\x00\x01", 2).toUShort());
    CPPUNIT_ASSERT_EQUAL((unsigned short)0x0100, ByteVector("\x00\x01", 2).toUShort(false));
    CPPUNIT_ASSERT_EQUAL((unsigned short)0xFF01, ByteVector("\xFF\x01", 2).toUShort());
    CPPUNIT_ASSERT_EQUAL((unsigned short)0x01FF, ByteVector("\xFF\x01", 2).toUShort(false));

    const uchar PI32LE[] = { 0x00, 0xdb, 0x0f, 0x49, 0x40 };
    const uchar PI32BE[] = { 0x00, 0x40, 0x49, 0x0f, 0xdb };
    const uchar PI64LE[] = { 0x00, 0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40 };
    const uchar PI64BE[] = { 0x00, 0x40, 0x09, 0x21, 0xfb, 0x54, 0x44, 0x2d, 0x18 };
    const uchar PI80LE[] = { 0x00, 0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9, 0x00, 0x40 };
    const uchar PI80BE[] = { 0x00, 0x40, 0x00, 0xc9, 0x0f, 0xda, 0xa2, 0x21, 0x68, 0xc0, 0x00 };

    ByteVector pi32le(reinterpret_cast<const char*>(PI32LE), 5);
    CPPUNIT_ASSERT_EQUAL(31415, static_cast<int>(pi32le.toFloat32LE(1) * 10000));

    ByteVector pi32be(reinterpret_cast<const char*>(PI32BE), 5);
    CPPUNIT_ASSERT_EQUAL(31415, static_cast<int>(pi32be.toFloat32BE(1) * 10000));

    ByteVector pi64le(reinterpret_cast<const char*>(PI64LE), 9);
    CPPUNIT_ASSERT_EQUAL(31415, static_cast<int>(pi64le.toFloat64LE(1) * 10000));

    ByteVector pi64be(reinterpret_cast<const char*>(PI64BE), 9);
    CPPUNIT_ASSERT_EQUAL(31415, static_cast<int>(pi64be.toFloat64BE(1) * 10000));

    ByteVector pi80le(reinterpret_cast<const char*>(PI80LE), 11);
    CPPUNIT_ASSERT_EQUAL(31415, static_cast<int>(pi80le.toFloat80LE(1) * 10000));

    ByteVector pi80be(reinterpret_cast<const char*>(PI80BE), 11);
    CPPUNIT_ASSERT_EQUAL(31415, static_cast<int>(pi80be.toFloat80BE(1) * 10000));

    ByteVector pi32le2 = ByteVector::fromFloat32LE(pi32le.toFloat32LE(1));
    CPPUNIT_ASSERT(memcmp(pi32le.data() + 1, pi32le2.data(), 4) == 0);

    ByteVector pi32be2 = ByteVector::fromFloat32BE(pi32be.toFloat32BE(1));
    CPPUNIT_ASSERT(memcmp(pi32be.data() + 1, pi32be2.data(), 4) == 0);

    ByteVector pi64le2 = ByteVector::fromFloat64LE(pi64le.toFloat64LE(1));
    CPPUNIT_ASSERT(memcmp(pi64le.data() + 1, pi64le2.data(), 8) == 0);

    ByteVector pi64be2 = ByteVector::fromFloat64BE(pi64be.toFloat64BE(1));
    CPPUNIT_ASSERT(memcmp(pi64be.data() + 1, pi64be2.data(), 8) == 0);
  }

  void testReplace()
  {
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector(""), ByteVector("<a>"));
      CPPUNIT_ASSERT_EQUAL(ByteVector("abcdabf"), a);
    }
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector("foobartoolong"), ByteVector("<a>"));
      CPPUNIT_ASSERT_EQUAL(ByteVector("abcdabf"), a);
    }
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector("xx"), ByteVector("yy"));
      CPPUNIT_ASSERT_EQUAL(ByteVector("abcdabf"), a);
    }
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector("a"), ByteVector("x"));
      CPPUNIT_ASSERT_EQUAL(ByteVector("xbcdxbf"), a);
    }
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector("ab"), ByteVector("xy"));
      CPPUNIT_ASSERT_EQUAL(ByteVector("xycdxyf"), a);
    }
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector("a"), ByteVector("<a>"));
      CPPUNIT_ASSERT_EQUAL(ByteVector("<a>bcd<a>bf"), a);
    }
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector("ab"), ByteVector("x"));
      CPPUNIT_ASSERT_EQUAL(ByteVector("xcdxf"), a);
    }
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector("ab"), ByteVector());
      CPPUNIT_ASSERT_EQUAL(ByteVector("cdf"), a);
    }
    {
      ByteVector a("abcdabf");
      a.replace(ByteVector("bf"), ByteVector("x"));
      CPPUNIT_ASSERT_EQUAL(ByteVector("abcdax"), a);
    }
  }

  void testIterator()
  {
    ByteVector v1("taglib");
    ByteVector v2 = v1;

    ByteVector::Iterator it1 = v1.begin();
    ByteVector::Iterator it2 = v2.begin();

    CPPUNIT_ASSERT_EQUAL('t', *it1);
    CPPUNIT_ASSERT_EQUAL('t', *it2);

    std::advance(it1, 4);
    std::advance(it2, 4);
    *it2 = 'I';
    CPPUNIT_ASSERT_EQUAL('i', *it1);
    CPPUNIT_ASSERT_EQUAL('I', *it2);

    ByteVector::ReverseIterator it3 = v1.rbegin();
    ByteVector::ReverseIterator it4 = v2.rbegin();

    CPPUNIT_ASSERT_EQUAL('b', *it3);
    CPPUNIT_ASSERT_EQUAL('b', *it4);

    std::advance(it3, 4);
    std::advance(it4, 4);
    *it4 = 'A';
    CPPUNIT_ASSERT_EQUAL('a', *it3);
    CPPUNIT_ASSERT_EQUAL('A', *it4);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestByteVector);
