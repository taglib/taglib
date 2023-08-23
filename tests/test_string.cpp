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

#include "tstring.h"
#include <cstring>
#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

TEST(String, testString)
{
  String s     = "taglib string";
  ByteVector v = "taglib string";
  ASSERT_EQ(v, s.data(String::Latin1));

  char str[] = "taglib string";
  ASSERT_STREQ(str, s.toCString());
  ASSERT_EQ(String("taglib string"), s);
  ASSERT_NE(String("taglib STRING"), s);
  ASSERT_NE(String("taglib"), s);
  ASSERT_NE(String("taglib string taglib"), s);
  ASSERT_EQ(String(L"taglib string"), s);
  ASSERT_NE(String(L"taglib STRING"), s);
  ASSERT_NE(String(L"taglib"), s);
  ASSERT_NE(String(L"taglib string taglib"), s);

  s.clear();
  ASSERT_TRUE(s.isEmpty());

  String unicode("José Carlos", String::UTF8);
  ASSERT_STREQ("Jos\xe9 Carlos", unicode.toCString());

  String latin = "Jos\xe9 Carlos";
  ASSERT_STREQ("José Carlos", latin.toCString(true));

  String c;
  c = "1";
  ASSERT_EQ(String(L"1"), c);

  c = L'\u4E00';
  ASSERT_EQ(String(L"\u4E00"), c);

  String unicode2(unicode.to8Bit(true), String::UTF8);
  ASSERT_EQ(unicode, unicode2);

  String unicode3(L"\u65E5\u672C\u8A9E");
  ASSERT_EQ(L'\u672C', *(unicode3.toCWString() + 1));

  String unicode4(L"\u65e5\u672c\u8a9e", String::UTF16BE);
  ASSERT_EQ(L'\u672c', unicode4[1]);

  String unicode5(L"\u65e5\u672c\u8a9e", String::UTF16LE);
  ASSERT_EQ(L'\u2c67', unicode5[1]);

  std::wstring stduni = L"\u65e5\u672c\u8a9e";

  String unicode6(stduni, String::UTF16BE);
  ASSERT_EQ(L'\u672c', unicode6[1]);

  String unicode7(stduni, String::UTF16LE);
  ASSERT_EQ(L'\u2c67', unicode7[1]);

  ASSERT_EQ(String("foo"), String("  foo  ").stripWhiteSpace());
  ASSERT_EQ(String("foo"), String("foo    ").stripWhiteSpace());
  ASSERT_EQ(String("foo"), String("    foo").stripWhiteSpace());
  ASSERT_EQ(String("foo"), String("foo").stripWhiteSpace());
  ASSERT_EQ(String("f o o"), String("f o o").stripWhiteSpace());
  ASSERT_EQ(String("f o o"), String(" f o o ").stripWhiteSpace());

  ASSERT_EQ(0, memcmp(String("foo").data(String::Latin1).data(), "foo", 3));
  ASSERT_EQ(0, memcmp(String("f").data(String::Latin1).data(), "f", 1));
}

TEST(String, testUTF16Encode)
{
  String a("foo");
  ByteVector b("\0f\0o\0o", 6);
  ByteVector c("f\0o\0o\0", 6);
  ByteVector d("\377\376f\0o\0o\0", 8);
  ASSERT_NE(a.data(String::UTF16BE), a.data(String::UTF16LE));
  ASSERT_EQ(b, a.data(String::UTF16BE));
  ASSERT_EQ(c, a.data(String::UTF16LE));
  ASSERT_EQ(d, a.data(String::UTF16));
}

TEST(String, testUTF16Decode)
{
  String a("foo");
  ByteVector b("\0f\0o\0o", 6);
  ByteVector c("f\0o\0o\0", 6);
  ByteVector d("\377\376f\0o\0o\0", 8);
  ASSERT_EQ(a, String(b, String::UTF16BE));
  ASSERT_EQ(a, String(c, String::UTF16LE));
  ASSERT_EQ(a, String(d, String::UTF16));
}

// this test is expected to print "TagLib: String::prepare() -
// Invalid UTF16 string." on the console 3 times
TEST(String, testUTF16DecodeInvalidBOM)
{
  ByteVector b(" ", 1);
  ByteVector c("  ", 2);
  ByteVector d("  \0f\0o\0o", 8);
  ASSERT_EQ(String(), String(b, String::UTF16));
  ASSERT_EQ(String(), String(c, String::UTF16));
  ASSERT_EQ(String(), String(d, String::UTF16));
}

TEST(String, testUTF16DecodeEmptyWithBOM)
{
  ByteVector a("\377\376", 2);
  ByteVector b("\376\377", 2);
  ASSERT_EQ(String(), String(a, String::UTF16));
  ASSERT_EQ(String(), String(b, String::UTF16));
}

