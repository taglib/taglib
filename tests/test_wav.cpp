#include <string>
#include <stdio.h>
#include <tag.h>
#include <id3v2tag.h>
#include <infotag.h>
#include <tbytevectorlist.h>
#include <wavfile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestWAV : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestWAV);
  CPPUNIT_TEST(testLength);
  CPPUNIT_TEST(testZeroSizeDataChunk);
  CPPUNIT_TEST(testID3v2Tag);
  CPPUNIT_TEST(testInfoTag);
  CPPUNIT_TEST(testStripTags);
  CPPUNIT_TEST(testDuplicateTags);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST(testInvalidChunk);
  CPPUNIT_TEST_SUITE_END();

public:

  void testLength()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("empty.wav"));
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
  }

  void testZeroSizeDataChunk()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("zero-size-chunk.wav"));
    CPPUNIT_ASSERT(!f.isValid());
  }

  void testID3v2Tag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());

      f.ID3v2Tag()->setTitle(L"Title");
      f.ID3v2Tag()->setArtist(L"Artist");
      f.save();
    }

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L"Title"),  f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L"Artist"), f.ID3v2Tag()->artist());

      f.ID3v2Tag()->setTitle(L"");
      f.ID3v2Tag()->setArtist(L"");
      f.save();
    }

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.ID3v2Tag()->artist());
    }
  }

  void testInfoTag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());

      f.InfoTag()->setTitle(L"Title");
      f.InfoTag()->setArtist(L"Artist");
      f.save();
    }

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L"Title"),  f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L"Artist"), f.InfoTag()->artist());

      f.InfoTag()->setTitle(L"");
      f.InfoTag()->setArtist(L"");
      f.save();
    }

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.InfoTag()->artist());
    }
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

  void testDuplicateTags()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("duplicate_tags.wav"));

    // duplicate_tags.wav has duplicate ID3v2/INFO tags.
    // title() returns "Title2" if can't skip the second tag.

    CPPUNIT_ASSERT(f.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.ID3v2Tag()->title());

    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.InfoTag()->title());
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

  void testInvalidChunk()
  {
    ScopedFileCopy copy("empty", ".wav");

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      f.seek(12);
      CPPUNIT_ASSERT_EQUAL(ByteVector("fmt "), f.readBlock(4));
      f.insert(ByteVector("f\x10t\x90"), 12, 4);
    }

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(!f.isValid());

      f.seek(12);
      CPPUNIT_ASSERT_EQUAL(ByteVector("f\x10t\x90"), f.readBlock(4));
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
