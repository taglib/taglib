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

#include "aifffile.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(AIFF, testAiffProperties)
{
  RIFF::AIFF::File f(TEST_FILE_PATH_C("empty.aiff"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(67, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(706, f.audioProperties()->bitrate());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(1, f.audioProperties()->channels());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(2941U, f.audioProperties()->sampleFrames());
  ASSERT_FALSE(f.audioProperties()->isAiffC());
}

TEST(AIFF, testAiffCProperties)
{
  RIFF::AIFF::File f(TEST_FILE_PATH_C("alaw.aifc"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(37, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(355, f.audioProperties()->bitrate());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(1, f.audioProperties()->channels());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(1622U, f.audioProperties()->sampleFrames());
  ASSERT_TRUE(f.audioProperties()->isAiffC());
  ASSERT_EQ(ByteVector("ALAW"), f.audioProperties()->compressionType());
  ASSERT_EQ(String("SGI CCITT G.711 A-law"), f.audioProperties()->compressionName());
}

TEST(AIFF, testSaveID3v2)
{
  ScopedFileCopy copy("empty", ".aiff");
  string newname = copy.fileName();

  {
    RIFF::AIFF::File f(newname.c_str());
    ASSERT_FALSE(f.hasID3v2Tag());

    f.tag()->setTitle(L"TitleXXX");
    f.save();
    ASSERT_TRUE(f.hasID3v2Tag());
  }
  {
    RIFF::AIFF::File f(newname.c_str());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(String(L"TitleXXX"), f.tag()->title());

    f.tag()->setTitle("");
    f.save();
    ASSERT_FALSE(f.hasID3v2Tag());
  }
  {
    RIFF::AIFF::File f(newname.c_str());
    ASSERT_FALSE(f.hasID3v2Tag());
  }
}

TEST(AIFF, testSaveID3v23)
{
  ScopedFileCopy copy("empty", ".aiff");
  string newname = copy.fileName();

  String xxx     = ByteVector(254, 'X');
  {
    RIFF::AIFF::File f(newname.c_str());
    ASSERT_FALSE(f.hasID3v2Tag());

    f.tag()->setTitle(xxx);
    f.tag()->setArtist("Artist A");
    f.save(ID3v2::v3);
    ASSERT_TRUE(f.hasID3v2Tag());
  }
  {
    RIFF::AIFF::File f2(newname.c_str());
    ASSERT_EQ(static_cast<unsigned int>(3), f2.tag()->header()->majorVersion());
    ASSERT_EQ(String("Artist A"), f2.tag()->artist());
    ASSERT_EQ(xxx, f2.tag()->title());
  }
}

TEST(AIFF, testDuplicateID3v2)
{
  ScopedFileCopy copy("duplicate_id3v2", ".aiff");

  // duplicate_id3v2.aiff has duplicate ID3v2 tag chunks.
  // title() returns "Title2" if can't skip the second tag.

  RIFF::AIFF::File f(copy.fileName().c_str());
  ASSERT_TRUE(f.hasID3v2Tag());
  ASSERT_EQ(String("Title1"), f.tag()->title());

  f.save();
  ASSERT_EQ(static_cast<offset_t>(7030), f.length());
  ASSERT_EQ(static_cast<offset_t>(-1), f.find("Title2"));
}

TEST(AIFF, testFuzzedFile1)
{
  RIFF::AIFF::File f(TEST_FILE_PATH_C("segfault.aif"));
  ASSERT_TRUE(f.isValid());
}

TEST(AIFF, testFuzzedFile2)
{
  RIFF::AIFF::File f(TEST_FILE_PATH_C("excessive_alloc.aif"));
  ASSERT_TRUE(f.isValid());
}
