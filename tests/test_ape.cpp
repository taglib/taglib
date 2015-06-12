#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <apefile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAPE : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAPE);
  CPPUNIT_TEST(testProperties399);
  CPPUNIT_TEST(testProperties399Tagged);
  CPPUNIT_TEST(testProperties399Id3v2);
  CPPUNIT_TEST(testProperties396);
  CPPUNIT_TEST(testProperties390);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST_SUITE_END();

public:

  void testProperties399()
  {
    APE::File f(TEST_FILE_PATH_C("mac-399.ape"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3550, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(192, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(156556U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3990, f.audioProperties()->version());
  }

  void testProperties399Tagged()
  {
    APE::File f(TEST_FILE_PATH_C("mac-399-tagged.ape"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3550, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(192, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(156556U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3990, f.audioProperties()->version());
  }

  void testProperties399Id3v2()
  {
    APE::File f(TEST_FILE_PATH_C("mac-399-id3v2.ape"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3550, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(192, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(156556U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3990, f.audioProperties()->version());
  }

  void testProperties396()
  {
    APE::File f(TEST_FILE_PATH_C("mac-396.ape"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(162496U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3960, f.audioProperties()->version());
  }

  void testProperties390()
  {
    APE::File f(TEST_FILE_PATH_C("mac-390-hdr.ape"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(15, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(15, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(15630, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(689262U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3900, f.audioProperties()->version());
  }

  void testFuzzedFile1()
  {
    APE::File f(TEST_FILE_PATH_C("longloop.ape"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFuzzedFile2()
  {
    APE::File f(TEST_FILE_PATH_C("zerodiv.ape"));
    CPPUNIT_ASSERT(f.isValid());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAPE);
