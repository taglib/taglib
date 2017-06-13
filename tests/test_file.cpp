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

#include <catch/catch.hpp>
#include <tfile.h>
#include "utils.h"

using namespace TagLib;

namespace
{
  // File subclass that gives tests access to filesystem operations
  class PlainFile : public File {
  public:
    PlainFile(FileName name) : File(name) { }
    Tag *tag() const { return NULL; }
    AudioProperties *audioProperties() const { return NULL; }
    bool save(){ return false; }
    void truncate(long length) { File::truncate(length); }
  };
}

TEST_CASE("Abstract file")
{
  SECTION("Find in small file")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    {
      PlainFile file(copy.fileName().c_str());
      file.seek(0);
      file.writeBlock(ByteVector("0123456239", 10));
      file.truncate(10);
    }
    {
      PlainFile file(copy.fileName().c_str());
      REQUIRE(file.length() == 10);
    
      REQUIRE(file.find(ByteVector("23", 2)) == 2);
      REQUIRE(file.find(ByteVector("23", 2), 2) == 2);
      REQUIRE(file.find(ByteVector("23", 2), 3) == 7);
    
      file.seek(0);
      const ByteVector v = file.readBlock(file.length());
      REQUIRE(v.size() == 10);
    
      REQUIRE(file.find("23") == v.find("23"));
      REQUIRE(file.find("23", 2) == v.find("23", 2));
      REQUIRE(file.find("23", 3) == v.find("23", 3));
    }
  }
  SECTION("RFind in small file")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    {
      PlainFile file(copy.fileName().c_str());
      file.seek(0);
      file.writeBlock(ByteVector("0123456239", 10));
      file.truncate(10);
    }
    {
      PlainFile file(copy.fileName().c_str());
      REQUIRE(file.length() == 10);
    
      REQUIRE(file.rfind(ByteVector("23", 2)) == 7);
      REQUIRE(file.rfind(ByteVector("23", 2), 7) == 7);
      REQUIRE(file.rfind(ByteVector("23", 2), 6) == 2);
    
      file.seek(0);
      const ByteVector v = file.readBlock(file.length());
      REQUIRE(v.size() == 10);
    
      REQUIRE(file.rfind("23") == v.rfind("23"));
      REQUIRE(file.rfind("23", 7) == v.rfind("23", 7));
      REQUIRE(file.rfind("23", 6) == v.rfind("23", 6));
    }
  }
  SECTION("Seek")
  {
    const ScopedFileCopy copy("empty", ".ogg");

    PlainFile f(copy.fileName().c_str());
    REQUIRE(f.tell() == 0);
    REQUIRE(f.length() == 4328);
    
    f.seek(100, File::Beginning);
    REQUIRE(f.tell() == 100);
    f.seek(100, File::Current);
    REQUIRE(f.tell() == 200);
    f.seek(-300, File::Current);
    REQUIRE(f.tell() == 200);
    
    f.seek(-100, File::End);
    REQUIRE(f.tell() == 4228);
    f.seek(-100, File::Current);
    REQUIRE(f.tell() == 4128);
    f.seek(300, File::Current);
    REQUIRE(f.tell() == 4428);
  }
  SECTION("Truncate")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    {
      PlainFile f(copy.fileName().c_str());
      REQUIRE(f.length() == 4328);
    
      f.truncate(2000);
      REQUIRE(f.length() == 2000);
    }
    {
      PlainFile f(copy.fileName().c_str());
      REQUIRE(f.length() == 2000);
    }
  }
}
