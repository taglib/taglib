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
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST(testSaveID3v2);
  CPPUNIT_TEST(testAiffCProperties);
  CPPUNIT_TEST(testDuplicateID3v2);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioProperties()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("empty.aiff"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(67, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(706, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
  }

  void testSaveID3v2()
  {
    ScopedFileCopy copy("empty", ".aiff");
    string newname = copy.fileName();

    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.tag()->setTitle(L"TitleXXX");
      f.save();
    }

    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L"TitleXXX"), f.tag()->title());
    }
  }

  void testAiffCProperties()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("alaw.aifc"));
    CPPUNIT_ASSERT(f.audioProperties()->isAiffC());
    CPPUNIT_ASSERT_EQUAL(ByteVector("ALAW"), f.audioProperties()->compressionType());
    CPPUNIT_ASSERT_EQUAL(String("SGI CCITT G.711 A-law"), f.audioProperties()->compressionName());
  }

  void testDuplicateID3v2()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("duplicate_id3v2.aiff"));

    // duplicate_id3v2.aiff has duplicate ID3v2 tags.
    // title() returns "Title2" if can't skip the second tag.

    CPPUNIT_ASSERT(f.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.tag()->title());
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAIFF);
