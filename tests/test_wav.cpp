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
  CPPUNIT_TEST(testSaveID3v2Twice);
  CPPUNIT_TEST(testSaveInfoTwice);
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

  void testSaveID3v2Twice()
  {
    ScopedFileCopy copy1("empty", ".wav");
    ScopedFileCopy copy2("empty", ".wav");

    {
      RIFF::WAV::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)14744, f.length());

      f.ID3v2Tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(RIFF::WAV::File::ID3v2, true);
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)15820, f.length());
    }

    {
      RIFF::WAV::File f(copy2.fileName().c_str());
      f.ID3v2Tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(RIFF::WAV::File::ID3v2, true);
      f.save(RIFF::WAV::File::ID3v2, true);
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)15820, f.length());
    }
  }

  void testSaveInfoTwice()
  {
    ScopedFileCopy copy1("empty", ".wav");
    ScopedFileCopy copy2("empty", ".wav");

    {
      RIFF::WAV::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL((long)14744, f.length());

      f.InfoTag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(RIFF::WAV::File::Info, true);
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL((long)14788, f.length());
    }

    {
      RIFF::WAV::File f(copy2.fileName().c_str());
      f.InfoTag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(RIFF::WAV::File::Info, true);
      f.save(RIFF::WAV::File::Info, true);
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL((long)14788, f.length());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
