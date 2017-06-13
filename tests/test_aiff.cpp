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

#include <catch/catch.hpp>
#include <aifffile.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("AIFF File")
{
  SECTION("Read audio properties (AIFF)")
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("empty.aiff"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 0);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 0);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 67);
    REQUIRE(f.audioProperties()->bitrate() == 706);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->channels() == 1);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->sampleWidth() == 16);
    REQUIRE(f.audioProperties()->sampleFrames() == 2941);
    REQUIRE_FALSE(f.audioProperties()->isAiffC());
  }
  SECTION("Read audio properties (AIFF-C)")
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("alaw.aifc"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 0);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 0);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 37);
    REQUIRE(f.audioProperties()->bitrate() == 355);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->channels() == 1);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->sampleWidth() == 16);
    REQUIRE(f.audioProperties()->sampleFrames() == 1622);
    REQUIRE(f.audioProperties()->isAiffC());
    REQUIRE(f.audioProperties()->compressionType() == "ALAW");
    REQUIRE(f.audioProperties()->compressionName() == "SGI CCITT G.711 A-law");
  }
  SECTION("Create and update ID3v2 tag")
  {
    const ScopedFileCopy copy("empty", ".aiff");
    {
      RIFF::AIFF::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());

      f.tag()->setTitle(L"TitleXXX");
      f.save();
      REQUIRE(f.hasID3v2Tag());
    }
    {
      RIFF::AIFF::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.tag()->title() == "TitleXXX");

      f.tag()->setTitle("TitleYYY");
      f.save();
      REQUIRE(f.hasID3v2Tag());
    }
    {
      RIFF::AIFF::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.tag()->title() == "TitleYYY");

      f.tag()->setTitle("");
      f.save();
      REQUIRE_FALSE(f.hasID3v2Tag());
    }
    {
      RIFF::AIFF::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());
    }
  }
  SECTION("Skip and remove duplicate ID3v2 tags")
  {
    const ScopedFileCopy copy("duplicate_id3v2", ".aiff");

    // duplicate_id3v2.aiff has duplicate ID3v2 tag chunks.
    // title() returns "Title2" if can't skip the second tag.

    RIFF::AIFF::File f(copy.fileName().c_str());
    REQUIRE(f.hasID3v2Tag());
    REQUIRE(f.tag()->title() == "Title1");

    f.save();
    REQUIRE(f.length() == 7030);
    REQUIRE(f.find("Title2") == -1);
  }
  SECTION("Open fuzzed file without crashing (1)")
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("segfault.aif"));
    REQUIRE_FALSE(f.isValid());
  }
  SECTION("Open fuzzed file without crashing (2)")
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("excessive_alloc.aif"));
    REQUIRE_FALSE(f.isValid());
  }
}
