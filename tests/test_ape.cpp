#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <apefile.h>
#include <apetag.h>
#include <id3v1tag.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAPE : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAPE);
  CPPUNIT_TEST(testProperties399);
  CPPUNIT_TEST(testProperties396);
  CPPUNIT_TEST(testProperties390);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST(testSaveID3v1Twice);
  CPPUNIT_TEST(testSaveAPETwice);
  CPPUNIT_TEST(testSaveTags1);
  CPPUNIT_TEST(testSaveTags2);
  CPPUNIT_TEST(testStripTags1);
  CPPUNIT_TEST(testStripTags2);
  CPPUNIT_TEST(testStripID3v2);
  CPPUNIT_TEST_SUITE_END();

public:

  void testProperties399()
  {
    APE::File f(TEST_FILE_PATH_C("mac-399.ape"));
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testProperties396()
  {
    APE::File f(TEST_FILE_PATH_C("mac-396.ape"));
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testProperties390()
  {
    APE::File f(TEST_FILE_PATH_C("mac-390-hdr.ape"));
    CPPUNIT_ASSERT_EQUAL(15, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testFuzzedFile1()
  {
    APE::File f(TEST_FILE_PATH_C("longloop.ape"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFuzzedFile2()
  {
    APE::File f(TEST_FILE_PATH_C("zerodiv.ape"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testSaveID3v1Twice()
  {
    ScopedFileCopy copy1("mac-399", ".ape");
    ScopedFileCopy copy2("mac-399", ".ape");

    ByteVector audioStream;
    {
      APE::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)172, f.length());

      f.seek(0x0000);
      audioStream = f.readBlock(172);

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)364, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));
    }

    {
      APE::File f(copy2.fileName().c_str());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)364, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));

      f.ID3v1Tag(true)->setTitle("");
      f.save();

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));
    }
  }

  void testSaveAPETwice()
  {
    ScopedFileCopy copy1("mac-399", ".ape");
    ScopedFileCopy copy2("mac-399", ".ape");

    ByteVector audioStream;
    {
      APE::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)172, f.length());

      f.seek(0x0000);
      audioStream = f.readBlock(172);

      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)273, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));
    }

    {
      APE::File f(copy2.fileName().c_str());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)273, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));

      f.APETag(true)->setTitle("");
      f.save();

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));
    }
  }

  void testSaveTags1()
  {
    ScopedFileCopy copy("mac-399", ".ape");

    ByteVector audioStream;
    {
      APE::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(172);

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
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)401, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));
    }
  }

  void testSaveTags2()
  {
    ScopedFileCopy copy("mac-399", ".ape");

    ByteVector audioStream;
    {
      APE::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(172);

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
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)401, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));
    }
  }

  void testStripTags1()
  {
    ScopedFileCopy copy("mac-399", ".ape");

    ByteVector audioStream;
    {
      APE::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(172);

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
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)401, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.strip(APE::File::ID3v1);
      f.save();
    }

    {
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)273, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.strip(APE::File::APE);
      f.save();
    }

    {
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)236, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));
    }
  }

  void testStripTags2()
  {
    ScopedFileCopy copy("mac-399", ".ape");

    ByteVector audioStream;
    {
      APE::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(172);

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
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)401, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.strip(APE::File::APE);
      f.save();
    }

    {
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)300, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.strip(APE::File::ID3v1);
      f.save();
    }

    {
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)236, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(172));
    }
  }

  void testStripID3v2()
  {
    ScopedFileCopy copy("mac-399-id3v2", ".ape");

    ByteVector audioStream;
    {
      APE::File f(copy.fileName().c_str());
      f.seek(0x0F67);
      audioStream = f.readBlock(16384);

      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)89155, f.length());
      f.strip(APE::File::ID3v2);
      f.save();
    }
    {
      APE::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)85276, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(16384));
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAPE);
