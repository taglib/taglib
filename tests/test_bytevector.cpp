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

#include <catch/catch.hpp>
#include <cstring>
#include <tbytevector.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("ByteVector")
{
  SECTION("Assign and detach")
  {
    ByteVector v1("a");
    ByteVector v2 = v1;
    v2 = '1';
    REQUIRE(v1 == "a");
    REQUIRE(v2 == "1");

    v1 = "foo";
    REQUIRE(v1 == "foo");
    REQUIRE(v2 == "1");
  }
  SECTION("Comparison")
  {
    const ByteVector v("taglib vector");
    REQUIRE(v == "taglib vector");
    REQUIRE_FALSE(v == "taglib VECTOR");
    REQUIRE_FALSE(v == "taglib");
    REQUIRE_FALSE(v == "taglib vector taglib");

    REQUIRE(memcmp(v.data(), "taglib vector", 13) == 0);
  }
  SECTION("Clear and empty")
  {
    REQUIRE(ByteVector().isEmpty());
    REQUIRE(ByteVector().size() == 0);
    REQUIRE(ByteVector("asdf").clear().isEmpty());
    REQUIRE(ByteVector("asdf").clear().size() == 0);
    REQUIRE(ByteVector("asdf").clear() == ByteVector());

    ByteVector v("blah blah");
    v.clear();
    REQUIRE(v.isEmpty());
    REQUIRE_FALSE(v.isNull());  // deprecated, but worth it to check.
  }
  SECTION("ContainsAt")
  {
    const ByteVector i("blah blah");
    const ByteVector j("blah");
    REQUIRE(i.containsAt(j, 5, 0));
    REQUIRE(i.containsAt(j, 6, 1));
    REQUIRE(i.containsAt(j, 6, 1, 3));
  }
  SECTION("Find (1)")
  {
    REQUIRE(ByteVector("....SggO.").find("SggO") == 4);
    REQUIRE(ByteVector("....SggO.").find("SggO", 0) == 4);
    REQUIRE(ByteVector("....SggO.").find("SggO", 1) == 4);
    REQUIRE(ByteVector("....SggO.").find("SggO", 2) == 4);
    REQUIRE(ByteVector("....SggO.").find("SggO", 3) == 4);
    REQUIRE(ByteVector("....SggO.").find("SggO", 4) == 4);
    REQUIRE(ByteVector("....SggO.").find("SggO", 5) == -1);
    REQUIRE(ByteVector("....SggO.").find("SggO", 6) == -1);
    REQUIRE(ByteVector("....SggO.").find("SggO", 7) == -1);
    REQUIRE(ByteVector("....SggO.").find("SggO", 8) == -1);

    REQUIRE(ByteVector("\x01", 1).find("\x01") == 0);
    REQUIRE(ByteVector("\x01\x02", 2).find("\x01\x02") == 0);
    REQUIRE(ByteVector("\x01", 1).find("\x02") == -1);
    REQUIRE(ByteVector("\x01\x02", 2).find("\x01\x03") == -1);
  }
  SECTION("Find (2)")
  {
    REQUIRE(ByteVector("....SggO.").find('S') == 4);
    REQUIRE(ByteVector("....SggO.").find('S', 0) == 4);
    REQUIRE(ByteVector("....SggO.").find('S', 1) == 4);
    REQUIRE(ByteVector("....SggO.").find('S', 2) == 4);
    REQUIRE(ByteVector("....SggO.").find('S', 3) == 4);
    REQUIRE(ByteVector("....SggO.").find('S', 4) == 4);
    REQUIRE(ByteVector("....SggO.").find('S', 5) == -1);
    REQUIRE(ByteVector("....SggO.").find('S', 6) == -1);
    REQUIRE(ByteVector("....SggO.").find('S', 7) == -1);
    REQUIRE(ByteVector("....SggO.").find('S', 8) == -1);
  }
  SECTION("Find with out-of-bounds garbage")
  {
    // Intentional out-of-bounds access.
    ByteVector v("0123456789x");
    v.resize(10);
    v.data()[10] = 'x';
    REQUIRE(v.find("789x", 7) == -1);
  }
  SECTION("RFind (1)")
  {
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 0) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 1) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 2) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 3) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 4) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 5) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 6) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 7) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS", 8) == 1);
    REQUIRE(ByteVector(".OggS....").rfind("OggS") == 1);
  }
  SECTION("RFind (2)")
  {
    const ByteVector r0("**************");
    const ByteVector r1("OggS**********");
    const ByteVector r2("**********OggS");
    const ByteVector r3("OggS******OggS");
    const ByteVector r4("OggS*OggS*OggS");

    REQUIRE(r0.find("OggS") == -1);
    REQUIRE(r0.rfind("OggS") == -1);
    REQUIRE(r1.find("OggS") == 0);
    REQUIRE(r1.rfind("OggS") == 0);
    REQUIRE(r2.find("OggS") == 10);
    REQUIRE(r2.rfind("OggS") == 10);
    REQUIRE(r3.find("OggS") == 0);
    REQUIRE(r3.rfind("OggS") == 10);
    REQUIRE(r4.rfind("OggS") == 10);
    REQUIRE(r4.rfind("OggS", 0) == 10);
    REQUIRE(r4.rfind("OggS", 7) == 5);
    REQUIRE(r4.rfind("OggS", 12) == 10);
  }
  SECTION("RFind (3)")
  {
    REQUIRE(ByteVector(".OggS....").rfind('O', 0) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O', 1) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O', 2) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O', 3) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O', 4) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O', 5) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O', 6) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O', 7) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O', 8) == 1);
    REQUIRE(ByteVector(".OggS....").rfind('O') == 1);
  }
  SECTION("Convert to hex expression")
  {
    const ByteVector v("\xf0\xe1\xd2\xc3\xb4\xa5\x96\x87\x78\x69\x5a\x4b\x3c\x2d\x1e\x0f", 16);
    REQUIRE(v.toHex() == "f0e1d2c3b4a5968778695a4b3c2d1e0f");
  }
  SECTION("Convert to integers")
  {
    const ByteVector data("\x00\xff\x01\xff\x00\xff\x01\xff\x00\xff\x01\xff\x00\xff", 14);

    REQUIRE(data.toShort() == (short)0x00ff);
    REQUIRE(ByteVector::fromShort(0xff) == data.mid(0, 2));

    REQUIRE(data.toShort(false) == (short)0xff00);
    REQUIRE(data.toShort(5U) == (short)0xff01);
    REQUIRE(data.toShort(5U, false) == (short)0x01ff);
    REQUIRE(data.toShort(13U) == (short)0xff);
    REQUIRE(data.toShort(13U, false) == (short)0xff);

    REQUIRE(data.toUShort() == (unsigned short)0x00ff);
    REQUIRE(data.toUShort(false) == (unsigned short)0xff00);
    REQUIRE(data.toUShort(5U) == (unsigned short)0xff01);
    REQUIRE(data.toUShort(5U, false) == (unsigned short)0x01ff);
    REQUIRE(data.toUShort(13U) == (unsigned short)0xff);
    REQUIRE(data.toUShort(13U, false) == (unsigned short)0xff);

    REQUIRE(data.toUInt() == 0x00ff01ffU);
    REQUIRE(data.toUInt(false) == 0xff01ff00U);
    REQUIRE(data.toUInt(5U) == 0xff01ff00U);
    REQUIRE(data.toUInt(5U, false) == 0x00ff01ffU);
    REQUIRE(data.toUInt(12U) == 0x00ffU);
    REQUIRE(data.toUInt(12U, false) == 0xff00U);

    REQUIRE(data.toUInt(0U, 3U) == 0x00ff01U);
    REQUIRE(data.toUInt(0U, 3U, false) == 0x01ff00U);
    REQUIRE(data.toUInt(5U, 3U) == 0xff01ffU);
    REQUIRE(data.toUInt(5U, 3U, false) == 0xff01ffU);
    REQUIRE(data.toUInt(12U, 3U) == 0x00ffU);
    REQUIRE(data.toUInt(12U, 3U, false) == 0xff00U);

    REQUIRE(data.toLongLong() == (long long)0x00ff01ff00ff01ffULL);
    REQUIRE(data.toLongLong(false) == (long long)0xff01ff00ff01ff00ULL);
    REQUIRE(data.toLongLong(5U) == (long long)0xff01ff00ff01ff00ULL);
    REQUIRE(data.toLongLong(5U, false) == (long long)0x00ff01ff00ff01ffULL);
    REQUIRE(data.toLongLong(12U) == (long long)0x00ffU);
    REQUIRE(data.toLongLong(12U, false) == (long long)0xff00U);
  }
  SECTION("Convert from/to floating point numbers")
  {
    const double PI = 3.14159265358979323846;

    const ByteVector pi32le("\xdb\x0f\x49\x40", 4);
    REQUIRE(pi32le.toFloat32LE(0) == Approx(PI));
    REQUIRE(ByteVector::fromFloat32LE(pi32le.toFloat32LE(0)) == pi32le);

    const ByteVector pi32be("\x40\x49\x0f\xdb", 4);
    REQUIRE(pi32be.toFloat32BE(0) == Approx(PI));
    REQUIRE(ByteVector::fromFloat32BE(pi32be.toFloat32BE(0)) == pi32be);

    const ByteVector pi64le("\x18\x2d\x44\x54\xfb\x21\x09\x40", 8);
    REQUIRE(pi64le.toFloat64LE(0) == Approx(PI));
    REQUIRE(ByteVector::fromFloat64LE(pi64le.toFloat64LE(0)) == pi64le);

    const ByteVector pi64be("\x40\x09\x21\xfb\x54\x44\x2d\x18", 8);
    REQUIRE(pi64be.toFloat64BE(0) == Approx(PI));
    REQUIRE(ByteVector::fromFloat64BE(pi64be.toFloat64BE(0)) == pi64be);

    const ByteVector pi80le("\x00\xc0\x68\x21\xa2\xda\x0f\xc9\x00\x40", 10);
    REQUIRE(pi80le.toFloat80LE(0) == Approx(PI));

    const ByteVector pi80be("\x40\x00\xc9\x0f\xda\xa2\x21\x68\xc0\x00", 10);
    REQUIRE(pi80be.toFloat80BE(0) == Approx(PI));
  }
  SECTION("Replace")
  {
    {
      ByteVector a("abcdabf");
      a.replace("", "<a>");
      REQUIRE(a == "abcdabf");
    }
    {
      ByteVector a("abcdabf");
      a.replace("foobartoolong", "<a>");
      REQUIRE(a == "abcdabf");
    }
    {
      ByteVector a("abcdabf");
      a.replace("xx", "yy");
      REQUIRE(a == "abcdabf");
    }
    {
      ByteVector a("abcdabf");
      ByteVector b = a;
      a.replace("a", "x");
      REQUIRE(a == "xbcdxbf");
      REQUIRE(b == "abcdabf");
      a.replace("x", "a");
      REQUIRE(a == "abcdabf");
    }
    {
      ByteVector a("abcdabf");
      ByteVector b = a;
      a.replace('a', 'x');
      REQUIRE(a == "xbcdxbf");
      REQUIRE(b == "abcdabf");
      a.replace('x', 'a');
      REQUIRE(a == "abcdabf");
    }
    {
      ByteVector a("abcdabf");
      ByteVector b = a;
      a.replace("ab", "xy");
      REQUIRE(a == "xycdxyf");
      REQUIRE(b == "abcdabf");
      a.replace("xy", "ab");
      REQUIRE(a == "abcdabf");
    }
    {
      ByteVector a("abcdabf");
      ByteVector b = a;
      a.replace("a", "<a>");
      REQUIRE(a == "<a>bcd<a>bf");
      REQUIRE(b == "abcdabf");
      a.replace("<a>", "a");
      REQUIRE(a == "abcdabf");
    }
    {
      ByteVector a("abcdabf");
      a.replace("b", "<b>");
      REQUIRE(a == "a<b>cda<b>f");
      a.replace("<b>", "b");
      REQUIRE(a == "abcdabf");
    }
    {
      ByteVector a("abcdabc");
      a.replace("c", "<c>");
      REQUIRE(a == "ab<c>dab<c>");
      a.replace("<c>", "c");
      REQUIRE(a == "abcdabc");
    }
    {
      ByteVector a("abcdaba");
      a.replace("a", "<a>");
      REQUIRE(a == "<a>bcd<a>b<a>");
      a.replace("<a>", "a");
      REQUIRE(a == "abcdaba");
    }
    {
      ByteVector a("ab<c>dab<c>");
      ByteVector b = a;
      a.replace("<c>", "c");
      REQUIRE(a == "abcdabc");
      REQUIRE(b == "ab<c>dab<c>");
    }
  }
  SECTION("Iterator")
  {
    ByteVector v1("taglib");
    ByteVector v2 = v1;
    
    ByteVector::Iterator it1 = v1.begin();
    ByteVector::Iterator it2 = v2.begin();
    
    REQUIRE(*it1 == 't');
    REQUIRE(*it2 == 't');
    
    std::advance(it1, 4);
    std::advance(it2, 4);
    *it2 = 'I';
    REQUIRE(*it1 == 'i');
    REQUIRE(*it2 == 'I');
    REQUIRE(v1 == "taglib");
    REQUIRE(v2 == "taglIb");
    
    ByteVector::ReverseIterator it3 = v1.rbegin();
    ByteVector::ReverseIterator it4 = v2.rbegin();
    
    REQUIRE(*it3 == 'b');
    REQUIRE(*it4 == 'b');
    
    std::advance(it3, 4);
    std::advance(it4, 4);
    *it4 = 'A';
    REQUIRE(*it3 == 'a');
    REQUIRE(*it4 == 'A');
    REQUIRE(v1 == "taglib");
    REQUIRE(v2 == "tAglIb");
    
    // Special case that refCount == 1 and d->offset != 0.
    ByteVector v3 = ByteVector("0123456789").mid(3, 4);

    it1 = v3.begin();
    it2 = v3.end() - 1;
    REQUIRE(*it1 == '3');
    REQUIRE(*it2 == '6');
    
    it3 = v3.rbegin();
    it4 = v3.rend() - 1;
    REQUIRE(*it3 == '6');
    REQUIRE(*it4 == '3');
  }
  SECTION("Resize")
  {
    ByteVector a = "0123456789";
    ByteVector b = a.mid(3, 4);
    b.resize(6, 'A');
    REQUIRE(b.size() == 6);
    REQUIRE(b[3] == '6');
    REQUIRE(b[4] == 'A');
    REQUIRE(b[5] == 'A');
    b.resize(10, 'B');
    REQUIRE(b.size() == 10);
    REQUIRE(b[3] == '6');
    REQUIRE(b[6] == 'B');
    REQUIRE(b[9] == 'B');
    b.resize(3, 'C');
    REQUIRE(b.size() == 3);
    REQUIRE(b.find('C') == -1);
    b.resize(3);
    REQUIRE(b.size() == 3);
    
    // Check if a and b were properly detached.
    REQUIRE(a.size() == 10);
    REQUIRE(a[3] == '3');
    REQUIRE(a[5] == '5');
    
    // Special case that refCount == 1 and d->offset != 0.
    ByteVector c = ByteVector("0123456789").mid(3, 4);
    c.resize(6, 'A');
    REQUIRE(c.size() == 6);
    REQUIRE(c[3] == '6');
    REQUIRE(c[4] == 'A');
    REQUIRE(c[5] == 'A');
    c.resize(10, 'B');
    REQUIRE(c.size() == 10);
    REQUIRE(c[3] == '6');
    REQUIRE(c[6] == 'B');
    REQUIRE(c[9] == 'B');
    c.resize(3, 'C');
    REQUIRE(c.size() == 3);
    REQUIRE(c.find('C') == -1);
  }
  SECTION("Append")
  {
    ByteVector v1("foo");
    v1.append("bar");
    REQUIRE(v1 == "foobar");
    
    ByteVector v2("foo");
    v2.append("b");
    REQUIRE(v2 == "foob");
    
    ByteVector v3;
    v3.append("b");
    REQUIRE(v3 == "b");
    
    ByteVector v4("foo");
    v4.append(v1);
    REQUIRE(v4 == "foofoobar");
    
    ByteVector v5("foo");
    v5.append('b');
    REQUIRE(v5 == "foob");
    
    ByteVector v6;
    v6.append('b');
    REQUIRE(v6 == "b");
    
    ByteVector v7("taglib");
    ByteVector v8 = v7;
    
    v7.append("ABC");
    REQUIRE(v7 == "taglibABC");
    v7.append('1');
    v7.append('2');
    v7.append('3');
    REQUIRE(v7 == "taglibABC123");
    REQUIRE(v8 == "taglib");

    // Append itself.
    ByteVector v9("1234");
    v9.append(v9);
    REQUIRE(v9 == "12341234");
  }
  SECTION("Convert from/to Base64")
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
    REQUIRE(sempty.toBase64() == eempty);
    REQUIRE(s0.toBase64() == e0);
    REQUIRE(s1.toBase64() == e1);
    REQUIRE(s2.toBase64() == e2);
    REQUIRE(s3.toBase64() == e3);
    
    // Decode
    REQUIRE(ByteVector::fromBase64(eempty) == sempty);
    REQUIRE(ByteVector::fromBase64(e0) == s0);
    REQUIRE(ByteVector::fromBase64(e1) == s1);
    REQUIRE(ByteVector::fromBase64(e2) == s2);
    REQUIRE(ByteVector::fromBase64(e3) == s3);
    
    REQUIRE(ByteVector::fromBase64(s0.toBase64()) == t0);
    REQUIRE(ByteVector::fromBase64(s1.toBase64()) == t1);
    REQUIRE(ByteVector::fromBase64(s2.toBase64()) == t2);
    REQUIRE(ByteVector::fromBase64(s3.toBase64()) == t3);
    
    ByteVector all((unsigned int)256);
    
    // in order
    {
      for(int i = 0; i < 256; i++){
        all[i]=(unsigned char)i;
        }
      ByteVector b64 = all.toBase64();
      ByteVector original = ByteVector::fromBase64(b64);
      REQUIRE(original == all);
    }
    
    // reverse
    {
      for(int i = 0; i < 256; i++){
        all[i]=(unsigned char)255-i;
        }
      ByteVector b64 = all.toBase64();
      ByteVector original = ByteVector::fromBase64(b64);
      REQUIRE(original == all);
    }
    
    // all zeroes
    {
      for(int i = 0; i < 256; i++){
        all[i]=0;
        }
      ByteVector b64 = all.toBase64();
      ByteVector original = ByteVector::fromBase64(b64);
      REQUIRE(original == all);
    }
    
    // all ones
    {
      for(int i = 0; i < 256; i++){
        all[i]=(unsigned char)0xff;
        }
      ByteVector b64 = all.toBase64();
      ByteVector original = ByteVector::fromBase64(b64);
      REQUIRE(original == all);
    }
    
    // Missing end bytes
    {
      // No missing bytes
      ByteVector m0("YW55IGNhcm5hbCBwbGVhc3VyZQ==");
      REQUIRE(ByteVector::fromBase64(m0) == s2);
    
      // 1 missing byte
      ByteVector m1("YW55IGNhcm5hbCBwbGVhc3VyZQ=");
      REQUIRE(ByteVector::fromBase64(m1) == sempty);
    
      // 2 missing bytes
      ByteVector m2("YW55IGNhcm5hbCBwbGVhc3VyZQ");
      REQUIRE(ByteVector::fromBase64(m2) == sempty);
    
      // 3 missing bytes
      ByteVector m3("YW55IGNhcm5hbCBwbGVhc3VyZ");
      REQUIRE(ByteVector::fromBase64(m3) == sempty);
    }
    
    // Grok invalid characters
    {
      ByteVector invalid("abd\x00\x01\x02\x03\x04");
      REQUIRE(ByteVector::fromBase64(invalid) == sempty);
    }
  }
}
