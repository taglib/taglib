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

#include "rifffile.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

class PublicRIFF : public RIFF::File
{
public:
  explicit PublicRIFF(FileName file) :
    RIFF::File(file, BigEndian) {};
  unsigned int riffSize() { return RIFF::File::riffSize(); };
  unsigned int chunkCount() { return RIFF::File::chunkCount(); };
  offset_t chunkOffset(unsigned int i) { return RIFF::File::chunkOffset(i); };
  unsigned int chunkPadding(unsigned int i) { return RIFF::File::chunkPadding(i); };
  unsigned int chunkDataSize(unsigned int i) { return RIFF::File::chunkDataSize(i); };
  ByteVector chunkName(unsigned int i) { return RIFF::File::chunkName(i); };
  ByteVector chunkData(unsigned int i) { return RIFF::File::chunkData(i); };
  void setChunkData(unsigned int i, const ByteVector &data)
  {
    RIFF::File::setChunkData(i, data);
  }
  void setChunkData(const ByteVector &name, const ByteVector &data)
  {
    RIFF::File::setChunkData(name, data);
  };
  TagLib::Tag* tag() const override { return nullptr; };
  TagLib::AudioProperties* audioProperties() const override { return nullptr;};
  bool save() override { return false; };
  void removeChunk(unsigned int i) { RIFF::File::removeChunk(i); }
  void removeChunk(const ByteVector &name) { RIFF::File::removeChunk(name); }
};

TEST(RIFF, testPadding)
{
  ScopedFileCopy copy("empty", ".aiff");
  string filename = copy.fileName();

  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(ByteVector("TEST"), f.chunkName(2));
    ASSERT_EQ(static_cast<offset_t>(0x1728 + 8), f.chunkOffset(2));

    f.setChunkData("TEST", "foo");
  }
  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(ByteVector("TEST"), f.chunkName(2));
    ASSERT_EQ(ByteVector("foo"), f.chunkData(2));
    ASSERT_EQ(static_cast<unsigned int>(3), f.chunkDataSize(2));
    ASSERT_EQ(static_cast<offset_t>(0x1728 + 8), f.chunkOffset(2));

    f.setChunkData("SSND", "abcd");

    ASSERT_EQ(ByteVector("SSND"), f.chunkName(1));
    ASSERT_EQ(ByteVector("abcd"), f.chunkData(1));

    f.seek(f.chunkOffset(1));
    ASSERT_EQ(ByteVector("abcd"), f.readBlock(4));

    ASSERT_EQ(ByteVector("TEST"), f.chunkName(2));
    ASSERT_EQ(ByteVector("foo"), f.chunkData(2));

    f.seek(f.chunkOffset(2));
    ASSERT_EQ(ByteVector("foo"), f.readBlock(3));
  }
  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(1));
    ASSERT_EQ(ByteVector("abcd"), f.chunkData(1));

    ASSERT_EQ(ByteVector("TEST"), f.chunkName(2));
    ASSERT_EQ(ByteVector("foo"), f.chunkData(2));
  }
}

TEST(RIFF, testLastChunkAtEvenPosition)
{
  ScopedFileCopy copy("noise", ".aif");
  string filename = copy.fileName();

  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(static_cast<offset_t>(0xff0 + 8), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4400), f.length());
    ASSERT_EQ(static_cast<unsigned int>(4399 - 8), f.riffSize());
    f.setChunkData("TEST", "abcd");
    ASSERT_EQ(static_cast<offset_t>(4088), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4408), f.chunkOffset(3));
    ASSERT_EQ(static_cast<unsigned int>(4), f.chunkDataSize(3));
    ASSERT_EQ(ByteVector("TEST"), f.chunkName(3));
    ASSERT_EQ(static_cast<unsigned int>(0), f.chunkPadding(3));
    ASSERT_EQ(static_cast<unsigned int>(4412 - 8), f.riffSize());
  }
  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(static_cast<offset_t>(4088), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4408), f.chunkOffset(3));
    ASSERT_EQ(static_cast<unsigned int>(4), f.chunkDataSize(3));
    ASSERT_EQ(ByteVector("TEST"), f.chunkName(3));
    ASSERT_EQ(static_cast<unsigned int>(0), f.chunkPadding(3));
    ASSERT_EQ(static_cast<offset_t>(4412), f.length());
  }
}

TEST(RIFF, testLastChunkAtEvenPosition2)
{
  ScopedFileCopy copy("noise_odd", ".aif");
  string filename = copy.fileName();

  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(static_cast<offset_t>(0xff0 + 8), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(0), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4399), f.length());
    ASSERT_EQ(static_cast<unsigned int>(4399 - 8), f.riffSize());
    f.setChunkData("TEST", "abcd");
    ASSERT_EQ(static_cast<offset_t>(4088), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4408), f.chunkOffset(3));
    ASSERT_EQ(static_cast<unsigned int>(4), f.chunkDataSize(3));
    ASSERT_EQ(ByteVector("TEST"), f.chunkName(3));
    ASSERT_EQ(static_cast<unsigned int>(0), f.chunkPadding(3));
    ASSERT_EQ(static_cast<unsigned int>(4412 - 8), f.riffSize());
  }
  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(static_cast<offset_t>(4088), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4408), f.chunkOffset(3));
    ASSERT_EQ(static_cast<unsigned int>(4), f.chunkDataSize(3));
    ASSERT_EQ(ByteVector("TEST"), f.chunkName(3));
    ASSERT_EQ(static_cast<unsigned int>(0), f.chunkPadding(3));
    ASSERT_EQ(static_cast<offset_t>(4412), f.length());
  }
}

