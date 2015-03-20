#include <tpropertymap.h>
#include <tag.h>
#include <id3v1tag.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace TagLib;

class TestPropertyMap : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestPropertyMap);
  CPPUNIT_TEST(testInvalidKeys);
  CPPUNIT_TEST(testGetSet);
  CPPUNIT_TEST_SUITE_END();

public:
  void testInvalidKeys()
  {
    PropertyMap map1;
    CPPUNIT_ASSERT(map1.isEmpty());
    map1[L"\x00c4\x00d6\x00dc"].append("test");
    CPPUNIT_ASSERT_EQUAL(map1.size(), 1u);

    PropertyMap map2;
    map2[L"\x00c4\x00d6\x00dc"].append("test");
    CPPUNIT_ASSERT(map1 == map2);
    CPPUNIT_ASSERT(map1.contains(map2));

    map2["ARTIST"] = String("Test Artist");
    CPPUNIT_ASSERT(map1 != map2);
    CPPUNIT_ASSERT(map2.contains(map1));

    map2[L"\x00c4\x00d6\x00dc"].append("test 2");
    CPPUNIT_ASSERT(!map2.contains(map1));

  }

  void testGetSet()
  {
    ID3v1::Tag tag;

    tag.setTitle("Test Title");
    tag.setArtist("Test Artist");
    tag.setAlbum("Test Album");
    tag.setYear(2015);
    tag.setTrack(10);

    {
      PropertyMap prop = tag.properties();
      CPPUNIT_ASSERT_EQUAL(String("Test Title"),  prop["TITLE"      ].front());
      CPPUNIT_ASSERT_EQUAL(String("Test Artist"), prop["ARTIST"     ].front());
      CPPUNIT_ASSERT_EQUAL(String("Test Album"),  prop["ALBUM"      ].front());
      CPPUNIT_ASSERT_EQUAL(String("2015"),        prop["DATE"       ].front());
      CPPUNIT_ASSERT_EQUAL(String("10"),          prop["TRACKNUMBER"].front());

      prop["TITLE"      ].front() = "Test Title 2";
      prop["ARTIST"     ].front() = "Test Artist 2";
      prop["TRACKNUMBER"].front() = "5";

      tag.setProperties(prop);
    }

    CPPUNIT_ASSERT_EQUAL(String("Test Title 2"),  tag.title());
    CPPUNIT_ASSERT_EQUAL(String("Test Artist 2"), tag.artist());
    CPPUNIT_ASSERT_EQUAL(5U, tag.track());

    tag.setProperties(PropertyMap());

    CPPUNIT_ASSERT_EQUAL(String(""), tag.title());
    CPPUNIT_ASSERT_EQUAL(String(""), tag.artist());
    CPPUNIT_ASSERT_EQUAL(0U, tag.track());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestPropertyMap);
