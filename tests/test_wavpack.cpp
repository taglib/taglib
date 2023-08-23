/***************************************************************************
    copyright           : (C) 2010 by Lukas Lalinsky
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

#include "apetag.h"
#include "id3v1tag.h"
#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "utils.h"
#include "wavpackfile.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(WavPack, testNoLengthProperties)
{
  WavPack::File f(TEST_FILE_PATH_C("no_length.wv"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3705, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(1, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_TRUE(f.audioProperties()->isLossless());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(163392U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1031, f.audioProperties()->version());
}

TEST(WavPack, testMultiChannelProperties)
{
  WavPack::File f(TEST_FILE_PATH_C("four_channels.wv"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3833, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(112, f.audioProperties()->bitrate());
  ASSERT_EQ(4, f.audioProperties()->channels());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_FALSE(f.audioProperties()->isLossless());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(169031U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1031, f.audioProperties()->version());
}

TEST(WavPack, testDsdStereoProperties)
{
  WavPack::File f(TEST_FILE_PATH_C("dsd_stereo.wv"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(200, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(2096, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(8, f.audioProperties()->bitsPerSample());
  ASSERT_TRUE(f.audioProperties()->isLossless());
  ASSERT_EQ(352800, f.audioProperties()->sampleRate());
  ASSERT_EQ(70560U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1040, f.audioProperties()->version());
}

TEST(WavPack, testNonStandardRateProperties)
{
  WavPack::File f(TEST_FILE_PATH_C("non_standard_rate.wv"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3675, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(0, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_TRUE(f.audioProperties()->isLossless());
  ASSERT_EQ(1000, f.audioProperties()->sampleRate());
  ASSERT_EQ(3675U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1040, f.audioProperties()->version());
}

TEST(WavPack, testTaggedProperties)
{
  WavPack::File f(TEST_FILE_PATH_C("tagged.wv"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3550, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(172, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_FALSE(f.audioProperties()->isLossless());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(156556U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1031, f.audioProperties()->version());
}

TEST(WavPack, testFuzzedFile)
{
  WavPack::File f(TEST_FILE_PATH_C("infloop.wv"));
  ASSERT_TRUE(f.isValid());
}

TEST(WavPack, testStripAndProperties)
{
  ScopedFileCopy copy("click", ".wv");

  {
    WavPack::File f(copy.fileName().c_str());
    f.APETag(true)->setTitle("APE");
    f.ID3v1Tag(true)->setTitle("ID3v1");
    f.save();
  }
  {
    WavPack::File f(copy.fileName().c_str());
    ASSERT_EQ(String("APE"), f.properties()["TITLE"].front());
    f.strip(WavPack::File::APE);
    ASSERT_EQ(String("ID3v1"), f.properties()["TITLE"].front());
    f.strip(WavPack::File::ID3v1);
    ASSERT_TRUE(f.properties().isEmpty());
  }
}

TEST(WavPack, testRepeatedSave)
{
  ScopedFileCopy copy("click", ".wv");

  {
    WavPack::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasAPETag());
    ASSERT_FALSE(f.hasID3v1Tag());

    f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.save();

    f.APETag()->setTitle("0");
    f.save();

    f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.APETag()->setTitle("01234 56789 ABCDE FGHIJ 01234 56789 ABCDE FGHIJ 01234 56789");
    f.save();
  }
  {
    WavPack::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasAPETag());
    ASSERT_TRUE(f.hasID3v1Tag());
  }
}
