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
#include <mpcfile.h>
#include <apetag.h>
#include <id3v1tag.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Musepack File")
{
  SECTION("Read audio properties (SV8)")
  {
    MPC::File f(TEST_FILE_PATH_C("sv8_header.mpc"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->mpcVersion() == 8);
    REQUIRE(f.audioProperties()->length() == 1);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 1);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 1497);
    REQUIRE(f.audioProperties()->bitrate() == 1);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->sampleFrames() == 66014);
  }
  SECTION("Read audio properties (SV7)")
  {
    MPC::File f(TEST_FILE_PATH_C("click.mpc"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->mpcVersion() == 7);
    REQUIRE(f.audioProperties()->length() == 0);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 0);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 40);
    REQUIRE(f.audioProperties()->bitrate() == 318);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->sampleFrames() == 1760);
    REQUIRE(f.audioProperties()->trackGain() == 14221);
    REQUIRE(f.audioProperties()->trackPeak() == 19848);
    REQUIRE(f.audioProperties()->albumGain() == 14221);
    REQUIRE(f.audioProperties()->albumPeak() == 19848);
  }
  SECTION("Read audio properties (SV5)")
  {
    MPC::File f(TEST_FILE_PATH_C("sv5_header.mpc"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->mpcVersion() == 5);
    REQUIRE(f.audioProperties()->length() == 26);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 26);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 26371);
    REQUIRE(f.audioProperties()->bitrate() == 0);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->sampleFrames() == 1162944);
  }
  SECTION("Read audio properties (SV4)")
  {
    MPC::File f(TEST_FILE_PATH_C("sv4_header.mpc"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->mpcVersion() == 4);
    REQUIRE(f.audioProperties()->length() == 26);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 26);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 26371);
    REQUIRE(f.audioProperties()->bitrate() == 0);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->sampleFrames() == 1162944);
  }
  SECTION("Open fuzzed file without crashing (1)")
  {
    MPC::File f(TEST_FILE_PATH_C("zerodiv.mpc"));
    REQUIRE(f.isValid());
  }
  SECTION("Open fuzzed file without crashing (2)")
  {
    MPC::File f(TEST_FILE_PATH_C("infloop.mpc"));
    REQUIRE(f.isValid());
  }
  SECTION("Open fuzzed file without crashing (3)")
  {
    MPC::File f(TEST_FILE_PATH_C("segfault.mpc"));
    REQUIRE(f.isValid());
  }
  SECTION("Open fuzzed file without crashing (4)")
  {
    MPC::File f(TEST_FILE_PATH_C("segfault2.mpc"));
    REQUIRE(f.isValid());
  }
  SECTION("Read properties correctly after stripping tags")
  {
    const ScopedFileCopy copy("click", ".mpc");
    {
      MPC::File f(copy.fileName().c_str());
      f.APETag(true)->setTitle("APE");
      f.ID3v1Tag(true)->setTitle("ID3v1");
      f.save();
    }
    {
      MPC::File f(copy.fileName().c_str());
      REQUIRE(f.properties()["TITLE"].front() == "APE");
      f.strip(MPC::File::APE);
      REQUIRE(f.properties()["TITLE"].front() == "ID3v1");
      f.strip(MPC::File::ID3v1);
      REQUIRE(f.properties().isEmpty());
    }
  }
  SECTION("Save tags repeatedly without breaking file")
  {
    const ScopedFileCopy copy("click", ".mpc");
    {
      MPC::File f(copy.fileName().c_str());
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
      MPC::File f(copy.fileName().c_str());
      REQUIRE(f.hasAPETag());
      REQUIRE(f.hasID3v1Tag());
    }
  }
}
