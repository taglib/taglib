#include <speexfile.h>
#include <oggpageheader.h>
#include <xiphcomment.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestSpeex : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestSpeex);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST(testSplitPackets);
  CPPUNIT_TEST(testSaveTagTwice);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioProperties()
  {
    Ogg::Speex::File f(TEST_FILE_PATH_C("empty.spx"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testSplitPackets()
  {
    ScopedFileCopy copy("empty", ".spx");
    string newname = copy.fileName();

    String text(std::string(128 * 1024, ' '));
    for(size_t i = 0; i < text.size(); ++i)
      text[i] = static_cast<char>('A' + i % 26);

    {
      Ogg::Speex::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(7, f.lastPageHeader()->pageSequenceNumber());
      f.tag()->setTitle(text);
      f.save();
    }

    {
      Ogg::Speex::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(23, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL(text, f.tag()->title());
      f.tag()->setTitle("");
      f.save();
    }

    {
      Ogg::Speex::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(7, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL(String(), f.tag()->title());
    }
  }

  void testSaveTagTwice()
  {
    ScopedFileCopy copy1("empty", ".spx");
    ScopedFileCopy copy2("empty", ".spx");

    ByteVector audioStream;
    {
      Ogg::Speex::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)24301, f.length());

      f.seek(0x00A9);
      audioStream = f.readBlock(8192);

      f.tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT_EQUAL((long)24335, f.length());

      f.seek(0x00CB);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8192));
    }

    {
      Ogg::Speex::File f(copy2.fileName().c_str());
      f.tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT_EQUAL((long)24335, f.length());

      f.seek(0x00CB);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8192));

      f.tag()->setTitle("");
      f.save();

      f.seek(0x00AA);
      CPPUNIT_ASSERT_EQUAL(audioStream, f.readBlock(8192));
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSpeex);
