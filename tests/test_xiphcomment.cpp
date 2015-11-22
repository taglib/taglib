#include <string>
#include <stdio.h>
#include <xiphcomment.h>
#include <vorbisfile.h>
#include <tpropertymap.h>
#include <tdebug.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestXiphComment : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestXiphComment);
  CPPUNIT_TEST(testYear);
  CPPUNIT_TEST(testSetYear);
  CPPUNIT_TEST(testTrack);
  CPPUNIT_TEST(testSetTrack);
  CPPUNIT_TEST(testInvalidKeys);
  CPPUNIT_TEST(testClearComment);
  CPPUNIT_TEST(testRemoveFields);
  CPPUNIT_TEST_SUITE_END();

public:

  void testYear()
  {
    Ogg::XiphComment cmt;
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(0), cmt.year());
    cmt.addField("YEAR", "2009");
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2009), cmt.year());
    cmt.addField("DATE", "2008");
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2008), cmt.year());
  }

  void testSetYear()
  {
    Ogg::XiphComment cmt;
    cmt.addField("YEAR", "2009");
    cmt.addField("DATE", "2008");
    cmt.setYear(1995);
    CPPUNIT_ASSERT(cmt.fieldListMap()["YEAR"].isEmpty());
    CPPUNIT_ASSERT_EQUAL(String("1995"), cmt.fieldListMap()["DATE"].front());
  }

  void testTrack()
  {
    Ogg::XiphComment cmt;
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(0), cmt.track());
    cmt.addField("TRACKNUM", "7");
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(7), cmt.track());
    cmt.addField("TRACKNUMBER", "8");
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(8), cmt.track());
  }

  void testSetTrack()
  {
    Ogg::XiphComment cmt;
    cmt.addField("TRACKNUM", "7");
    cmt.addField("TRACKNUMBER", "8");
    cmt.setTrack(3);
    CPPUNIT_ASSERT(cmt.fieldListMap()["TRACKNUM"].isEmpty());
    CPPUNIT_ASSERT_EQUAL(String("3"), cmt.fieldListMap()["TRACKNUMBER"].front());
  }

  void testInvalidKeys()
  {
    PropertyMap map;
    map[""] = String("invalid key: empty string");
    map["A=B"] = String("invalid key: contains '='");
    map["A~B"] = String("invalid key: contains '~'");

    Ogg::XiphComment cmt;
    PropertyMap unsuccessful = cmt.setProperties(map);
    CPPUNIT_ASSERT_EQUAL(size_t(3), unsuccessful.size());
    CPPUNIT_ASSERT(cmt.properties().isEmpty());
  }

  void testClearComment()
  {
    ScopedFileCopy copy("empty", ".ogg");

    {
      Ogg::Vorbis::File f(copy.fileName().c_str());
      f.tag()->addField("COMMENT", "Comment1");
      f.save();
    }
    {
      Ogg::Vorbis::File f(copy.fileName().c_str());
      f.tag()->setComment("");
      CPPUNIT_ASSERT_EQUAL(String(""), f.tag()->comment());
    }
  }

  void testRemoveFields()
  {
    Ogg::Vorbis::File f(TEST_FILE_PATH_C("empty.ogg"));
    f.tag()->addField("title", "Title1");
    f.tag()->addField("Title", "Title1", false);
    f.tag()->addField("titlE", "Title2", false);
    f.tag()->addField("TITLE", "Title3", false);
    f.tag()->addField("artist", "Artist1");
    f.tag()->addField("ARTIST", "Artist2", false);
    CPPUNIT_ASSERT_EQUAL(String("Title1 Title1 Title2 Title3"), f.tag()->title());
    CPPUNIT_ASSERT_EQUAL(String("Artist1 Artist2"), f.tag()->artist());

    f.tag()->removeFields("title", "Title1");
    CPPUNIT_ASSERT_EQUAL(String("Title2 Title3"), f.tag()->title());
    CPPUNIT_ASSERT_EQUAL(String("Artist1 Artist2"), f.tag()->artist());

    f.tag()->removeFields("Artist");
    CPPUNIT_ASSERT_EQUAL(String("Title2 Title3"), f.tag()->title());
    CPPUNIT_ASSERT(f.tag()->artist().isEmpty());

    f.tag()->removeAllFields();
    CPPUNIT_ASSERT(f.tag()->title().isEmpty());
    CPPUNIT_ASSERT(f.tag()->artist().isEmpty());
    CPPUNIT_ASSERT_EQUAL(String("Xiph.Org libVorbis I 20050304"), f.tag()->vendorID());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestXiphComment);
