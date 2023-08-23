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

#include "apefile.h"
#include "apetag.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "tdebug.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(ApeTag, testIsEmpty)
{
  APE::Tag tag;
  ASSERT_TRUE(tag.isEmpty());
  tag.addValue("COMPOSER", "Mike Oldfield");
  ASSERT_FALSE(tag.isEmpty());
}

TEST(ApeTag, testIsEmpty2)
{
  APE::Tag tag;
  ASSERT_TRUE(tag.isEmpty());
  tag.setArtist("Mike Oldfield");
  ASSERT_FALSE(tag.isEmpty());
}

TEST(ApeTag, testPropertyInterface1)
{
  APE::Tag tag;
  PropertyMap dict = tag.properties();
  ASSERT_TRUE(dict.isEmpty());
  dict["ARTIST"] = String("artist 1");
  dict["ARTIST"].append("artist 2");
  dict["TRACKNUMBER"].append("17");
  tag.setProperties(dict);
  ASSERT_EQ(String("17"), tag.itemListMap()["TRACK"].values()[0]);
  ASSERT_EQ(2u, tag.itemListMap()["ARTIST"].values().size());
  ASSERT_EQ(String("artist 1 artist 2"), tag.artist());
  ASSERT_EQ(17u, tag.track());
  const APE::Item &textItem = tag.itemListMap()["TRACK"];
  ASSERT_EQ(APE::Item::Text, textItem.type());
  ASSERT_FALSE(textItem.isEmpty());
  ASSERT_EQ(9 + 5 + 2, textItem.size());
}

TEST(ApeTag, testPropertyInterface2)
{
  APE::Tag tag;
  APE::Item item1 = APE::Item("TRACK", String("17"));
  tag.setItem("TRACK", item1);

  APE::Item item2 = APE::Item();
  item2.setType(APE::Item::Binary);
  ByteVector binaryData1("first");
  item2.setBinaryData(binaryData1);
  tag.setItem("TESTBINARY", item2);

  PropertyMap properties = tag.properties();
  ASSERT_EQ(1u, properties.unsupportedData().size());
  ASSERT_TRUE(properties.contains("TRACKNUMBER"));
  ASSERT_FALSE(properties.contains("TRACK"));
  ASSERT_TRUE(tag.itemListMap().contains("TESTBINARY"));
  ASSERT_EQ(binaryData1,
            tag.itemListMap()["TESTBINARY"].binaryData());
  ByteVector binaryData2("second");
  tag.setData("TESTBINARY", binaryData2);
  const APE::Item &binaryItem = tag.itemListMap()["TESTBINARY"];
  ASSERT_EQ(APE::Item::Binary, binaryItem.type());
  ASSERT_FALSE(binaryItem.isEmpty());
  ASSERT_EQ(9 + 10 + static_cast<int>(binaryData2.size()),
            binaryItem.size());
  ASSERT_EQ(binaryData2, binaryItem.binaryData());

  tag.removeUnsupportedProperties(properties.unsupportedData());
  ASSERT_FALSE(tag.itemListMap().contains("TESTBINARY"));

  APE::Item item3 = APE::Item("TRACKNUMBER", String("29"));
  tag.setItem("TRACKNUMBER", item3);
  properties = tag.properties();
  ASSERT_EQ(static_cast<unsigned int>(2), properties["TRACKNUMBER"].size());
  ASSERT_EQ(String("17"), properties["TRACKNUMBER"][0]);
  ASSERT_EQ(String("29"), properties["TRACKNUMBER"][1]);
}

TEST(ApeTag, testInvalidKeys)
{
  PropertyMap properties;
  properties["A"]             = String("invalid key: one character");
  properties["MP+"]           = String("invalid key: forbidden string");
  properties[L"\x1234\x3456"] = String("invalid key: Unicode");
  properties["A B~C"]         = String("valid key: space and tilde");
  properties["ARTIST"]        = String("valid key: normal one");

  APE::Tag tag;
  PropertyMap unsuccessful = tag.setProperties(properties);
  ASSERT_EQ(static_cast<unsigned int>(3), unsuccessful.size());
  ASSERT_TRUE(unsuccessful.contains("A"));
  ASSERT_TRUE(unsuccessful.contains("MP+"));
  ASSERT_TRUE(unsuccessful.contains(L"\x1234\x3456"));

  ASSERT_EQ(static_cast<unsigned int>(2), tag.itemListMap().size());
  tag.addValue("VALID KEY", "Test Value 1");
  tag.addValue("INVALID KEY \x7f", "Test Value 2");
  tag.addValue(L"INVALID KEY \x1234\x3456", "Test Value 3");
  ASSERT_EQ(static_cast<unsigned int>(3), tag.itemListMap().size());
}

TEST(ApeTag, testTextBinary)
{
  APE::Item item = APE::Item("DUMMY", String("Test Text"));
  ASSERT_EQ(String("Test Text"), item.toString());
  ASSERT_EQ(ByteVector(), item.binaryData());

  ByteVector data("Test Data");
  item.setBinaryData(data);
  ASSERT_TRUE(item.values().isEmpty());
  ASSERT_EQ(String(), item.toString());
  ASSERT_EQ(data, item.binaryData());

  item.setValue("Test Text 2");
  ASSERT_EQ(String("Test Text 2"), item.toString());
  ASSERT_EQ(ByteVector(), item.binaryData());
}

TEST(ApeTag, testID3v1Collision)
{
  ScopedFileCopy copy("no-tags", ".mpc");
  string newname = copy.fileName();

  {
    APE::File f(newname.c_str());
    f.APETag(true)->setArtist("Filltointersect    ");
    f.APETag()->setTitle("Filltointersect    ");
    f.save();
  }

  {
    APE::File f(newname.c_str());
    ASSERT_FALSE(f.hasID3v1Tag());
  }
}
