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

#include <catch/catch.hpp>
#include <wavpackfile.h>
#include <apetag.h>
#include <id3v1tag.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Wavpack File")
{
  SECTION("Read audio properties (No length)")
  {
    WavPack::File f(TEST_FILE_PATH_C("no_length.wv"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3705);
    REQUIRE(f.audioProperties()->bitrate() == 1);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->isLossless());
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->sampleFrames() == 163392);
    REQUIRE(f.audioProperties()->version() == 1031);
  }
  SECTION("Read audio properties (Multi channel)")
  {
    WavPack::File f(TEST_FILE_PATH_C("four_channels.wv"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3833);
    REQUIRE(f.audioProperties()->bitrate() == 112);
    REQUIRE(f.audioProperties()->channels() == 4);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE_FALSE(f.audioProperties()->isLossless());
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->sampleFrames() == 169031);
    REQUIRE(f.audioProperties()->version() == 1031);
  }
  SECTION("Read audio properties (With tag)")
  {
    WavPack::File f(TEST_FILE_PATH_C("tagged.wv"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3550);
    REQUIRE(f.audioProperties()->bitrate() == 172);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE_FALSE(f.audioProperties()->isLossless());
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->sampleFrames() == 156556);
    REQUIRE(f.audioProperties()->version() == 1031);
  }
  SECTION("Open fuzzed file without crashing")
  {
    WavPack::File f(TEST_FILE_PATH_C("infloop.wv"));
    REQUIRE(f.isValid());
  }
  SECTION("Read properties correctly after stripping tags")
  {
    const ScopedFileCopy copy("click", ".wv");
    {
      WavPack::File f(copy.fileName().c_str());
      f.APETag(true)->setTitle("APE");
      f.ID3v1Tag(true)->setTitle("ID3v1");
      f.save();
    }
    {
      WavPack::File f(copy.fileName().c_str());
      REQUIRE(f.properties()["TITLE"].front() == "APE");
      f.strip(WavPack::File::APE);
      REQUIRE(f.properties()["TITLE"].front() == "ID3v1");
      f.strip(WavPack::File::ID3v1);
      REQUIRE(f.properties().isEmpty());
    }
  }
  SECTION("Save tags repeatedly without breaking file")
  {
    const ScopedFileCopy copy("click", ".wv");
    {
      WavPack::File f(copy.fileName().c_str());
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
      WavPack::File f(copy.fileName().c_str());
      REQUIRE(f.hasAPETag());
      REQUIRE(f.hasID3v1Tag());
    }
  }
}
