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
  CPPUNIT_TEST(testStripTags);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
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

  void testStripTags()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    RIFF::WAV::File *f = new RIFF::WAV::File(filename.c_str());
    f->ID3v2Tag()->setTitle("test title");
    f->InfoTag()->setTitle("test title");
    f->save();
    delete f;

    f = new RIFF::WAV::File(filename.c_str());
    CPPUNIT_ASSERT(f->hasID3v2Tag());
    CPPUNIT_ASSERT(f->hasInfoTag());
    f->save(RIFF::WAV::File::ID3v2, true);
    delete f;

    f = new RIFF::WAV::File(filename.c_str());
    CPPUNIT_ASSERT(f->hasID3v2Tag());
    CPPUNIT_ASSERT(!f->hasInfoTag());
    f->ID3v2Tag()->setTitle("test title");
    f->InfoTag()->setTitle("test title");
    f->save();
    delete f;

    f = new RIFF::WAV::File(filename.c_str());
    CPPUNIT_ASSERT(f->hasID3v2Tag());
    CPPUNIT_ASSERT(f->hasInfoTag());
    f->save(RIFF::WAV::File::Info, true);
    delete f;

    f = new RIFF::WAV::File(filename.c_str());
    CPPUNIT_ASSERT(!f->hasID3v2Tag());
    CPPUNIT_ASSERT(f->hasInfoTag());
    delete f;
  }

  void testFuzzedFile1()
  {
    RIFF::WAV::File f1(TEST_FILE_PATH_C("infloop.wav"));
    CPPUNIT_ASSERT(!f1.isValid());
  }

  void testFuzzedFile2()
  {
    RIFF::WAV::File f2(TEST_FILE_PATH_C("segfault.wav"));
    CPPUNIT_ASSERT(f2.isValid());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
