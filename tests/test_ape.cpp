#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <apefile.h>
#include <apetag.h>
#include <apeitem.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAPE : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAPE);
  CPPUNIT_TEST(testProperties399);
  CPPUNIT_TEST(testProperties396);
  CPPUNIT_TEST(testProperties390);
  CPPUNIT_TEST(testPropertyinterface);
  CPPUNIT_TEST_SUITE_END();

public:

  void testProperties399()
  {
    APE::File f(TEST_FILE_PATH_C("mac-399.ape"));
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testProperties396()
  {
    APE::File f(TEST_FILE_PATH_C("mac-396.ape"));
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testProperties390()
  {
    APE::File f(TEST_FILE_PATH_C("mac-390-hdr.ape"));
    CPPUNIT_ASSERT_EQUAL(15, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testPropertyinterface()
  {
    APE::Tag tag;
    APE::Item item1 = APE::Item("TRACK", "17");
    tag.setItem("TRACK", item1);

    APE::Item item2 = APE::Item();
    item2.setType(APE::Item::Binary);
    tag.setItem("TESTBINARY", item2);

    PropertyMap properties = tag.properties();
    CPPUNIT_ASSERT_EQUAL(1u, properties.unsupportedData().size());
    CPPUNIT_ASSERT(properties.contains("TRACKNUMBER"));
    CPPUNIT_ASSERT(!properties.contains("TRACK"));
    CPPUNIT_ASSERT(tag.itemListMap().contains("TESTBINARY"));

    tag.removeUnsupportedProperties(properties.unsupportedData());
    CPPUNIT_ASSERT(!tag.itemListMap().contains("TESTBINARY"));

  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAPE);
