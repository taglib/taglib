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

#include <iostream>

#include <tbytevector.h>
#include <tbytevectorlist.h>
#include <tstring.h>
#include <tlist.h>
#include <tdebug.h>

#include <id3v2synchdata.h>

using namespace TagLib;
using namespace std;

void testString();
void testByteVector();
void testSynchData();
void testList();

static int resultCount = 1;

int main()
{
  testString();
  resultCount = 1;
  testByteVector();
  resultCount = 1;
  testSynchData();
  resultCount = 1;
  testList();

  return 0;
}

void printResult(bool result)
{
  if(result)
    cout << "(" << resultCount << ")\tpass" << endl;
  else
    cout << "(" << resultCount << ")\tFAIL" << endl;

  resultCount++;
}

void testString()
{
  cout << "*** Testing TagLib::String ***" << endl;

  String s = "taglib string";
  ByteVector v = "taglib string";
  printResult(v == s.data(String::Latin1));

  char str[] = "taglib string";
  printResult(strcmp(s.toCString(), str) == 0);

  String unicode("JosÃ© Carlos", String::UTF8);
  printResult(strcmp(unicode.toCString(), "José Carlos") == 0);

  String latin = "José Carlos";
  printResult(strcmp(latin.toCString(true), "JosÃ© Carlos") == 0);

  String unicode2(unicode.to8Bit(true), String::UTF8);
  printResult(unicode == unicode2);
  
  printResult(strcmp(String::number(0).toCString(), "0") == 0);
  printResult(strcmp(String::number(12345678).toCString(), "12345678") == 0);
  printResult(strcmp(String::number(-12345678).toCString(), "-12345678") == 0);

  String n = "123";
  printResult(n.toInt() == 123);

  n = "-123";
  printResult(n.toInt() == -123);

  printResult(String("0").toInt() == 0);
  printResult(String("1").toInt() == 1);

  printResult(String("  foo  ").stripWhiteSpace() == String("foo"));
  printResult(String("foo    ").stripWhiteSpace() == String("foo"));
  printResult(String("    foo").stripWhiteSpace() == String("foo"));

  printResult(memcmp(String("foo").data(String::Latin1).data(), "foo", 3) == 0);
  printResult(memcmp(String("f").data(String::Latin1).data(), "f", 1) == 0);

  ByteVector utf16 = unicode.data(String::UTF16);

  // Check to make sure that the BOM is there and that the data size is correct

  printResult(utf16.size() == 2 + (unicode.size() * 2));

  printResult(unicode == String(utf16, String::UTF16));
}

void testConversion(unsigned int i, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
  ByteVector v(4, 0);

  v[3] = a;
  v[2] = b;
  v[1] = c;
  v[0] = d;
  printResult(v.toUInt(false) == i);

  v[0] = a;
  v[1] = b;
  v[2] = c;
  v[3] = d;
  printResult(v.toUInt() == i);
}


void testByteVector()
{
  cout << "*** Testing TagLib::ByteVector ***" << endl;
  ByteVector v("foobar");

  printResult(v.find("ob") == 2);
  printResult(v.find('b') == 3);

  ByteVector n(4, 0);
  n[0] = 1;
  printResult(n.toUInt(true) == 16777216);
  printResult(n.toUInt(false) == 1);
  printResult(ByteVector::fromUInt(16777216, true) == n);
  printResult(ByteVector::fromUInt(1, false) == n);

  printResult(ByteVector::fromUInt(0xa0).toUInt() == 0xa0);

  testConversion(0x000000a0, 0x00, 0x00, 0x00, 0xa0);
  testConversion(0xd50bf072, 0xd5, 0x0b, 0xf0, 0x72);

  ByteVector intVector(2, 0);
  intVector[0] = char(0xfc);
  intVector[1] = char(0x00);
  printResult(intVector.toShort() == -1024);
  intVector[0] = char(0x04);
  intVector[1] = char(0x00);
  printResult(intVector.toShort() == 1024);
  
  ByteVector r0("**************");
  ByteVector r1("OggS**********");
  ByteVector r2("**********OggS");
  ByteVector r3("OggS******OggS");
  ByteVector r4("OggS*OggS*OggS");

  printResult(r0.find("OggS") == -1);
  printResult(r0.rfind("OggS") == -1);
  printResult(r1.find("OggS") == r1.rfind("OggS"));
  printResult(r2.find("OggS") == r2.rfind("OggS"));
  printResult(r3.find("OggS") == 0);
  printResult(r3.rfind("OggS") == 10);
  printResult(r4.rfind("OggS") == 10);
  printResult(r4.rfind("OggS", 12) == 5);

  printResult(ByteVector::fromLongLong(1).toLongLong() == 1);
  printResult(ByteVector::fromLongLong(0).toLongLong() == 0);
  printResult(ByteVector::fromLongLong(0xffffffffffffffffLL).toLongLong() == -1);
  printResult(ByteVector::fromLongLong(0xfffffffffffffffeLL).toLongLong() == -2);
  printResult(ByteVector::fromLongLong(1024).toLongLong() == 1024);

  ByteVector a1("foo");
  a1.append("bar");
  printResult(a1 == "foobar");

  ByteVector a2("foo");
  a2.append("b");
  printResult(a2 == "foob");

  ByteVector a3;
  a3.append("b");
  printResult(a3 == "b");

  ByteVector s1("foo");
  printResult(ByteVectorList::split(s1, " ").size() == 1);

  ByteVector s2("f");
  printResult(ByteVectorList::split(s2, " ").size() == 1);


  printResult(ByteVector().size() == 0);
  printResult(ByteVector("asdf").clear().size() == 0);
  printResult(ByteVector("asdf").clear() == ByteVector());
}

void testSynchData()
{
  cout << "*** Testing TagLib::ID3v2::SynchData ***" << endl;

  { // test 1
    char data[] = { 0, 0, 0, 127 };
    ByteVector v(data, 4);

    printResult(ID3v2::SynchData::toUInt(v) == 127);
    printResult(ID3v2::SynchData::fromUInt(127) == v);
  }
  { // test 2
    char data[] = { 0, 0, 1, 0 };
    ByteVector v(data, 4);

    printResult(ID3v2::SynchData::toUInt(v) == 128);
    printResult(ID3v2::SynchData::fromUInt(128) == v);
  }
  { // test 3
    char data[] = { 0, 0, 1, 1 };
    ByteVector v(data, 4);

    printResult(ID3v2::SynchData::toUInt(v) == 129);
    printResult(ID3v2::SynchData::fromUInt(129) == v);
  }
}

void testList()
{
  cout << "*** Testing TagLib::List<T> ***" << endl;
  List<int> l1;
  List<int> l2;
  List<int> l3;
  l1.append(2);
  l2.append(3);
  l2.append(4);
  l1.append(l2);
  l1.prepend(1);
  l3.append(1);
  l3.append(2);
  l3.append(3);
  l3.append(4);
  printResult(l1 == l3);
}
