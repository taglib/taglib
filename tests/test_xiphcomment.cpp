#include <string>
#include <stdio.h>
#include <xiphcomment.h>
#include <tpropertymap.h>
#include <flacfile.h>
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
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(3), unsuccessful.size());
    CPPUNIT_ASSERT(cmt.properties().isEmpty());
  }

  void testRemoveFields()
  {
    ScopedFileCopy copy("sinewave", ".flac");

    {
      FLAC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL(String("reference libFLAC 1.3.1 20141125"), f.xiphComment()->vendorID());
      f.xiphComment()->addField("TITLE", "Title1");
      f.xiphComment()->addField("TITLE", "Title2", false);
      f.xiphComment()->addField("TITLE", "Title2", false);
      f.xiphComment()->addField("ARTIST", "Artist1");
      f.xiphComment()->addField("ARTIST", "Artist2", false);
      f.xiphComment()->addField("ALBUM", "Album1");
      f.xiphComment()->addField("ALBUM", "Album2", false);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL(String("Title1 Title2 Title2"), f.xiphComment()->title());
      f.xiphComment()->removeFields("TITLE", "Title2");
      CPPUNIT_ASSERT_EQUAL(String("Title1"), f.xiphComment()->title());
      f.xiphComment()->removeFields("ARTIST");
      CPPUNIT_ASSERT_EQUAL(String(), f.xiphComment()->artist());
      f.xiphComment()->removeAllFields();
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL(String("reference libFLAC 1.3.1 20141125"), f.xiphComment()->vendorID());
      CPPUNIT_ASSERT(f.xiphComment()->fieldListMap().isEmpty());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestXiphComment);
