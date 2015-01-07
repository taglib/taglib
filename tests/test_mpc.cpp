#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <mpcfile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMPC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMPC);
  CPPUNIT_TEST(testPropertiesSV8);
  CPPUNIT_TEST(testPropertiesSV7);
  CPPUNIT_TEST(testPropertiesSV5);
  CPPUNIT_TEST(testPropertiesSV4);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST(testFuzzedFile3);
  CPPUNIT_TEST_SUITE_END();

public:

  void testPropertiesSV8()
  {
    MPC::File f(TEST_FILE_PATH_C("sv8_header.mpc"));
    CPPUNIT_ASSERT_EQUAL(8, f.audioProperties()->mpcVersion());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testPropertiesSV7()
  {
    MPC::File f(TEST_FILE_PATH_C("click.mpc"));
    CPPUNIT_ASSERT_EQUAL(7, f.audioProperties()->mpcVersion());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testPropertiesSV5()
  {
    MPC::File f(TEST_FILE_PATH_C("sv5_header.mpc"));
    CPPUNIT_ASSERT_EQUAL(5, f.audioProperties()->mpcVersion());
    CPPUNIT_ASSERT_EQUAL(26, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testPropertiesSV4()
  {
    MPC::File f(TEST_FILE_PATH_C("sv4_header.mpc"));
    CPPUNIT_ASSERT_EQUAL(4, f.audioProperties()->mpcVersion());
    CPPUNIT_ASSERT_EQUAL(26, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testFuzzedFile1()
  {
    MPC::File f(TEST_FILE_PATH_C("zerodiv.mpc"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFuzzedFile2()
  {
    MPC::File f(TEST_FILE_PATH_C("infloop.mpc"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFuzzedFile3()
  {
    MPC::File f(TEST_FILE_PATH_C("segfault.mpc"));
    CPPUNIT_ASSERT(f.isValid());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMPC);