TEST(RIFF, testLastChunkAtEvenPosition3)
{
  ScopedFileCopy copy("noise_odd", ".aif");
  string filename = copy.fileName();

  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(static_cast<offset_t>(0xff0 + 8), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(0), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4399), f.length());
    ASSERT_EQ(static_cast<unsigned int>(4399 - 8), f.riffSize());
    f.setChunkData("TEST", "abc");
    ASSERT_EQ(static_cast<offset_t>(4088), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4408), f.chunkOffset(3));
    ASSERT_EQ(static_cast<unsigned int>(3), f.chunkDataSize(3));
    ASSERT_EQ(ByteVector("TEST"), f.chunkName(3));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(3));
    ASSERT_EQ(static_cast<unsigned int>(4412 - 8), f.riffSize());
  }
  {
    PublicRIFF f(filename.c_str());
    ASSERT_EQ(static_cast<offset_t>(4088), f.chunkOffset(2));
    ASSERT_EQ(static_cast<unsigned int>(311), f.chunkDataSize(2));
    ASSERT_EQ(ByteVector("SSND"), f.chunkName(2));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(2));
    ASSERT_EQ(static_cast<offset_t>(4408), f.chunkOffset(3));
    ASSERT_EQ(static_cast<unsigned int>(3), f.chunkDataSize(3));
    ASSERT_EQ(ByteVector("TEST"), f.chunkName(3));
    ASSERT_EQ(static_cast<unsigned int>(1), f.chunkPadding(3));
    ASSERT_EQ(static_cast<offset_t>(4412), f.length());
  }
}

TEST(RIFF, testChunkOffset)
{
  ScopedFileCopy copy("empty", ".aiff");
  string filename = copy.fileName();

  PublicRIFF f(filename.c_str());

  ASSERT_EQ(5928U, f.riffSize());
  ASSERT_EQ(static_cast<offset_t>(5936), f.length());
  ASSERT_EQ(ByteVector("COMM"), f.chunkName(0));
  ASSERT_EQ(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));
  ASSERT_EQ(ByteVector("SSND"), f.chunkName(1));
  ASSERT_EQ(static_cast<offset_t>(0x0026 + 8), f.chunkOffset(1));
  ASSERT_EQ(ByteVector("TEST"), f.chunkName(2));
  ASSERT_EQ(static_cast<offset_t>(0x1728 + 8), f.chunkOffset(2));

  const ByteVector data(0x400, ' ');
  f.setChunkData("SSND", data);
  ASSERT_EQ(1070U, f.riffSize());
  ASSERT_EQ(static_cast<offset_t>(1078), f.length());
  ASSERT_EQ(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));
  ASSERT_EQ(static_cast<offset_t>(0x0026 + 8), f.chunkOffset(1));
  ASSERT_EQ(static_cast<offset_t>(0x042E + 8), f.chunkOffset(2));

  f.seek(f.chunkOffset(0) - 8);
  ASSERT_EQ(ByteVector("COMM"), f.readBlock(4));
  f.seek(f.chunkOffset(1) - 8);
  ASSERT_EQ(ByteVector("SSND"), f.readBlock(4));
  f.seek(f.chunkOffset(2) - 8);
  ASSERT_EQ(ByteVector("TEST"), f.readBlock(4));

  f.setChunkData(0, data);
  ASSERT_EQ(2076U, f.riffSize());
  ASSERT_EQ(static_cast<offset_t>(2084), f.length());
  ASSERT_EQ(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));
  ASSERT_EQ(static_cast<offset_t>(0x0414 + 8), f.chunkOffset(1));
  ASSERT_EQ(static_cast<offset_t>(0x081C + 8), f.chunkOffset(2));

  f.seek(f.chunkOffset(0) - 8);
  ASSERT_EQ(ByteVector("COMM"), f.readBlock(4));
  f.seek(f.chunkOffset(1) - 8);
  ASSERT_EQ(ByteVector("SSND"), f.readBlock(4));
  f.seek(f.chunkOffset(2) - 8);
  ASSERT_EQ(ByteVector("TEST"), f.readBlock(4));

  f.removeChunk("SSND");
  ASSERT_EQ(1044U, f.riffSize());
  ASSERT_EQ(static_cast<offset_t>(1052), f.length());
  ASSERT_EQ(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));
  ASSERT_EQ(static_cast<offset_t>(0x0414 + 8), f.chunkOffset(1));

  f.seek(f.chunkOffset(0) - 8);
  ASSERT_EQ(ByteVector("COMM"), f.readBlock(4));
  f.seek(f.chunkOffset(1) - 8);
  ASSERT_EQ(ByteVector("TEST"), f.readBlock(4));

  f.removeChunk(0);
  ASSERT_EQ(12U, f.riffSize());
  ASSERT_EQ(static_cast<offset_t>(20), f.length());
  ASSERT_EQ(static_cast<offset_t>(0x000C + 8), f.chunkOffset(0));

  f.seek(f.chunkOffset(0) - 8);
  ASSERT_EQ(ByteVector("TEST"), f.readBlock(4));
}
