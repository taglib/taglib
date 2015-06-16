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
  CPPUNIT_TEST(testSaveTags1);
  CPPUNIT_TEST(testSaveTags2);
  CPPUNIT_TEST(testStripTags1);
  CPPUNIT_TEST(testStripTags2);
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

    ByteVector audioStream;
    {
      TrueAudio::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)79538, f.length());

      f.seek(0x0000);
      audioStream = f.readBlock(79538);

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)79666, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));
    }

    {
      TrueAudio::File f(copy2.fileName().c_str());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)79666, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));

      f.ID3v1Tag(true)->setTitle("");
      f.save();

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));
    }
  }

  void testSaveID3v2Twice()
  {
    ScopedFileCopy copy1("empty", ".tta");
    ScopedFileCopy copy2("empty", ".tta");

    ByteVector audioStream;
    {
      TrueAudio::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)79538, f.length());

      f.seek(0x0000);
      audioStream = f.readBlock(79538);

      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)80606, f.length());

      f.seek(0x042C);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));
    }

    {
      TrueAudio::File f(copy2.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)80606, f.length());

      f.seek(0x042C);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));

      f.ID3v2Tag(true)->setTitle("");
      f.save();

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));
    }
  }

  void testSaveTags1()
  {
    ScopedFileCopy copy("empty", ".tta");

    ByteVector audioStream;
    {
      TrueAudio::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(79538);

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
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)80734, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());

      f.seek(0x042C);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));
    }
  }

  void testSaveTags2()
  {
    ScopedFileCopy copy("empty", ".tta");

    ByteVector audioStream;
    {
      TrueAudio::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(79538);

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
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)80734, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());

      f.seek(0x042C);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));
    }
  }

  void testStripTags1()
  {
    ScopedFileCopy copy("empty", ".tta");

    ByteVector audioStream;
    {
      TrueAudio::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(79538);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
    }

    {
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)80734, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());

      f.strip(TrueAudio::File::ID3v1);
      f.save();
    }

    {
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)80606, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());

      f.strip(TrueAudio::File::ID3v2);
      f.save();
    }

    {
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)79538, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));
    }
  }

  void testStripTags2()
  {
    ScopedFileCopy copy("empty", ".tta");

    ByteVector audioStream;
    {
      TrueAudio::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(79538);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
    }

    {
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)80734, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());

      f.strip(TrueAudio::File::ID3v2);
      f.save();
    }

    {
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)79666, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());

      f.strip(TrueAudio::File::ID3v1);
      f.save();
    }

    {
      TrueAudio::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)79538, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(79538));
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTrueAudio);
