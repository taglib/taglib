/***************************************************************************
    copyright           : (C) 2009 by Lukas Lalinsky
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

#include <string>
#include <cstdio>

#include "tbytevectorlist.h"
#include "tag.h"
#include "rifffile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class PublicRIFF : public RIFF::File
{
public:
  PublicRIFF(FileName file) : RIFF::File(file, BigEndian)
  {
  }
  unsigned int riffSize() const
  {
    return RIFF::File::riffSize();
  }
  unsigned int chunkCount() const
  {
    return RIFF::File::chunkCount();
  }
  offset_t chunkOffset(unsigned int i) const
  {
    return RIFF::File::chunkOffset(i);
  }
  unsigned int chunkPadding(unsigned int i) const
  {
    return RIFF::File::chunkPadding(i);
  }
  unsigned int chunkDataSize(unsigned int i) const
  {
    return RIFF::File::chunkDataSize(i);
  }
  ByteVector chunkName(unsigned int i) const
  {
    return RIFF::File::chunkName(i);
  }
  ByteVector chunkData(unsigned int i)
  {
    return RIFF::File::chunkData(i);
  }
  void setChunkData(unsigned int i, const ByteVector &data) {
    RIFF::File::setChunkData(i, data);
  }
  void setChunkData(const ByteVector &name, const ByteVector &data) {
    RIFF::File::setChunkData(name, data);
  }
  TagLib::Tag *tag() const override
  {
    return nullptr;
  }
  TagLib::AudioProperties *audioProperties() const override
  {
    return nullptr;
  }
  bool save() override
  {
    return false;
  }
  void removeChunk(unsigned int i) { RIFF::File::removeChunk(i); }
  void removeChunk(const ByteVector &name) { RIFF::File::removeChunk(name); }
};

class TestRIFF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestRIFF);
  CPPUNIT_TEST(testPadding);
  CPPUNIT_TEST(testLastChunkAtEvenPosition);
  CPPUNIT_TEST(testLastChunkAtEvenPosition2);
  CPPUNIT_TEST(testLastChunkAtEvenPosition3);
  CPPUNIT_TEST(testChunkOffset);
  CPPUNIT_TEST_SUITE_END();

public:

  void testPadding()
  {
    ScopedFileCopy copy("empty", ".aiff");
    string filename = copy.fileName();

    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x1728 + 8), f.chunkOffset(2));

      f.setChunkData("TEST", "foo");
    }
    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("foo"), f.chunkData(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x1728 + 8), f.chunkOffset(2));

      f.setChunkData("SSND", "abcd");

      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(1));
      CPPUNIT_ASSERT_EQUAL(ByteVector("abcd"), f.chunkData(1));

      f.seek(f.chunkOffset(1));
      CPPUNIT_ASSERT_EQUAL(ByteVector("abcd"), f.readBlock(4));

      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("foo"), f.chunkData(2));

      f.seek(f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("foo"), f.readBlock(3));
    }
    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(1));
      CPPUNIT_ASSERT_EQUAL(ByteVector("abcd"), f.chunkData(1));

      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("foo"), f.chunkData(2));
    }
  }

  void testLastChunkAtEvenPosition()
  {
    ScopedFileCopy copy("noise", ".aif");
    string filename = copy.fileName();

    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0xff0 + 8), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4400), f.length());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4399 - 8), f.riffSize());
      f.setChunkData("TEST", "abcd");
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4088), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4408), f.chunkOffset(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4), f.chunkDataSize(3));
      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), f.chunkPadding(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4412 - 8), f.riffSize());
    }
    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4088), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4408), f.chunkOffset(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4), f.chunkDataSize(3));
      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), f.chunkPadding(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4412), f.length());
    }
  }

  void testLastChunkAtEvenPosition2()
  {
    ScopedFileCopy copy("noise_odd", ".aif");
    string filename = copy.fileName();

    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0xff0 + 8), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4399), f.length());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4399 - 8), f.riffSize());
      f.setChunkData("TEST", "abcd");
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4088), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4408), f.chunkOffset(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4), f.chunkDataSize(3));
      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), f.chunkPadding(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4412 - 8), f.riffSize());
    }
    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4088), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4408), f.chunkOffset(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4), f.chunkDataSize(3));
      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), f.chunkPadding(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4412), f.length());
    }
  }

  void testLastChunkAtEvenPosition3()
  {
    ScopedFileCopy copy("noise_odd", ".aif");
    string filename = copy.fileName();

    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0xff0 + 8), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4399), f.length());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4399 - 8), f.riffSize());
      f.setChunkData("TEST", "abc");
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4088), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4408), f.chunkOffset(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), f.chunkDataSize(3));
      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4412 - 8), f.riffSize());
    }
    {
      PublicRIFF f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4088), f.chunkOffset(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(311), f.chunkDataSize(2));
      CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(2));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4408), f.chunkOffset(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), f.chunkDataSize(3));
      CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), f.chunkPadding(3));
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(4412), f.length());
    }
  }

  void testChunkOffset()
  {
    ScopedFileCopy copy("empty", ".aiff");
    string filename = copy.fileName();

    PublicRIFF f(filename.c_str());

    CPPUNIT_ASSERT_EQUAL(5928U, f.riffSize());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(5936), f.length());
    CPPUNIT_ASSERT_EQUAL(ByteVector("COMM"), f.chunkName(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));
    CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.chunkName(1));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x0026 + 8), f.chunkOffset(1));
    CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.chunkName(2));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x1728 + 8), f.chunkOffset(2));

    const ByteVector data(0x400, ' ');
    f.setChunkData("SSND", data);
    CPPUNIT_ASSERT_EQUAL(1070U, f.riffSize());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(1078), f.length());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x0026 + 8), f.chunkOffset(1));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x042E + 8), f.chunkOffset(2));

    f.seek(f.chunkOffset(0) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("COMM"), f.readBlock(4));
    f.seek(f.chunkOffset(1) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.readBlock(4));
    f.seek(f.chunkOffset(2) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.readBlock(4));

    f.setChunkData(0, data);
    CPPUNIT_ASSERT_EQUAL(2076U, f.riffSize());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(2084), f.length());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x0414 + 8), f.chunkOffset(1));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x081C + 8), f.chunkOffset(2));

    f.seek(f.chunkOffset(0) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("COMM"), f.readBlock(4));
    f.seek(f.chunkOffset(1) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("SSND"), f.readBlock(4));
    f.seek(f.chunkOffset(2) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.readBlock(4));

    f.removeChunk("SSND");
    CPPUNIT_ASSERT_EQUAL(1044U, f.riffSize());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(1052), f.length());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x0414 + 8), f.chunkOffset(1));

    f.seek(f.chunkOffset(0) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("COMM"), f.readBlock(4));
    f.seek(f.chunkOffset(1) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.readBlock(4));

    f.removeChunk(0);
    CPPUNIT_ASSERT_EQUAL(12U, f.riffSize());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(20), f.length());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));

    f.seek(f.chunkOffset(0) - 8);
    CPPUNIT_ASSERT_EQUAL(ByteVector("TEST"), f.readBlock(4));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestRIFF);
