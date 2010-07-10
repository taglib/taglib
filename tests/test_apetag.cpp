#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <apetag.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAPETag : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAPETag);
  CPPUNIT_TEST(testIsEmpty);
  CPPUNIT_TEST(testIsEmpty2);
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAPETag);
