/***************************************************************************
    copyright           : (C) 2012 by Michael Helmling
    email               : helmling@mathematik.uni-kl.de
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
#include <tpropertymap.h>
#include <id3v1tag.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("PropertyMap")
{
  SECTION("Safely handle invalid keys")
  {
    PropertyMap map1;
    REQUIRE(map1.isEmpty());
    map1[L"\x00c4\x00d6\x00dc"].append("test");
    REQUIRE(map1.size() == 1);
    
    PropertyMap map2;
    map2[L"\x00c4\x00d6\x00dc"].append("test");
    REQUIRE(map1 == map2);
    REQUIRE(map1.contains(map2));
    
    map2["ARTIST"] = String("Test Artist");
    REQUIRE(map1 != map2);
    REQUIRE(map2.contains(map1));
    
    map2[L"\x00c4\x00d6\x00dc"].append("test 2");
    REQUIRE_FALSE(map2.contains(map1));
  }
  SECTION("Get and set properties")
  {
    ID3v1::Tag tag;
    
    tag.setTitle("Test Title");
    tag.setArtist("Test Artist");
    tag.setAlbum("Test Album");
    tag.setYear(2015);
    tag.setTrack(10);
    
    {
      PropertyMap prop = tag.properties();
      REQUIRE(prop["TITLE"      ].front() == "Test Title");
      REQUIRE(prop["ARTIST"     ].front() == "Test Artist");
      REQUIRE(prop["ALBUM"      ].front() == "Test Album");
      REQUIRE(prop["DATE"       ].front() == "2015");
      REQUIRE(prop["TRACKNUMBER"].front() == "10");
    
      prop["TITLE"      ].front() = "Test Title 2";
      prop["ARTIST"     ].front() = "Test Artist 2";
      prop["TRACKNUMBER"].front() = "5";
    
      tag.setProperties(prop);
    }
    
    REQUIRE(tag.title() == "Test Title 2");
    REQUIRE(tag.artist() == "Test Artist 2");
    REQUIRE(tag.track() == 5);
    
    tag.setProperties(PropertyMap());
    
    REQUIRE(tag.title().isEmpty());
    REQUIRE(tag.artist().isEmpty());
    REQUIRE(tag.track() == 0);
  }
}
