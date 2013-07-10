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
  CPPUNIT_TEST(testInfoTag);
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
  
  void testInfoTag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();
    
    RIFF::WAV::File *f = new RIFF::WAV::File(filename.c_str());
    CPPUNIT_ASSERT(f->isValid());
    
    RIFF::Info::Tag *tag = f->InfoTag();
    tag->setTitle("test title");
    tag->setArtist("test artist");
    tag->setFieldText("ICRD", "1999-01-23");
    tag->setYear(2001);
    f->save();
    delete f;

    f = new RIFF::WAV::File(filename.c_str());
    CPPUNIT_ASSERT(f->isValid());
    
    tag = f->InfoTag();
    CPPUNIT_ASSERT(!tag->isEmpty());
    CPPUNIT_ASSERT_EQUAL(String("test title"), tag->title());
    CPPUNIT_ASSERT_EQUAL(String("test artist"), tag->artist());
    CPPUNIT_ASSERT_EQUAL(uint(2001), tag->year());
    CPPUNIT_ASSERT_EQUAL(String("2001-01-23"), tag->fieldText("ICRD"));
    tag->setTitle("ttest title");
    tag->setArtist("ttest artist");
    tag->setYear(794);
    f->save();
    delete f;

    f = new RIFF::WAV::File(filename.c_str());
    CPPUNIT_ASSERT(f->isValid());
    
    tag = f->InfoTag();
    CPPUNIT_ASSERT(!tag->isEmpty());
    CPPUNIT_ASSERT_EQUAL(String("ttest title"), tag->title());
    CPPUNIT_ASSERT_EQUAL(String("ttest artist"), tag->artist());
    CPPUNIT_ASSERT_EQUAL(uint(794), tag->year());
    CPPUNIT_ASSERT_EQUAL(String("0794-01-23"), tag->fieldText("ICRD"));
    delete f;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
