#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <tpropertymap.h>
#include <flacfile.h>
#include <xiphcomment.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestFLAC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFLAC);
  CPPUNIT_TEST(testSignature);
  CPPUNIT_TEST(testMultipleCommentBlocks);
  CPPUNIT_TEST(testReadPicture);
  CPPUNIT_TEST(testAddPicture);
  CPPUNIT_TEST(testReplacePicture);
  CPPUNIT_TEST(testRemoveAllPictures);
  CPPUNIT_TEST(testRepeatedSave);
  CPPUNIT_TEST(testSaveMultipleValues);
  CPPUNIT_TEST(testDict);
  CPPUNIT_TEST(testInvalid);
  CPPUNIT_TEST(testShrinkPadding);
  CPPUNIT_TEST(testSaveXiphTwice);
  CPPUNIT_TEST(testSaveID3v1Twice);
  CPPUNIT_TEST(testSaveID3v2Twice);
  CPPUNIT_TEST(testSaveTagCombination);
  CPPUNIT_TEST_SUITE_END();

public:

  void testSignature()
  {
    FLAC::File f(TEST_FILE_PATH_C("no-tags.flac"));
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

  void testReadPicture()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    List<FLAC::Picture *> lst = f->pictureList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(1), lst.size());

    FLAC::Picture *pic = lst.front();
    CPPUNIT_ASSERT_EQUAL(FLAC::Picture::FrontCover, pic->type());
    CPPUNIT_ASSERT_EQUAL(1, pic->width());
    CPPUNIT_ASSERT_EQUAL(1, pic->height());
    CPPUNIT_ASSERT_EQUAL(24, pic->colorDepth());
    CPPUNIT_ASSERT_EQUAL(0, pic->numColors());
    CPPUNIT_ASSERT_EQUAL(String("image/png"), pic->mimeType());
    CPPUNIT_ASSERT_EQUAL(String("A pixel."), pic->description());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(150), pic->data().size());

    delete f;
  }

  void testAddPicture()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    List<FLAC::Picture *> lst = f->pictureList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(1), lst.size());

    FLAC::Picture *newpic = new FLAC::Picture();
    newpic->setType(FLAC::Picture::BackCover);
    newpic->setWidth(5);
    newpic->setHeight(6);
    newpic->setColorDepth(16);
    newpic->setNumColors(7);
    newpic->setMimeType("image/jpeg");
    newpic->setDescription("new image");
    newpic->setData("JPEG data");
    f->addPicture(newpic);
    f->save();
    delete f;

    f = new FLAC::File(newname.c_str());
    lst = f->pictureList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2), lst.size());

    FLAC::Picture *pic = lst[0];
    CPPUNIT_ASSERT_EQUAL(FLAC::Picture::FrontCover, pic->type());
    CPPUNIT_ASSERT_EQUAL(1, pic->width());
    CPPUNIT_ASSERT_EQUAL(1, pic->height());
    CPPUNIT_ASSERT_EQUAL(24, pic->colorDepth());
    CPPUNIT_ASSERT_EQUAL(0, pic->numColors());
    CPPUNIT_ASSERT_EQUAL(String("image/png"), pic->mimeType());
    CPPUNIT_ASSERT_EQUAL(String("A pixel."), pic->description());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(150), pic->data().size());

    pic = lst[1];
    CPPUNIT_ASSERT_EQUAL(FLAC::Picture::BackCover, pic->type());
    CPPUNIT_ASSERT_EQUAL(5, pic->width());
    CPPUNIT_ASSERT_EQUAL(6, pic->height());
    CPPUNIT_ASSERT_EQUAL(16, pic->colorDepth());
    CPPUNIT_ASSERT_EQUAL(7, pic->numColors());
    CPPUNIT_ASSERT_EQUAL(String("image/jpeg"), pic->mimeType());
    CPPUNIT_ASSERT_EQUAL(String("new image"), pic->description());
    CPPUNIT_ASSERT_EQUAL(ByteVector("JPEG data"), pic->data());

    newpic = new FLAC::Picture();
    newpic->setType(FLAC::Picture::Artist);
    newpic->setWidth(5);
    newpic->setHeight(6);
    newpic->setColorDepth(16);
    newpic->setNumColors(7);
    newpic->setMimeType("image/jpeg");
    newpic->setDescription("new image");
    newpic->setData(ByteVector(16777216, '\x55'));
    f->addPicture(newpic);
    CPPUNIT_ASSERT(!f->save());
    delete f;
  }

  void testReplacePicture()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    List<FLAC::Picture *> lst = f->pictureList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(1), lst.size());

    FLAC::Picture *newpic = new FLAC::Picture();
    newpic->setType(FLAC::Picture::BackCover);
    newpic->setWidth(5);
    newpic->setHeight(6);
    newpic->setColorDepth(16);
    newpic->setNumColors(7);
    newpic->setMimeType("image/jpeg");
    newpic->setDescription("new image");
    newpic->setData("JPEG data");
    f->removePictures();
    f->addPicture(newpic);
    f->save();
    delete f;

    f = new FLAC::File(newname.c_str());
    lst = f->pictureList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(1), lst.size());

    FLAC::Picture *pic = lst[0];
    CPPUNIT_ASSERT_EQUAL(FLAC::Picture::BackCover, pic->type());
    CPPUNIT_ASSERT_EQUAL(5, pic->width());
    CPPUNIT_ASSERT_EQUAL(6, pic->height());
    CPPUNIT_ASSERT_EQUAL(16, pic->colorDepth());
    CPPUNIT_ASSERT_EQUAL(7, pic->numColors());
    CPPUNIT_ASSERT_EQUAL(String("image/jpeg"), pic->mimeType());
    CPPUNIT_ASSERT_EQUAL(String("new image"), pic->description());
    CPPUNIT_ASSERT_EQUAL(ByteVector("JPEG data"), pic->data());
    delete f;
  }

  void testRemoveAllPictures()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    List<FLAC::Picture *> lst = f->pictureList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(1), lst.size());

    f->removePictures();
    f->save();
    delete f;

    f = new FLAC::File(newname.c_str());
    lst = f->pictureList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(0), lst.size());
    delete f;
  }

  void testRepeatedSave()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    Tag *tag = f->tag();
    CPPUNIT_ASSERT_EQUAL(String("Silence"), tag->title());
    tag->setTitle("NEW TITLE");
    f->save();
    CPPUNIT_ASSERT_EQUAL(String("NEW TITLE"), tag->title());
    tag->setTitle("NEW TITLE 2");
    f->save();
    CPPUNIT_ASSERT_EQUAL(String("NEW TITLE 2"), tag->title());
    delete f;

    f = new FLAC::File(newname.c_str());
    tag = f->tag();
    CPPUNIT_ASSERT_EQUAL(String("NEW TITLE 2"), tag->title());
    delete f;
  }

  void testSaveMultipleValues()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    Ogg::XiphComment* c = f->xiphComment(true);
    c->addField("ARTIST", "artist 1", true);
    c->addField("ARTIST", "artist 2", false);
    f->save();
    delete f;

    f = new FLAC::File(newname.c_str());
    c = f->xiphComment(true);
    Ogg::FieldListMap m = c->fieldListMap();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2), m["ARTIST"].size());
    CPPUNIT_ASSERT_EQUAL(String("artist 1"), m["ARTIST"][0]);
    CPPUNIT_ASSERT_EQUAL(String("artist 2"), m["ARTIST"][1]);
    delete f;
  }

  void testDict()
  {
    // test unicode & multiple values with dict interface
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    PropertyMap dict;
    dict["ARTIST"].append("artøst 1");
    dict["ARTIST"].append("artöst 2");
    f->setProperties(dict);
    f->save();
    delete f;

    f = new FLAC::File(newname.c_str());
    dict = f->properties();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2), dict["ARTIST"].size());
    CPPUNIT_ASSERT_EQUAL(String("artøst 1"), dict["ARTIST"][0]);
    CPPUNIT_ASSERT_EQUAL(String("artöst 2"), dict["ARTIST"][1]);
    delete f;
  }

  void testInvalid()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    PropertyMap map;
    map[L"H\x00c4\x00d6"] = String("bla");
    FLAC::File f(copy.fileName().c_str());
    PropertyMap invalid = f.setProperties(map);
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(1), invalid.size());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(0), f.properties().size());
  }

  void testShrinkPadding()
  {
    ScopedFileCopy copy("silence-44-s", ".flac");
    string newname = copy.fileName();

    {
      FLAC::File f(newname.c_str());
      f.addPicture(new FLAC::Picture(ByteVector(1000 * 1024, '\xff')));
      f.save();
      CPPUNIT_ASSERT(f.length() > 1000 * 1024);
    }

    {
      FLAC::File f(newname.c_str());
      f.removePictures();
      f.save();
      CPPUNIT_ASSERT(f.length() < 100 * 1024);
    }
  }

  void testSaveXiphTwice()
  {
    ScopedFileCopy copy1("no-tags", ".flac");
    ScopedFileCopy copy2("no-tags", ".flac");

    {
      FLAC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)4692, f.length());

      f.xiphComment(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)4692, f.length());
    }

    {
      FLAC::File f(copy2.fileName().c_str());
      f.xiphComment(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasXiphComment());
      CPPUNIT_ASSERT_EQUAL((long)4692, f.length());
    }
  }

  void testSaveID3v1Twice()
  {
    ScopedFileCopy copy1("no-tags", ".flac");
    ScopedFileCopy copy2("no-tags", ".flac");

    {
      FLAC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)4692, f.length());

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)4820, f.length());
    }

    {
      FLAC::File f(copy2.fileName().c_str());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT_EQUAL((long)4820, f.length());
    }
  }

  void testSaveID3v2Twice()
  {
    ScopedFileCopy copy1("no-tags", ".flac");
    ScopedFileCopy copy2("no-tags", ".flac");

    {
      FLAC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)4692, f.length());

      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)5760, f.length());
    }

    {
      FLAC::File f(copy2.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL((long)5760, f.length());
    }
  }

  void testSaveTagCombination()
  {
    ScopedFileCopy copy1("no-tags", ".flac");

    {
      FLAC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasXiphComment());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
      f.xiphComment(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
    }

    {
      FLAC::File f(copy1.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)5888, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.xiphComment()->title());
    }

    ScopedFileCopy copy2("no-tags", ".flac");

    {
      FLAC::File f(copy2.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasXiphComment());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
      f.xiphComment(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
    }

    {
      FLAC::File f(copy2.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)5888, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.xiphComment()->title());
    }

    ScopedFileCopy copy3("no-tags", ".flac");

    {
      FLAC::File f(copy3.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasXiphComment());
      f.xiphComment(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(!f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
      f.ID3v2Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();

      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());
    }

    {
      FLAC::File f(copy3.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL((long)5888, f.length());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasXiphComment());

      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v1Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("01234 56789 ABCDE FGHIJ"), f.xiphComment()->title());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFLAC);