TEST(String, testSurrogatePair)
{
  // Make sure that a surrogate pair is converted into single UTF-8 char
  // and vice versa.

  const ByteVector v1("\xff\xfe\x42\xd8\xb7\xdf\xce\x91\x4b\x5c");
  const ByteVector v2("\xf0\xa0\xae\xb7\xe9\x87\x8e\xe5\xb1\x8b");

  const String s1(v1, String::UTF16);
  ASSERT_EQ(s1.data(String::UTF8), v2);

  const String s2(v2, String::UTF8);
  ASSERT_EQ(s2.data(String::UTF16), v1);

  const ByteVector v3("\xfe\xff\xd8\x01\x30\x42");
  ASSERT_TRUE(String(v3, String::UTF16).data(String::UTF8).isEmpty());

  const ByteVector v4("\xfe\xff\x30\x42\xdc\x01");
  ASSERT_TRUE(String(v4, String::UTF16).data(String::UTF8).isEmpty());

  const ByteVector v5("\xfe\xff\xdc\x01\xd8\x01");
  ASSERT_TRUE(String(v5, String::UTF16).data(String::UTF8).isEmpty());
}

TEST(String, testAppendStringDetach)
{
  String a("a");
  String b = a;
  a += "b";
  ASSERT_EQ(String("ab"), a);
  ASSERT_EQ(String("a"), b);
}

TEST(String, testAppendCharDetach)
{
  String a("a");
  String b = a;
  a += 'b';
  ASSERT_EQ(String("ab"), a);
  ASSERT_EQ(String("a"), b);
}

TEST(String, testRfind)
{
  ASSERT_EQ(-1, String("foo.bar").rfind(".", 0));
  ASSERT_EQ(-1, String("foo.bar").rfind(".", 1));
  ASSERT_EQ(-1, String("foo.bar").rfind(".", 2));
  ASSERT_EQ(3, String("foo.bar").rfind(".", 3));
  ASSERT_EQ(3, String("foo.bar").rfind(".", 4));
  ASSERT_EQ(3, String("foo.bar").rfind(".", 5));
  ASSERT_EQ(3, String("foo.bar").rfind(".", 6));
  ASSERT_EQ(3, String("foo.bar").rfind(".", 7));
  ASSERT_EQ(3, String("foo.bar").rfind("."));
}

TEST(String, testToInt)
{
  bool ok;
  ASSERT_EQ(String("123").toInt(&ok), 123);
  ASSERT_EQ(ok, true);

  ASSERT_EQ(String("-123").toInt(&ok), -123);
  ASSERT_EQ(ok, true);

  ASSERT_EQ(String("abc").toInt(&ok), 0);
  ASSERT_EQ(ok, false);

  ASSERT_EQ(String("1x").toInt(&ok), 1);
  ASSERT_EQ(ok, false);

  ASSERT_EQ(String("").toInt(&ok), 0);
  ASSERT_EQ(ok, false);

  ASSERT_EQ(String("-").toInt(&ok), 0);
  ASSERT_EQ(ok, false);

  ASSERT_EQ(String("123").toInt(), 123);
  ASSERT_EQ(String("-123").toInt(), -123);
  ASSERT_EQ(String("123aa").toInt(), 123);
  ASSERT_EQ(String("-123aa").toInt(), -123);

  ASSERT_EQ(String("0000").toInt(), 0);
  ASSERT_EQ(String("0001").toInt(), 1);

  String("2147483648").toInt(&ok);
  ASSERT_EQ(ok, false);

  String("-2147483649").toInt(&ok);
  ASSERT_EQ(ok, false);
}

TEST(String, testFromInt)
{
  ASSERT_EQ(String::number(0), String("0"));
  ASSERT_EQ(String::number(12345678), String("12345678"));
  ASSERT_EQ(String::number(-12345678), String("-12345678"));
}

TEST(String, testSubstr)
{
  ASSERT_EQ(String("01"), String("0123456").substr(0, 2));
  ASSERT_EQ(String("12"), String("0123456").substr(1, 2));
  ASSERT_EQ(String("123456"), String("0123456").substr(1, 200));
  ASSERT_EQ(String("0123456"), String("0123456").substr(0, 7));
  ASSERT_EQ(String("0123456"), String("0123456").substr(0, 200));
}

TEST(String, testNewline)
{
  ByteVector cr("abc\x0dxyz", 7);
  ByteVector lf("abc\x0axyz", 7);
  ByteVector crlf("abc\x0d\x0axyz", 8);

  ASSERT_EQ(static_cast<unsigned int>(7), String(cr).size());
  ASSERT_EQ(static_cast<unsigned int>(7), String(lf).size());
  ASSERT_EQ(static_cast<unsigned int>(8), String(crlf).size());

  ASSERT_EQ(L'\x0d', String(cr)[3]);
  ASSERT_EQ(L'\x0a', String(lf)[3]);
  ASSERT_EQ(L'\x0d', String(crlf)[3]);
  ASSERT_EQ(L'\x0a', String(crlf)[4]);
}

