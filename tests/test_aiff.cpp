#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <aifffile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAIFF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAIFF);
  CPPUNIT_TEST(testReading);
  CPPUNIT_TEST(testSaveID3v24);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testAiffCProperties);
  CPPUNIT_TEST(testDuplicateID3v2);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST(testSaveTagTwice);
  CPPUNIT_TEST_SUITE_END();

public:

  void testReading()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("empty.aiff"));
    CPPUNIT_ASSERT_EQUAL(705, f.audioProperties()->bitrate());
  }

  void testSaveID3v24()
  {
    ScopedFileCopy copy("empty", ".aiff");
    string newname = copy.fileName();

    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.tag()->setTitle(L"TitleXXX");
      f.save();
    }

    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(TagLib::uint(4), f.tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String(L"TitleXXX"), f.tag()->title());
    }
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("empty", ".aiff");
    string newname = copy.fileName();

    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.tag()->setTitle(L"TitleXXX");
      f.save(3);
    }

    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(TagLib::uint(3), f.tag()->header()->majorVersion());
    }
  }

  void testAiffCProperties()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("alaw.aifc"));
    CPPUNIT_ASSERT(f.audioProperties()->isAiffC());
    CPPUNIT_ASSERT_EQUAL(ByteVector("ALAW"), f.audioProperties()->compressionType());
    CPPUNIT_ASSERT_EQUAL(String("SGI CCITT G.711 A-law"), f.audioProperties()->compressionName());
  }

  void testDuplicateID3v2()
  {
    ScopedFileCopy copy("duplicate_id3v2", ".aiff");
    string newname = copy.fileName();

    {
      RIFF::AIFF::File f(newname.c_str());

      // duplicate_id3v2.aiff has duplicate ID3v2 tags.
      // title() returns "Title2" if can't skip the second tag.

      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String("Title1"), f.tag()->title());

      ID3v2::FrameList frames = f.tag()->frameList();
      for(ID3v2::FrameList::ConstIterator it = frames.begin(); it != frames.end(); ++it) {
        f.tag()->removeFrame(*it);
      }
      CPPUNIT_ASSERT(f.tag()->isEmpty());
      f.save();
    }
    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
  }

  void testFuzzedFile1()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("segfault.aif"));
    CPPUNIT_ASSERT(!f.isValid());
  }

  void testFuzzedFile2()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("excessive_alloc.aif"));
    CPPUNIT_ASSERT(!f.isValid());
  }

  void testSaveTagTwice()
  {
    ScopedFileCopy copy1("empty", ".aiff");
    ScopedFileCopy copy2("empty", ".aiff");

    ByteVector audioStream;
    {
      RIFF::AIFF::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)5936, f.length());

      f.seek(0x0008);
      audioStream = f.readBlock(5920);

      f.tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT_EQUAL((long)7012, f.length());

      f.seek(0x0008);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(5920));
    }

    {
      RIFF::AIFF::File f(copy2.fileName().c_str());
      f.tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT_EQUAL((long)7012, f.length());

      f.seek(0x0008);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(5920));

      f.tag()->setTitle("");
      f.save();

      f.seek(0x0008);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(5920));
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAIFF);
