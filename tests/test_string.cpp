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

#include <tstring.h>
#include <string.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace TagLib;

class TestString : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestString);
  CPPUNIT_TEST(testString);
  CPPUNIT_TEST(testRfind);
  CPPUNIT_TEST(testUTF16Encode);
  CPPUNIT_TEST(testUTF16Decode);
  CPPUNIT_TEST(testUTF16DecodeInvalidBOM);
  CPPUNIT_TEST(testUTF16DecodeEmptyWithBOM);
  CPPUNIT_TEST(testSurrogatePair);
  CPPUNIT_TEST(testAppendCharDetach);
  CPPUNIT_TEST(testAppendStringDetach);
  CPPUNIT_TEST(testToInt);
  CPPUNIT_TEST(testSubstr);
  CPPUNIT_TEST(testNewline);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST(testIterator);
  CPPUNIT_TEST_SUITE_END();

public:

  void testString()
  {
    String s = "taglib string";
    ByteVector v = "taglib string";
    CPPUNIT_ASSERT(v == s.data(String::Latin1));

    char str[] = "taglib string";
    CPPUNIT_ASSERT(strcmp(s.toCString(), str) == 0);
    CPPUNIT_ASSERT(s == "taglib string");
    CPPUNIT_ASSERT(s != "taglib STRING");
    CPPUNIT_ASSERT(s != "taglib");
    CPPUNIT_ASSERT(s != "taglib string taglib");
    CPPUNIT_ASSERT(s == L"taglib string");
    CPPUNIT_ASSERT(s != L"taglib STRING");
    CPPUNIT_ASSERT(s != L"taglib");
    CPPUNIT_ASSERT(s != L"taglib string taglib");

    String unicode("José Carlos", String::UTF8);
    CPPUNIT_ASSERT(strcmp(unicode.toCString(), "Jos\xe9 Carlos") == 0);

    String latin = "Jos\xe9 Carlos";
    CPPUNIT_ASSERT(strcmp(latin.toCString(true), "José Carlos") == 0);

    String c;
    c = "1";
    CPPUNIT_ASSERT(c == L"1");

    c = L'\u4E00';
    CPPUNIT_ASSERT(c == L"\u4E00");

    String unicode2(unicode.to8Bit(true), String::UTF8);
    CPPUNIT_ASSERT(unicode == unicode2);

    String unicode3(L"\u65E5\u672C\u8A9E");
    CPPUNIT_ASSERT(*(unicode3.toCWString() + 1) == L'\u672C');

    String unicode4(L"\u65e5\u672c\u8a9e", String::UTF16BE);
    CPPUNIT_ASSERT(unicode4[1] == L'\u672c');

    String unicode5(L"\u65e5\u672c\u8a9e", String::UTF16LE);
    CPPUNIT_ASSERT(unicode5[1] == L'\u2c67');

    std::wstring stduni = L"\u65e5\u672c\u8a9e";

    String unicode6(stduni, String::UTF16BE);
    CPPUNIT_ASSERT(unicode6[1] == L'\u672c');

    String unicode7(stduni, String::UTF16LE);
    CPPUNIT_ASSERT(unicode7[1] == L'\u2c67');

    CPPUNIT_ASSERT(strcmp(String::number(0).toCString(), "0") == 0);
    CPPUNIT_ASSERT(strcmp(String::number(12345678).toCString(), "12345678") == 0);
    CPPUNIT_ASSERT(strcmp(String::number(-12345678).toCString(), "-12345678") == 0);

    String n = "123";
    CPPUNIT_ASSERT(n.toInt() == 123);

    n = "-123";
    CPPUNIT_ASSERT(n.toInt() == -123);

    CPPUNIT_ASSERT(String("0").toInt() == 0);
    CPPUNIT_ASSERT(String("1").toInt() == 1);

    CPPUNIT_ASSERT(String("  foo  ").stripWhiteSpace() == String("foo"));
    CPPUNIT_ASSERT(String("foo    ").stripWhiteSpace() == String("foo"));
    CPPUNIT_ASSERT(String("    foo").stripWhiteSpace() == String("foo"));

    CPPUNIT_ASSERT(memcmp(String("foo").data(String::Latin1).data(), "foo", 3) == 0);
    CPPUNIT_ASSERT(memcmp(String("f").data(String::Latin1).data(), "f", 1) == 0);

    // Check to make sure that the BOM is there and that the data size is correct

    const ByteVector utf16 = unicode.data(String::UTF16);
    CPPUNIT_ASSERT(utf16.size() == 2 + (unicode.size() * 2));
    CPPUNIT_ASSERT(unicode == String(utf16, String::UTF16));
  }

  void testUTF16Encode()
  {
    String a("foo");
    ByteVector b("\0f\0o\0o", 6);
    ByteVector c("f\0o\0o\0", 6);
    ByteVector d("\377\376f\0o\0o\0", 8);
    CPPUNIT_ASSERT(a.data(String::UTF16BE) != a.data(String::UTF16LE));
    CPPUNIT_ASSERT(b == a.data(String::UTF16BE));
    CPPUNIT_ASSERT(c == a.data(String::UTF16LE));
    CPPUNIT_ASSERT_EQUAL(d, a.data(String::UTF16));
  }

  void testUTF16Decode()
  {
    String a("foo");
    ByteVector b("\0f\0o\0o", 6);
    ByteVector c("f\0o\0o\0", 6);
    ByteVector d("\377\376f\0o\0o\0", 8);
    CPPUNIT_ASSERT_EQUAL(a, String(b, String::UTF16BE));
    CPPUNIT_ASSERT_EQUAL(a, String(c, String::UTF16LE));
    CPPUNIT_ASSERT_EQUAL(a, String(d, String::UTF16));
  }

  // this test is expected to print "TagLib: String::prepare() -
  // Invalid UTF16 string." on the console 3 times
  void testUTF16DecodeInvalidBOM()
  {
    ByteVector b(" ", 1);
    ByteVector c("  ", 2);
    ByteVector d("  \0f\0o\0o", 8);
    CPPUNIT_ASSERT_EQUAL(String(), String(b, String::UTF16));
    CPPUNIT_ASSERT_EQUAL(String(), String(c, String::UTF16));
    CPPUNIT_ASSERT_EQUAL(String(), String(d, String::UTF16));
  }

  void testUTF16DecodeEmptyWithBOM()
  {
    ByteVector a("\377\376", 2);
    ByteVector b("\376\377", 2);
    CPPUNIT_ASSERT_EQUAL(String(), String(a, String::UTF16));
    CPPUNIT_ASSERT_EQUAL(String(), String(b, String::UTF16));
  }

  void testSurrogatePair()
  {
    // Make sure that a surrogate pair is converted into single UTF-8 char
    // and vice versa.

    const ByteVector v1("\xff\xfe\x42\xd8\xb7\xdf\xce\x91\x4b\x5c");
    const ByteVector v2("\xf0\xa0\xae\xb7\xe9\x87\x8e\xe5\xb1\x8b");

    const String s1(v1, String::UTF16);
    CPPUNIT_ASSERT_EQUAL(s1.data(String::UTF8), v2);

    const String s2(v2, String::UTF8);
    CPPUNIT_ASSERT_EQUAL(s2.data(String::UTF16), v1);
  }

  void testAppendStringDetach()
  {
    String a("a");
    String b = a;
    a += "b";
    CPPUNIT_ASSERT_EQUAL(String("ab"), a);
    CPPUNIT_ASSERT_EQUAL(String("a"), b);
  }

  void testAppendCharDetach()
  {
    String a("a");
    String b = a;
    a += 'b';
    CPPUNIT_ASSERT_EQUAL(String("ab"), a);
    CPPUNIT_ASSERT_EQUAL(String("a"), b);
  }

  void testRfind()
  {
    CPPUNIT_ASSERT_EQUAL(-1, String("foo.bar").rfind(".", 0));
    CPPUNIT_ASSERT_EQUAL(-1, String("foo.bar").rfind(".", 1));
    CPPUNIT_ASSERT_EQUAL(-1, String("foo.bar").rfind(".", 2));
    CPPUNIT_ASSERT_EQUAL(3, String("foo.bar").rfind(".", 3));
    CPPUNIT_ASSERT_EQUAL(3, String("foo.bar").rfind(".", 4));
    CPPUNIT_ASSERT_EQUAL(3, String("foo.bar").rfind(".", 5));
    CPPUNIT_ASSERT_EQUAL(3, String("foo.bar").rfind(".", 6));
    CPPUNIT_ASSERT_EQUAL(3, String("foo.bar").rfind(".", 7));
    CPPUNIT_ASSERT_EQUAL(3, String("foo.bar").rfind("."));
  }

  void testToInt()
  {
    bool ok;
    CPPUNIT_ASSERT_EQUAL(String("123").toInt(&ok), 123);
    CPPUNIT_ASSERT_EQUAL(ok, true);

    CPPUNIT_ASSERT_EQUAL(String("-123").toInt(&ok), -123);
    CPPUNIT_ASSERT_EQUAL(ok, true);

    CPPUNIT_ASSERT_EQUAL(String("abc").toInt(&ok), 0);
    CPPUNIT_ASSERT_EQUAL(ok, false);

    CPPUNIT_ASSERT_EQUAL(String("1x").toInt(&ok), 1);
    CPPUNIT_ASSERT_EQUAL(ok, false);

    CPPUNIT_ASSERT_EQUAL(String("").toInt(&ok), 0);
    CPPUNIT_ASSERT_EQUAL(ok, false);

    CPPUNIT_ASSERT_EQUAL(String("-").toInt(&ok), 0);
    CPPUNIT_ASSERT_EQUAL(ok, false);

    CPPUNIT_ASSERT_EQUAL(String("123").toInt(), 123);
    CPPUNIT_ASSERT_EQUAL(String("-123").toInt(), -123);
    CPPUNIT_ASSERT_EQUAL(String("123aa").toInt(), 123);
    CPPUNIT_ASSERT_EQUAL(String("-123aa").toInt(), -123);
  }

  void testSubstr()
  {
    CPPUNIT_ASSERT_EQUAL(String("01"), String("0123456").substr(0, 2));
    CPPUNIT_ASSERT_EQUAL(String("12"), String("0123456").substr(1, 2));
    CPPUNIT_ASSERT_EQUAL(String("123456"), String("0123456").substr(1, 200));
  }

  void testNewline()
  {
    ByteVector cr("abc\x0dxyz", 7);
    ByteVector lf("abc\x0axyz", 7);
    ByteVector crlf("abc\x0d\x0axyz", 8);

    CPPUNIT_ASSERT_EQUAL(uint(7), String(cr).size());
    CPPUNIT_ASSERT_EQUAL(uint(7), String(lf).size());
    CPPUNIT_ASSERT_EQUAL(uint(8), String(crlf).size());

    CPPUNIT_ASSERT_EQUAL(L'\x0d', String(cr)[3]);
    CPPUNIT_ASSERT_EQUAL(L'\x0a', String(lf)[3]);
    CPPUNIT_ASSERT_EQUAL(L'\x0d', String(crlf)[3]);
    CPPUNIT_ASSERT_EQUAL(L'\x0a', String(crlf)[4]);
  }

  void testEncode()
  {
    String jpn(L"\u65E5\u672C\u8A9E");
    ByteVector jpn1 = jpn.data(String::Latin1);
    ByteVector jpn2 = jpn.data(String::UTF8);
    ByteVector jpn3 = jpn.data(String::UTF16);
    ByteVector jpn4 = jpn.data(String::UTF16LE);
    ByteVector jpn5 = jpn.data(String::UTF16BE);
    std::string jpn6 = jpn.to8Bit(false);
    std::string jpn7 = jpn.to8Bit(true);

    CPPUNIT_ASSERT_EQUAL(ByteVector("\xE5\x2C\x9E"), jpn1);
    CPPUNIT_ASSERT_EQUAL(ByteVector("\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"), jpn2);
    CPPUNIT_ASSERT_EQUAL(ByteVector("\xFF\xFE\xE5\x65\x2C\x67\x9E\x8A"), jpn3);
    CPPUNIT_ASSERT_EQUAL(ByteVector("\xE5\x65\x2C\x67\x9E\x8A"), jpn4);
    CPPUNIT_ASSERT_EQUAL(ByteVector("\x65\xE5\x67\x2C\x8A\x9E"), jpn5);
    CPPUNIT_ASSERT_EQUAL(std::string("\xE5\x2C\x9E"), jpn6);
    CPPUNIT_ASSERT_EQUAL(std::string("\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"), jpn7);

    String empty;
    ByteVector empty1 = empty.data(String::Latin1);
    ByteVector empty2 = empty.data(String::UTF8);
    ByteVector empty3 = empty.data(String::UTF16);
    ByteVector empty4 = empty.data(String::UTF16LE);
    ByteVector empty5 = empty.data(String::UTF16BE);
    std::string empty6 = empty.to8Bit(false);
    std::string empty7 = empty.to8Bit(true);

    CPPUNIT_ASSERT(empty1.isEmpty());
    CPPUNIT_ASSERT(empty2.isEmpty());
    CPPUNIT_ASSERT_EQUAL(ByteVector("\xFF\xFE"), empty3);
    CPPUNIT_ASSERT(empty4.isEmpty());
    CPPUNIT_ASSERT(empty5.isEmpty());
    CPPUNIT_ASSERT(empty6.empty());
    CPPUNIT_ASSERT(empty7.empty());
  }

  void testIterator()
  {
    String s1 = "taglib string";
    String s2 = s1;

    String::Iterator it1 = s1.begin();
    String::Iterator it2 = s2.begin();

    CPPUNIT_ASSERT_EQUAL(L't', *it1);
    CPPUNIT_ASSERT_EQUAL(L't', *it2);

    std::advance(it1, 4);
    std::advance(it2, 4);
    *it2 = L'I';
    CPPUNIT_ASSERT_EQUAL(L'i', *it1);
    CPPUNIT_ASSERT_EQUAL(L'I', *it2);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestString);
