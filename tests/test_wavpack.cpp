#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <wavpackfile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestWavPack : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestWavPack);
  CPPUNIT_TEST(testNoLengthProperties);
  CPPUNIT_TEST(testMultiChannelProperties);
  CPPUNIT_TEST(testTaggedProperties);
  CPPUNIT_TEST(testFuzzedFile);
  CPPUNIT_TEST_SUITE_END();

public:

  void testNoLengthProperties()
  {
    WavPack::File f(TEST_FILE_PATH_C("no_length.wv"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3705, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(true, f.audioProperties()->isLossless());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(163392U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1031, f.audioProperties()->version());
  }

  void testMultiChannelProperties()
  {
    WavPack::File f(TEST_FILE_PATH_C("four_channels.wv"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3833, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(112, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(4, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isLossless());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(169031U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1031, f.audioProperties()->version());
  }

  void testTaggedProperties()
  {
    WavPack::File f(TEST_FILE_PATH_C("tagged.wv"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3550, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(172, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isLossless());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(156556U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1031, f.audioProperties()->version());
  }

  void testFuzzedFile()
  {
    WavPack::File f(TEST_FILE_PATH_C("infloop.wv"));
    CPPUNIT_ASSERT(f.isValid());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWavPack);
