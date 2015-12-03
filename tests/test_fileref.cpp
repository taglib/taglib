#include <string>
#include <stdio.h>
#include <tag.h>
#include <fileref.h>
#include <oggflacfile.h>
#include <vorbisfile.h>
#include <mpegfile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"
#include <tfilestream.h>

using namespace std;
using namespace TagLib;

namespace
{
  class DummyResolver : public FileRef::FileTypeResolver
  {
  public:
    virtual File *createFile(FileName fileName, bool, AudioProperties::ReadStyle) const
    {
      return new Ogg::Vorbis::File(fileName);
    }
  };
}

class TestFileRef : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFileRef);
  CPPUNIT_TEST(testASF);
  CPPUNIT_TEST(testMusepack);
  CPPUNIT_TEST(testVorbis);
  CPPUNIT_TEST(testSpeex);
  CPPUNIT_TEST(testFLAC);
  CPPUNIT_TEST(testMP3);
  CPPUNIT_TEST(testOGA_FLAC);
  CPPUNIT_TEST(testOGA_Vorbis);
  CPPUNIT_TEST(testMP4_1);
  CPPUNIT_TEST(testMP4_2);
  CPPUNIT_TEST(testMP4_3);
  CPPUNIT_TEST(testMP4_4);
  CPPUNIT_TEST(testTrueAudio);
  CPPUNIT_TEST(testAPE);
  CPPUNIT_TEST(testWav);
  CPPUNIT_TEST(testAIFF);
  CPPUNIT_TEST(testAIFC);
  CPPUNIT_TEST(testUnsupported);
  CPPUNIT_TEST(testFileResolver);
  CPPUNIT_TEST_SUITE_END();

public:

  void fileRefSave(const string &filename, const string &ext)
  {
    ScopedFileCopy copy(filename, ext);
    string newname = copy.fileName();

    {
      FileRef f(newname.c_str());
      CPPUNIT_ASSERT(!f.isNull());
      f.tag()->setArtist("test artist");
      f.tag()->setTitle("test title");
      f.tag()->setGenre("Test!");
      f.tag()->setAlbum("albummmm");
      f.tag()->setTrack(5);
      f.tag()->setYear(2020);
      f.save();
    }
    {
      FileRef f(newname.c_str());
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("test artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("test title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("Test!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("albummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), (unsigned int)5);
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), (unsigned int)2020);
      f.tag()->setArtist("ttest artist");
      f.tag()->setTitle("ytest title");
      f.tag()->setGenre("uTest!");
      f.tag()->setAlbum("ialbummmm");
      f.tag()->setTrack(7);
      f.tag()->setYear(2080);
      f.save();
    }
    {
      FileRef f(newname.c_str());
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("ttest artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("ytest title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("uTest!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("ialbummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), (unsigned int)7);
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), (unsigned int)2080);
    }
    {
      FileStream fs(newname.c_str());
      FileRef f(&fs);
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("ttest artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("ytest title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("uTest!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("ialbummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), (unsigned int)7);
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), (unsigned int)2080);
    }
  }

  void testMusepack()
  {
    fileRefSave("click", ".mpc");
  }

  void testASF()
  {
    fileRefSave("silence-1", ".wma");
  }

  void testVorbis()
  {
    fileRefSave("empty", ".ogg");
  }

  void testSpeex()
  {
    fileRefSave("empty", ".spx");
  }

  void testFLAC()
  {
    fileRefSave("no-tags", ".flac");
  }

  void testMP3()
  {
    fileRefSave("xing", ".mp3");
  }

  void testTrueAudio()
  {
    fileRefSave("empty", ".tta");
  }

  void testMP4_1()
  {
    fileRefSave("has-tags", ".m4a");
  }

  void testMP4_2()
  {
    fileRefSave("no-tags", ".m4a");
  }

  void testMP4_3()
  {
    fileRefSave("no-tags", ".3g2");
  }

  void testMP4_4()
  {
    fileRefSave("blank_video", ".m4v");
  }

  void testWav()
  {
    fileRefSave("empty", ".wav");
  }

  void testAIFF()
  {
    fileRefSave("empty", ".aiff");
  }

  void testAIFC()
  {
    fileRefSave("alaw", ".aifc");
  }

  void testOGA_FLAC()
  {
    FileRef f(TEST_FILE_PATH_C("empty_flac.oga"));
    CPPUNIT_ASSERT(dynamic_cast<Ogg::Vorbis::File *>(f.file()) == NULL);
    CPPUNIT_ASSERT(dynamic_cast<Ogg::FLAC::File *>(f.file()) != NULL);
  }

  void testOGA_Vorbis()
  {
    FileRef f(TEST_FILE_PATH_C("empty_vorbis.oga"));
    CPPUNIT_ASSERT(dynamic_cast<Ogg::Vorbis::File *>(f.file()) != NULL);
    CPPUNIT_ASSERT(dynamic_cast<Ogg::FLAC::File *>(f.file()) == NULL);
  }

  void testAPE()
  {
    fileRefSave("mac-399", ".ape");
  }

  void testUnsupported()
  {
    FileRef f1(TEST_FILE_PATH_C("no-extension"));
    CPPUNIT_ASSERT(f1.isNull());

    FileRef f2(TEST_FILE_PATH_C("unsupported-extension.xxx"));
    CPPUNIT_ASSERT(f2.isNull());
  }

  void testFileResolver()
  {
    {
      FileRef f(TEST_FILE_PATH_C("xing.mp3"));
      CPPUNIT_ASSERT(dynamic_cast<MPEG::File *>(f.file()) != NULL);
    }

    DummyResolver resolver;
    FileRef::addFileTypeResolver(&resolver);

    {
      FileRef f(TEST_FILE_PATH_C("xing.mp3"));
      CPPUNIT_ASSERT(dynamic_cast<Ogg::Vorbis::File *>(f.file()) != NULL);
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFileRef);
