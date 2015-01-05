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
  CPPUNIT_TEST(testSaveID3v2);
  CPPUNIT_TEST(testAiffCProperties);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST_SUITE_END();

public:

  void testReading()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("empty.aiff"));
    CPPUNIT_ASSERT_EQUAL(705, f.audioProperties()->bitrate());
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
    CPPUNIT_ASSERT(f.audioProperties()->compressionType() == "ALAW");
    CPPUNIT_ASSERT(f.audioProperties()->compressionName() == "SGI CCITT G.711 A-law");
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
