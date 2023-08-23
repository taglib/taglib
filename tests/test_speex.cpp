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

#include "oggpageheader.h"
#include "speexfile.h"
#include "utils.h"
#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

TEST(Speex, testAudioProperties)
{
  Ogg::Speex::File f(TEST_FILE_PATH_C("empty.spx"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(53, f.audioProperties()->bitrate());
  ASSERT_EQ(-1, f.audioProperties()->bitrateNominal());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
}

TEST(Speex, testSplitPackets)
{
  ScopedFileCopy copy("empty", ".spx");
  string newname    = copy.fileName();

  const String text = longText(128 * 1024, true);

  {
    Ogg::Speex::File f(newname.c_str());
    f.tag()->setTitle(text);
    f.save();
  }
  {
    Ogg::Speex::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(156330), f.length());
    ASSERT_EQ(23, f.lastPageHeader()->pageSequenceNumber());
    ASSERT_EQ(80U, f.packet(0).size());
    ASSERT_EQ(131116U, f.packet(1).size());
    ASSERT_EQ(93U, f.packet(2).size());
    ASSERT_EQ(93U, f.packet(3).size());
    ASSERT_EQ(text, f.tag()->title());

    ASSERT_TRUE(f.audioProperties());
    ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());

    f.tag()->setTitle("ABCDE");
    f.save();
  }
  {
    Ogg::Speex::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(24317), f.length());
    ASSERT_EQ(7, f.lastPageHeader()->pageSequenceNumber());
    ASSERT_EQ(80U, f.packet(0).size());
    ASSERT_EQ(49U, f.packet(1).size());
    ASSERT_EQ(93U, f.packet(2).size());
    ASSERT_EQ(93U, f.packet(3).size());
    ASSERT_EQ(String("ABCDE"), f.tag()->title());

    ASSERT_TRUE(f.audioProperties());
    ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());
  }
}
