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
#include <wavfile.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("WAV File")
{
  SECTION("Read audio properties (PCM)")
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("empty.wav"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3675);
    REQUIRE(f.audioProperties()->bitrate() == 32);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 1000);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->sampleWidth() == 16);
    REQUIRE(f.audioProperties()->sampleFrames() == 3675);
    REQUIRE(f.audioProperties()->format() == 1);
  }
  SECTION("Read audio properties (PCM with 'fact' chunk)")
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("pcm_with_fact_chunk.wav"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3675);
    REQUIRE(f.audioProperties()->bitrate() == 32);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 1000);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->sampleWidth() == 16);
    REQUIRE(f.audioProperties()->sampleFrames() == 3675);
    REQUIRE(f.audioProperties()->format() == 1);
  }
  SECTION("Read audio properties (ALAW)")
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("alaw.wav"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3550);
    REQUIRE(f.audioProperties()->bitrate() == 128);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 8000);
    REQUIRE(f.audioProperties()->bitsPerSample() == 8);
    REQUIRE(f.audioProperties()->sampleWidth() == 8);
    REQUIRE(f.audioProperties()->sampleFrames() == 28400);
    REQUIRE(f.audioProperties()->format() == 6);
  }
  SECTION("Read audio properties (Floating point)")
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("float64.wav"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 0);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 0);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 97);
    REQUIRE(f.audioProperties()->bitrate() == 5645);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->bitsPerSample() == 64);
    REQUIRE(f.audioProperties()->sampleWidth() == 64);
    REQUIRE(f.audioProperties()->sampleFrames() == 4281);
    REQUIRE(f.audioProperties()->format() == 3);
  }
  SECTION("Treat zero-sized 'data' chunk as error")
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("zero-size-chunk.wav"));
    REQUIRE_FALSE(f.isValid());
  }
  SECTION("Read and write ID3v2 tag")
  {
    const ScopedFileCopy copy("empty", ".wav");
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE_FALSE(f.hasID3v2Tag());

      f.ID3v2Tag()->setTitle("Title");
      f.ID3v2Tag()->setArtist("Artist");
      f.save();
      REQUIRE(f.hasID3v2Tag());
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.ID3v2Tag()->title() == "Title");
      REQUIRE(f.ID3v2Tag()->artist() == "Artist");

      f.ID3v2Tag()->setTitle("");
      f.ID3v2Tag()->setArtist("");
      f.save();
      REQUIRE_FALSE(f.hasID3v2Tag());
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE_FALSE(f.hasID3v2Tag());
      REQUIRE(f.ID3v2Tag()->title().isEmpty());
      REQUIRE(f.ID3v2Tag()->artist().isEmpty());
    }
  }
  SECTION("Read and write INFO tag")
  {
    const ScopedFileCopy copy("empty", ".wav");
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE_FALSE(f.hasInfoTag());

      f.InfoTag()->setTitle("Title");
      f.InfoTag()->setArtist("Artist");
      f.save();
      REQUIRE(f.hasInfoTag());
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.hasInfoTag());
      REQUIRE(f.InfoTag()->title() == "Title");
      REQUIRE(f.InfoTag()->artist() == "Artist");

      f.InfoTag()->setTitle("");
      f.InfoTag()->setArtist("");
      f.save();
      REQUIRE_FALSE(f.hasInfoTag());
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE_FALSE(f.hasInfoTag());
      REQUIRE(f.InfoTag()->title().isEmpty());
      REQUIRE(f.InfoTag()->artist().isEmpty());
    }
  }
  SECTION("Create and strip tags")
  {
    const ScopedFileCopy copy("empty", ".wav");
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      f.ID3v2Tag()->setTitle("test title");
      f.InfoTag()->setTitle("test title");
      f.save();
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.hasInfoTag());
      f.save(RIFF::WAV::File::ID3v2, true);
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE_FALSE(f.hasInfoTag());
      f.ID3v2Tag()->setTitle("test title");
      f.InfoTag()->setTitle("test title");
      f.save();
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.hasInfoTag());
      f.save(RIFF::WAV::File::Info, true);
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());
      REQUIRE(f.hasInfoTag());
    }
  }
  SECTION("Skip and remove duplicate tags")
  {
    const ScopedFileCopy copy("duplicate_tags", ".wav");

    RIFF::WAV::File f(copy.fileName().c_str());
    REQUIRE(f.length() == 17052);

    // duplicate_tags.wav has duplicate ID3v2/INFO tags.
    // title() returns "Title2" if can't skip the second tag.

    REQUIRE(f.hasID3v2Tag());
    REQUIRE(f.ID3v2Tag()->title() == "Title1");

    REQUIRE(f.hasInfoTag());
    REQUIRE(f.InfoTag()->title() == "Title1");

    f.save();
    REQUIRE(f.length() == 15898);
    REQUIRE(f.find("Title2") == -1);
  }
  SECTION("Read properties correctly after stripping tags")
  {
    const ScopedFileCopy copy("empty", ".wav");
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      f.ID3v2Tag()->setTitle("ID3v2");
      f.InfoTag()->setTitle("INFO");
      f.save();
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      REQUIRE(f.properties()["TITLE"].front() == "ID3v2");
      f.strip(RIFF::WAV::File::ID3v2);
      REQUIRE(f.properties()["TITLE"].front() == "INFO");
      f.strip(RIFF::WAV::File::Info);
      REQUIRE(f.properties().isEmpty());
    }
  }
  SECTION("Open fuzzed file without crashing (1)")
  {
    RIFF::WAV::File f1(TEST_FILE_PATH_C("infloop.wav"));
    REQUIRE_FALSE(f1.isValid());
  }
  SECTION("Open fuzzed file without crashing (2)")
  {
    RIFF::WAV::File f2(TEST_FILE_PATH_C("segfault.wav"));
    REQUIRE(f2.isValid());
  }
}
