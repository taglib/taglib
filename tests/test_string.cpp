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
#include <tstring.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("String")
{
  SECTION("Assign and detach")
  {
    String s1("a");
    String s2 = s1;
    s2 = '1';
    REQUIRE(s1 == "a");
    REQUIRE(s2 == "1");

    s2 = L'\u4E00';
    REQUIRE(s1 == "a");
    REQUIRE(s2 == L"\u4E00");

    s1 = "foo";
    REQUIRE(s1 == "foo");
    REQUIRE(s2 == L"\u4E00");

    s1 = L"\u65E5\u672C\u8A9E";
    REQUIRE(s1 == L"\u65E5\u672C\u8A9E");
    REQUIRE(s2 == L"\u4E00");
  }
  SECTION("Comparison")
  {
    const String s("taglib string");
    REQUIRE(s == "taglib string");
    REQUIRE_FALSE(s == "taglib STRING");
    REQUIRE_FALSE(s == "taglib");
    REQUIRE_FALSE(s == "taglib string taglib");
    REQUIRE(s == L"taglib string");
    REQUIRE_FALSE(s == L"taglib STRING");
    REQUIRE_FALSE(s == L"taglib");
    REQUIRE_FALSE(s == L"taglib string taglib");

    REQUIRE(strcmp(s.toCString(), "taglib string") == 0);
    REQUIRE(wcscmp(s.toCWString(), L"taglib string") == 0);
    REQUIRE(s.to8Bit(true) == "taglib string");
    REQUIRE(s.toWString() == L"taglib string");
  }
  SECTION("Clear and empty")
  {
    REQUIRE(String().isEmpty());
    REQUIRE(String().size() == 0);
    REQUIRE(String("asdf").clear().isEmpty());
    REQUIRE(String("asdf").clear().size() == 0);
    REQUIRE(String("asdf").clear() == String());
    REQUIRE(String("asdf").clear() == ByteVector());

    String s("taglib string");
    s.clear();
    REQUIRE(s.isEmpty());
    REQUIRE_FALSE(s.isNull()); // deprecated, but still worth it to check.
  }
  SECTION("RFind")
  {
    REQUIRE(String("foo.bar").rfind(".", 0) == -1);
    REQUIRE(String("foo.bar").rfind(".", 1) == -1);
    REQUIRE(String("foo.bar").rfind(".", 2) == -1);
    REQUIRE(String("foo.bar").rfind(".", 3) == 3);
    REQUIRE(String("foo.bar").rfind(".", 4) == 3);
    REQUIRE(String("foo.bar").rfind(".", 5) == 3);
    REQUIRE(String("foo.bar").rfind(".", 6) == 3);
    REQUIRE(String("foo.bar").rfind(".", 7) == 3);
    REQUIRE(String("foo.bar").rfind(".") == 3);
  }
  SECTION("Strip whitespaces")
  {
    REQUIRE(String("  foo  ").stripWhiteSpace() == "foo");
    REQUIRE(String("foo    ").stripWhiteSpace() == "foo");
    REQUIRE(String("    foo").stripWhiteSpace() == "foo");
    REQUIRE(String("foo").stripWhiteSpace() == "foo");
    REQUIRE(String("f o o").stripWhiteSpace() == "f o o");
    REQUIRE(String(" f o o ").stripWhiteSpace() == "f o o");
  }
  SECTION("Encode latin-1 string")
  {
    String latin1("foo");
    REQUIRE(latin1.data(String::Latin1) == "foo");
    REQUIRE(latin1.data(String::UTF8) == "foo");
    REQUIRE(latin1.data(String::UTF16) == ByteVector("\xFF\xFE" "f\0o\0o\0", 8));
    REQUIRE(latin1.data(String::UTF16LE) == ByteVector("f\0o\0o\0", 6));
    REQUIRE(latin1.data(String::UTF16BE) == ByteVector("\0f\0o\0o", 6));
    REQUIRE(latin1.to8Bit(false) == "foo");
    REQUIRE(latin1.to8Bit(true) == "foo");
  }
  SECTION("Encode non-latin1 string")
  {
    const String jpn(L"\u65E5\u672C\u8A9E");
    REQUIRE(jpn.data(String::Latin1) == "\xE5\x2C\x9E");
    REQUIRE(jpn.data(String::UTF8) == "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E");
    REQUIRE(jpn.data(String::UTF16) == "\xFF\xFE\xE5\x65\x2C\x67\x9E\x8A");
    REQUIRE(jpn.data(String::UTF16LE) == "\xE5\x65\x2C\x67\x9E\x8A");
    REQUIRE(jpn.data(String::UTF16BE) == "\x65\xE5\x67\x2C\x8A\x9E");
    REQUIRE(jpn.to8Bit(false) == "\xE5\x2C\x9E");
    REQUIRE(jpn.to8Bit(true) == "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E");
  }
  SECTION("Encode non-BMP string")
  {
    const ByteVector a("\xFF\xFE\x3C\xD8\x50\xDD\x40\xD8\xF5\xDC\x3C\xD8\x00\xDE", 14);
    const ByteVector b("\xF0\x9F\x85\x90\xF0\xA0\x83\xB5\xF0\x9F\x88\x80");
    REQUIRE(String(a, String::UTF16).data(String::UTF8) == b);
  }
  SECTION("Encode empty string")
  {
    const String empty;
    REQUIRE(empty.data(String::Latin1).isEmpty());
    REQUIRE(empty.data(String::UTF8).isEmpty());
    REQUIRE(empty.data(String::UTF16) == "\xFF\xFE");
    REQUIRE(empty.data(String::UTF16LE).isEmpty());
    REQUIRE(empty.data(String::UTF16BE).isEmpty());
    REQUIRE(empty.to8Bit(false).empty());
    REQUIRE(empty.to8Bit(true).empty());
  }
  SECTION("Decode UTF-16 strings")
  {
    const String a("foo");
    REQUIRE(String(ByteVector("\0f\0o\0o", 6), String::UTF16BE) == a);
    REQUIRE(String(ByteVector("f\0o\0o\0", 6), String::UTF16LE) == a);
    REQUIRE(String(ByteVector("\377\376f\0o\0o\0", 8), String::UTF16) == a);

    // Invalid BOMs
    REQUIRE(String(ByteVector("\xfe", 1), String::UTF16).isEmpty());
    REQUIRE(String(ByteVector("\xfe\xfe", 2), String::UTF16).isEmpty());
    REQUIRE(String(ByteVector("  \0f\0o\0o", 8), String::UTF16).isEmpty());

    // Empty strings with valid BOMs
    REQUIRE(String(ByteVector("\xff\xfe", 2), String::UTF16).isEmpty());
    REQUIRE(String(ByteVector("\xfe\xff", 2), String::UTF16).isEmpty());
  }
  SECTION("Decode illegal UTF-8 sequences")
  {
    REQUIRE(String(ByteVector("\x2F"), String::UTF8) == String("/"));
    REQUIRE(String(ByteVector("\xC0\xAF"), String::UTF8).isEmpty());
    REQUIRE(String(ByteVector("\xE0\x80\xAF"), String::UTF8).isEmpty());
    REQUIRE(String(ByteVector("\xF0\x80\x80\xAF"), String::UTF8).isEmpty());

    REQUIRE(String(ByteVector("\xF8\x80\x80\x80\x80"), String::UTF8).isEmpty());
    REQUIRE(String(ByteVector("\xFC\x80\x80\x80\x80\x80"), String::UTF8).isEmpty());

    REQUIRE(String(ByteVector("\xC2"), String::UTF8).isEmpty());
    REQUIRE(String(ByteVector("\xE0\x80"), String::UTF8).isEmpty());
    REQUIRE(String(ByteVector("\xF0\x80\x80"), String::UTF8).isEmpty());
    REQUIRE(String(ByteVector("\xF8\x80\x80\x80"), String::UTF8).isEmpty());
    REQUIRE(String(ByteVector("\xFC\x80\x80\x80\x80"), String::UTF8).isEmpty());

    REQUIRE(String('\x80', String::UTF8).isEmpty());

    REQUIRE(String(ByteVector("\xED\xA0\x80\xED\xB0\x80"), String::UTF8).isEmpty());
    REQUIRE(String(ByteVector("\xED\xB0\x80\xED\xA0\x80"), String::UTF8).isEmpty());
  }
  SECTION("Surrogate Pair")
  {
    // Make sure that a surrogate pair is converted into single UTF-8 char
    // and vice versa.
    const ByteVector v1("\xff\xfe\x42\xd8\xb7\xdf\xce\x91\x4b\x5c");
    const ByteVector v2("\xf0\xa0\xae\xb7\xe9\x87\x8e\xe5\xb1\x8b");

    const String s1(v1, String::UTF16);
    REQUIRE(s1.data(String::UTF8) == v2);

    const String s2(v2, String::UTF8);
    REQUIRE(s2.data(String::UTF16) == v1);

    const ByteVector v3("\xfe\xff\xd8\x01\x30\x42");
    REQUIRE(String(v3, String::UTF16).data(String::UTF8).isEmpty());

    const ByteVector v4("\xfe\xff\x30\x42\xdc\x01");
    REQUIRE(String(v4, String::UTF16).data(String::UTF8).isEmpty());

    const ByteVector v5("\xfe\xff\xdc\x01\xd8\x01");
    REQUIRE(String(v5, String::UTF16).data(String::UTF8).isEmpty());
  }
  SECTION("Append string and detach")
  {
    String a("a");
    String b = a;
    a += "b";
    REQUIRE(a == "ab");
    REQUIRE(b == "a");
  }
  SECTION("Append char and detach")
  {
    String a("a");
    String b = a;
    a += 'b';
    REQUIRE(a == "ab");
    REQUIRE(b == "a");
  }
  SECTION("Convert to integer")
  {
    bool ok;
    REQUIRE(String("123").toInt(&ok) == 123);
    REQUIRE(ok);

    REQUIRE(String("123aa").toInt(&ok) == 123);
    REQUIRE_FALSE(ok);

    REQUIRE(String("aa123").toInt(&ok) == 0);
    REQUIRE_FALSE(ok);

    REQUIRE(String("-123").toInt(&ok) == -123);
    REQUIRE(ok);

    REQUIRE(String("-123aa").toInt(&ok) == -123);
    REQUIRE_FALSE(ok);

    REQUIRE(String("aa-123").toInt(&ok) == 0);
    REQUIRE_FALSE(ok);

    REQUIRE(String("").toInt(&ok) == 0);
    REQUIRE_FALSE(ok);

    REQUIRE(String("-").toInt(&ok) == 0);
    REQUIRE_FALSE(ok);

    REQUIRE(String("0000").toInt() == 0);
    REQUIRE(String("0001").toInt() == 1);

    String("2147483648").toInt(&ok);
    REQUIRE_FALSE(ok);

    String("-2147483649").toInt(&ok);
    REQUIRE_FALSE(ok);
  }
  SECTION("Convert from integer")
  {
      REQUIRE(String::number(0) == "0");
      REQUIRE(String::number(12345678) == "12345678");
      REQUIRE(String::number(-12345678) == "-12345678");
  }
  SECTION("Substring")
  {
    REQUIRE(String("0123456").substr(0, 2) == "01");
    REQUIRE(String("0123456").substr(1, 2) == "12");
    REQUIRE(String("0123456").substr(1, 200) == "123456");
    REQUIRE(String("0123456").substr(1) == "123456");
    REQUIRE(String("0123456").substr(0, 7) == "0123456");
    REQUIRE(String("0123456").substr(0, 200) == "0123456");
    REQUIRE(String("0123456").substr(0) == "0123456");
  }
  SECTION("New line")
  {
    ByteVector cr("abc\x0dxyz", 7);
    ByteVector lf("abc\x0axyz", 7);
    ByteVector crlf("abc\x0d\x0axyz", 8);

    REQUIRE(String(cr).size() == 7);
    REQUIRE(String(lf).size() == 7);
    REQUIRE(String(crlf).size() == 8);

    REQUIRE(String(cr)[3] == L'\x0d');
    REQUIRE(String(lf)[3] == L'\x0a');
    REQUIRE(String(crlf)[3] == L'\x0d');
    REQUIRE(String(crlf)[4] == L'\x0a');
  }
  SECTION("Make upper case and detach")
  {
    String s1 = "tagLIB 012 strING";
    String s2 = s1.upper();
    REQUIRE(s1 == "tagLIB 012 strING");
    REQUIRE(s2 == "TAGLIB 012 STRING");
  }
  SECTION("Iterator")
  {
    String s1 = "taglib string";
    String s2 = s1;

    String::Iterator it1 = s1.begin();
    String::Iterator it2 = s2.begin();

    REQUIRE(*it1 == L't');
    REQUIRE(*it2 == L't');

    std::advance(it1, 4);
    std::advance(it2, 4);
    *it2 = L'I';
    REQUIRE(*it1 == L'i');
    REQUIRE(*it2 == L'I');
  }
}
