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
#include <trueaudiofile.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("TrueAudio File")
{
  SECTION("Read audio properties (Without ID3v2 tag)")
  {
    TrueAudio::File f(TEST_FILE_PATH_C("empty.tta"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3685);
    REQUIRE(f.audioProperties()->bitrate() == 173);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->sampleFrames() == 162496);
    REQUIRE(f.audioProperties()->ttaVersion() == 1);
  }
  SECTION("Read audio properties (With tags)")
  {
    TrueAudio::File f(TEST_FILE_PATH_C("tagged.tta"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3685);
    REQUIRE(f.audioProperties()->bitrate() == 173);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->sampleFrames() == 162496);
    REQUIRE(f.audioProperties()->ttaVersion() == 1);
  }
  SECTION("Read property map correctly after stripping tags")
  {
    const ScopedFileCopy copy("empty", ".tta");
    {
      TrueAudio::File f(copy.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("ID3v2");
      f.ID3v1Tag(true)->setTitle("ID3v1");
      f.save();
    }
    {
      TrueAudio::File f(copy.fileName().c_str());
      REQUIRE(f.properties()["TITLE"].front() == String("ID3v2"));
      f.strip(TrueAudio::File::ID3v2);
      REQUIRE(f.properties()["TITLE"].front() == String("ID3v1"));
      f.strip(TrueAudio::File::ID3v1);
      REQUIRE(f.properties().isEmpty());
    }
  }
  SECTION("Save tags repeatedly without breaking file")
  {
    const ScopedFileCopy copy("empty", ".tta");
    {
      TrueAudio::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());
      REQUIRE_FALSE(f.hasID3v1Tag());

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
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.hasID3v1Tag());
    }
  }
}
