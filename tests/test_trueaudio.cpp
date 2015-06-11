#include <string>
#include <stdio.h>
#include <trueaudiofile.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestTrueAudio : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestTrueAudio);
  CPPUNIT_TEST(testReadPropertiesWithoutID3v2);
  CPPUNIT_TEST(testSaveID3v1Twice);
  CPPUNIT_TEST(testSaveID3v2Twice);
  CPPUNIT_TEST(testSaveTagCombination);
  CPPUNIT_TEST_SUITE_END();

public:

  void testReadPropertiesWithoutID3v2()
  {
    TrueAudio::File f(TEST_FILE_PATH_C("empty.tta"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
  }

  void testSaveID3v1Twice()
  {
    ScopedFileCopy copy1("empty", ".tta");
    ScopedFileCopy copy2("empty", ".tta");

    {
      TrueAudio::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)79538, f.length());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)79666, f.length());
    }

    {
      TrueAudio::File f(copy2.fileName().c_str());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)79666, f.length());
    }
  }

  void testSaveID3v2Twice()
  {
    ScopedFileCopy copy1("empty", ".tta");
    ScopedFileCopy copy2("empty", ".tta");

    {
      TrueAudio::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)79538, f.length());

      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)80606, f.length());
    }

    {
      TrueAudio::File f(copy2.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)80606, f.length());
    }
  }

  void testSaveTagCombination()
  {
    ScopedFileCopy copy1("empty", ".tta");

    {
      TrueAudio::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }

    {
      TrueAudio::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)80734, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
    }

    ScopedFileCopy copy2("empty", ".tta");

    {
      TrueAudio::File f(copy2.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }

    {
      TrueAudio::File f(copy2.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)80734, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTrueAudio);
