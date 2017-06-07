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
#include <oggflacfile.h>
#include <oggpageheader.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Ogg FLAC File")
{
  SECTION("Read audio properties")
  {
    Ogg::FLAC::File f(TEST_FILE_PATH_C("empty_flac.oga"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3705);
    REQUIRE(f.audioProperties()->bitrate() == 2);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->sampleWidth() == 16);
    REQUIRE(f.audioProperties()->sampleFrames() == 163392);
    REQUIRE(f.audioProperties()->signature().toHex() == "e64b8cd44ac2a3a9ba1d53fc79b17ed1");
  }
  SECTION("Read and write simple values")
  {
    const ScopedFileCopy copy("empty_flac", ".oga");
    {
      Ogg::FLAC::File f(copy.fileName().c_str());
      f.tag()->setArtist("The Artist");
      f.save();
    }
    {
      Ogg::FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->artist() == "The Artist");

      f.seek(0, File::End);
      REQUIRE(f.tell() == 9134);
    }
  }
  SECTION("Split and merge packets")
  {
    const ScopedFileCopy copy("empty_flac", ".oga");
    const String text = longText(128 * 1024, true);
    {
      Ogg::FLAC::File f(copy.fileName().c_str());
      f.tag()->setTitle(text);
      f.save();
    }
    {
      Ogg::FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.length() == 141141);
      REQUIRE(f.lastPageHeader()->pageSequenceNumber() == 21);
      REQUIRE(f.packet(0).size() == 51);
      REQUIRE(f.packet(1).size() == 131126);
      REQUIRE(f.packet(2).size() == 22);
      REQUIRE(f.packet(3).size() == 8196);
      REQUIRE(f.tag()->title() == text);

      REQUIRE(f.audioProperties());
      REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3705);

      f.tag()->setTitle("ABCDE");
      f.save();
    }
    {
      Ogg::FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.length() == 9128);
      REQUIRE(f.lastPageHeader()->pageSequenceNumber() == 5);
      REQUIRE(f.packet(0).size() == 51);
      REQUIRE(f.packet(1).size() == 59);
      REQUIRE(f.packet(2).size() == 22);
      REQUIRE(f.packet(3).size() == 8196);
      REQUIRE(f.tag()->title() == "ABCDE");

      REQUIRE(f.audioProperties());
      REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3705);
    }
  }
  SECTION("Open fuzzed file without crashing")
  {
    Ogg::FLAC::File f(TEST_FILE_PATH_C("segfault.oga"));
    REQUIRE_FALSE(f.isValid());
  }
}