TEST(String, testUpper)
{
  String s1 = "tagLIB 012 strING";
  String s2 = s1.upper();
  ASSERT_EQ(String("tagLIB 012 strING"), s1);
  ASSERT_EQ(String("TAGLIB 012 STRING"), s2);
}

TEST(String, testEncodeNonLatin1)
{
  const String jpn(L"\u65E5\u672C\u8A9E");
  ASSERT_EQ(ByteVector("\xE5\x2C\x9E"), jpn.data(String::Latin1));
  ASSERT_EQ(ByteVector("\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"), jpn.data(String::UTF8));
  ASSERT_EQ(ByteVector("\xFF\xFE\xE5\x65\x2C\x67\x9E\x8A"), jpn.data(String::UTF16));
  ASSERT_EQ(ByteVector("\xE5\x65\x2C\x67\x9E\x8A"), jpn.data(String::UTF16LE));
  ASSERT_EQ(ByteVector("\x65\xE5\x67\x2C\x8A\x9E"), jpn.data(String::UTF16BE));
  ASSERT_EQ(std::string("\xE5\x2C\x9E"), jpn.to8Bit(false));
  ASSERT_EQ(std::string("\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"), jpn.to8Bit(true));
}

TEST(String, testEncodeEmpty)
{
  const String empty;
  ASSERT_TRUE(empty.data(String::Latin1).isEmpty());
  ASSERT_TRUE(empty.data(String::UTF8).isEmpty());
  ASSERT_EQ(ByteVector("\xFF\xFE"), empty.data(String::UTF16));
  ASSERT_TRUE(empty.data(String::UTF16LE).isEmpty());
  ASSERT_TRUE(empty.data(String::UTF16BE).isEmpty());
  ASSERT_TRUE(empty.to8Bit(false).empty());
  ASSERT_TRUE(empty.to8Bit(true).empty());
}

TEST(String, testEncodeNonBMP)
{
  const ByteVector a("\xFF\xFE\x3C\xD8\x50\xDD\x40\xD8\xF5\xDC\x3C\xD8\x00\xDE", 14);
  const ByteVector b("\xF0\x9F\x85\x90\xF0\xA0\x83\xB5\xF0\x9F\x88\x80");
  ASSERT_EQ(b, String(a, String::UTF16).data(String::UTF8));
}

TEST(String, testIterator)
{
  String s1            = "taglib string";
  String s2            = s1;

  String::Iterator it1 = s1.begin();
  String::Iterator it2 = s2.begin();

  ASSERT_EQ(L't', *it1);
  ASSERT_EQ(L't', *it2);

  std::advance(it1, 4);
  std::advance(it2, 4);
  *it2 = L'I';
  ASSERT_EQ(L'i', *it1);
  ASSERT_EQ(L'I', *it2);
}

TEST(String, testInvalidUTF8)
{
  ASSERT_EQ(String("/"), String(ByteVector("\x2F"), String::UTF8));
  ASSERT_TRUE(String(ByteVector("\xC0\xAF"), String::UTF8).isEmpty());
  ASSERT_TRUE(String(ByteVector("\xE0\x80\xAF"), String::UTF8).isEmpty());
  ASSERT_TRUE(String(ByteVector("\xF0\x80\x80\xAF"), String::UTF8).isEmpty());

  ASSERT_TRUE(String(ByteVector("\xF8\x80\x80\x80\x80"), String::UTF8).isEmpty());
  ASSERT_TRUE(String(ByteVector("\xFC\x80\x80\x80\x80\x80"), String::UTF8).isEmpty());

  ASSERT_TRUE(String(ByteVector("\xC2"), String::UTF8).isEmpty());
  ASSERT_TRUE(String(ByteVector("\xE0\x80"), String::UTF8).isEmpty());
  ASSERT_TRUE(String(ByteVector("\xF0\x80\x80"), String::UTF8).isEmpty());
  ASSERT_TRUE(String(ByteVector("\xF8\x80\x80\x80"), String::UTF8).isEmpty());
  ASSERT_TRUE(String(ByteVector("\xFC\x80\x80\x80\x80"), String::UTF8).isEmpty());

  ASSERT_TRUE(String('\x80', String::UTF8).isEmpty());

  ASSERT_TRUE(String(ByteVector("\xED\xA0\x80\xED\xB0\x80"), String::UTF8).isEmpty());
  ASSERT_TRUE(String(ByteVector("\xED\xB0\x80\xED\xA0\x80"), String::UTF8).isEmpty());
}
