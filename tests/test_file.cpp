/***************************************************************************
    copyright           : (C) 2015 by Tsuda Kageyu
    email               : tsuda.kageyu@gmail.com
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

#include <tfile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace TagLib;

// File subclass that gives tests access to filesystem operations
class PlainFile : public File {
public:
  PlainFile(FileName name) : File(name) { }
  Tag *tag() const { return NULL; }
  AudioProperties *audioProperties() const { return NULL; }
  bool save(){ return false; }
  void truncate(long length) { File::truncate(length); }
};

class TestFile : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFile);
  CPPUNIT_TEST(testFindInSmallFile);
  CPPUNIT_TEST(testRFindInSmallFile);
  CPPUNIT_TEST(testSeek);
  CPPUNIT_TEST(testTruncate);
  CPPUNIT_TEST(testFindLargePattern);
  CPPUNIT_TEST(testRFindLargePattern);
  CPPUNIT_TEST(testFindPartialMatch);
  CPPUNIT_TEST(testRFindPartialMatch);
  CPPUNIT_TEST_SUITE_END();

public:

  void testFindInSmallFile()
  {
    ScopedFileCopy copy("empty", ".ogg");
    std::string name = copy.fileName();
    {
      PlainFile file(name.c_str());
      file.seek(0);
      file.writeBlock(ByteVector("0123456239", 10));
      file.truncate(10);
    }
    {
      PlainFile file(name.c_str());
      CPPUNIT_ASSERT_EQUAL(10l, file.length());

      CPPUNIT_ASSERT_EQUAL(2l, file.find(ByteVector("23", 2)));
      CPPUNIT_ASSERT_EQUAL(2l, file.find(ByteVector("23", 2), 2));
      CPPUNIT_ASSERT_EQUAL(7l, file.find(ByteVector("23", 2), 3));

      file.seek(0);
      const ByteVector v = file.readBlock(file.length());
      CPPUNIT_ASSERT_EQUAL((unsigned int)10, v.size());

      CPPUNIT_ASSERT_EQUAL((long)v.find("23"),    file.find("23"));
      CPPUNIT_ASSERT_EQUAL((long)v.find("23", 2), file.find("23", 2));
      CPPUNIT_ASSERT_EQUAL((long)v.find("23", 3), file.find("23", 3));

      file.seek(5);
      CPPUNIT_ASSERT_EQUAL(0l, file.find("0123456239"));
      CPPUNIT_ASSERT_EQUAL(-1l, file.find("0123456239", 1));
      CPPUNIT_ASSERT_EQUAL(3l, file.find("34", 0, "56"));
      CPPUNIT_ASSERT_EQUAL(3l, file.find("34", 0, "34"));
      CPPUNIT_ASSERT_EQUAL(-1l, file.find("56", 0, "34"));
      CPPUNIT_ASSERT_EQUAL(5l, file.tell());
    }
  }

  void testRFindInSmallFile()
  {
    ScopedFileCopy copy("empty", ".ogg");
    std::string name = copy.fileName();
    {
      PlainFile file(name.c_str());
      file.seek(0);
      file.writeBlock(ByteVector("0123456239", 10));
      file.truncate(10);
    }
    {
      PlainFile file(name.c_str());
      CPPUNIT_ASSERT_EQUAL(10l, file.length());

      CPPUNIT_ASSERT_EQUAL(7l, file.rfind(ByteVector("23", 2)));
      CPPUNIT_ASSERT_EQUAL(7l, file.rfind(ByteVector("23", 2), 7));
      CPPUNIT_ASSERT_EQUAL(2l, file.rfind(ByteVector("23", 2), 6));

      file.seek(0);
      const ByteVector v = file.readBlock(file.length());
      CPPUNIT_ASSERT_EQUAL((unsigned int)10, v.size());

      CPPUNIT_ASSERT_EQUAL((long)v.rfind("23"),    file.rfind("23"));
      CPPUNIT_ASSERT_EQUAL((long)v.rfind("23", 7), file.rfind("23", 7));
      CPPUNIT_ASSERT_EQUAL((long)v.rfind("23", 6), file.rfind("23", 6));

      file.seek(5);
      CPPUNIT_ASSERT_EQUAL(0l, file.rfind("0123456239"));
      CPPUNIT_ASSERT_EQUAL(0l, file.rfind("0123456239", 1));
      CPPUNIT_ASSERT_EQUAL(-1l, file.rfind("34", 0, "56"));
      CPPUNIT_ASSERT_EQUAL(5l, file.rfind("56", 0, "34"));
      CPPUNIT_ASSERT_EQUAL(5l, file.rfind("56", 0, "34"));
      CPPUNIT_ASSERT_EQUAL(5l, file.tell());
    }
  }

  void testSeek()
  {
    ScopedFileCopy copy("empty", ".ogg");
    std::string name = copy.fileName();

    PlainFile f(name.c_str());
    CPPUNIT_ASSERT_EQUAL((long)0, f.tell());
    CPPUNIT_ASSERT_EQUAL((long)4328, f.length());

    f.seek(100, File::Beginning);
    CPPUNIT_ASSERT_EQUAL((long)100, f.tell());
    f.seek(100, File::Current);
    CPPUNIT_ASSERT_EQUAL((long)200, f.tell());
    f.seek(-300, File::Current);
    CPPUNIT_ASSERT_EQUAL((long)200, f.tell());

    f.seek(-100, File::End);
    CPPUNIT_ASSERT_EQUAL((long)4228, f.tell());
    f.seek(-100, File::Current);
    CPPUNIT_ASSERT_EQUAL((long)4128, f.tell());
    f.seek(300, File::Current);
    CPPUNIT_ASSERT_EQUAL((long)4428, f.tell());
  }

  void testTruncate()
  {
    ScopedFileCopy copy("empty", ".ogg");
    std::string name = copy.fileName();

    {
      PlainFile f(name.c_str());
      CPPUNIT_ASSERT_EQUAL(4328L, f.length());

      f.truncate(2000);
      CPPUNIT_ASSERT_EQUAL(2000L, f.length());
    }
    {
      PlainFile f(name.c_str());
      CPPUNIT_ASSERT_EQUAL(2000L, f.length());
    }
  }

  void testFindLargePattern()
  {
    ByteVector pattern;
    {
      PlainFile f(TEST_FILE_PATH_C("random.bin"));
      f.seek(567);
      pattern = f.readBlock(8 * 1024);
    }
    {
      PlainFile f(TEST_FILE_PATH_C("random.bin"));
      CPPUNIT_ASSERT_EQUAL(567L, f.find(pattern));
    }
  }

  void testRFindLargePattern()
  {
    ByteVector pattern;
    {
      PlainFile f(TEST_FILE_PATH_C("random.bin"));
      f.seek(765);
      pattern = f.readBlock(8 * 1024);
    }
    {
      PlainFile f(TEST_FILE_PATH_C("random.bin"));
      CPPUNIT_ASSERT_EQUAL(765L, f.rfind(pattern));
    }
  }

  void testFindPartialMatch()
  {
    PlainFile f(TEST_FILE_PATH_C("random.bin"));
    CPPUNIT_ASSERT_EQUAL(1022L, f.find(ByteVector("\x7b\x8e\xac\xaf", 4)));
  }

  void testRFindPartialMatch()
  {
    PlainFile f(TEST_FILE_PATH_C("random.bin"));
    CPPUNIT_ASSERT_EQUAL(15362L, f.rfind(ByteVector("\x7b\x80\xa3\xa8", 4)));
    CPPUNIT_ASSERT_EQUAL(15358L, f.rfind(ByteVector("\x09\xc3\xe1\x2c", 4)));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFile);

