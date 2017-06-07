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
#include <apetag.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("APE Tag")
{
  SECTION("Check if tag is empty")
  {
    {
      APE::Tag tag;
      REQUIRE(tag.isEmpty());
      tag.addValue("COMPOSER", "Mike Oldfield");
      REQUIRE_FALSE(tag.isEmpty());
    }
    {
      APE::Tag tag;
      REQUIRE(tag.isEmpty());
      tag.setArtist("Mike Oldfield");
      REQUIRE_FALSE(tag.isEmpty());
    }
  }
  SECTION("Read and write property map (1)")
  {
    APE::Tag tag;
    PropertyMap dict = tag.properties();
    REQUIRE(dict.isEmpty());
    dict["ARTIST"].append("artist 1");
    dict["ARTIST"].append("artist 2");
    dict["TRACKNUMBER"].append("17");
    tag.setProperties(dict);
    REQUIRE(tag.itemListMap()["TRACK"].values()[0] == "17");
    REQUIRE(tag.itemListMap()["ARTIST"].values().size() == 2);
    REQUIRE(tag.artist() == "artist 1 artist 2");
    REQUIRE(tag.track() == 17);
  }
  SECTION("Read and write property map (2)")
  {
    APE::Tag tag;
    APE::Item item1 = APE::Item("TRACK", "17");
    tag.setItem("TRACK", item1);

    APE::Item item2 = APE::Item();
    item2.setType(APE::Item::Binary);
    tag.setItem("TESTBINARY", item2);

    PropertyMap properties = tag.properties();
    REQUIRE(properties.unsupportedData().size() == 1);
    REQUIRE(properties.contains("TRACKNUMBER"));
    REQUIRE_FALSE(properties.contains("TRACK"));
    REQUIRE(tag.itemListMap().contains("TESTBINARY"));

    tag.removeUnsupportedProperties(properties.unsupportedData());
    REQUIRE_FALSE(tag.itemListMap().contains("TESTBINARY"));

    APE::Item item3 = APE::Item("TRACKNUMBER", "29");
    tag.setItem("TRACKNUMBER", item3);
    properties = tag.properties();
    REQUIRE(properties["TRACKNUMBER"].size() == 2);
    REQUIRE(properties["TRACKNUMBER"][0] == "17");
    REQUIRE(properties["TRACKNUMBER"][1] == "29");
  }
  SECTION("Skip invalid property map keys")
  {
    PropertyMap properties;
    properties["A"] = String("invalid key: one character");
    properties["MP+"] = String("invalid key: forbidden string");
    properties[L"\x1234\x3456"] = String("invalid key: Unicode");
    properties["A B~C"] = String("valid key: space and tilde");
    properties["ARTIST"] = String("valid key: normal one");

    APE::Tag tag;
    const PropertyMap unsuccessful = tag.setProperties(properties);
    REQUIRE(unsuccessful.size() == 3);
    REQUIRE(unsuccessful.contains("A"));
    REQUIRE(unsuccessful.contains("MP+"));
    REQUIRE(unsuccessful.contains(L"\x1234\x3456"));

    REQUIRE(tag.itemListMap().size() == 2);
    tag.addValue("VALID KEY", "Test Value 1");
    tag.addValue("INVALID KEY \x7f", "Test Value 2");
    tag.addValue(L"INVALID KEY \x1234\x3456", "Test Value 3");
    REQUIRE(tag.itemListMap().size() == 3);
  }
  SECTION("Set binary and string values without crashing")
  {
    APE::Item item = APE::Item("DUMMY", "Test Text");
    REQUIRE(item.toString() == "Test Text");
    REQUIRE(item.binaryData().isEmpty());

    const ByteVector data("Test Data");
    item.setBinaryData(data);
    REQUIRE(item.values().isEmpty());
    REQUIRE(item.toString().isEmpty());
    REQUIRE(item.binaryData() == data);

    item.setValue("Test Text 2");
    REQUIRE(item.toString() == "Test Text 2");
    REQUIRE(item.binaryData().isEmpty());
  }
}
