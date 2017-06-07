/***************************************************************************
    copyright           : (C) 2015 by Tsuda Kageyu
    email               : tsuda.kageyu@gmail.com
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
#include <speexfile.h>
#include <oggpageheader.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Ogg Speex File")
{
  SECTION("Read audio properties")
  {
    Ogg::Speex::File f(TEST_FILE_PATH_C("empty.spx"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3685);
    REQUIRE(f.audioProperties()->bitrate() == 53);
    REQUIRE(f.audioProperties()->bitrateNominal() == -1);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
  }
  SECTION("Split and merge packets")
  {
    const ScopedFileCopy copy("empty", ".spx");
    const String text = longText(128 * 1024, true);
    {
      Ogg::Speex::File f(copy.fileName().c_str());
      f.tag()->setTitle(text);
      f.save();
    }
    {
      Ogg::Speex::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.length() == 156330);
      REQUIRE(f.lastPageHeader()->pageSequenceNumber() == 23);
      REQUIRE(f.packet(0).size() == 80);
      REQUIRE(f.packet(1).size() == 131116);
      REQUIRE(f.packet(2).size() == 93);
      REQUIRE(f.packet(3).size() == 93);
      REQUIRE(f.tag()->title() == text);

      REQUIRE(f.audioProperties());
      REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3685);

      f.tag()->setTitle("ABCDE");
      f.save();
    }
    {
      Ogg::Speex::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.length() == 24317);
      REQUIRE(f.lastPageHeader()->pageSequenceNumber() == 7);
      REQUIRE(f.packet(0).size() == 80);
      REQUIRE(f.packet(1).size() == 49);
      REQUIRE(f.packet(2).size() == 93);
      REQUIRE(f.packet(3).size() == 93);
      REQUIRE(f.tag()->title() == "ABCDE");

      REQUIRE(f.audioProperties());
      REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3685);
    }
  }
}
