#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <xiphcomment.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestOggFLAC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestOggFLAC);
  CPPUNIT_TEST(testFramingBit);
  CPPUNIT_TEST(testFuzzedFile);
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

  void testSaveTagTwice()
  {
    ScopedFileCopy copy1("empty_flac", ".oga");
    ScopedFileCopy copy2("empty_flac", ".oga");

    {
      Ogg::FLAC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)9113, f.length());

      f.tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)9146, f.length());
    }

    {
      Ogg::FLAC::File f(copy2.fileName().c_str());
      f.tag()->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)9146, f.length());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestOggFLAC);
