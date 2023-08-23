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

#include "oggpageheader.h"
#include "opusfile.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(Opus, testAudioProperties)
{
  Ogg::Opus::File f(TEST_FILE_PATH_C("correctness_gain_silent_output.opus"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(7, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(7737, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(36, f.audioProperties()->bitrate());
  ASSERT_EQ(1, f.audioProperties()->channels());
  ASSERT_EQ(48000, f.audioProperties()->sampleRate());
  ASSERT_EQ(48000, f.audioProperties()->inputSampleRate());
  ASSERT_EQ(1, f.audioProperties()->opusVersion());
}

TEST(Opus, testReadComments)
{
  Ogg::Opus::File f(TEST_FILE_PATH_C("correctness_gain_silent_output.opus"));
  ASSERT_EQ(StringList("Xiph.Org Opus testvectormaker"), f.tag()->fieldListMap()["ENCODER"]);
  ASSERT_TRUE(f.tag()->fieldListMap().contains("TESTDESCRIPTION"));
  ASSERT_FALSE(f.tag()->fieldListMap().contains("ARTIST"));
  ASSERT_EQ(String("libopus 0.9.11-66-g64c2dd7"), f.tag()->vendorID());
}

TEST(Opus, testWriteComments)
{
  ScopedFileCopy copy("correctness_gain_silent_output", ".opus");
  string filename = copy.fileName();

  {
    Ogg::Opus::File f(filename.c_str());
    f.tag()->setArtist("Your Tester");
    f.save();
  }
  {
    Ogg::Opus::File f(filename.c_str());
    ASSERT_EQ(StringList("Xiph.Org Opus testvectormaker"), f.tag()->fieldListMap()["ENCODER"]);
    ASSERT_TRUE(f.tag()->fieldListMap().contains("TESTDESCRIPTION"));
    ASSERT_EQ(StringList("Your Tester"), f.tag()->fieldListMap()["ARTIST"]);
    ASSERT_EQ(String("libopus 0.9.11-66-g64c2dd7"), f.tag()->vendorID());
  }
}

TEST(Opus, testSplitPackets)
{
  ScopedFileCopy copy("correctness_gain_silent_output", ".opus");
  string newname    = copy.fileName();

  const String text = longText(128 * 1024, true);

  {
    Ogg::Opus::File f(newname.c_str());
    f.tag()->setTitle(text);
    f.save();
  }
  {
    Ogg::Opus::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(167534), f.length());
    ASSERT_EQ(27, f.lastPageHeader()->pageSequenceNumber());
    ASSERT_EQ(19U, f.packet(0).size());
    ASSERT_EQ(131380U, f.packet(1).size());
    ASSERT_EQ(5U, f.packet(2).size());
    ASSERT_EQ(5U, f.packet(3).size());
    ASSERT_EQ(text, f.tag()->title());

    ASSERT_TRUE(f.audioProperties());
    ASSERT_EQ(7737, f.audioProperties()->lengthInMilliseconds());

    f.tag()->setTitle("ABCDE");
    f.save();
  }
  {
    Ogg::Opus::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(35521), f.length());
    ASSERT_EQ(11, f.lastPageHeader()->pageSequenceNumber());
    ASSERT_EQ(19U, f.packet(0).size());
    ASSERT_EQ(313U, f.packet(1).size());
    ASSERT_EQ(5U, f.packet(2).size());
    ASSERT_EQ(5U, f.packet(3).size());
    ASSERT_EQ(String("ABCDE"), f.tag()->title());

    ASSERT_TRUE(f.audioProperties());
    ASSERT_EQ(7737, f.audioProperties()->lengthInMilliseconds());
  }
}
