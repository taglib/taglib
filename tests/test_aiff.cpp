#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <aifffile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAIFF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAIFF);
  CPPUNIT_TEST(testReading);
  CPPUNIT_TEST(testAiffCProperties);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST(testDuplicateID3v2);
  CPPUNIT_TEST_SUITE_END();

public:

  void testReading()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("empty.aiff"));
    CPPUNIT_ASSERT_EQUAL(705, f.audioProperties()->bitrate());
  }

  void testAiffCProperties()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("alaw.aifc"));
    CPPUNIT_ASSERT(f.audioProperties()->isAiffC());
    CPPUNIT_ASSERT_EQUAL(ByteVector("ALAW"), f.audioProperties()->compressionType());
    CPPUNIT_ASSERT_EQUAL(String("SGI CCITT G.711 A-law"), f.audioProperties()->compressionName());
  }

  void testFuzzedFile1()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("segfault.aif"));
    CPPUNIT_ASSERT(!f.isValid());
  }

  void testFuzzedFile2()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("excessive_alloc.aif"));
    CPPUNIT_ASSERT(!f.isValid());
  }

  void testDuplicateID3v2()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("duplicate_id3v2.aiff"));

    // duplicate_id3v2.aiff has duplicate ID3v2 tags.
    // title() returns "Title2" if the second tag has been read.

    CPPUNIT_ASSERT(f.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.tag()->title());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAIFF);
