#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <oggpageheader.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestOggFLAC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestOggFLAC);
  CPPUNIT_TEST(testFramingBit);
  CPPUNIT_TEST(testFuzzedFile);
  CPPUNIT_TEST(testSplitPackets);
  CPPUNIT_TEST(testSaveTagTwice);
  CPPUNIT_TEST_SUITE_END();

public:

  void testFramingBit()
  {
    ScopedFileCopy copy("empty_flac", ".oga");
    string newname = copy.fileName();

    Ogg::FLAC::File *f = new Ogg::FLAC::File(newname.c_str());
    f->tag()->setArtist("The Artist");
    f->save();
    delete f;

    f = new Ogg::FLAC::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String("The Artist"), f->tag()->artist());

    f->seek(0, File::End);
    int size = f->tell();
    CPPUNIT_ASSERT_EQUAL(9134, size);

    delete f;
  }

  void testFuzzedFile()
  {
    Ogg::FLAC::File f(TEST_FILE_PATH_C("segfault.oga"));
    CPPUNIT_ASSERT(!f.isValid());
  }

  void testSplitPackets()
  {
    ScopedFileCopy copy("empty_flac", ".oga");
    string newname = copy.fileName();

    String text(std::string(128 * 1024, ' '));
    for (size_t i = 0; i < text.size(); ++i)
      text[i] = static_cast<char>('A' + i % 26);

    {
      Ogg::FLAC::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(5, f.lastPageHeader()->pageSequenceNumber());
      f.tag()->setTitle(text);
      f.save();
    }

    {
      Ogg::FLAC::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(21, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL(text, f.tag()->title());
      f.tag()->setTitle("");
      f.save();
    }

    {
      Ogg::FLAC::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(5, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL(String(), f.tag()->title());
    }
  }

  void testSaveTagTwice()
  {
    ScopedFileCopy copy1("empty_flac", ".oga");
    ScopedFileCopy copy2("empty_flac", ".oga");

    ByteVector audioStream;
    {
      Ogg::FLAC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)9113, f.length());

      f.seek(0x0097);
      audioStream = f.readBlock(8192);

      f.tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)9146, f.length());

      f.seek(0x00B8);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8192));
    }

    {
      Ogg::FLAC::File f(copy2.fileName().c_str());
      f.tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)9146, f.length());

      f.seek(0x00B8);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8192));

      f.tag()->setTitle("");
      f.save();

      f.seek(0x0097);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8192));
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestOggFLAC);
