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
  CPPUNIT_TEST(testResize);
  CPPUNIT_TEST(testAppend);
  CPPUNIT_TEST(testBase64);
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

    ByteVector i("blah blah");
    ByteVector j("blah");
    CPPUNIT_ASSERT(i.containsAt(j, 5, 0));
    CPPUNIT_ASSERT(i.containsAt(j, 6, 1));
    CPPUNIT_ASSERT(i.containsAt(j, 6, 1, 3));

    i.clear();
    CPPUNIT_ASSERT(i.isEmpty());
  }

  void testFind1()
  {
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO"));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 0));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 1));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 2));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 3));
    CPPUNIT_ASSERT_EQUAL((size_t)4, ByteVector("....SggO."). find("SggO", 4));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), ByteVector("....SggO."). find("SggO", 5));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), ByteVector("....SggO."). find("SggO", 6));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), ByteVector("....SggO."). find("SggO", 7));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), ByteVector("....SggO."). find("SggO", 8));

    // Intentional out-of-bounds access.
    ByteVector v("0123456789x");
    v.resize(10);
    v.data()[10] = 'x';
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), v.find("789x", 7));
  }

  void testFind2()
  {
    CPPUNIT_ASSERT_EQUAL((size_t)0, ByteVector("\x01", 1).find("\x01"));
    CPPUNIT_ASSERT_EQUAL((size_t)0, ByteVector("\x01\x02", 2).find("\x01\x02"));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), ByteVector("\x01", 1).find("\x02"));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), ByteVector("\x01\x02", 2).find("\x01\x03"));
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

    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), r0.find("OggS"));
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), r0.rfind("OggS"));
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
    // n = { 0x00, 0x88, 0x11, 0x99, ..., 0x77, 0xFF }
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

    CPPUNIT_ASSERT(ByteVector::fromUInt16LE(n.toInt16LE(5))  == n.mid(5, 2));
    CPPUNIT_ASSERT(ByteVector::fromUInt16BE(n.toInt16BE(9))  == n.mid(9, 2));
    CPPUNIT_ASSERT(ByteVector::fromUInt32LE(n.toUInt32LE(4)) == n.mid(4, 4));
    CPPUNIT_ASSERT(ByteVector::fromUInt32BE(n.toUInt32BE(7)) == n.mid(7, 4));
    CPPUNIT_ASSERT(ByteVector::fromUInt64LE(n.toInt64LE(1))  == n.mid(1, 8));
    CPPUNIT_ASSERT(ByteVector::fromUInt64BE(n.toInt64BE(6))  == n.mid(6, 8));

    CPPUNIT_ASSERT(ByteVector::fromUInt16LE(4386) == ByteVector::fromUInt16BE(8721));
    CPPUNIT_ASSERT(ByteVector::fromUInt32LE(287454020) == ByteVector::fromUInt32BE(1144201745));
    CPPUNIT_ASSERT(ByteVector::fromUInt64LE(1234605615291183940ll) == ByteVector::fromUInt64BE(4914309075945333265ll));

    const unsigned char PI32LE[] = { 0x00, 0xdb, 0x0f, 0x49, 0x40 };
    const unsigned char PI32BE[] = { 0x00, 0x40, 0x49, 0x0f, 0xdb };
    const unsigned char PI64LE[] = { 0x00, 0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40 };
    const unsigned char PI64BE[] = { 0x00, 0x40, 0x09, 0x21, 0xfb, 0x54, 0x44, 0x2d, 0x18 };
    const unsigned char PI80LE[] = { 0x00, 0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9, 0x00, 0x40 };
    const unsigned char PI80BE[] = { 0x00, 0x40, 0x00, 0xc9, 0x0f, 0xda, 0xa2, 0x21, 0x68, 0xc0, 0x00 };

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
    CPPUNIT_ASSERT_EQUAL(ByteVector("taglib"), v1);
    CPPUNIT_ASSERT_EQUAL(ByteVector("taglIb"), v2);

    ByteVector::ReverseIterator it3 = v1.rbegin();
    ByteVector::ReverseIterator it4 = v2.rbegin();

    CPPUNIT_ASSERT_EQUAL('b', *it3);
    CPPUNIT_ASSERT_EQUAL('b', *it4);

    std::advance(it3, 4);
    std::advance(it4, 4);
    *it4 = 'A';
    CPPUNIT_ASSERT_EQUAL('a', *it3);
    CPPUNIT_ASSERT_EQUAL('A', *it4);
    CPPUNIT_ASSERT_EQUAL(ByteVector("taglib"), v1);
    CPPUNIT_ASSERT_EQUAL(ByteVector("tAglIb"), v2);

    ByteVector v3;
    v3 = ByteVector("0123456789").mid(3, 4);

    it1 = v3.begin();
    it2 = v3.end() - 1;
    CPPUNIT_ASSERT_EQUAL('3', *it1);
    CPPUNIT_ASSERT_EQUAL('6', *it2);

    it3 = v3.rbegin();
    it4 = v3.rend() - 1;
    CPPUNIT_ASSERT_EQUAL('6', *it3);
    CPPUNIT_ASSERT_EQUAL('3', *it4);
  }

  void testResize()
  {
    ByteVector a = ByteVector("0123456789");
    ByteVector b = a.mid(3, 4);
    b.resize(6, 'A');
    CPPUNIT_ASSERT_EQUAL((size_t)6, b.size());
    CPPUNIT_ASSERT_EQUAL('6', b[3]);
    CPPUNIT_ASSERT_EQUAL('A', b[4]);
    CPPUNIT_ASSERT_EQUAL('A', b[5]);
    b.resize(10, 'B');
    CPPUNIT_ASSERT_EQUAL((size_t)10, b.size());
    CPPUNIT_ASSERT_EQUAL('6', b[3]);
    CPPUNIT_ASSERT_EQUAL('B', b[6]);
    CPPUNIT_ASSERT_EQUAL('B', b[9]);
    b.resize(3, 'C');
    CPPUNIT_ASSERT_EQUAL((size_t)3, b.size());
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), b.find('C'));
    b.resize(3);
    CPPUNIT_ASSERT_EQUAL((size_t)3, b.size());

    // Check if a and b were properly detached.

    CPPUNIT_ASSERT_EQUAL((size_t)10, a.size());
    CPPUNIT_ASSERT_EQUAL('3', a[3]);
    CPPUNIT_ASSERT_EQUAL('5', a[5]);

    // Special case that refCount == 1 and d->offset != 0.

    ByteVector c = ByteVector("0123456789").mid(3, 4);
    c.resize(6, 'A');
    CPPUNIT_ASSERT_EQUAL(size_t(6), c.size());
    CPPUNIT_ASSERT_EQUAL('6', c[3]);
    CPPUNIT_ASSERT_EQUAL('A', c[4]);
    CPPUNIT_ASSERT_EQUAL('A', c[5]);
    c.resize(10, 'B');
    CPPUNIT_ASSERT_EQUAL((size_t)10, c.size());
    CPPUNIT_ASSERT_EQUAL('6', c[3]);
    CPPUNIT_ASSERT_EQUAL('B', c[6]);
    CPPUNIT_ASSERT_EQUAL('B', c[9]);
    c.resize(3, 'C');
    CPPUNIT_ASSERT_EQUAL((size_t)3, c.size());
    CPPUNIT_ASSERT_EQUAL(ByteVector::npos(), c.find('C'));
  }

  void testAppend()
  {
    ByteVector v1("taglib");
    ByteVector v2 = v1;

    v1.append("ABC");
    CPPUNIT_ASSERT_EQUAL(ByteVector("taglibABC"), v1);
    v1.append('1');
    v1.append('2');
    v1.append('3');
    CPPUNIT_ASSERT_EQUAL(ByteVector("taglibABC123"), v1);
    CPPUNIT_ASSERT_EQUAL(ByteVector("taglib"), v2);
  }

  void testBase64()
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
    CPPUNIT_ASSERT_EQUAL(eempty, sempty.toBase64());
    CPPUNIT_ASSERT_EQUAL(e0, s0.toBase64());
    CPPUNIT_ASSERT_EQUAL(e1, s1.toBase64());
    CPPUNIT_ASSERT_EQUAL(e2, s2.toBase64());
    CPPUNIT_ASSERT_EQUAL(e3, s3.toBase64());

    // Decode
    CPPUNIT_ASSERT_EQUAL(sempty, eempty.toBase64());
    CPPUNIT_ASSERT_EQUAL(s0, ByteVector::fromBase64(e0));
    CPPUNIT_ASSERT_EQUAL(s1, ByteVector::fromBase64(e1));
    CPPUNIT_ASSERT_EQUAL(s2, ByteVector::fromBase64(e2));
    CPPUNIT_ASSERT_EQUAL(s3, ByteVector::fromBase64(e3));

    CPPUNIT_ASSERT_EQUAL(t0, ByteVector::fromBase64(s0.toBase64()));
    CPPUNIT_ASSERT_EQUAL(t1, ByteVector::fromBase64(s1.toBase64()));
    CPPUNIT_ASSERT_EQUAL(t2, ByteVector::fromBase64(s2.toBase64()));
    CPPUNIT_ASSERT_EQUAL(t3, ByteVector::fromBase64(s3.toBase64()));

    ByteVector all((size_t)256, '\0');

    // in order
    {
      for(int i = 0; i < 256; i++){
        all[i]=(unsigned char)i;
        }
      ByteVector b64 = all.toBase64();
      ByteVector original = ByteVector::fromBase64(b64);
      CPPUNIT_ASSERT_EQUAL(all,original);
    }

    // reverse
    {
      for(int i = 0; i < 256; i++){
        all[i]=(unsigned char)255-i;
        }
      ByteVector b64 = all.toBase64();
      ByteVector original = ByteVector::fromBase64(b64);
      CPPUNIT_ASSERT_EQUAL(all,original);
    }

    // all zeroes
    {
      for(int i = 0; i < 256; i++){
        all[i]=0;
        }
      ByteVector b64 = all.toBase64();
      ByteVector original = ByteVector::fromBase64(b64);
      CPPUNIT_ASSERT_EQUAL(all,original);
    }

    // all ones
    {
      for(int i = 0; i < 256; i++){
        all[i]=(unsigned char)0xff;
        }
      ByteVector b64 = all.toBase64();
      ByteVector original = ByteVector::fromBase64(b64);
      CPPUNIT_ASSERT_EQUAL(all,original);
    }

    // Missing end bytes
    {
      // No missing bytes
      ByteVector m0("YW55IGNhcm5hbCBwbGVhc3VyZQ==");
      CPPUNIT_ASSERT_EQUAL(s2,ByteVector::fromBase64(m0));

      // 1 missing byte
      ByteVector m1("YW55IGNhcm5hbCBwbGVhc3VyZQ=");
      CPPUNIT_ASSERT_EQUAL(sempty,ByteVector::fromBase64(m1));

      // 2 missing bytes
      ByteVector m2("YW55IGNhcm5hbCBwbGVhc3VyZQ");
      CPPUNIT_ASSERT_EQUAL(sempty,ByteVector::fromBase64(m2));

      // 3 missing bytes
      ByteVector m3("YW55IGNhcm5hbCBwbGVhc3VyZ");
      CPPUNIT_ASSERT_EQUAL(sempty,ByteVector::fromBase64(m3));
    }

    // Grok invalid characters
    {
      ByteVector invalid("abd\x00\x01\x02\x03\x04");
      CPPUNIT_ASSERT_EQUAL(sempty,ByteVector::fromBase64(invalid));
    }

  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestByteVector);

