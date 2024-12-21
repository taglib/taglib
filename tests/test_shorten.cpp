#include <string>
#include <cstdio>

#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tag.h"
#include "shortenfile.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestShorten : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestShorten);
  CPPUNIT_TEST(testBasic);
  CPPUNIT_TEST(testTags);
  CPPUNIT_TEST_SUITE_END();

public:

  void testBasic()
  {
    Shorten::File f(TEST_FILE_PATH_C("2sec-silence.shn"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(2000, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(1411, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->shortenVersion());
    CPPUNIT_ASSERT_EQUAL(5, f.audioProperties()->fileType());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned long>(88200), f.audioProperties()->sampleFrames());
  }

  void testTags()
  {
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestShorten);
