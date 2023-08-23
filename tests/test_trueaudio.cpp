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

#include "id3v1tag.h"
#include "id3v2tag.h"
#include "tpropertymap.h"
#include "trueaudiofile.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(TrueAudio, testReadPropertiesWithoutID3v2)
{
  TrueAudio::File f(TEST_FILE_PATH_C("empty.tta"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(173, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(162496U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1, f.audioProperties()->ttaVersion());
}

TEST(TrueAudio, testReadPropertiesWithTags)
{
  TrueAudio::File f(TEST_FILE_PATH_C("tagged.tta"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(173, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(162496U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1, f.audioProperties()->ttaVersion());
}

TEST(TrueAudio, testStripAndProperties)
{
  ScopedFileCopy copy("empty", ".tta");

  {
    TrueAudio::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle("ID3v2");
    f.ID3v1Tag(true)->setTitle("ID3v1");
    f.save();
  }
  {
    TrueAudio::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasID3v1Tag());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(String("ID3v2"), f.properties()["TITLE"].front());
    f.strip(TrueAudio::File::ID3v2);
    ASSERT_EQ(String("ID3v1"), f.properties()["TITLE"].front());
    f.strip(TrueAudio::File::ID3v1);
    ASSERT_TRUE(f.properties().isEmpty());
    f.save();
  }
  {
    TrueAudio::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasID3v1Tag());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_TRUE(f.properties()["TITLE"].isEmpty());
    ASSERT_TRUE(f.properties().isEmpty());
  }
}

TEST(TrueAudio, testRepeatedSave)
{
  ScopedFileCopy copy("empty", ".tta");

  {
    TrueAudio::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_FALSE(f.hasID3v1Tag());

    f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.save();

    f.ID3v2Tag()->setTitle("0");
    f.save();

    f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.ID3v2Tag()->setTitle("01234 56789 ABCDE FGHIJ 01234 56789 ABCDE FGHIJ 01234 56789");
    f.save();
  }
  {
    TrueAudio::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_TRUE(f.hasID3v1Tag());
  }
}
