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
#include <id3v2synchdata.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("ID3v2 SynchData") 
{
  SECTION("Decode and encode normal integers")
  {
    const ByteVector v1("\x00\x00\x00\x7F", 4);
    REQUIRE(ID3v2::SynchData::toUInt(v1) == 127);
    REQUIRE(ID3v2::SynchData::fromUInt(127) == v1);

    const ByteVector v2("\x00\x00\x01\x00", 4);
    REQUIRE(ID3v2::SynchData::toUInt(v2) == 128);
    REQUIRE(ID3v2::SynchData::fromUInt(128) == v2);

    const ByteVector v3("\x00\x00\x01\x01", 4);
    REQUIRE(ID3v2::SynchData::toUInt(v3) == 129);
    REQUIRE(ID3v2::SynchData::fromUInt(129) == v3);
  }
  SECTION("Decode broken and/or too large integers")
  {
    REQUIRE(ID3v2::SynchData::toUInt(ByteVector("\x00\x00\x00\xFF", 4)) == 255);
    REQUIRE(ID3v2::SynchData::toUInt(ByteVector("\x00\x00\xFF\xFF", 4)) == 65535);
    REQUIRE(ID3v2::SynchData::toUInt(ByteVector("\x00\x00\x01\x00\x00", 5)) == 128);
    REQUIRE(ID3v2::SynchData::toUInt(ByteVector("\x00\x00\x00\xFF\x00", 5)) == 255);
  }
  SECTION("Decode unsynchronised data")
  {
    REQUIRE(ID3v2::SynchData::decode(ByteVector("\xFF\x00\x00", 3)) == ByteVector("\xFF\x00", 2));
    REQUIRE(ID3v2::SynchData::decode(ByteVector("\xFF\x44\x00", 3)) == ByteVector("\xFF\x44\x00", 3));
    REQUIRE(ID3v2::SynchData::decode(ByteVector("\xFF\xFF\x00", 3)) == ByteVector("\xFF\xFF", 2));
    REQUIRE(ID3v2::SynchData::decode(ByteVector("\xFF\xFF\xFF", 3)) == ByteVector("\xFF\xFF\xFF", 3));
  }
}
