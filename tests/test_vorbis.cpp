#include <vorbisfile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestVorbis : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestVorbis);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioProperties()
  {
    Ogg::Vorbis::File f(TEST_FILE_PATH_C("empty.ogg"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3684, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(9, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestVorbis);
