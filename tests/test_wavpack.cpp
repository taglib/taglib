#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <wavpackfile.h>
#include <apetag.h>
#include <id3v1tag.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestWavPack : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestWavPack);
  CPPUNIT_TEST(testBasic);
  CPPUNIT_TEST(testLengthScan);
  CPPUNIT_TEST(testSaveID3v1Twice);
  CPPUNIT_TEST(testSaveAPETwice);
  CPPUNIT_TEST(testSaveTagCombination);
  CPPUNIT_TEST_SUITE_END();

public:

  void testBasic()
  {
    WavPack::File f(TEST_FILE_PATH_C("no_length.wv"));
    WavPack::Properties *props = f.audioProperties();
    CPPUNIT_ASSERT_EQUAL(44100, props->sampleRate());
    CPPUNIT_ASSERT_EQUAL(2, props->channels());
    CPPUNIT_ASSERT_EQUAL(1, props->bitrate());
    CPPUNIT_ASSERT_EQUAL(0x407, props->version());
  }

  void testLengthScan()
  {
    WavPack::File f(TEST_FILE_PATH_C("no_length.wv"));
    WavPack::Properties *props = f.audioProperties();
    CPPUNIT_ASSERT_EQUAL(4, props->length());
  }

  void testFuzzedFile()
  {
    WavPack::File f(TEST_FILE_PATH_C("infloop.wv"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testSaveID3v1Twice()
  {
    ScopedFileCopy copy1("click", ".wv");
    ScopedFileCopy copy2("click", ".wv");

    {
      WavPack::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)3176, f.length());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)3368, f.length());
    }

    {
      WavPack::File f(copy2.fileName().c_str());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)3368, f.length());
    }
  }

  void testSaveAPETwice()
  {
    ScopedFileCopy copy1("click", ".wv");
    ScopedFileCopy copy2("click", ".wv");

    {
      WavPack::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)3176, f.length());

      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)3277, f.length());
    }

    {
      WavPack::File f(copy2.fileName().c_str());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)3277, f.length());
    }
  }

  void testSaveTagCombination()
  {
    ScopedFileCopy copy1("click", ".wv");

    {
      WavPack::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
    }

    {
      WavPack::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)3405, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());
    }

    ScopedFileCopy copy2("click", ".wv");

    {
      WavPack::File f(copy2.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
    }

    {
      WavPack::File f(copy2.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)3405, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWavPack);
