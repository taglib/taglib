#include <string>
#include <stdio.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <tpropertymap.h>
#include <trueaudiofile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestTrueAudio : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestTrueAudio);
  CPPUNIT_TEST(testReadPropertiesWithoutID3v2);
  CPPUNIT_TEST(testReadPropertiesWithTags);
  CPPUNIT_TEST(testStripAndProperties);
  CPPUNIT_TEST_SUITE_END();

public:

  void testReadPropertiesWithoutID3v2()
  {
    TrueAudio::File f(TEST_FILE_PATH_C("empty.tta"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(173, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(162496U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->ttaVersion());
  }

  void testReadPropertiesWithTags()
  {
    TrueAudio::File f(TEST_FILE_PATH_C("tagged.tta"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(173, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(162496U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->ttaVersion());
  }

  void testStripAndProperties()
  {
    ScopedFileCopy copy("empty", ".tta");

    {
      TrueAudio::File f(copy.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("ID3v2");
      f.ID3v1Tag(true)->setTitle("ID3v1");
      f.save();
    }
    {
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL(String("ID3v2"), f.properties()["TITLE"].front());
      f.strip(TrueAudio::File::ID3v2);
      CPPUNIT_ASSERT_EQUAL(String("ID3v1"), f.properties()["TITLE"].front());
      f.strip(TrueAudio::File::ID3v1);
      CPPUNIT_ASSERT(f.properties().isEmpty());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTrueAudio);
