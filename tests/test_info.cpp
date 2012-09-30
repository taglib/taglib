#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <infotag.h>
#include <tpropertymap.h>
#include <tdebug.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestInfoTag : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestInfoTag);
  CPPUNIT_TEST(testTitle);
  CPPUNIT_TEST(testNumericFields);
  CPPUNIT_TEST_SUITE_END();

public:
  void testTitle()
  {
    RIFF::Info::Tag tag;

    CPPUNIT_ASSERT_EQUAL(String(""), tag.title());
    tag.setTitle("Test title 1");
    CPPUNIT_ASSERT_EQUAL(String("Test title 1"), tag.title());
  }

  void testNumericFields()
  {
    RIFF::Info::Tag tag;

    CPPUNIT_ASSERT_EQUAL((uint)0, tag.track());
    tag.setTrack(1234);
    CPPUNIT_ASSERT_EQUAL((uint)1234, tag.track());
    CPPUNIT_ASSERT_EQUAL(String("1234"), tag.fieldText("IPRT"));

    CPPUNIT_ASSERT_EQUAL((uint)0, tag.year());
    tag.setYear(1234);
    CPPUNIT_ASSERT_EQUAL((uint)1234, tag.year());
    CPPUNIT_ASSERT_EQUAL(String("1234"), tag.fieldText("ICRD"));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestInfoTag);
