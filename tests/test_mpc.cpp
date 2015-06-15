#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <mpcfile.h>
#include <apetag.h>
#include <id3v1tag.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMPC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMPC);
  CPPUNIT_TEST(testPropertiesSV8);
  CPPUNIT_TEST(testPropertiesSV7);
  CPPUNIT_TEST(testPropertiesSV5);
  CPPUNIT_TEST(testPropertiesSV4);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST(testFuzzedFile3);
  CPPUNIT_TEST(testFuzzedFile4);
  CPPUNIT_TEST(testSaveID3v1Twice);
  CPPUNIT_TEST(testSaveAPETwice);
  CPPUNIT_TEST(testSaveTags1);
  CPPUNIT_TEST(testSaveTags2);
  CPPUNIT_TEST(testStripTags1);
  CPPUNIT_TEST(testStripTags2);
  CPPUNIT_TEST_SUITE_END();

public:

  void testPropertiesSV8()
  {
    MPC::File f(TEST_FILE_PATH_C("sv8_header.mpc"));
    CPPUNIT_ASSERT_EQUAL(8, f.audioProperties()->mpcVersion());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testPropertiesSV7()
  {
    MPC::File f(TEST_FILE_PATH_C("click.mpc"));
    CPPUNIT_ASSERT_EQUAL(7, f.audioProperties()->mpcVersion());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testPropertiesSV5()
  {
    MPC::File f(TEST_FILE_PATH_C("sv5_header.mpc"));
    CPPUNIT_ASSERT_EQUAL(5, f.audioProperties()->mpcVersion());
    CPPUNIT_ASSERT_EQUAL(26, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testPropertiesSV4()
  {
    MPC::File f(TEST_FILE_PATH_C("sv4_header.mpc"));
    CPPUNIT_ASSERT_EQUAL(4, f.audioProperties()->mpcVersion());
    CPPUNIT_ASSERT_EQUAL(26, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testFuzzedFile1()
  {
    MPC::File f(TEST_FILE_PATH_C("zerodiv.mpc"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFuzzedFile2()
  {
    MPC::File f(TEST_FILE_PATH_C("infloop.mpc"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFuzzedFile3()
  {
    MPC::File f(TEST_FILE_PATH_C("segfault.mpc"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFuzzedFile4()
  {
    MPC::File f(TEST_FILE_PATH_C("segfault2.mpc"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testSaveID3v1Twice()
  {
    ScopedFileCopy copy1("click", ".mpc");
    ScopedFileCopy copy2("click", ".mpc");

    {
      MPC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)1588, f.length());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)1780, f.length());
    }

    {
      MPC::File f(copy2.fileName().c_str());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)1780, f.length());
    }
  }

  void testSaveAPETwice()
  {
    ScopedFileCopy copy1("click", ".mpc");
    ScopedFileCopy copy2("click", ".mpc");

    {
      MPC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)1588, f.length());

      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)1689, f.length());
    }

    {
      MPC::File f(copy2.fileName().c_str());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)1689, f.length());
    }
  }

  void testSaveTags1()
  {
    ScopedFileCopy copy("click", ".mpc");

    {
      MPC::File f(copy.fileName().c_str());
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
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)1817, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());
    }
  }

  void testSaveTags2()
  {
    ScopedFileCopy copy("click", ".mpc");

    {
      MPC::File f(copy.fileName().c_str());
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
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)1817, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());
    }
  }

  void testStripTags1()
  {
    ScopedFileCopy copy("click", ".mpc");

    {
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
    }

    {
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)1817, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.strip(MPC::File::ID3v1);
      f.save();
    }

    {
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)1689, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.strip(MPC::File::APE);
      f.save();
    }

    {
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)1652, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
    }
  }

  void testStripTags2()
  {
    ScopedFileCopy copy("click", ".mpc");

    {
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
    }

    {
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)1817, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.strip(MPC::File::APE);
      f.save();
    }

    {
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)1716, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.strip(MPC::File::ID3v1);
      f.save();
    }

    {
      MPC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)1652, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMPC);
