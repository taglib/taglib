#include <string>
#include <stdio.h>
#include <tstring.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <mpegproperties.h>
#include <xingheader.h>
#include <mpegheader.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMPEG : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMPEG);
  CPPUNIT_TEST(testAudioPropertiesXingHeaderCBR);
  CPPUNIT_TEST(testAudioPropertiesXingHeaderVBR);
  CPPUNIT_TEST(testAudioPropertiesVBRIHeader);
  CPPUNIT_TEST(testAudioPropertiesNoVBRHeaders);
  CPPUNIT_TEST(testVersion2DurationWithXingHeader);
  CPPUNIT_TEST(testSaveID3v24);
  CPPUNIT_TEST(testSaveID3v24WrongParam);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testDuplicateID3v2);
  CPPUNIT_TEST(testFuzzedFile);
  CPPUNIT_TEST(testFrameOffset);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioPropertiesXingHeaderCBR()
  {
    MPEG::File f(TEST_FILE_PATH_C("lame_cbr.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(1887, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(1887, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(1887164, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(MPEG::XingHeader::Xing, f.audioProperties()->xingHeader()->type());
  }

  void testAudioPropertiesXingHeaderVBR()
  {
    MPEG::File f(TEST_FILE_PATH_C("lame_vbr.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(1887, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(1887, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(1887164, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(70, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(MPEG::XingHeader::Xing, f.audioProperties()->xingHeader()->type());
  }

  void testAudioPropertiesVBRIHeader()
  {
    MPEG::File f(TEST_FILE_PATH_C("vbri.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(222, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(222, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(222198, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(233, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(MPEG::XingHeader::VBRI, f.audioProperties()->xingHeader()->type());
  }

  void testAudioPropertiesNoVBRHeaders()
  {
    MPEG::File f(TEST_FILE_PATH_C("bladeenc.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3553, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT(!f.audioProperties()->xingHeader());

    long last = f.lastFrameOffset();

    f.seek(last);
    MPEG::Header lastHeader(f.readBlock(4));

    while (!lastHeader.isValid()) {

      last = f.previousFrameOffset(last);

      f.seek(last);
      lastHeader = MPEG::Header(f.readBlock(4));
    }

    CPPUNIT_ASSERT_EQUAL(28213L, last);
    CPPUNIT_ASSERT_EQUAL(209, lastHeader.frameLength());
  }

  void testVersion2DurationWithXingHeader()
  {
    MPEG::File f(TEST_FILE_PATH_C("mpeg2.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(5387, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(5387, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(5387285, f.audioProperties()->lengthInMilliseconds());
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMPEG);
