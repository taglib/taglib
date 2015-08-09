#include <string>
#include <stdio.h>
#include <tag.h>
#include <tpropertymap.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

#include <ebmlmatroskafile.h>
#include <ebmlmatroskaaudio.h>

using namespace std;
using namespace TagLib;

class TestMatroska : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMatroska);
  CPPUNIT_TEST(testPredefined);
  CPPUNIT_TEST(testInsertAndExtract);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST_SUITE_END();

public:
  void testPredefined()
  {
    ScopedFileCopy copy("matroska", ".mka");
    string filename = copy.fileName();

    EBML::Matroska::File f(filename.c_str());
    CPPUNIT_ASSERT(f.isValid());

    PropertyMap pm = f.properties();
    PropertyMap::Iterator i = pm.find("ENCODER");
    CPPUNIT_ASSERT(i != pm.end());

    CPPUNIT_ASSERT_EQUAL(String("Lavf54.63.104"), i->second.front());
  }

  void testInsertAndExtract()
  {
    ScopedFileCopy copy("matroska", ".mka");
    string filename = copy.fileName();

    {
      EBML::Matroska::File f1(filename.c_str());
      CPPUNIT_ASSERT(f1.isValid());

      Tag* t = f1.tag();

      CPPUNIT_ASSERT(t != 0);
      t->setTitle("Seconds of Silence");
      t->setArtist("Nobody");
      t->setAlbum("TagLib Test Suite");
      t->setComment("Well, there's nothing to say - a few special signs: ©’…ä–€ſ");
      t->setGenre("Air");
      t->setYear(2013);
      t->setTrack(15);

      CPPUNIT_ASSERT(f1.save());
    }
    {
      EBML::Matroska::File f2(filename.c_str());
      CPPUNIT_ASSERT(f2.isValid());

      Tag* t = f2.tag();

      CPPUNIT_ASSERT(t != 0);
      CPPUNIT_ASSERT_EQUAL(String("Seconds of Silence"), t->title());
      CPPUNIT_ASSERT_EQUAL(String("Nobody"), t->artist());
      CPPUNIT_ASSERT_EQUAL(String("TagLib Test Suite"), t->album());
      CPPUNIT_ASSERT_EQUAL(String("Well, there's nothing to say - a few special signs: ©’…ä–€ſ"), t->comment());
      CPPUNIT_ASSERT_EQUAL(String("Air"), t->genre());
      CPPUNIT_ASSERT_EQUAL(2013u, t->year());
      CPPUNIT_ASSERT_EQUAL(15u, t->track());

      PropertyMap pm = f2.properties();
      pm.erase("COMMENT");
      f2.setProperties(pm);

      CPPUNIT_ASSERT(f2.save());
    }

    {
      EBML::Matroska::File f3(filename.c_str());
      CPPUNIT_ASSERT(f3.isValid());

      PropertyMap pm = f3.properties();
      PropertyMap::Iterator i = pm.find("GENRE");

      CPPUNIT_ASSERT(i != pm.end());
      CPPUNIT_ASSERT_EQUAL(String("Air"), i->second.front());
    }
  }

  void testAudioProperties()
  {
    ScopedFileCopy copy("matroska", ".mka");
    string filename = copy.fileName();

    EBML::Matroska::File f(filename.c_str());
    CPPUNIT_ASSERT(f.isValid());

    CPPUNIT_ASSERT(f.audioProperties());

    // Not a very nice assertion...
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(735, f.audioProperties()->lengthInMilliseconds());

    // Bitrate is not nice and thus not tested.
    CPPUNIT_ASSERT_EQUAL(51, f.audioProperties()->bitrate());

    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMatroska);
