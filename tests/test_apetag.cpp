#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <tpropertymap.h>
#include <apetag.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAPETag : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAPETag);
  CPPUNIT_TEST(testIsEmpty);
  CPPUNIT_TEST(testIsEmpty2);
  CPPUNIT_TEST(testDict);
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

  void testDict()
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
	  CPPUNIT_ASSERT_EQUAL(String("artist 1"), tag.artist());
	  CPPUNIT_ASSERT_EQUAL(17u, tag.track());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAPETag);
