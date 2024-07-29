/***************************************************************************
    copyright           : (C) 2008 by Lukas Lalinsky
    email               : lukas@oxygene.sk
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <string>
#include <cstdio>

#include "tbytevectorlist.h"
#include "tbytevectorstream.h"
#include "tpropertymap.h"
#include "tag.h"
#include "mp4tag.h"
#include "mp4atom.h"
#include "mp4file.h"
#include "mp4itemfactory.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

namespace
{

  class CustomItemFactory : public MP4::ItemFactory {
  public:
    CustomItemFactory(const CustomItemFactory &) = delete;
    CustomItemFactory &operator=(const CustomItemFactory &) = delete;
    static CustomItemFactory *instance() { return &factory; }
  protected:
    CustomItemFactory() = default;
    ~CustomItemFactory() = default;
    NameHandlerMap nameHandlerMap() const override
    {
      return MP4::ItemFactory::nameHandlerMap()
        .insert("tsti", ItemHandlerType::Int)
        .insert("tstt", ItemHandlerType::Text);
    }

    Map<ByteVector, String> namePropertyMap() const override
    {
      return MP4::ItemFactory::namePropertyMap()
        .insert("tsti", "TESTINTEGER");
    }
  private:
    static CustomItemFactory factory;
  };

  CustomItemFactory CustomItemFactory::factory;
}  // namespace

class TestMP4 : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMP4);
  CPPUNIT_TEST(testPropertiesAAC);
  CPPUNIT_TEST(testPropertiesAACWithoutBitrate);
  CPPUNIT_TEST(testPropertiesALAC);
  CPPUNIT_TEST(testPropertiesALACWithoutBitrate);
  CPPUNIT_TEST(testPropertiesAACWithoutLength);
  CPPUNIT_TEST(testPropertiesM4V);
  CPPUNIT_TEST(testFreeForm);
  CPPUNIT_TEST(testCheckValid);
  CPPUNIT_TEST(testHasTag);
  CPPUNIT_TEST(testIsEmpty);
  CPPUNIT_TEST(testUpdateStco);
  CPPUNIT_TEST(testSaveExisingWhenIlstIsLast);
  CPPUNIT_TEST(test64BitAtom);
  CPPUNIT_TEST(testGnre);
  CPPUNIT_TEST(testCovrRead);
  CPPUNIT_TEST(testCovrWrite);
  CPPUNIT_TEST(testCovrRead2);
  CPPUNIT_TEST(testProperties);
  CPPUNIT_TEST(testPropertiesAllSupported);
  CPPUNIT_TEST(testPropertiesMovement);
  CPPUNIT_TEST(testFuzzedFile);
  CPPUNIT_TEST(testRepeatedSave);
  CPPUNIT_TEST(testWithZeroLengthAtom);
  CPPUNIT_TEST(testEmptyValuesRemoveItems);
  CPPUNIT_TEST(testRemoveMetadata);
  CPPUNIT_TEST(testNonFullMetaAtom);
  CPPUNIT_TEST(testItemFactory);
  CPPUNIT_TEST(testNonPrintableAtom);
  CPPUNIT_TEST_SUITE_END();

public:

  void testPropertiesAAC()
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3708, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(MP4::Properties::AAC, f.audioProperties()->codec());
  }

  void testPropertiesAACWithoutBitrate()
  {
    ByteVector aacData = PlainFile(TEST_FILE_PATH_C("has-tags.m4a")).readAll();
    CPPUNIT_ASSERT_GREATER(1960U, aacData.size());
    CPPUNIT_ASSERT_EQUAL(ByteVector("mp4a"), aacData.mid(1890, 4));
    // Set the bitrate to zero
    for (int offset = 1956; offset < 1960; ++offset) {
      aacData[offset] = 0;
    }
    ByteVectorStream aacStream(aacData);
    MP4::File f(&aacStream);
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3708, f.audioProperties()->lengthInMilliseconds());
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
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3705, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(MP4::Properties::ALAC, f.audioProperties()->codec());
  }

  void testPropertiesALACWithoutBitrate()
  {
    ByteVector alacData = PlainFile(TEST_FILE_PATH_C("empty_alac.m4a")).readAll();
    CPPUNIT_ASSERT_GREATER(474U, alacData.size());
    CPPUNIT_ASSERT_EQUAL(ByteVector("alac"), alacData.mid(446, 4));
    // Set the bitrate to zero
    for (int offset = 470; offset < 474; ++offset) {
      alacData[offset] = 0;
    }
    ByteVectorStream alacStream(alacData);
    MP4::File f(&alacStream);
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3705, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(MP4::Properties::ALAC, f.audioProperties()->codec());
  }

  void testPropertiesAACWithoutLength()
  {
    ByteVector m4aData = PlainFile(TEST_FILE_PATH_C("no-tags.m4a")).readAll();
    CPPUNIT_ASSERT_EQUAL(2898U, m4aData.size());
    CPPUNIT_ASSERT_EQUAL(ByteVector("mdhd"), m4aData.mid(1749, 4));
    // Set the length to zero
    for (int offset = 1769; offset < 1773; ++offset) {
      m4aData[offset] = 0;
    }
    ByteVectorStream m4aStream(m4aData);
    MP4::File f(&m4aStream);
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3707, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(MP4::Properties::AAC, f.audioProperties()->codec());
  }

  void testPropertiesM4V()
  {
    MP4::File f(TEST_FILE_PATH_C("blank_video.m4v"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(975, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(96, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(MP4::Properties::AAC, f.audioProperties()->codec());
  }

  void testCheckValid()
  {
    MP4::File f(TEST_FILE_PATH_C("empty.aiff"));
    CPPUNIT_ASSERT(!f.isValid());
  }

  void testHasTag()
  {
    {
      MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasMP4Tag());
    }

    ScopedFileCopy copy("no-tags", ".m4a");

    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasMP4Tag());
      f.tag()->setTitle("TITLE");
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasMP4Tag());
    }
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

    ByteVectorList data1;

    {
      MP4::File f(filename.c_str());
      f.tag()->setArtist(ByteVector(3000, 'x'));

      MP4::Atoms a(&f);
      MP4::Atom *stco = a.find("moov")->findall("stco", true)[0];
      f.seek(stco->offset() + 12);
      ByteVector data = f.readBlock(stco->length() - 12);
      unsigned int count = data.mid(0, 4).toUInt();
      int pos = 4;
      while (count--) {
        unsigned int offset = data.mid(pos, 4).toUInt();
        f.seek(offset);
        data1.append(f.readBlock(20));
        pos += 4;
      }

      f.save();
    }

    {
      MP4::File f(filename.c_str());

      MP4::Atoms a(&f);
      MP4::Atom *stco = a.find("moov")->findall("stco", true)[0];
      f.seek(stco->offset() + 12);
      ByteVector data = f.readBlock(stco->length() - 12);
      unsigned int count = data.mid(0, 4).toUInt();
      int pos = 4, i = 0;
      while (count--) {
        unsigned int offset = data.mid(pos, 4).toUInt();
        f.seek(offset);
        CPPUNIT_ASSERT_EQUAL(data1[i], f.readBlock(20));
        pos += 4;
        i++;
      }
    }
  }

  void testFreeForm()
  {
    ScopedFileCopy copy("has-tags", ".m4a");
    string filename = copy.fileName();

    {
      MP4::File f(filename.c_str());
      CPPUNIT_ASSERT(f.tag()->contains("----:com.apple.iTunes:iTunNORM"));
      f.tag()->setItem("----:org.kde.TagLib:Foo", StringList("Bar"));
      f.save();
    }
    {
      MP4::File f(filename.c_str());
      CPPUNIT_ASSERT(f.tag()->contains("----:org.kde.TagLib:Foo"));
      CPPUNIT_ASSERT_EQUAL(String("Bar"),
                           f.tag()->item("----:org.kde.TagLib:Foo").toStringList().front());
      f.save();
    }
  }

  void testSaveExisingWhenIlstIsLast()
  {
    ScopedFileCopy copy("ilst-is-last", ".m4a");
    string filename = copy.fileName();

    {
      MP4::File f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(String("82,164"),
        f.tag()->item("----:com.apple.iTunes:replaygain_track_minmax").toStringList().front());
      CPPUNIT_ASSERT_EQUAL(String("Pearl Jam"), f.tag()->artist());
      f.tag()->setComment("foo");
      f.save();
    }
    {
      MP4::File f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(String("82,164"),
                           f.tag()->item("----:com.apple.iTunes:replaygain_track_minmax").toStringList().front());
      CPPUNIT_ASSERT_EQUAL(String("Pearl Jam"), f.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(String("foo"), f.tag()->comment());
    }
  }

  void test64BitAtom()
  {
    ScopedFileCopy copy("64bit", ".mp4");
    string filename = copy.fileName();

    {
      MP4::File f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(true, f.tag()->itemMap()["cpil"].toBool());

      MP4::Atoms atoms(&f);
      MP4::Atom *moov = atoms.atoms()[0];
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(77), moov->length());

      f.tag()->setItem("pgap", true);
      f.save();
    }
    {
      MP4::File f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(true, f.tag()->item("cpil").toBool());
      CPPUNIT_ASSERT_EQUAL(true, f.tag()->item("pgap").toBool());

      MP4::Atoms atoms(&f);
      MP4::Atom *moov = atoms.atoms()[0];
      // original size + 'pgap' size + padding
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(77 + 25 + 974), moov->length());
    }
  }

  void testGnre()
  {
    MP4::File f(TEST_FILE_PATH_C("gnre.m4a"));
    CPPUNIT_ASSERT_EQUAL(TagLib::String("Ska"), f.tag()->genre());
  }

  void testCovrRead()
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    CPPUNIT_ASSERT(f.tag()->contains("covr"));
    MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), l.size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[0].format());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(79), l[0].data().size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, l[1].format());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(287), l[1].data().size());
  }

  void testCovrWrite()
  {
    ScopedFileCopy copy("has-tags", ".m4a");
    string filename = copy.fileName();

    {
      MP4::File f(filename.c_str());
      CPPUNIT_ASSERT(f.tag()->contains("covr"));
      MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
      l.append(MP4::CoverArt(MP4::CoverArt::PNG, "foo"));
      f.tag()->setItem("covr", l);
      f.save();
    }
    {
      MP4::File f(filename.c_str());
      CPPUNIT_ASSERT(f.tag()->contains("covr"));
      MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), l.size());
      CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[0].format());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(79), l[0].data().size());
      CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, l[1].format());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(287), l[1].data().size());
      CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[2].format());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), l[2].data().size());
    }
  }

  void testCovrRead2()
  {
    MP4::File f(TEST_FILE_PATH_C("covr-junk.m4a"));
    CPPUNIT_ASSERT(f.tag()->contains("covr"));
    MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), l.size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[0].format());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(79), l[0].data().size());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, l[1].format());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(287), l[1].data().size());
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
    tags["REMIXEDBY"] = StringList("Remixed by");
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

    CPPUNIT_ASSERT(f.tag()->contains("----:com.apple.iTunes:REMIXEDBY"));
    CPPUNIT_ASSERT_EQUAL(StringList("Remixed by"),
      f.tag()->item("----:com.apple.iTunes:REMIXEDBY").toStringList());
    CPPUNIT_ASSERT_EQUAL(StringList("Remixed by"), tags["REMIXEDBY"]);

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

  void testPropertiesAllSupported()
  {
    PropertyMap tags;
    tags["ALBUM"] = StringList("Album");
    tags["ALBUMARTIST"] = StringList("Album Artist");
    tags["ALBUMARTISTSORT"] = StringList("Album Artist Sort");
    tags["ALBUMSORT"] = StringList("Album Sort");
    tags["ARTIST"] = StringList("Artist");
    tags["ARTISTS"] = StringList("Artists");
    tags["ARTISTSORT"] = StringList("Artist Sort");
    tags["ASIN"] = StringList("ASIN");
    tags["BARCODE"] = StringList("Barcode");
    tags["BPM"] = StringList("123");
    tags["CATALOGNUMBER"] = StringList("Catalog Number");
    tags["COMMENT"] = StringList("Comment");
    tags["COMPILATION"] = StringList("1");
    tags["COMPOSER"] = StringList("Composer");
    tags["COMPOSERSORT"] = StringList("Composer Sort");
    tags["CONDUCTOR"] = StringList("Conductor");
    tags["COPYRIGHT"] = StringList("2021 Copyright");
    tags["DATE"] = StringList("2021-01-03 12:29:23");
    tags["DISCNUMBER"] = StringList("3/5");
    tags["DISCSUBTITLE"] = StringList("Disc Subtitle");
    tags["DJMIXER"] = StringList("DJ Mixer");
    tags["ENCODEDBY"] = StringList("Encoded by");
    tags["ENCODING"] = StringList("Encoding");
    tags["ENGINEER"] = StringList("Engineer");
    tags["GAPLESSPLAYBACK"] = StringList("1");
    tags["GENRE"] = StringList("Genre");
    tags["GROUPING"] = StringList("Grouping");
    tags["ISRC"] = StringList("UKAAA0500001");
    tags["LABEL"] = StringList("Label");
    tags["LANGUAGE"] = StringList("eng");
    tags["LICENSE"] = StringList("License");
    tags["LYRICIST"] = StringList("Lyricist");
    tags["LYRICS"] = StringList("Lyrics");
    tags["MEDIA"] = StringList("Media");
    tags["MIXER"] = StringList("Mixer");
    tags["MOOD"] = StringList("Mood");
    tags["MOVEMENTCOUNT"] = StringList("3");
    tags["MOVEMENTNAME"] = StringList("Movement Name");
    tags["MOVEMENTNUMBER"] = StringList("2");
    tags["MUSICBRAINZ_ALBUMARTISTID"] = StringList("MusicBrainz_AlbumartistID");
    tags["MUSICBRAINZ_ALBUMID"] = StringList("MusicBrainz_AlbumID");
    tags["MUSICBRAINZ_ARTISTID"] = StringList("MusicBrainz_ArtistID");
    tags["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
    tags["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
    tags["MUSICBRAINZ_TRACKID"] = StringList("MusicBrainz_TrackID");
    tags["MUSICBRAINZ_WORKID"] = StringList("MusicBrainz_WorkID");
    tags["ORIGINALDATE"] = StringList("2021-01-03 13:52:19");
    tags["OWNER"] = StringList("Owner");
    tags["PODCAST"] = StringList("1");
    tags["PODCASTCATEGORY"] = StringList("Podcast Category");
    tags["PODCASTDESC"] = StringList("Podcast Description");
    tags["PODCASTID"] = StringList("Podcast ID");
    tags["PODCASTURL"] = StringList("Podcast URL");
    tags["PRODUCER"] = StringList("Producer");
    tags["RELEASECOUNTRY"] = StringList("Release Country");
    tags["RELEASESTATUS"] = StringList("Release Status");
    tags["RELEASETYPE"] = StringList("Release Type");
    tags["REMIXER"] = StringList("Remixer");
    tags["SCRIPT"] = StringList("Script");
    tags["SHOWSORT"] = StringList("Show Sort");
    tags["SHOWWORKMOVEMENT"] = StringList("1");
    tags["SUBTITLE"] = StringList("Subtitle");
    tags["TITLE"] = StringList("Title");
    tags["TITLESORT"] = StringList("Title Sort");
    tags["TRACKNUMBER"] = StringList("2/4");
    tags["TVEPISODE"] = StringList("3");
    tags["TVEPISODEID"] = StringList("TV Episode ID");
    tags["TVNETWORK"] = StringList("TV Network");
    tags["TVSEASON"] = StringList("2");
    tags["TVSHOW"] = StringList("TV Show");
    tags["WORK"] = StringList("Work");

    ScopedFileCopy copy("no-tags", ".m4a");
    {
      MP4::File f(copy.fileName().c_str());
      PropertyMap properties = f.properties();
      CPPUNIT_ASSERT(properties.isEmpty());
      f.setProperties(tags);
      f.save();
    }
    {
      const MP4::File f(copy.fileName().c_str());
      PropertyMap properties = f.properties();
      if (tags != properties) {
        CPPUNIT_ASSERT_EQUAL(tags.toString(), properties.toString());
      }
      CPPUNIT_ASSERT(tags == properties);
    }
  }

  void testPropertiesMovement()
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));

    PropertyMap tags = f.properties();

    tags["WORK"] = StringList("Foo");
    tags["MOVEMENTNAME"] = StringList("Bar");
    tags["MOVEMENTNUMBER"] = StringList("2");
    tags["MOVEMENTCOUNT"] = StringList("3");
    tags["SHOWWORKMOVEMENT"] = StringList("1");
    f.setProperties(tags);

    tags = f.properties();

    CPPUNIT_ASSERT(f.tag()->contains("\251wrk"));
    CPPUNIT_ASSERT_EQUAL(StringList("Foo"), f.tag()->item("\251wrk").toStringList());
    CPPUNIT_ASSERT_EQUAL(StringList("Foo"), tags["WORK"]);

    CPPUNIT_ASSERT(f.tag()->contains("\251mvn"));
    CPPUNIT_ASSERT_EQUAL(StringList("Bar"), f.tag()->item("\251mvn").toStringList());
    CPPUNIT_ASSERT_EQUAL(StringList("Bar"), tags["MOVEMENTNAME"]);

    CPPUNIT_ASSERT(f.tag()->contains("\251mvi"));
    CPPUNIT_ASSERT_EQUAL(2, f.tag()->item("\251mvi").toInt());
    CPPUNIT_ASSERT_EQUAL(StringList("2"), tags["MOVEMENTNUMBER"]);

    CPPUNIT_ASSERT(f.tag()->contains("\251mvc"));
    CPPUNIT_ASSERT_EQUAL(3, f.tag()->item("\251mvc").toInt());
    CPPUNIT_ASSERT_EQUAL(StringList("3"), tags["MOVEMENTCOUNT"]);

    CPPUNIT_ASSERT(f.tag()->contains("shwm"));
    CPPUNIT_ASSERT_EQUAL(true, f.tag()->item("shwm").toBool());
    CPPUNIT_ASSERT_EQUAL(StringList("1"), tags["SHOWWORKMOVEMENT"]);

    tags["SHOWWORKMOVEMENT"] = StringList("0");
    f.setProperties(tags);

    tags = f.properties();

    CPPUNIT_ASSERT(f.tag()->contains("shwm"));
    CPPUNIT_ASSERT_EQUAL(false, f.tag()->item("shwm").toBool());
    CPPUNIT_ASSERT_EQUAL(StringList("0"), tags["SHOWWORKMOVEMENT"]);

    tags["WORK"] = StringList();
    tags["MOVEMENTNAME"] = StringList();
    tags["MOVEMENTNUMBER"] = StringList();
    tags["MOVEMENTCOUNT"] = StringList();
    tags["SHOWWORKMOVEMENT"] = StringList();
    f.setProperties(tags);
  }

  void testFuzzedFile()
  {
    MP4::File f(TEST_FILE_PATH_C("infloop.m4a"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testRepeatedSave()
  {
    ScopedFileCopy copy("no-tags", ".m4a");

    MP4::File f(copy.fileName().c_str());
    f.tag()->setTitle("0123456789");
    f.save();
    f.save();
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(2862), f.find("0123456789"));
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(-1), f.find("0123456789", 2863));
  }

  void testWithZeroLengthAtom()
  {
    MP4::File f(TEST_FILE_PATH_C("zero-length-mdat.m4a"));
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT_EQUAL(1115, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(22050, f.audioProperties()->sampleRate());
  }

  void testEmptyValuesRemoveItems()
  {
    const MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    MP4::Tag *tag = f.tag();
    const String testTitle("Title");
    const String testArtist("Artist");
    const String testAlbum("Album");
    const String testComment("Comment");
    const String testGenre("Genre");
    const String nullString;
    constexpr unsigned int testYear = 2020;
    constexpr unsigned int testTrack = 1;
    constexpr unsigned int zeroUInt = 0;

    tag->setTitle(testTitle);
    CPPUNIT_ASSERT_EQUAL(testTitle, tag->title());
    CPPUNIT_ASSERT(tag->contains("\251nam"));
    tag->setArtist(testArtist);
    CPPUNIT_ASSERT_EQUAL(testArtist, tag->artist());
    CPPUNIT_ASSERT(tag->contains("\251ART"));
    tag->setAlbum(testAlbum);
    CPPUNIT_ASSERT_EQUAL(testAlbum, tag->album());
    CPPUNIT_ASSERT(tag->contains("\251alb"));
    tag->setComment(testComment);
    CPPUNIT_ASSERT_EQUAL(testComment, tag->comment());
    CPPUNIT_ASSERT(tag->contains("\251cmt"));
    tag->setGenre(testGenre);
    CPPUNIT_ASSERT_EQUAL(testGenre, tag->genre());
    CPPUNIT_ASSERT(tag->contains("\251gen"));
    tag->setYear(testYear);
    CPPUNIT_ASSERT_EQUAL(testYear, tag->year());
    CPPUNIT_ASSERT(tag->contains("\251day"));
    tag->setTrack(testTrack);
    CPPUNIT_ASSERT_EQUAL(testTrack, tag->track());
    CPPUNIT_ASSERT(tag->contains("trkn"));

    tag->setTitle(nullString);
    CPPUNIT_ASSERT_EQUAL(nullString, tag->title());
    CPPUNIT_ASSERT(!tag->contains("\251nam"));
    tag->setArtist(nullString);
    CPPUNIT_ASSERT_EQUAL(nullString, tag->artist());
    CPPUNIT_ASSERT(!tag->contains("\251ART"));
    tag->setAlbum(nullString);
    CPPUNIT_ASSERT_EQUAL(nullString, tag->album());
    CPPUNIT_ASSERT(!tag->contains("\251alb"));
    tag->setComment(nullString);
    CPPUNIT_ASSERT_EQUAL(nullString, tag->comment());
    CPPUNIT_ASSERT(!tag->contains("\251cmt"));
    tag->setGenre(nullString);
    CPPUNIT_ASSERT_EQUAL(nullString, tag->genre());
    CPPUNIT_ASSERT(!tag->contains("\251gen"));
    tag->setYear(zeroUInt);
    CPPUNIT_ASSERT_EQUAL(zeroUInt, tag->year());
    CPPUNIT_ASSERT(!tag->contains("\251day"));
    tag->setTrack(zeroUInt);
    CPPUNIT_ASSERT_EQUAL(zeroUInt, tag->track());
    CPPUNIT_ASSERT(!tag->contains("trkn"));
  }

  void testRemoveMetadata()
  {
    ScopedFileCopy copy("no-tags", ".m4a");

    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasMP4Tag());
      MP4::Tag *tag = f.tag();
      CPPUNIT_ASSERT(tag->isEmpty());
      tag->setTitle("TITLE");
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasMP4Tag());
      CPPUNIT_ASSERT(!f.tag()->isEmpty());
      f.strip();
    }
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasMP4Tag());
      CPPUNIT_ASSERT(f.tag()->isEmpty());
      CPPUNIT_ASSERT(fileEqual(
        copy.fileName(),
        testFilePath("no-tags.m4a")));
    }
  }

  void testNonFullMetaAtom()
  {
    {
      MP4::File f(TEST_FILE_PATH_C("non-full-meta.m4a"));
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasMP4Tag());

      CPPUNIT_ASSERT(f.tag()->contains("covr"));
      MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), l.size());
      CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[0].format());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(79), l[0].data().size());
      CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, l[1].format());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(287), l[1].data().size());

      PropertyMap properties = f.properties();
      CPPUNIT_ASSERT_EQUAL(StringList("Test Artist!!!!"), properties["ARTIST"]);
      CPPUNIT_ASSERT_EQUAL(StringList("FAAC 1.24"), properties["ENCODING"]);
    }
  }

  void testItemFactory()
  {
    ScopedFileCopy copy("no-tags", ".m4a");
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasMP4Tag());
      MP4::Tag *tag = f.tag();
      tag->setItem("tsti", MP4::Item(123));
      tag->setItem("tstt", MP4::Item(StringList("Test text")));
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasMP4Tag());
      MP4::Tag *tag = f.tag();
      // Without a custom item factory, only custom text atoms with four
      // letter names are possible.
      MP4::Item item = tag->item("tsti");
      CPPUNIT_ASSERT(!item.isValid());
      CPPUNIT_ASSERT(item.toInt() != 123);
      item = tag->item("tstt");
      CPPUNIT_ASSERT(item.isValid());
      CPPUNIT_ASSERT_EQUAL(StringList("Test text"), item.toStringList());
      f.strip();
    }
    {
      MP4::File f(copy.fileName().c_str(),
                  true, MP4::Properties::Average, CustomItemFactory::instance());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasMP4Tag());
      MP4::Tag *tag = f.tag();
      tag->setItem("tsti", MP4::Item(123));
      tag->setItem("tstt", MP4::Item(StringList("Test text")));
      tag->setItem("trkn", MP4::Item(2, 10));
      tag->setItem("rate", MP4::Item(80));
      tag->setItem("plID", MP4::Item(1540934238LL));
      tag->setItem("rtng", MP4::Item(static_cast<unsigned char>(2)));
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str(),
                  true, MP4::Properties::Average, CustomItemFactory::instance());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasMP4Tag());
      MP4::Tag *tag = f.tag();
      MP4::Item item = tag->item("tsti");
      CPPUNIT_ASSERT(item.isValid());
      CPPUNIT_ASSERT_EQUAL(123, item.toInt());
      item = tag->item("tstt");
      CPPUNIT_ASSERT(item.isValid());
      CPPUNIT_ASSERT_EQUAL(StringList("Test text"), item.toStringList());
      item = tag->item("trkn");
      CPPUNIT_ASSERT(item.isValid());
      CPPUNIT_ASSERT_EQUAL(2, item.toIntPair().first);
      CPPUNIT_ASSERT_EQUAL(10, item.toIntPair().second);
      CPPUNIT_ASSERT_EQUAL(80, tag->item("rate").toInt());
      CPPUNIT_ASSERT_EQUAL(1540934238LL, tag->item("plID").toLongLong());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(2), tag->item("rtng").toByte());
      PropertyMap properties = tag->properties();
      CPPUNIT_ASSERT_EQUAL(StringList("123"), properties.value("TESTINTEGER"));
      CPPUNIT_ASSERT_EQUAL(StringList("2/10"), properties.value("TRACKNUMBER"));
      properties["TESTINTEGER"] = StringList("456");
      tag->setProperties(properties);
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str(),
                  true, MP4::Properties::Average, CustomItemFactory::instance());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasMP4Tag());
      MP4::Tag *tag = f.tag();
      MP4::Item item = tag->item("tsti");
      CPPUNIT_ASSERT(item.isValid());
      CPPUNIT_ASSERT_EQUAL(456, item.toInt());
      PropertyMap properties = tag->properties();
      CPPUNIT_ASSERT_EQUAL(StringList("456"), properties.value("TESTINTEGER"));
    }
  }

  void testNonPrintableAtom()
  {
    ScopedFileCopy copy("nonprintable-atom-type", ".m4a");
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
      CPPUNIT_ASSERT_EQUAL(32000, f.audioProperties()->sampleRate());
      f.tag()->setTitle("TITLE");
      f.save();
    }
    {
        MP4::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.isValid());
        CPPUNIT_ASSERT(f.hasMP4Tag());
        CPPUNIT_ASSERT_EQUAL(String("TITLE"), f.tag()->title());
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMP4);
