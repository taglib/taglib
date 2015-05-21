#include <speexfile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestSpeex : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestSpeex);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioProperties()
  {
    Ogg::Speex::File f(TEST_FILE_PATH_C("empty.spx"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(53, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(-1, f.audioProperties()->bitrateNominal());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSpeex);
