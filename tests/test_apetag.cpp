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

#include <string>
#include <cstdio>

#include "tstringlist.h"
#include "tpropertymap.h"
#include "tag.h"
#include "apefile.h"
#include "apetag.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAPETag : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAPETag);
  CPPUNIT_TEST(testIsEmpty);
  CPPUNIT_TEST(testIsEmpty2);
  CPPUNIT_TEST(testPropertyInterface1);
  CPPUNIT_TEST(testPropertyInterface2);
  CPPUNIT_TEST(testInvalidKeys);
  CPPUNIT_TEST(testTextBinary);
  CPPUNIT_TEST(testID3v1Collision);
  CPPUNIT_TEST_SUITE_END();

public:

  void testIsEmpty()
  {
    APE::Tag tag;
    CPPUNIT_ASSERT(tag.isEmpty());
    tag.addValue("COMPOSER", "Mike Oldfield");
    CPPUNIT_ASSERT(!tag.isEmpty());
  }

  void testIsEmpty2()
  {
    APE::Tag tag;
    CPPUNIT_ASSERT(tag.isEmpty());
    tag.setArtist("Mike Oldfield");
    CPPUNIT_ASSERT(!tag.isEmpty());
  }

  void testPropertyInterface1()
  {
    APE::Tag tag;
    PropertyMap dict = tag.properties();
    CPPUNIT_ASSERT(dict.isEmpty());
    dict["ARTIST"] = String("artist 1");
    dict["ARTIST"].append("artist 2");
    dict["TRACKNUMBER"].append("17");
    tag.setProperties(dict);
    CPPUNIT_ASSERT_EQUAL(String("17"), tag.itemListMap()["TRACK"].values()[0]);
    CPPUNIT_ASSERT_EQUAL(2u, tag.itemListMap()["ARTIST"].values().size());
    CPPUNIT_ASSERT_EQUAL(String("artist 1 / artist 2"), tag.artist());
    CPPUNIT_ASSERT_EQUAL(17u, tag.track());
    const APE::Item &textItem = tag.itemListMap()["TRACK"];
    CPPUNIT_ASSERT_EQUAL(APE::Item::Text, textItem.type());
    CPPUNIT_ASSERT(!textItem.isEmpty());
    CPPUNIT_ASSERT_EQUAL(9 + 5 + 2, textItem.size());
  }

  void testPropertyInterface2()
  {
    APE::Tag tag;
    APE::Item item1("TRACK", String("17"));
    tag.setItem("TRACK", item1);

    APE::Item item2;
    item2.setType(APE::Item::Binary);
    ByteVector binaryData1("first");
    item2.setBinaryData(binaryData1);
    tag.setItem("TESTBINARY", item2);

    PropertyMap properties = tag.properties();
    CPPUNIT_ASSERT_EQUAL(1u, properties.unsupportedData().size());
    CPPUNIT_ASSERT(properties.contains("TRACKNUMBER"));
    CPPUNIT_ASSERT(!properties.contains("TRACK"));
    CPPUNIT_ASSERT(tag.itemListMap().contains("TESTBINARY"));
    CPPUNIT_ASSERT_EQUAL(binaryData1,
                         tag.itemListMap()["TESTBINARY"].binaryData());
    ByteVector binaryData2("second");
    tag.setData("TESTBINARY", binaryData2);
    const APE::Item &binaryItem = tag.itemListMap()["TESTBINARY"];
    CPPUNIT_ASSERT_EQUAL(APE::Item::Binary, binaryItem.type());
    CPPUNIT_ASSERT(!binaryItem.isEmpty());
    CPPUNIT_ASSERT_EQUAL(9 + 10 + static_cast<int>(binaryData2.size()),
                         binaryItem.size());
    CPPUNIT_ASSERT_EQUAL(binaryData2, binaryItem.binaryData());

    tag.removeUnsupportedProperties(properties.unsupportedData());
    CPPUNIT_ASSERT(!tag.itemListMap().contains("TESTBINARY"));

    APE::Item item3("TRACKNUMBER", String("29"));
    tag.setItem("TRACKNUMBER", item3);
    properties = tag.properties();
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), properties["TRACKNUMBER"].size());
    CPPUNIT_ASSERT_EQUAL(String("17"), properties["TRACKNUMBER"][0]);
    CPPUNIT_ASSERT_EQUAL(String("29"), properties["TRACKNUMBER"][1]);

  }

  void testInvalidKeys()
  {
    PropertyMap properties;
    properties["A"] = String("invalid key: one character");
    properties["MP+"] = String("invalid key: forbidden string");
    properties[L"\x1234\x3456"] = String("invalid key: Unicode");
    properties["A B~C"] = String("valid key: space and tilde");
    properties["ARTIST"] = String("valid key: normal one");

    APE::Tag tag;
    PropertyMap unsuccessful = tag.setProperties(properties);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), unsuccessful.size());
    CPPUNIT_ASSERT(unsuccessful.contains("A"));
    CPPUNIT_ASSERT(unsuccessful.contains("MP+"));
    CPPUNIT_ASSERT(unsuccessful.contains(L"\x1234\x3456"));

    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), tag.itemListMap().size());
    tag.addValue("VALID KEY", "Test Value 1");
    tag.addValue("INVALID KEY \x7f", "Test Value 2");
    tag.addValue(L"INVALID KEY \x1234\x3456", "Test Value 3");
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), tag.itemListMap().size());
  }

  void testTextBinary()
  {
    APE::Item item("DUMMY", String("Test Text"));
    CPPUNIT_ASSERT_EQUAL(String("Test Text"), item.toString());
    CPPUNIT_ASSERT_EQUAL(ByteVector(), item.binaryData());

    ByteVector data("Test Data");
    item.setBinaryData(data);
    CPPUNIT_ASSERT(item.values().isEmpty());
    CPPUNIT_ASSERT_EQUAL(String(), item.toString());
    CPPUNIT_ASSERT_EQUAL(data, item.binaryData());

    item.setValue("Test Text 2");
    CPPUNIT_ASSERT_EQUAL(String("Test Text 2"), item.toString());
    CPPUNIT_ASSERT_EQUAL(ByteVector(), item.binaryData());
  }

  void testID3v1Collision()
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
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAPETag);
