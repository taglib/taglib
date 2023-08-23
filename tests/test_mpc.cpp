/***************************************************************************
    copyright           : (C) 2012 by Lukas Lalinsky
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
#include "mpcfile.h"
#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(MPC, testPropertiesSV8)
{
  MPC::File f(TEST_FILE_PATH_C("sv8_header.mpc"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(8, f.audioProperties()->mpcVersion());
  ASSERT_EQ(1, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(1497, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(1, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(66014U, f.audioProperties()->sampleFrames());
}

TEST(MPC, testPropertiesSV7)
{
  MPC::File f(TEST_FILE_PATH_C("click.mpc"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(7, f.audioProperties()->mpcVersion());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(40, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(318, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(1760U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(14221, f.audioProperties()->trackGain());
  ASSERT_EQ(19848, f.audioProperties()->trackPeak());
  ASSERT_EQ(14221, f.audioProperties()->albumGain());
  ASSERT_EQ(19848, f.audioProperties()->albumPeak());
}

TEST(MPC, testPropertiesSV5)
{
  MPC::File f(TEST_FILE_PATH_C("sv5_header.mpc"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(5, f.audioProperties()->mpcVersion());
  ASSERT_EQ(26, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(26371, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(0, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(1162944U, f.audioProperties()->sampleFrames());
}

TEST(MPC, testPropertiesSV4)
{
  MPC::File f(TEST_FILE_PATH_C("sv4_header.mpc"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(4, f.audioProperties()->mpcVersion());
  ASSERT_EQ(26, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(26371, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(0, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(1162944U, f.audioProperties()->sampleFrames());
}

TEST(MPC, testFuzzedFile1)
{
  MPC::File f(TEST_FILE_PATH_C("zerodiv.mpc"));
  ASSERT_TRUE(f.isValid());
}

TEST(MPC, testFuzzedFile2)
{
  MPC::File f(TEST_FILE_PATH_C("infloop.mpc"));
  ASSERT_TRUE(f.isValid());
}

TEST(MPC, testFuzzedFile3)
{
  MPC::File f(TEST_FILE_PATH_C("segfault.mpc"));
  ASSERT_TRUE(f.isValid());
}

TEST(MPC, testFuzzedFile4)
{
  MPC::File f(TEST_FILE_PATH_C("segfault2.mpc"));
  ASSERT_TRUE(f.isValid());
}

TEST(MPC, testStripAndProperties)
{
  ScopedFileCopy copy("click", ".mpc");

  {
    MPC::File f(copy.fileName().c_str());
    f.APETag(true)->setTitle("APE");
    f.ID3v1Tag(true)->setTitle("ID3v1");
    f.save();
  }
  {
    MPC::File f(copy.fileName().c_str());
    ASSERT_EQ(String("APE"), f.properties()["TITLE"].front());
    f.strip(MPC::File::APE);
    ASSERT_EQ(String("ID3v1"), f.properties()["TITLE"].front());
    f.strip(MPC::File::ID3v1);
    ASSERT_TRUE(f.properties().isEmpty());
    f.save();
  }
  {
    MPC::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasAPETag());
    ASSERT_FALSE(f.hasID3v1Tag());
    ASSERT_TRUE(f.properties()["TITLE"].isEmpty());
    ASSERT_TRUE(f.properties().isEmpty());
  }
}

TEST(MPC, testRepeatedSave)
{
  ScopedFileCopy copy("click", ".mpc");

  {
    MPC::File f(copy.fileName().c_str());
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
    MPC::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasAPETag());
    ASSERT_TRUE(f.hasID3v1Tag());
  }
}
