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

#include "oggfile.h"
#include "oggflacfile.h"
#include "oggpageheader.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "tstringlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(OggFLAC, testFramingBit)
{
  ScopedFileCopy copy("empty_flac", ".oga");
  string newname = copy.fileName();

  {
    Ogg::FLAC::File f(newname.c_str());
    f.tag()->setArtist("The Artist");
    f.save();
  }
  {
    Ogg::FLAC::File f(newname.c_str());
    ASSERT_EQ(String("The Artist"), f.tag()->artist());

    f.seek(0, File::End);
    ASSERT_EQ(static_cast<offset_t>(9134), f.tell());
  }
}

TEST(OggFLAC, testFuzzedFile)
{
  Ogg::FLAC::File f(TEST_FILE_PATH_C("segfault.oga"));
  ASSERT_FALSE(f.isValid());
}

TEST(OggFLAC, testSplitPackets)
{
  ScopedFileCopy copy("empty_flac", ".oga");
  string newname    = copy.fileName();

  const String text = longText(128 * 1024, true);

  {
    Ogg::FLAC::File f(newname.c_str());
    f.tag()->setTitle(text);
    f.save();
  }
  {
    Ogg::FLAC::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(141141), f.length());
    ASSERT_EQ(21, f.lastPageHeader()->pageSequenceNumber());
    ASSERT_EQ(51U, f.packet(0).size());
    ASSERT_EQ(131126U, f.packet(1).size());
    ASSERT_EQ(22U, f.packet(2).size());
    ASSERT_EQ(8196U, f.packet(3).size());
    ASSERT_EQ(text, f.tag()->title());

    ASSERT_TRUE(f.audioProperties());
    ASSERT_EQ(3705, f.audioProperties()->lengthInMilliseconds());

    f.tag()->setTitle("ABCDE");
    f.save();
  }
  {
    Ogg::FLAC::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(9128), f.length());
    ASSERT_EQ(5, f.lastPageHeader()->pageSequenceNumber());
    ASSERT_EQ(51U, f.packet(0).size());
    ASSERT_EQ(59U, f.packet(1).size());
    ASSERT_EQ(22U, f.packet(2).size());
    ASSERT_EQ(8196U, f.packet(3).size());
    ASSERT_EQ(String("ABCDE"), f.tag()->title());

    ASSERT_TRUE(f.audioProperties());
    ASSERT_EQ(3705, f.audioProperties()->lengthInMilliseconds());
  }
}
