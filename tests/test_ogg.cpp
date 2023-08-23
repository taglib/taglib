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
#include "oggpageheader.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "utils.h"
#include "vorbisfile.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(Ogg, testSimple)
{
  ScopedFileCopy copy("empty", ".ogg");
  string newname = copy.fileName();

  {
    Vorbis::File f(newname.c_str());
    f.tag()->setArtist("The Artist");
    f.save();
  }
  {
    Vorbis::File f(newname.c_str());
    ASSERT_EQ(String("The Artist"), f.tag()->artist());
  }
}

TEST(Ogg, testSplitPackets1)
{
  ScopedFileCopy copy("empty", ".ogg");
  string newname    = copy.fileName();

  const String text = longText(128 * 1024, true);

  {
    Vorbis::File f(newname.c_str());
    f.tag()->setTitle(text);
    f.save();
  }
  {
    Vorbis::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(136383), f.length());
    ASSERT_EQ(19, f.lastPageHeader()->pageSequenceNumber());
    ASSERT_EQ(30U, f.packet(0).size());
    ASSERT_EQ(131127U, f.packet(1).size());
    ASSERT_EQ(3832U, f.packet(2).size());
    ASSERT_EQ(text, f.tag()->title());

    ASSERT_TRUE(f.audioProperties());
    ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());

    f.tag()->setTitle("ABCDE");
    f.save();
  }
  {
    Vorbis::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(4370), f.length());
    ASSERT_EQ(3, f.lastPageHeader()->pageSequenceNumber());
    ASSERT_EQ(30U, f.packet(0).size());
    ASSERT_EQ(60U, f.packet(1).size());
    ASSERT_EQ(3832U, f.packet(2).size());
    ASSERT_EQ(String("ABCDE"), f.tag()->title());

    ASSERT_TRUE(f.audioProperties());
    ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());
  }
}

TEST(Ogg, testSplitPackets2)
{
  ScopedFileCopy copy("empty", ".ogg");
  string newname    = copy.fileName();

  const String text = longText(60890, true);

  {
    Vorbis::File f(newname.c_str());
    f.tag()->setTitle(text);
    f.save();
  }
  {
    Vorbis::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(text, f.tag()->title());

    f.tag()->setTitle("ABCDE");
    f.save();
  }
  {
    Vorbis::File f(newname.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(String("ABCDE"), f.tag()->title());
  }
}

TEST(Ogg, testDictInterface1)
{
  ScopedFileCopy copy("empty", ".ogg");
  string newname = copy.fileName();

  Vorbis::File f(newname.c_str());

  ASSERT_EQ(static_cast<unsigned int>(0), f.tag()->properties().size());

  PropertyMap newTags;
  StringList values("value 1");
  values.append("value 2");
  newTags["ARTIST"] = values;
  f.tag()->setProperties(newTags);

  PropertyMap map = f.tag()->properties();
  ASSERT_EQ(static_cast<unsigned int>(1), map.size());
  ASSERT_EQ(static_cast<unsigned int>(2), map["ARTIST"].size());
  ASSERT_EQ(String("value 1"), map["ARTIST"][0]);
}

TEST(Ogg, testDictInterface2)
{
  ScopedFileCopy copy("test", ".ogg");
  string newname = copy.fileName();

  Vorbis::File f(newname.c_str());
  PropertyMap tags = f.tag()->properties();

  ASSERT_EQ(static_cast<unsigned int>(2), tags["UNUSUALTAG"].size());
  ASSERT_EQ(String("usual value"), tags["UNUSUALTAG"][0]);
  ASSERT_EQ(String("another value"), tags["UNUSUALTAG"][1]);
  ASSERT_EQ(
    String("\xC3\xB6\xC3\xA4\xC3\xBC\x6F\xCE\xA3\xC3\xB8", String::UTF8),
    tags["UNICODETAG"][0]);

  tags["UNICODETAG"][0] = String(
    "\xCE\xBD\xCE\xB5\xCF\x89\x20\xCE\xBD\xCE\xB1\xCE\xBB\xCF\x85\xCE\xB5", String::UTF8);
  tags.erase("UNUSUALTAG");
  f.tag()->setProperties(tags);
  ASSERT_EQ(
    String("\xCE\xBD\xCE\xB5\xCF\x89\x20\xCE\xBD\xCE\xB1\xCE\xBB\xCF\x85\xCE\xB5", String::UTF8),
    f.tag()->properties()["UNICODETAG"][0]);
  ASSERT_FALSE(f.tag()->properties().contains("UNUSUALTAG"));
}

TEST(Ogg, testAudioProperties)
{
  Ogg::Vorbis::File f(TEST_FILE_PATH_C("empty.ogg"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(1, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(0, f.audioProperties()->vorbisVersion());
  ASSERT_EQ(0, f.audioProperties()->bitrateMaximum());
  ASSERT_EQ(112000, f.audioProperties()->bitrateNominal());
  ASSERT_EQ(0, f.audioProperties()->bitrateMinimum());
}

TEST(Ogg, testPageChecksum)
{
  ScopedFileCopy copy("empty", ".ogg");

  {
    Vorbis::File f(copy.fileName().c_str());
    f.tag()->setArtist("The Artist");
    f.save();

    f.seek(0x50);
    ASSERT_EQ(static_cast<unsigned int>(0x3d3bd92d), f.readBlock(4).toUInt(0, true));
  }
  {
    Vorbis::File f(copy.fileName().c_str());
    f.tag()->setArtist("The Artist 2");
    f.save();

    f.seek(0x50);
    ASSERT_EQ(static_cast<unsigned int>(0xd985291c), f.readBlock(4).toUInt(0, true));
  }
}

TEST(Ogg, testPageGranulePosition)
{
  ScopedFileCopy copy("empty", ".ogg");
  {
    Vorbis::File f(copy.fileName().c_str());
    // Force the Vorbis comment packet to span more than one page and
    // check if the granule position is -1 indicating that no packets
    // finish on this page.
    f.tag()->setComment(String(ByteVector(70000, 'A')));
    f.save();

    f.seek(0x3a);
    ASSERT_EQ(ByteVector("OggS\0\0", 6), f.readBlock(6));
    ASSERT_EQ(static_cast<long long>(-1), f.readBlock(8).toLongLong());
  }
  {
    Vorbis::File f(copy.fileName().c_str());
    // Use a small Vorbis comment package which ends on the seconds page and
    // check if the granule position is zero.
    f.tag()->setComment("A small comment");
    f.save();

    f.seek(0x3a);
    ASSERT_EQ(ByteVector("OggS\0\0", 6), f.readBlock(6));
    ASSERT_EQ(static_cast<long long>(0), f.readBlock(8).toLongLong());
  }
}
