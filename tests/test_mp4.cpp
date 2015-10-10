#include <string>
#include <stdio.h>
#include <tag.h>
#include <mp4tag.h>
#include <tbytevectorlist.h>
#include <tpropertymap.h>
#include <mp4atom.h>
#include <mp4file.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMP4 : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMP4);
  CPPUNIT_TEST(testPropertiesAAC);
  CPPUNIT_TEST(testPropertiesALAC);
  CPPUNIT_TEST(testFreeForm);
  CPPUNIT_TEST(testCheckValid);
  CPPUNIT_TEST(testIsEmpty);
  CPPUNIT_TEST(testUpdateStco);
  CPPUNIT_TEST(testSaveExisingWhenIlstIsLast);
  CPPUNIT_TEST(test64BitAtom);
  CPPUNIT_TEST(testGnre);
  CPPUNIT_TEST(testCovrRead);
  CPPUNIT_TEST(testCovrWrite);
  CPPUNIT_TEST(testCovrRead2);
  CPPUNIT_TEST(testProperties);
  CPPUNIT_TEST(testFuzzedFile);
  CPPUNIT_TEST_SUITE_END();

public:

  void testPropertiesAAC()
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3707, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(MP4::Properties::AAC, f.audioProperties()->codec());
  }

  void testPropertiesALAC()
  {
    MP4::File f(TEST_FILE_PATH_C("empty_alac.m4a"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3705, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(MP4::Properties::ALAC, f.audioProperties()->codec());
  }

  void testCheckValid()
  {
    MP4::File f(TEST_FILE_PATH_C("empty.aiff"));
    CPPUNIT_ASSERT(!f.isValid());
    MP4::File f2(TEST_FILE_PATH_C("has-tags.m4a"));
    CPPUNIT_ASSERT(f2.isValid());
  }

  void testIsEmpty()
  {
    MP4::Tag t1;
    CPPUNIT_ASSERT(t1.isEmpty());
    t1.setArtist("Foo");
    CPPUNIT_ASSERT(!t1.isEmpty());

    MP4::Tag t2;
    t2.setItem("foo", "bar");
    CPPUNIT_ASSERT(!t2.isEmpty());
  }

  void testUpdateStco()
  {
    ScopedFileCopy copy("no-tags", ".3g2");
    string filename = copy.fileName();

    MP4::File *f = new MP4::File(filename.c_str());
    f->tag()->setArtist(ByteVector(3000, 'x'));

    ByteVectorList data1;
    {
      MP4::Atoms a(f);
      MP4::Atom *stco = a.find("moov")->findall("stco", true)[0];
      f->seek(stco->offset + 12);
      ByteVector data = f->readBlock(stco->length - 12);
      unsigned int count = data.mid(0, 4).toUInt();
      int pos = 4;
      while (count--) {
        unsigned int offset = data.mid(pos, 4).toUInt();
        f->seek(offset);
        data1.append(f->readBlock(20));
        pos += 4;
      }
    }

    f->save();
    delete f;
    f = new MP4::File(filename.c_str());

    {
      MP4::Atoms a(f);
      MP4::Atom *stco = a.find("moov")->findall("stco", true)[0];
      f->seek(stco->offset + 12);
      ByteVector data = f->readBlock(stco->length - 12);
      unsigned int count = data.mid(0, 4).toUInt();
      int pos = 4, i = 0;
      while (count--) {
        unsigned int offset = data.mid(pos, 4).toUInt();
        f->seek(offset);
        CPPUNIT_ASSERT_EQUAL(data1[i], f->readBlock(20));
        pos += 4;
        i++;
      }
    }

    delete f;
  }

  void testFreeForm()
  {
    ScopedFileCopy copy("has-tags", ".m4a");
    string filename = copy.fileName();

    MP4::File *f = new MP4::File(filename.c_str());
    CPPUNIT_ASSERT(f->tag()->contains("----:com.apple.iTunes:iTunNORM"));
    f->tag()->setItem("----:org.kde.TagLib:Foo", StringList("Bar"));
    f->save();
    delete f;

    f = new MP4::File(filename.c_str());
    CPPUNIT_ASSERT(f->tag()->contains("----:org.kde.TagLib:Foo"));
    CPPUNIT_ASSERT_EQUAL(String("Bar"),
                         f->tag()->item("----:org.kde.TagLib:Foo").toStringList().front());
    f->save();
    delete f;
  }

  void testSaveExisingWhenIlstIsLast()
  {
    ScopedFileCopy copy("ilst-is-last", ".m4a");
    string filename = copy.fileName();

    MP4::File *f = new MP4::File(filename.c_str());
    CPPUNIT_ASSERT_EQUAL(String("82,164"),
                         f->tag()->item("----:com.apple.iTunes:replaygain_track_minmax").toStringList().front());
    CPPUNIT_ASSERT_EQUAL(String("Pearl Jam"), f->tag()->artist());
    f->tag()->setComment("foo");
    f->save();
    delete f;

    f = new MP4::File(filename.c_str());
    CPPUNIT_ASSERT_EQUAL(String("82,164"),
                         f->tag()->item("----:com.apple.iTunes:replaygain_track_minmax").toStringList().front());
    CPPUNIT_ASSERT_EQUAL(String("Pearl Jam"), f->tag()->artist());
    CPPUNIT_ASSERT_EQUAL(String("foo"), f->tag()->comment());
    delete f;
  }

  void test64BitAtom()
  {
    ScopedFileCopy copy("64bit", ".mp4");
    string filename = copy.fileName();

    MP4::File *f = new MP4::File(filename.c_str());
    CPPUNIT_ASSERT_EQUAL(true, f->tag()->itemMap()["cpil"].toBool());

    MP4::Atoms *atoms = new MP4::Atoms(f);
    MP4::Atom *moov = atoms->atoms[0];
    CPPUNIT_ASSERT_EQUAL(long(77), moov->length);

    f->tag()->setItem("pgap", true);
    f->save();
    delete atoms;
    delete f;

    f = new MP4::File(filename.c_str());
    CPPUNIT_ASSERT_EQUAL(true, f->tag()->item("cpil").toBool());
    CPPUNIT_ASSERT_EQUAL(true, f->tag()->item("pgap").toBool());

    atoms = new MP4::Atoms(f);
    moov = atoms->atoms[0];
    // original size + 'pgap' size + padding
    CPPUNIT_ASSERT_EQUAL(long(77 + 25 + 974), moov->length);
    delete atoms;
    delete f;
  }

  void testGnre()
  {
    MP4::File *f = new MP4::File(TEST_FILE_PATH_C("gnre.m4a"));
    CPPUNIT_ASSERT_EQUAL(TagLib::String("Ska"), f->tag()->genre());
    delete f;
  }

  void testCovrRead()
  {
    MP4::File *f = new MP4::File(TEST_FILE_PATH_C("has-tags.m4a"));
    CPPUNIT_ASSERT(f->tag()->contains("covr"));
    MP4::CoverArtList l = f->tag()->item("covr").toCoverArtList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2), l.size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[0].format());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(79), l[0].data().size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, l[1].format());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(287), l[1].data().size());
    delete f;
  }

  void testCovrWrite()
  {
    ScopedFileCopy copy("has-tags", ".m4a");
    string filename = copy.fileName();

    MP4::File *f = new MP4::File(filename.c_str());
    CPPUNIT_ASSERT(f->tag()->contains("covr"));
    MP4::CoverArtList l = f->tag()->item("covr").toCoverArtList();
    l.append(MP4::CoverArt(MP4::CoverArt::PNG, "foo"));
    f->tag()->setItem("covr", l);
    f->save();
    delete f;

    f = new MP4::File(filename.c_str());
    CPPUNIT_ASSERT(f->tag()->contains("covr"));
    l = f->tag()->item("covr").toCoverArtList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(3), l.size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[0].format());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(79), l[0].data().size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, l[1].format());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(287), l[1].data().size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[2].format());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(3), l[2].data().size());
    delete f;
  }

  void testCovrRead2()
  {
    MP4::File *f = new MP4::File(TEST_FILE_PATH_C("covr-junk.m4a"));
    CPPUNIT_ASSERT(f->tag()->contains("covr"));
    MP4::CoverArtList l = f->tag()->item("covr").toCoverArtList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(2), l.size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[0].format());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(79), l[0].data().size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, l[1].format());
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(287), l[1].data().size());
    delete f;
  }

  void testProperties()
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));

    PropertyMap tags = f.properties();

    CPPUNIT_ASSERT_EQUAL(StringList("Test Artist"), tags["ARTIST"]);

    tags["TRACKNUMBER"] = StringList("2/4");
    tags["DISCNUMBER"] = StringList("3/5");
    tags["BPM"] = StringList("123");
    tags["ARTIST"] = StringList("Foo Bar");
    tags["COMPILATION"] = StringList("1");
    f.setProperties(tags);

    tags = f.properties();

    CPPUNIT_ASSERT(f.tag()->contains("trkn"));
    CPPUNIT_ASSERT_EQUAL(2, f.tag()->item("trkn").toIntPair().first);
    CPPUNIT_ASSERT_EQUAL(4, f.tag()->item("trkn").toIntPair().second);
    CPPUNIT_ASSERT_EQUAL(StringList("2/4"), tags["TRACKNUMBER"]);

    CPPUNIT_ASSERT(f.tag()->contains("disk"));
    CPPUNIT_ASSERT_EQUAL(3, f.tag()->item("disk").toIntPair().first);
    CPPUNIT_ASSERT_EQUAL(5, f.tag()->item("disk").toIntPair().second);
    CPPUNIT_ASSERT_EQUAL(StringList("3/5"), tags["DISCNUMBER"]);

    CPPUNIT_ASSERT(f.tag()->contains("tmpo"));
    CPPUNIT_ASSERT_EQUAL(123, f.tag()->item("tmpo").toInt());
    CPPUNIT_ASSERT_EQUAL(StringList("123"), tags["BPM"]);

    CPPUNIT_ASSERT(f.tag()->contains("\251ART"));
    CPPUNIT_ASSERT_EQUAL(StringList("Foo Bar"), f.tag()->item("\251ART").toStringList());
    CPPUNIT_ASSERT_EQUAL(StringList("Foo Bar"), tags["ARTIST"]);

    CPPUNIT_ASSERT(f.tag()->contains("cpil"));
    CPPUNIT_ASSERT_EQUAL(true, f.tag()->item("cpil").toBool());
    CPPUNIT_ASSERT_EQUAL(StringList("1"), tags["COMPILATION"]);

    tags["COMPILATION"] = StringList("0");
    f.setProperties(tags);

    tags = f.properties();

    CPPUNIT_ASSERT(f.tag()->contains("cpil"));
    CPPUNIT_ASSERT_EQUAL(false, f.tag()->item("cpil").toBool());
    CPPUNIT_ASSERT_EQUAL(StringList("0"), tags["COMPILATION"]);

    // Empty properties do not result in access violations
    // when converting integers
    tags["TRACKNUMBER"] = StringList();
    tags["DISCNUMBER"] = StringList();
    tags["BPM"] = StringList();
    tags["COMPILATION"] = StringList();
    f.setProperties(tags);
  }

  void testFuzzedFile()
  {
    MP4::File f(TEST_FILE_PATH_C("infloop.m4a"));
    CPPUNIT_ASSERT(f.isValid());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMP4);
