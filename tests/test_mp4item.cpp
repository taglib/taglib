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
#include <mp4item.h>
#include <mp4coverart.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("MP4 Item")
{
  SECTION("Copy cover art")
  {
    MP4::CoverArt c(MP4::CoverArt::PNG, "foo");
    REQUIRE(c.format() == MP4::CoverArt::PNG);
    REQUIRE(c.data() == "foo");

    MP4::CoverArt c2(c);
    REQUIRE(c2.format() == MP4::CoverArt::PNG);
    REQUIRE(c2.data() == "foo");

    MP4::CoverArt c3 = c;
    REQUIRE(c3.format() == MP4::CoverArt::PNG);
    REQUIRE(c3.data() == "foo");
  }
  SECTION("Read and write cover art list")
  {
    MP4::CoverArtList l;
    l.append(MP4::CoverArt(MP4::CoverArt::PNG, "foo"));
    l.append(MP4::CoverArt(MP4::CoverArt::JPEG, "bar"));
    
    MP4::Item i(l);
    MP4::CoverArtList l2 = i.toCoverArtList();
    
    REQUIRE(l[0].format() == MP4::CoverArt::PNG);
    REQUIRE(l[0].data() == "foo");
    REQUIRE(l[1].format() == MP4::CoverArt::JPEG);
    REQUIRE(l[1].data() == "bar");
  }
}
