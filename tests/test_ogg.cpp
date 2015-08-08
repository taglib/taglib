#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <tpropertymap.h>
#include <oggfile.h>
#include <vorbisfile.h>
#include <oggpageheader.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestOGG : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestOGG);
  CPPUNIT_TEST(testSimple);
  CPPUNIT_TEST(testSplitPackets);
  CPPUNIT_TEST(testDictInterface1);
  CPPUNIT_TEST(testDictInterface2);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST_SUITE_END();

public:

  void testSimple()
  {
    ScopedFileCopy copy("empty", ".ogg");
    string newname = copy.fileName();

    Vorbis::File *f = new Vorbis::File(newname.c_str());
    f->tag()->setArtist("The Artist");
    f->save();
    delete f;

    f = new Vorbis::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String("The Artist"), f->tag()->artist());
    delete f;
  }

  void testSplitPackets()
  {
    ScopedFileCopy copy("empty", ".ogg");
    string newname = copy.fileName();

    String longText(std::string(128 * 1024, ' ').c_str());
    for (size_t i = 0; i < longText.length(); ++i)
      longText[i] = static_cast<wchar>(L'A' + (i % 26));

    Vorbis::File *f = new Vorbis::File(newname.c_str());
    f->tag()->setTitle(longText);
    f->save();
    delete f;

    f = new Vorbis::File(newname.c_str());
    CPPUNIT_ASSERT(f->isValid());
    CPPUNIT_ASSERT_EQUAL(19, f->lastPageHeader()->pageSequenceNumber());
    CPPUNIT_ASSERT_EQUAL(longText, f->tag()->title());
    f->tag()->setTitle("ABCDE");
    f->save();
    delete f;

    f = new Vorbis::File(newname.c_str());
    CPPUNIT_ASSERT(f->isValid());
    CPPUNIT_ASSERT_EQUAL(3, f->lastPageHeader()->pageSequenceNumber());
    CPPUNIT_ASSERT_EQUAL(String("ABCDE"), f->tag()->title());
    delete f;
  }

  void testDictInterface1()
  {
    ScopedFileCopy copy("empty", ".ogg");
    string newname = copy.fileName();

    Vorbis::File *f = new Vorbis::File(newname.c_str());

    CPPUNIT_ASSERT_EQUAL(TagLib::uint(0), f->tag()->properties().size());

    PropertyMap newTags;
    StringList values("value 1");
    values.append("value 2");
    newTags["ARTIST"] = values;
    f->tag()->setProperties(newTags);

    PropertyMap map = f->tag()->properties();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(1), map.size());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2), map["ARTIST"].size());
    CPPUNIT_ASSERT_EQUAL(String("value 1"), map["ARTIST"][0]);
    delete f;

  }

  void testDictInterface2()
  {
    ScopedFileCopy copy("test", ".ogg");
    string newname = copy.fileName();

    Vorbis::File *f = new Vorbis::File(newname.c_str());
    PropertyMap tags = f->tag()->properties();

    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2), tags["UNUSUALTAG"].size());
    CPPUNIT_ASSERT_EQUAL(String("usual value"), tags["UNUSUALTAG"][0]);
    CPPUNIT_ASSERT_EQUAL(String("another value"), tags["UNUSUALTAG"][1]);
    CPPUNIT_ASSERT_EQUAL(
      String("\xC3\xB6\xC3\xA4\xC3\xBC\x6F\xCE\xA3\xC3\xB8", String::UTF8),
      tags["UNICODETAG"][0]);

    tags["UNICODETAG"][0] = String(
      "\xCE\xBD\xCE\xB5\xCF\x89\x20\xCE\xBD\xCE\xB1\xCE\xBB\xCF\x85\xCE\xB5", String::UTF8);
    tags.erase("UNUSUALTAG");
    f->tag()->setProperties(tags);
    CPPUNIT_ASSERT_EQUAL(
      String("\xCE\xBD\xCE\xB5\xCF\x89\x20\xCE\xBD\xCE\xB1\xCE\xBB\xCF\x85\xCE\xB5", String::UTF8),
      f->tag()->properties()["UNICODETAG"][0]);
    CPPUNIT_ASSERT_EQUAL(false, f->tag()->properties().contains("UNUSUALTAG"));

    delete f;
  }

  void testAudioProperties()
  {
    Ogg::Vorbis::File f(TEST_FILE_PATH_C("empty.ogg"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(9, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->vorbisVersion());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrateMaximum());
    CPPUNIT_ASSERT_EQUAL(112000, f.audioProperties()->bitrateNominal());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->bitrateMinimum());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestOGG);
