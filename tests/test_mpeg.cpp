#include <string>
#include <stdio.h>
#include <tstring.h>
#include <mpegfile.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <apetag.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMPEG : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMPEG);
  CPPUNIT_TEST(testVersion2DurationWithXingHeader);
  CPPUNIT_TEST(testSaveID3v24);
  CPPUNIT_TEST(testSaveID3v24WrongParam);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testDuplicateID3v2);
  CPPUNIT_TEST(testFuzzedFile);
  CPPUNIT_TEST(testFrameOffset);
  CPPUNIT_TEST(testSaveID3v1Twice);
  CPPUNIT_TEST(testSaveID3v2Twice);
  CPPUNIT_TEST(testSaveAPETwice);
  CPPUNIT_TEST(testSaveTags1);
  CPPUNIT_TEST(testSaveTags2);
  CPPUNIT_TEST(testSaveTags3);
  CPPUNIT_TEST(testStripTags1);
  CPPUNIT_TEST(testStripTags2);
  CPPUNIT_TEST(testStripTags3);
  CPPUNIT_TEST(testSaveDuplicateTags);
  CPPUNIT_TEST(testID3v2AfterMPEGFrame);
  CPPUNIT_TEST_SUITE_END();

public:

  void testVersion2DurationWithXingHeader()
  {
    MPEG::File f(TEST_FILE_PATH_C("mpeg2.mp3"));
    CPPUNIT_ASSERT_EQUAL(5387, f.audioProperties()->length());
  }

  void testSaveID3v24()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      MPEG::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(MPEG::File::AllTags, true, 4);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      MPEG::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(TagLib::uint(4), f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testSaveID3v24WrongParam()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      MPEG::File f(newname.c_str());
      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(MPEG::File::AllTags, true, 8);
    }
    {
      MPEG::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(TagLib::uint(4), f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      MPEG::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(MPEG::File::AllTags, true, 3);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      MPEG::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(TagLib::uint(3), f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testDuplicateID3v2()
  {
    MPEG::File f(TEST_FILE_PATH_C("duplicate_id3v2.mp3"));

    // duplicate_id3v2.mp3 has duplicate ID3v2 tags.
    // Sample rate will be 32000 if can't skip the second tag.

    CPPUNIT_ASSERT(f.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testFuzzedFile()
  {
    MPEG::File f(TEST_FILE_PATH_C("excessive_alloc.mp3"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFrameOffset()
  {
    {
      MPEG::File f(TEST_FILE_PATH_C("ape.mp3"));
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL((long)0x0000, f.firstFrameOffset());
      CPPUNIT_ASSERT_EQUAL((long)0x1FD6, f.lastFrameOffset());
    }
    {
      MPEG::File f(TEST_FILE_PATH_C("ape-id3v1.mp3"));
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL((long)0x0000, f.firstFrameOffset());
      CPPUNIT_ASSERT_EQUAL((long)0x1FD6, f.lastFrameOffset());
    }
    {
      MPEG::File f(TEST_FILE_PATH_C("ape-id3v2.mp3"));
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL((long)0x041A, f.firstFrameOffset());
      CPPUNIT_ASSERT_EQUAL((long)0x23F0, f.lastFrameOffset());
    }
  }

  void testSaveID3v1Twice()
  {
    ScopedFileCopy copy1("xing", ".mp3");
    ScopedFileCopy copy2("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)8208, f.length());

      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v1, true);
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)8336, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }

    {
      MPEG::File f(copy2.fileName().c_str());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v1, true);
      f.save(MPEG::File::ID3v1, true);
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)8336, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));

      f.ID3v1Tag(true)->setTitle("");
      f.save();

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }
  }

  void testSaveID3v2Twice()
  {
    ScopedFileCopy copy1("xing", ".mp3");
    ScopedFileCopy copy2("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)8208, f.length());

      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v2, true);
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)9276, f.length());

      f.seek(0x042C);
      audioStream = f.readBlock(8208);
    }

    {
      MPEG::File f(copy2.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v2, true);
      f.save(MPEG::File::ID3v2, true);
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)9276, f.length());

      f.seek(0x042C);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));

      f.ID3v2Tag(true)->setTitle("");
      f.save();

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }
  }

  void testSaveAPETwice()
  {
    ScopedFileCopy copy1("xing", ".mp3");
    ScopedFileCopy copy2("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)8208, f.length());

      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::APE, true);
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)8309, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }

    {
      MPEG::File f(copy2.fileName().c_str());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::APE, true);
      f.save(MPEG::File::APE, true);
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT_EQUAL((long)8309, f.length());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));

      f.APETag(true)->setTitle("");
      f.save();

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }
  }

  void testSaveTags1()
  {
    ScopedFileCopy copy("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v1, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)8336, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v2, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9404, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::APE, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9505, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());

      f.seek(0x042C);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }
  }

  void testSaveTags2()
  {
    ScopedFileCopy copy("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v2, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9276, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v1, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9404, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::APE, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9505, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());

      f.seek(0x042C);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }
  }

  void testSaveTags3()
  {
    ScopedFileCopy copy("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::APE, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)8309, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v1, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)8437, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v2, false);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9505, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());

      f.seek(0x042C);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }
  }

  void testStripTags1()
  {
    ScopedFileCopy copy("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::AllTags);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9505, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.save(~MPEG::File::ID3v1);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9377, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.save(~MPEG::File::ID3v2);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)8309, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.save(~MPEG::File::APE);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(fileEqual(copy.fileName().c_str(), TEST_FILE_PATH_C("xing.mp3")));
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }
  }

  void testStripTags2()
  {
    ScopedFileCopy copy("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::AllTags);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9505, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.save(~MPEG::File::ID3v2);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)8437, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.save(~MPEG::File::ID3v1);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)8309, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.save(~MPEG::File::APE);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(fileEqual(copy.fileName().c_str(), TEST_FILE_PATH_C("xing.mp3")));
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }
  }

  void testStripTags3()
  {
    ScopedFileCopy copy("xing", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy.fileName().c_str());
      f.seek(0x0000);
      audioStream = f.readBlock(8208);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::AllTags);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9505, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      f.save(~MPEG::File::APE);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9404, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.save(~MPEG::File::ID3v1);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)9276, f.length());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.save(~MPEG::File::ID3v2);
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(fileEqual(copy.fileName().c_str(), TEST_FILE_PATH_C("xing.mp3")));
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());

      f.seek(0x0000);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8208));
    }

  }

  void testSaveDuplicateTags()
  {
    ScopedFileCopy copy("duplicate_id3v2", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy.fileName().c_str());
      f.seek(0x1F71);
      audioStream = f.readBlock(2089);

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v1, false);

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::ID3v2, false);

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasAPETag());
      f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save(MPEG::File::APE, false);

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());
    }

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)3887, f.length());
      CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasAPETag());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.APETag()->title());

      f.seek(0x0621);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(2089));
    }
  }

  void testID3v2AfterMPEGFrame()
  {
    MPEG::File f(TEST_FILE_PATH_C("id3v2_after_mpeg_frame.mp3"));
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(!f.hasID3v2Tag());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMPEG);
