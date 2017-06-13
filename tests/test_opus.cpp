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

#include <catch/catch.hpp>
#include <opusfile.h>
#include <oggpageheader.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Ogg Opus File")
{
  SECTION("Read audio properties")
  {
    Ogg::Opus::File f(TEST_FILE_PATH_C("correctness_gain_silent_output.opus"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 7);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 7);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 7737);
    REQUIRE(f.audioProperties()->bitrate() == 37);
    REQUIRE(f.audioProperties()->channels() == 1);
    REQUIRE(f.audioProperties()->sampleRate() == 48000);
    REQUIRE(f.audioProperties()->inputSampleRate() == 48000);
    REQUIRE(f.audioProperties()->opusVersion() == 1);
  }
  SECTION("Read and write comments")
  {
    const ScopedFileCopy copy("correctness_gain_silent_output", ".opus");
    {
      Ogg::Opus::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->fieldListMap()["ENCODER"] == StringList("Xiph.Org Opus testvectormaker"));
      REQUIRE(f.tag()->fieldListMap().contains("TESTDESCRIPTION"));
      REQUIRE_FALSE(f.tag()->fieldListMap().contains("ARTIST"));
      REQUIRE(f.tag()->vendorID() == "libopus 0.9.11-66-g64c2dd7");

      f.tag()->setArtist("Your Tester");
      f.save();
    }
    {
      Ogg::Opus::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->fieldListMap()["ENCODER"] == StringList("Xiph.Org Opus testvectormaker"));
      REQUIRE(f.tag()->fieldListMap().contains("TESTDESCRIPTION"));
      REQUIRE(f.tag()->fieldListMap()["ARTIST"] == StringList("Your Tester"));
      REQUIRE(f.tag()->vendorID() == "libopus 0.9.11-66-g64c2dd7");
    }
  }
  SECTION("Split and merge packets")
  {
    const ScopedFileCopy copy("correctness_gain_silent_output", ".opus");
    const String text = longText(128 * 1024, true);
    {
      Ogg::Opus::File f(copy.fileName().c_str());
      f.tag()->setTitle(text);
      f.save();
    }
    {
      Ogg::Opus::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.length() == 167534);
      REQUIRE(f.lastPageHeader()->pageSequenceNumber() == 27);
      REQUIRE(f.packet(0).size() == 19);
      REQUIRE(f.packet(1).size() == 131380);
      REQUIRE(f.packet(2).size() == 5);
      REQUIRE(f.packet(3).size() == 5);
      REQUIRE(f.tag()->title() == text);
    
      REQUIRE(f.audioProperties());
      REQUIRE(f.audioProperties()->lengthInMilliseconds() == 7737);
    
      f.tag()->setTitle("ABCDE");
      f.save();
    }
    {
      Ogg::Opus::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.length() == 35521);
      REQUIRE(f.lastPageHeader()->pageSequenceNumber() == 11);
      REQUIRE(f.packet(0).size() == 19);
      REQUIRE(f.packet(1).size() == 313);
      REQUIRE(f.packet(2).size() == 5);
      REQUIRE(f.packet(3).size() == 5);
      REQUIRE(f.tag()->title() == "ABCDE");
    
      REQUIRE(f.audioProperties());
      REQUIRE(f.audioProperties()->lengthInMilliseconds() == 7737);
    }
  }
}
