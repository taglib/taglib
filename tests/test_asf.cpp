#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <asffile.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestASF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestASF);
  CPPUNIT_TEST(testProperties);
  CPPUNIT_TEST(testRead);
  CPPUNIT_TEST(testSaveMultipleValues);
  CPPUNIT_TEST(testSaveStream);
  CPPUNIT_TEST(testSaveLanguage);
  CPPUNIT_TEST(testDWordTrackNumber);
  CPPUNIT_TEST(testSaveLargeValue);
  CPPUNIT_TEST_SUITE_END();

public:

  void testProperties()
  {
    ASF::File f("data/silence-1.wma");
    CPPUNIT_ASSERT_EQUAL(4, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(48000, f.audioProperties()->sampleRate());
  }

  void testRead()
  {
    ASF::File f("data/silence-1.wma");
    CPPUNIT_ASSERT_EQUAL(String("test"), f.tag()->title());
  }

  void testSaveMultipleValues()
  {
    ScopedFileCopy copy("silence-1", ".wma");
    string newname = copy.fileName();

    ASF::File *f = new ASF::File(newname.c_str());
    ASF::AttributeList values;
    values.append("Foo");
    values.append("Bar");
    f->tag()->attributeListMap()["WM/AlbumTitle"] = values;
    f->save();
    delete f;

    f = new ASF::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(2, (int)f->tag()->attributeListMap()["WM/AlbumTitle"].size());
    delete f;
  }

  void testDWordTrackNumber()
  {
    ScopedFileCopy copy("silence-1", ".wma");
    string newname = copy.fileName();

    ASF::File *f = new ASF::File(newname.c_str());
    CPPUNIT_ASSERT(!f->tag()->attributeListMap().contains("WM/TrackNumber"));
    f->tag()->setAttribute("WM/TrackNumber", (unsigned int)(123));
    f->save();
    delete f;

    f = new ASF::File(newname.c_str());
    CPPUNIT_ASSERT(f->tag()->attributeListMap().contains("WM/TrackNumber"));
    CPPUNIT_ASSERT_EQUAL(ASF::Attribute::DWordType, f->tag()->attributeListMap()["WM/TrackNumber"].front().type());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(123), f->tag()->track());
    f->tag()->setTrack(234);
    f->save();
    delete f;

    f = new ASF::File(newname.c_str());
    CPPUNIT_ASSERT(f->tag()->attributeListMap().contains("WM/TrackNumber"));
    CPPUNIT_ASSERT_EQUAL(ASF::Attribute::UnicodeType, f->tag()->attributeListMap()["WM/TrackNumber"].front().type());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(234), f->tag()->track());
    delete f;
  }

  void testSaveStream()
  {
    ScopedFileCopy copy("silence-1", ".wma");
    string newname = copy.fileName();

    ASF::File *f = new ASF::File(newname.c_str());
    ASF::AttributeList values;
    ASF::Attribute attr("Foo");
    attr.setStream(43);
    values.append(attr);
    f->tag()->attributeListMap()["WM/AlbumTitle"] = values;
    f->save();
    delete f;

    f = new ASF::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(43, f->tag()->attributeListMap()["WM/AlbumTitle"][0].stream());
    delete f;
  }

  void testSaveLanguage()
  {
    ScopedFileCopy copy("silence-1", ".wma");
    string newname = copy.fileName();

    ASF::File *f = new ASF::File(newname.c_str());
    ASF::AttributeList values;
    ASF::Attribute attr("Foo");
    attr.setStream(32);
    attr.setLanguage(56);
    values.append(attr);
    f->tag()->attributeListMap()["WM/AlbumTitle"] = values;
    f->save();
    delete f;

    f = new ASF::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(32, f->tag()->attributeListMap()["WM/AlbumTitle"][0].stream());
    CPPUNIT_ASSERT_EQUAL(56, f->tag()->attributeListMap()["WM/AlbumTitle"][0].language());
    delete f;
  }

  void testSaveLargeValue()
  {
    ScopedFileCopy copy("silence-1", ".wma");
    string newname = copy.fileName();

    ASF::File *f = new ASF::File(newname.c_str());
    ASF::AttributeList values;
    ASF::Attribute attr(ByteVector(70000, 'x'));
    values.append(attr);
    f->tag()->attributeListMap()["WM/Blob"] = values;
    f->save();
    delete f;

    f = new ASF::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(ByteVector(70000, 'x'), f->tag()->attributeListMap()["WM/Blob"][0].toByteVector());
    delete f;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestASF);
