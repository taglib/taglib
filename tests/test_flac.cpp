#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <flacfile.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestFLAC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFLAC);
  CPPUNIT_TEST(testSignature);
  CPPUNIT_TEST(testMultipleCommentBlocks);
  CPPUNIT_TEST(testPicture);
  CPPUNIT_TEST_SUITE_END();

public:

  void testSignature()
  {
    FLAC::File f("data/no-tags.flac");
    CPPUNIT_ASSERT_EQUAL(ByteVector("a1b141f766e9849ac3db1030a20a3c77"), f.audioProperties()->signature().toHex());
  }

  void testMultipleCommentBlocks()
  {
    ScopedFileCopy copy("multiple-vc", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String("Artist 1"), f->tag()->artist());
    f->tag()->setArtist("The Artist");
    f->save();
    delete f;

    f = new FLAC::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String("The Artist"), f->tag()->artist());
    delete f;
  }

  void testPicture()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    List<FLAC::Picture *> lst = f->pictureList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(1), lst.size());

    FLAC::Picture *pic = lst.front();
    CPPUNIT_ASSERT_EQUAL(3, int(pic->type()));
    CPPUNIT_ASSERT_EQUAL(1, pic->width());
    CPPUNIT_ASSERT_EQUAL(1, pic->height());
    CPPUNIT_ASSERT_EQUAL(24, pic->colorDepth());
    CPPUNIT_ASSERT_EQUAL(0, pic->numColors());
    CPPUNIT_ASSERT_EQUAL(String("image/png"), pic->mimeType());
    CPPUNIT_ASSERT_EQUAL(String("A pixel."), pic->description());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(150), pic->data().size());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFLAC);
