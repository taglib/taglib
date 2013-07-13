#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <wavfile.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestWAV : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestWAV);
  CPPUNIT_TEST(testLength);
  CPPUNIT_TEST(testZeroSizeDataChunk);
  CPPUNIT_TEST(testFormat);
  CPPUNIT_TEST_SUITE_END();

public:

  void testLength()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("empty.wav"));
    CPPUNIT_ASSERT_EQUAL(true, f.isValid());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
  }

  void testZeroSizeDataChunk()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("zero-size-chunk.wav"));
    CPPUNIT_ASSERT_EQUAL(false, f.isValid());
  }
  
  void testFormat()
  {
    RIFF::WAV::File f1(TEST_FILE_PATH_C("empty.wav"));
    CPPUNIT_ASSERT_EQUAL(true, f1.isValid());
    CPPUNIT_ASSERT_EQUAL((uint)1, f1.audioProperties()->format());

    RIFF::WAV::File f2(TEST_FILE_PATH_C("alaw.wav"));
    CPPUNIT_ASSERT_EQUAL(true, f2.isValid());
    CPPUNIT_ASSERT_EQUAL((uint)6, f2.audioProperties()->format());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
