#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <apetag.h>
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
  CPPUNIT_TEST(testReadAPEv2);
  CPPUNIT_TEST(testReadAPEv2WithLyrics3v2);
  CPPUNIT_TEST(testSaveAPEv2);
  CPPUNIT_TEST(testSaveAPEv2WithLyrics3v2);
  CPPUNIT_TEST(testSaveAPEv2EmptyWithLyrics3v2);
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
    MPEG::File f(newname.c_str());
    f.tag()->setTitle(xxx);
    f.tag()->setArtist("Artist A");
    f.save(MPEG::File::AllTags, true, 4);

    MPEG::File f2(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(4), f2.ID3v2Tag()->header()->majorVersion());
    CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
    CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
  }

  void testSaveID3v24WrongParam()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    MPEG::File f(newname.c_str());
    f.tag()->setTitle(xxx);
    f.tag()->setArtist("Artist A");
    f.save(MPEG::File::AllTags, true, 8);

    MPEG::File f2(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(4), f2.ID3v2Tag()->header()->majorVersion());
    CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
    CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    MPEG::File f(newname.c_str());
    f.tag()->setTitle(xxx);
    f.tag()->setArtist("Artist A");
    f.save(MPEG::File::AllTags, true, 3);

    MPEG::File f2(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(3), f2.ID3v2Tag()->header()->majorVersion());
    CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
    CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
  }

  void testReadAPEv2()
  {
    ScopedFileCopy copy("apetag-replaygain", ".mp3");
    string newname = copy.fileName();

    MPEG::File f(newname.c_str());
    String s = f.APETag()->itemListMap()["MP3GAIN_ALBUM_MINMAX"].toString();
    CPPUNIT_ASSERT_EQUAL(String("129,170"), s);
  }

  void testReadAPEv2WithLyrics3v2()
  {
    ScopedFileCopy copy("apetag-replaygain-lyrics3v2", ".mp3");
    string newname = copy.fileName();

    MPEG::File f(newname.c_str());
    String s = f.APETag()->itemListMap()["MP3GAIN_ALBUM_MINMAX"].toString();
    CPPUNIT_ASSERT_EQUAL(String("129,170"), s);
  }

  void testSaveAPEv2()
  {
    ScopedFileCopy copy("apetag-replaygain", ".mp3");
    string newname = copy.fileName();

    MPEG::File f(newname.c_str());
    f.APETag()->addValue("MP3GAIN_ALBUM_MINMAX", String("xxx"));
    f.save();

    MPEG::File f2(newname.c_str());
    String s = f2.APETag()->itemListMap()["MP3GAIN_ALBUM_MINMAX"].toString();
    CPPUNIT_ASSERT_EQUAL(String("xxx"), s);
  }

  void testSaveAPEv2WithLyrics3v2()
  {
    ScopedFileCopy copy("apetag-replaygain-lyrics3v2", ".mp3");
    string newname = copy.fileName();

    MPEG::File f(newname.c_str());
    f.APETag()->addValue("MP3GAIN_ALBUM_MINMAX", String("xxx"));
    f.save();

    MPEG::File f2(newname.c_str());
    String s = f2.APETag()->itemListMap()["MP3GAIN_ALBUM_MINMAX"].toString();
    CPPUNIT_ASSERT_EQUAL(String("xxx"), s);
    f2.seek(-9, File::End);
    CPPUNIT_ASSERT_EQUAL(ByteVector("LYRICS200"), f2.readBlock(9));
  }

  void testSaveAPEv2EmptyWithLyrics3v2()
  {
    ScopedFileCopy copy("lyrics3v2", ".mp3", false);
    string newname = copy.fileName();

    {
      MPEG::File f(newname.c_str());
      f.APETag(true)->addValue("MP3GAIN_ALBUM_MINMAX", String("xxx"));
      f.save();
    }

    MPEG::File f2(newname.c_str());
    CPPUNIT_ASSERT(f2.APETag());
    String s = f2.APETag()->itemListMap()["MP3GAIN_ALBUM_MINMAX"].toString();
    CPPUNIT_ASSERT_EQUAL(String("xxx"), s);
    f2.seek(-9, File::End);
    CPPUNIT_ASSERT_EQUAL(ByteVector("LYRICS200"), f2.readBlock(9));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMPEG);
