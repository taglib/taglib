#include <string>
#include <cstdio>
#include <tag.h>
#include <tbytevectorlist.h>
#include <dsdifffile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestDSDIFF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestDSDIFF);
  CPPUNIT_TEST(testProperties);
  CPPUNIT_TEST(testTags);
  CPPUNIT_TEST(testSaveID3v2);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testStrip);
  CPPUNIT_TEST(testRepeatedSave);
  CPPUNIT_TEST_SUITE_END();

public:

  void testProperties()
  {
    DSDIFF::File f(TEST_FILE_PATH_C("empty10ms.dff"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(10, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(5644, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(2822400, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(static_cast<long long>(28224), f.audioProperties()->sampleCount());
  }

  void testTags()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    DSDIFF::File *f = new DSDIFF::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String(""), f->tag()->artist());
    f->tag()->setArtist("The Artist");
    f->save();
    delete f;

    f = new DSDIFF::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String("The Artist"), f->tag()->artist());
    delete f;
  }

  void testSaveID3v2()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());

      f.tag()->setTitle(L"TitleXXX");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L"TitleXXX"), f.tag()->title());

      f.tag()->setTitle("");
      f.save();
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.tag()->setTitle(L"TitleXXX");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L"TitleXXX"), f.tag()->title());
    }
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(DSDIFF::File::AllTags, File::StripOthers, ID3v2::v3);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      DSDIFF::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL((unsigned int)3, f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testStrip()
  {
    {
      ScopedFileCopy copy("empty10ms", ".dff");
      {
        DSDIFF::File f(copy.fileName().c_str());
        f.tag()->setArtist("X");
        f.save();
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.hasID3v2Tag());
        CPPUNIT_ASSERT(f.hasDIINTag());
        f.strip();
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(!f.hasID3v2Tag());
        CPPUNIT_ASSERT(!f.hasDIINTag());
      }
    }

    {
      ScopedFileCopy copy("empty10ms", ".dff");
      {
        DSDIFF::File f(copy.fileName().c_str());
        f.ID3v2Tag(true);
        f.DIINTag(true);
        f.tag()->setArtist("X");
        f.save();
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.hasID3v2Tag());
        CPPUNIT_ASSERT(f.hasDIINTag());
        f.strip(DSDIFF::File::ID3v2);
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(!f.hasID3v2Tag());
        CPPUNIT_ASSERT(f.hasDIINTag());
      }
    }

    {
      ScopedFileCopy copy("empty10ms", ".dff");
      {
        DSDIFF::File f(copy.fileName().c_str());
        f.tag()->setArtist("X");
        f.save();
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.hasID3v2Tag());
        CPPUNIT_ASSERT(f.hasDIINTag());
        f.strip(DSDIFF::File::DIIN);
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.hasID3v2Tag());
        CPPUNIT_ASSERT(!f.hasDIINTag());
      }
    }
  }

  void testRepeatedSave()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String(""), f.tag()->title());
      f.tag()->setTitle("NEW TITLE");
      f.save();
      CPPUNIT_ASSERT_EQUAL(String("NEW TITLE"), f.tag()->title());
      f.tag()->setTitle("NEW TITLE 2");
      f.save();
      CPPUNIT_ASSERT_EQUAL(String("NEW TITLE 2"), f.tag()->title());
      CPPUNIT_ASSERT_EQUAL(static_cast<long long>(8252), f.length());
      f.save();
      CPPUNIT_ASSERT_EQUAL(static_cast<long long>(8252), f.length());
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String("NEW TITLE 2"), f.tag()->title());
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDSDIFF);
