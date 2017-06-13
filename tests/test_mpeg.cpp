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

#include <catch/catch.hpp>
#include <mpegfile.h>
#include <xingheader.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <apetag.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("MPEG File")
{
  SECTION("Read audio properties (CBR with Xing header)")
  {
    MPEG::File f(TEST_FILE_PATH_C("lame_cbr.mp3"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 1887);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 1887);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 1887164);
    REQUIRE(f.audioProperties()->bitrate() == 64);
    REQUIRE(f.audioProperties()->channels() == 1);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->xingHeader()->type() == MPEG::XingHeader::Xing);
  }
  SECTION("Read audio properties (VBR with Xing header)")
  {
    MPEG::File f(TEST_FILE_PATH_C("lame_vbr.mp3"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 1887);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 1887);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 1887164);
    REQUIRE(f.audioProperties()->bitrate() == 70);
    REQUIRE(f.audioProperties()->channels() == 1);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->xingHeader()->type() == MPEG::XingHeader::Xing);
  }
  SECTION("Read audio properties (VBR with VBRI header)")
  {
    MPEG::File f(TEST_FILE_PATH_C("rare_frames.mp3"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 222);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 222);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 222198);
    REQUIRE(f.audioProperties()->bitrate() == 233);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->xingHeader()->type() == MPEG::XingHeader::VBRI);
  }
  SECTION("Read audio properties (CBR without VBR headers)")
  {
    MPEG::File f(TEST_FILE_PATH_C("bladeenc.mp3"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3553);
    REQUIRE(f.audioProperties()->bitrate() == 64);
    REQUIRE(f.audioProperties()->channels() == 1);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE_FALSE(f.audioProperties()->xingHeader());

    const long last = f.lastFrameOffset();
    const MPEG::Header lastHeader(&f, last, false);

    REQUIRE(last == 28213);
    REQUIRE(lastHeader.frameLength() == 209);
  }
  SECTION("Read audio properties (MPEG2 with Xing header)")
  {
    MPEG::File f(TEST_FILE_PATH_C("mpeg2.mp3"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 5387);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 5387);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 5387285);
    REQUIRE(f.audioProperties()->bitrate() == 25);
    REQUIRE(f.audioProperties()->channels() == 1);
    REQUIRE(f.audioProperties()->sampleRate() == 22050);
    REQUIRE(f.audioProperties()->xingHeader()->type() == MPEG::XingHeader::Xing);
  }
  SECTION("Skip broken MPEG frames (1)")
  {
    MPEG::File f(TEST_FILE_PATH_C("invalid-frames1.mp3"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 0);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 0);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 392);
    REQUIRE(f.audioProperties()->bitrate() == 160);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE_FALSE(f.audioProperties()->xingHeader());
  }
  SECTION("Skip broken MPEG frames (2)")
  {
    MPEG::File f(TEST_FILE_PATH_C("invalid-frames2.mp3"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 0);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 0);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 314);
    REQUIRE(f.audioProperties()->bitrate() == 192);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE_FALSE(f.audioProperties()->xingHeader());
  }
  SECTION("Skip broken MPEG frames (3)")
  {
    MPEG::File f(TEST_FILE_PATH_C("invalid-frames3.mp3"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 0);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 0);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 183);
    REQUIRE(f.audioProperties()->bitrate() == 320);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE_FALSE(f.audioProperties()->xingHeader());
  }
  SECTION("Skip garbage and update ID3v2 tag correctly")
  {
    const ScopedFileCopy copy("garbage", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.firstFrameOffset() == 2255);
      REQUIRE(f.lastFrameOffset() == 6015);
      REQUIRE(f.ID3v2Tag()->title() == "Title A");
      f.ID3v2Tag()->setTitle("Title B");
      f.save();
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.ID3v2Tag()->title() == "Title B");
    }
  }
  SECTION("Save ID3v2.4 tag")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    const String xxx = longText(254);
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(MPEG::File::AllTags, true, 4);
      REQUIRE(f.hasID3v2Tag());
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.ID3v2Tag()->header()->majorVersion() == 4);
      REQUIRE(f.tag()->artist() == "Artist A");
      REQUIRE(f.tag()->title() == xxx);
    }
  }
  SECTION("Save ID3v2.4 tag with wrong parameter")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    const String xxx = longText(254);
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(MPEG::File::AllTags, true, 8);
      REQUIRE(f.hasID3v2Tag());
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.ID3v2Tag()->header()->majorVersion() == 4);
      REQUIRE(f.tag()->artist() == "Artist A");
      REQUIRE(f.tag()->title() == xxx);
    }
  }
  SECTION("Save ID3v2.3 tag")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    const String xxx = longText(254);
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(MPEG::File::AllTags, true, 3);
      REQUIRE(f.hasID3v2Tag());
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.ID3v2Tag()->header()->majorVersion() == (unsigned int)3);
      REQUIRE(f.tag()->artist() == "Artist A");
      REQUIRE(f.tag()->title() == xxx);
    }
  }
  SECTION("Skip duplicate ID3v2 tags")
  {
    MPEG::File f(TEST_FILE_PATH_C("duplicate_id3v2.mp3"));

    // duplicate_id3v2.mp3 has duplicate ID3v2 tags.
    // Sample rate will be 32000 if can't skip the second tag.

    REQUIRE(f.hasID3v2Tag());
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
  }
  SECTION("Open fuzzed file without crashing")
  {
    MPEG::File f(TEST_FILE_PATH_C("excessive_alloc.mp3"));
    REQUIRE(f.isValid());
  }
  SECTION("Locate MPEG frames in tagged files")
  {
    {
      MPEG::File f(TEST_FILE_PATH_C("ape.mp3"));
      REQUIRE(f.isValid());
      REQUIRE(f.firstFrameOffset() == 0x0000);
      REQUIRE(f.lastFrameOffset() == 0x1FD6);
    }
    {
      MPEG::File f(TEST_FILE_PATH_C("ape-id3v1.mp3"));
      REQUIRE(f.isValid());
      REQUIRE(f.firstFrameOffset() == 0x0000);
      REQUIRE(f.lastFrameOffset() == 0x1FD6);
    }
    {
      MPEG::File f(TEST_FILE_PATH_C("ape-id3v2.mp3"));
      REQUIRE(f.isValid());
      REQUIRE(f.firstFrameOffset() == 0x041A);
      REQUIRE(f.lastFrameOffset() == 0x23F0);
    }
  }
  SECTION("Read property map correctly after stripping tags")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("ID3v2");
      f.APETag(true)->setTitle("APE");
      f.ID3v1Tag(true)->setTitle("ID3v1");
      f.save();
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.properties()["TITLE"].front() == "ID3v2");
      f.strip(MPEG::File::ID3v2);
      REQUIRE(f.properties()["TITLE"].front() == "APE");
      f.strip(MPEG::File::APE);
      REQUIRE(f.properties()["TITLE"].front() == "ID3v1");
      f.strip(MPEG::File::ID3v1);
      REQUIRE(f.properties().isEmpty());
    }
  }
  SECTION("Save tags repeatedly without breaking file (1)")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    const String text = longText(4096);
    {
      MPEG::File f(copy.fileName().c_str());
      f.ID3v2Tag(true)->setTitle(text);
      f.save();
    }
    {
      MPEG::File f(copy.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("");
      f.save();
      f.ID3v2Tag(true)->setTitle(text);
      f.save();
      REQUIRE(f.firstFrameOffset() == 5141);
    }
  }
  SECTION("Save tags repeatedly without breaking file (2)")
  {
    const ScopedFileCopy copy("xing", ".mp3");

    MPEG::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle("0123456789");
    f.save();
    f.save();
    REQUIRE(f.find("ID3", 3) == -1);
  }
  SECTION("Save tags repeatedly without breaking file (3)")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasAPETag());
      REQUIRE_FALSE(f.hasID3v1Tag());

      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.APETag()->setTitle("0");
      f.save();
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.APETag()->setTitle("01234 56789 ABCDE FGHIJ 01234 56789 ABCDE FGHIJ 01234 56789");
      f.save();
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasAPETag());
      REQUIRE(f.hasID3v1Tag());
    }
  }
  SECTION("Create and remove ID3v2 tag")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());
      f.ID3v2Tag(true)->setTitle("0123456789");
      f.save(MPEG::File::ID3v2);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      f.ID3v2Tag(true)->setTitle("");
      f.save(MPEG::File::ID3v2, false);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());
    }
  }
  SECTION("Create and remove ID3v1 tag")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v1Tag());
      f.ID3v1Tag(true)->setTitle("0123456789");
      f.save(MPEG::File::ID3v1);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v1Tag());
      f.ID3v1Tag(true)->setTitle("");
      f.save(MPEG::File::ID3v1, false);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v1Tag());
    }
  }
  SECTION("Create and remove APE tag")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasAPETag());
      f.APETag(true)->setTitle("0123456789");
      f.save(MPEG::File::APE);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasAPETag());
      f.APETag(true)->setTitle("");
      f.save(MPEG::File::APE, false);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasAPETag());
    }
  }
}
