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
  CPPUNIT_TEST_SUITE_END();

public:

  void testByteVector()
  {
    ByteVector v("foobar");

    CPPUNIT_ASSERT(v.find("ob") == 2);
    CPPUNIT_ASSERT(v.find('b') == 3);

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
  }

  void testFind1()
  {
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO"));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 0));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 1));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 2));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 3));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 4));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos, ByteVector("....SggO."). find("SggO", 5));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos, ByteVector("....SggO."). find("SggO", 6));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos, ByteVector("....SggO."). find("SggO", 7));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos, ByteVector("....SggO."). find("SggO", 8));
  }

  void testFind2()
  {
    CPPUNIT_ASSERT_EQUAL((size_t)0, ByteVector("\x01", 1).find("\x01"));
    CPPUNIT_ASSERT_EQUAL((size_t)0, ByteVector("\x01\x02", 2).find("\x01\x02"));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos, ByteVector("\x01", 1).find("\x02"));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos, ByteVector("\x01\x02", 2).find("\x01\x03"));
  }

  void testRfind1()
  {
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 0));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 2));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 3));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 4));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 5));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 6));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 7));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS", 8));
    CPPUNIT_ASSERT_EQUAL((size_t)1, ByteVector(".OggS....").rfind("OggS"));
  }

  void testRfind2()
  {
    ByteVector r0("**************");
    ByteVector r1("OggS**********");
    ByteVector r2("**********OggS");
    ByteVector r3("OggS******OggS");
    ByteVector r4("OggS*OggS*OggS");

    CPPUNIT_ASSERT_EQUAL(ByteVector::npos, r0.find("OggS"));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos, r0.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL((size_t)0, r1.find("OggS"));
    CPPUNIT_ASSERT_EQUAL((size_t)0, r1.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL((size_t)10, r2.find("OggS"));
    CPPUNIT_ASSERT_EQUAL((size_t)10, r2.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL((size_t)0, r3.find("OggS"));
    CPPUNIT_ASSERT_EQUAL((size_t)10, r3.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL((size_t)10, r4.rfind("OggS"));
    CPPUNIT_ASSERT_EQUAL((size_t)10, r4.rfind("OggS", 0));
    CPPUNIT_ASSERT_EQUAL((size_t)5, r4.rfind("OggS", 7));
    CPPUNIT_ASSERT_EQUAL((size_t)10, r4.rfind("OggS", 12));
  }

  void testToHex()
  {
    ByteVector v("\xf0\xe1\xd2\xc3\xb4\xa5\x96\x87\x78\x69\x5a\x4b\x3c\x2d\x1e\x0f", 16);

    CPPUNIT_ASSERT_EQUAL(ByteVector("f0e1d2c3b4a5968778695a4b3c2d1e0f"), v.toHex());
  }

  void testNumericCoversion()
  {
    ByteVector n(16, 0);
    for(size_t i = 0; i < 8; ++i) {
      n[i * 2    ] = static_cast<unsigned char>(0x11 * i);
      n[i * 2 + 1] = static_cast<unsigned char>(0x11 * (i + 8));
    }

    CPPUNIT_ASSERT(n.toUInt16LE(1) == 4488);
    CPPUNIT_ASSERT(n.toUInt16BE(2) == 4505);
    CPPUNIT_ASSERT(n.toUInt24LE(3) == 11149977);
    CPPUNIT_ASSERT(n.toUInt24BE(4) == 2271795);
    CPPUNIT_ASSERT(n.toUInt32LE(5) == 1153119146);
    CPPUNIT_ASSERT(n.toUInt32BE(6) == 867910860);
    CPPUNIT_ASSERT(n.toInt16LE(3)  == 8857);
    CPPUNIT_ASSERT(n.toInt16BE(7)  == -17596);
    CPPUNIT_ASSERT(n.toInt16LE(10) == -8875);
    CPPUNIT_ASSERT(n.toInt16BE(14) == 30719);
    CPPUNIT_ASSERT(n.toInt64LE(5)  == 7412174897536512938ll);
    CPPUNIT_ASSERT(n.toInt64BE(3)  == -7412174897536512939ll);
    CPPUNIT_ASSERT(n.toInt64LE(6)  == -1268082884489200845ll);
    CPPUNIT_ASSERT(n.toInt64BE(4)  == 2497865822736504285ll);
    
    // Shows the message "toNumber<T>() -- offset is out of range. Returning 0." 2 times.
    CPPUNIT_ASSERT(n.toUInt32LE(13) == 0);
    CPPUNIT_ASSERT(n.toUInt16BE(15) == 0);

    CPPUNIT_ASSERT(ByteVector::fromUInt16LE(n.toInt16LE(5))  == n.mid(5, 2));
    CPPUNIT_ASSERT(ByteVector::fromUInt16BE(n.toInt16BE(9))  == n.mid(9, 2));
    CPPUNIT_ASSERT(ByteVector::fromUInt32LE(n.toUInt32LE(4)) == n.mid(4, 4));
    CPPUNIT_ASSERT(ByteVector::fromUInt32BE(n.toUInt32BE(7)) == n.mid(7, 4));
    CPPUNIT_ASSERT(ByteVector::fromUInt64LE(n.toInt64LE(1))  == n.mid(1, 8));
    CPPUNIT_ASSERT(ByteVector::fromUInt64BE(n.toInt64BE(6))  == n.mid(6, 8));

    CPPUNIT_ASSERT(ByteVector::fromUInt16LE(4386) == ByteVector::fromUInt16BE(8721));
    CPPUNIT_ASSERT(ByteVector::fromUInt32LE(287454020) == ByteVector::fromUInt32BE(1144201745));
    CPPUNIT_ASSERT(ByteVector::fromUInt64LE(1234605615291183940) == ByteVector::fromUInt64BE(4914309075945333265));
  

	const uchar PI32[] = { 0x00, 0x40, 0x49, 0x0f, 0xdb };
	const uchar PI64[] = { 0x00, 0x40, 0x09, 0x21, 0xfb, 0x54, 0x44, 0x2d, 0x18 };

	ByteVector pi32(reinterpret_cast<const char*>(PI32), 5);
	CPPUNIT_ASSERT(static_cast<int>(pi32.toFloat32BE(1) * 100) == 314);

	ByteVector pi64(reinterpret_cast<const char*>(PI64), 9);
	CPPUNIT_ASSERT(static_cast<int>(pi64.toFloat64BE(1) * 100) == 314);
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
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestByteVector);
