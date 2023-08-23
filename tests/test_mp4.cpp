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

#include "mp4atom.h"
#include "mp4file.h"
#include "mp4tag.h"
#include "plainfile.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "tbytevectorstream.h"
#include "tpropertymap.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(MP4, testPropertiesAAC)
{
  MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3708, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(3, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_FALSE(f.audioProperties()->isEncrypted());
  ASSERT_EQ(MP4::Properties::AAC, f.audioProperties()->codec());
}

TEST(MP4, testPropertiesAACWithoutBitrate)
{
  ByteVector aacData = PlainFile(TEST_FILE_PATH_C("has-tags.m4a")).readAll();
  ASSERT_LT(1960U, aacData.size());
  ASSERT_EQ(ByteVector("mp4a"), aacData.mid(1890, 4));
  // Set the bitrate to zero
  for(int offset = 1956; offset < 1960; ++offset) {
    aacData[offset] = 0;
  }
  ByteVectorStream aacStream(aacData);
  MP4::File f(&aacStream);
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3708, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(3, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_FALSE(f.audioProperties()->isEncrypted());
  ASSERT_EQ(MP4::Properties::AAC, f.audioProperties()->codec());
}

TEST(MP4, testPropertiesALAC)
{
  MP4::File f(TEST_FILE_PATH_C("empty_alac.m4a"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3705, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(3, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_FALSE(f.audioProperties()->isEncrypted());
  ASSERT_EQ(MP4::Properties::ALAC, f.audioProperties()->codec());
}

TEST(MP4, testPropertiesALACWithoutBitrate)
{
  ByteVector alacData = PlainFile(TEST_FILE_PATH_C("empty_alac.m4a")).readAll();
  ASSERT_LT(474U, alacData.size());
  ASSERT_EQ(ByteVector("alac"), alacData.mid(446, 4));
  // Set the bitrate to zero
  for(int offset = 470; offset < 474; ++offset) {
    alacData[offset] = 0;
  }
  ByteVectorStream alacStream(alacData);
  MP4::File f(&alacStream);
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3705, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(2, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_FALSE(f.audioProperties()->isEncrypted());
  ASSERT_EQ(MP4::Properties::ALAC, f.audioProperties()->codec());
}

TEST(MP4, testPropertiesM4V)
{
  MP4::File f(TEST_FILE_PATH_C("blank_video.m4v"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(975, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(96, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_FALSE(f.audioProperties()->isEncrypted());
  ASSERT_EQ(MP4::Properties::AAC, f.audioProperties()->codec());
}

TEST(MP4, testCheckValid)
{
  MP4::File f(TEST_FILE_PATH_C("empty.aiff"));
  ASSERT_FALSE(f.isValid());
}

TEST(MP4, testHasTag)
{
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasMP4Tag());
  }

  ScopedFileCopy copy("no-tags", ".m4a");

  {
    MP4::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_FALSE(f.hasMP4Tag());
    f.tag()->setTitle("TITLE");
    f.save();
  }
  {
    MP4::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasMP4Tag());
  }
}

TEST(MP4, testIsEmpty)
{
  MP4::Tag t1;
  ASSERT_TRUE(t1.isEmpty());
  t1.setArtist("Foo");
  ASSERT_FALSE(t1.isEmpty());

  MP4::Tag t2;
  t2.setItem("foo", "bar");
  ASSERT_FALSE(t2.isEmpty());
}

TEST(MP4, testUpdateStco)
{
  ScopedFileCopy copy("no-tags", ".3g2");
  string filename = copy.fileName();

  ByteVectorList data1;

  {
    MP4::File f(filename.c_str());
    f.tag()->setArtist(ByteVector(3000, 'x'));

    MP4::Atoms a(&f);
    MP4::Atom *stco = a.find("moov")->findall("stco", true)[0];
    f.seek(stco->offset + 12);
    ByteVector data    = f.readBlock(stco->length - 12);
    unsigned int count = data.mid(0, 4).toUInt();
    int pos            = 4;
    while(count--) {
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
    f.seek(stco->offset + 12);
    ByteVector data    = f.readBlock(stco->length - 12);
    unsigned int count = data.mid(0, 4).toUInt();
    int pos = 4, i = 0;
    while(count--) {
      unsigned int offset = data.mid(pos, 4).toUInt();
      f.seek(offset);
      ASSERT_EQ(data1[i], f.readBlock(20));
      pos += 4;
      i++;
    }
  }
}

TEST(MP4, testFreeForm)
{
  ScopedFileCopy copy("has-tags", ".m4a");
  string filename = copy.fileName();

  {
    MP4::File f(filename.c_str());
    ASSERT_TRUE(f.tag()->contains("----:com.apple.iTunes:iTunNORM"));
    f.tag()->setItem("----:org.kde.TagLib:Foo", StringList("Bar"));
    f.save();
  }
  {
    MP4::File f(filename.c_str());
    ASSERT_TRUE(f.tag()->contains("----:org.kde.TagLib:Foo"));
    ASSERT_EQ(String("Bar"),
              f.tag()->item("----:org.kde.TagLib:Foo").toStringList().front());
    f.save();
  }
}

TEST(MP4, testSaveExisingWhenIlstIsLast)
{
  ScopedFileCopy copy("ilst-is-last", ".m4a");
  string filename = copy.fileName();

  {
    MP4::File f(filename.c_str());
    ASSERT_EQ(String("82,164"),
              f.tag()->item("----:com.apple.iTunes:replaygain_track_minmax").toStringList().front());
    ASSERT_EQ(String("Pearl Jam"), f.tag()->artist());
    f.tag()->setComment("foo");
    f.save();
  }
  {
    MP4::File f(filename.c_str());
    ASSERT_EQ(String("82,164"),
              f.tag()->item("----:com.apple.iTunes:replaygain_track_minmax").toStringList().front());
    ASSERT_EQ(String("Pearl Jam"), f.tag()->artist());
    ASSERT_EQ(String("foo"), f.tag()->comment());
  }
}

TEST(MP4, test64BitAtom)
{
  ScopedFileCopy copy("64bit", ".mp4");
  string filename = copy.fileName();

  {
    MP4::File f(filename.c_str());
    ASSERT_TRUE(f.tag()->itemMap()["cpil"].toBool());

    MP4::Atoms atoms(&f);
    MP4::Atom *moov = atoms.atoms[0];
    ASSERT_EQ(static_cast<offset_t>(77), moov->length);

    f.tag()->setItem("pgap", true);
    f.save();
  }
  {
    MP4::File f(filename.c_str());
    ASSERT_TRUE(f.tag()->item("cpil").toBool());
    ASSERT_TRUE(f.tag()->item("pgap").toBool());

    MP4::Atoms atoms(&f);
    MP4::Atom *moov = atoms.atoms[0];
    // original size + 'pgap' size + padding
    ASSERT_EQ(static_cast<offset_t>(77 + 25 + 974), moov->length);
  }
}

TEST(MP4, testGnre)
{
  MP4::File f(TEST_FILE_PATH_C("gnre.m4a"));
  ASSERT_EQ(TagLib::String("Ska"), f.tag()->genre());
}

TEST(MP4, testCovrRead)
{
  MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
  ASSERT_TRUE(f.tag()->contains("covr"));
  MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
  ASSERT_EQ(static_cast<unsigned int>(2), l.size());
  ASSERT_EQ(MP4::CoverArt::PNG, l[0].format());
  ASSERT_EQ(static_cast<unsigned int>(79), l[0].data().size());
  ASSERT_EQ(MP4::CoverArt::JPEG, l[1].format());
  ASSERT_EQ(static_cast<unsigned int>(287), l[1].data().size());
}

TEST(MP4, testCovrWrite)
{
  ScopedFileCopy copy("has-tags", ".m4a");
  string filename = copy.fileName();

  {
    MP4::File f(filename.c_str());
    ASSERT_TRUE(f.tag()->contains("covr"));
    MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
    l.append(MP4::CoverArt(MP4::CoverArt::PNG, "foo"));
    f.tag()->setItem("covr", l);
    f.save();
  }
  {
    MP4::File f(filename.c_str());
    ASSERT_TRUE(f.tag()->contains("covr"));
    MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
    ASSERT_EQ(static_cast<unsigned int>(3), l.size());
    ASSERT_EQ(MP4::CoverArt::PNG, l[0].format());
    ASSERT_EQ(static_cast<unsigned int>(79), l[0].data().size());
    ASSERT_EQ(MP4::CoverArt::JPEG, l[1].format());
    ASSERT_EQ(static_cast<unsigned int>(287), l[1].data().size());
    ASSERT_EQ(MP4::CoverArt::PNG, l[2].format());
    ASSERT_EQ(static_cast<unsigned int>(3), l[2].data().size());
  }
}

TEST(MP4, testCovrRead2)
{
  MP4::File f(TEST_FILE_PATH_C("covr-junk.m4a"));
  ASSERT_TRUE(f.tag()->contains("covr"));
  MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
  ASSERT_EQ(static_cast<unsigned int>(2), l.size());
  ASSERT_EQ(MP4::CoverArt::PNG, l[0].format());
  ASSERT_EQ(static_cast<unsigned int>(79), l[0].data().size());
  ASSERT_EQ(MP4::CoverArt::JPEG, l[1].format());
  ASSERT_EQ(static_cast<unsigned int>(287), l[1].data().size());
}

TEST(MP4, testProperties)
{
  MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));

  PropertyMap tags = f.properties();

  ASSERT_EQ(StringList("Test Artist"), tags["ARTIST"]);

  tags["TRACKNUMBER"] = StringList("2/4");
  tags["DISCNUMBER"]  = StringList("3/5");
  tags["BPM"]         = StringList("123");
  tags["ARTIST"]      = StringList("Foo Bar");
  tags["COMPILATION"] = StringList("1");
  f.setProperties(tags);

  tags = f.properties();

  ASSERT_TRUE(f.tag()->contains("trkn"));
  ASSERT_EQ(2, f.tag()->item("trkn").toIntPair().first);
  ASSERT_EQ(4, f.tag()->item("trkn").toIntPair().second);
  ASSERT_EQ(StringList("2/4"), tags["TRACKNUMBER"]);

  ASSERT_TRUE(f.tag()->contains("disk"));
  ASSERT_EQ(3, f.tag()->item("disk").toIntPair().first);
  ASSERT_EQ(5, f.tag()->item("disk").toIntPair().second);
  ASSERT_EQ(StringList("3/5"), tags["DISCNUMBER"]);

  ASSERT_TRUE(f.tag()->contains("tmpo"));
  ASSERT_EQ(123, f.tag()->item("tmpo").toInt());
  ASSERT_EQ(StringList("123"), tags["BPM"]);

  ASSERT_TRUE(f.tag()->contains("\251ART"));
  ASSERT_EQ(StringList("Foo Bar"), f.tag()->item("\251ART").toStringList());
  ASSERT_EQ(StringList("Foo Bar"), tags["ARTIST"]);

  ASSERT_TRUE(f.tag()->contains("cpil"));
  ASSERT_TRUE(f.tag()->item("cpil").toBool());
  ASSERT_EQ(StringList("1"), tags["COMPILATION"]);

  tags["COMPILATION"] = StringList("0");
  f.setProperties(tags);

  tags = f.properties();

  ASSERT_TRUE(f.tag()->contains("cpil"));
  ASSERT_FALSE(f.tag()->item("cpil").toBool());
  ASSERT_EQ(StringList("0"), tags["COMPILATION"]);

  // Empty properties do not result in access violations
  // when converting integers
  tags["TRACKNUMBER"] = StringList();
  tags["DISCNUMBER"]  = StringList();
  tags["BPM"]         = StringList();
  tags["COMPILATION"] = StringList();
  f.setProperties(tags);
}

TEST(MP4, testPropertiesAllSupported)
{
  PropertyMap tags;
  tags["ALBUM"]                      = StringList("Album");
  tags["ALBUMARTIST"]                = StringList("Album Artist");
  tags["ALBUMARTISTSORT"]            = StringList("Album Artist Sort");
  tags["ALBUMSORT"]                  = StringList("Album Sort");
  tags["ARTIST"]                     = StringList("Artist");
  tags["ARTISTS"]                    = StringList("Artists");
  tags["ARTISTSORT"]                 = StringList("Artist Sort");
  tags["ASIN"]                       = StringList("ASIN");
  tags["BARCODE"]                    = StringList("Barcode");
  tags["BPM"]                        = StringList("123");
  tags["CATALOGNUMBER"]              = StringList("Catalog Number");
  tags["COMMENT"]                    = StringList("Comment");
  tags["COMPILATION"]                = StringList("1");
  tags["COMPOSER"]                   = StringList("Composer");
  tags["COMPOSERSORT"]               = StringList("Composer Sort");
  tags["CONDUCTOR"]                  = StringList("Conductor");
  tags["COPYRIGHT"]                  = StringList("2021 Copyright");
  tags["DATE"]                       = StringList("2021-01-03 12:29:23");
  tags["DISCNUMBER"]                 = StringList("3/5");
  tags["DISCSUBTITLE"]               = StringList("Disc Subtitle");
  tags["DJMIXER"]                    = StringList("DJ Mixer");
  tags["ENCODEDBY"]                  = StringList("Encoded by");
  tags["ENGINEER"]                   = StringList("Engineer");
  tags["GAPLESSPLAYBACK"]            = StringList("1");
  tags["GENRE"]                      = StringList("Genre");
  tags["GROUPING"]                   = StringList("Grouping");
  tags["ISRC"]                       = StringList("UKAAA0500001");
  tags["LABEL"]                      = StringList("Label");
  tags["LANGUAGE"]                   = StringList("eng");
  tags["LICENSE"]                    = StringList("License");
  tags["LYRICIST"]                   = StringList("Lyricist");
  tags["LYRICS"]                     = StringList("Lyrics");
  tags["MEDIA"]                      = StringList("Media");
  tags["MIXER"]                      = StringList("Mixer");
  tags["MOOD"]                       = StringList("Mood");
  tags["MOVEMENTCOUNT"]              = StringList("3");
  tags["MOVEMENTNAME"]               = StringList("Movement Name");
  tags["MOVEMENTNUMBER"]             = StringList("2");
  tags["MUSICBRAINZ_ALBUMARTISTID"]  = StringList("MusicBrainz_AlbumartistID");
  tags["MUSICBRAINZ_ALBUMID"]        = StringList("MusicBrainz_AlbumID");
  tags["MUSICBRAINZ_ARTISTID"]       = StringList("MusicBrainz_ArtistID");
  tags["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
  tags["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
  tags["MUSICBRAINZ_TRACKID"]        = StringList("MusicBrainz_TrackID");
  tags["MUSICBRAINZ_WORKID"]         = StringList("MusicBrainz_WorkID");
  tags["ORIGINALDATE"]               = StringList("2021-01-03 13:52:19");
  tags["PODCAST"]                    = StringList("1");
  tags["PODCASTCATEGORY"]            = StringList("Podcast Category");
  tags["PODCASTDESC"]                = StringList("Podcast Description");
  tags["PODCASTID"]                  = StringList("Podcast ID");
  tags["PODCASTURL"]                 = StringList("Podcast URL");
  tags["PRODUCER"]                   = StringList("Producer");
  tags["RELEASECOUNTRY"]             = StringList("Release Country");
  tags["RELEASESTATUS"]              = StringList("Release Status");
  tags["RELEASETYPE"]                = StringList("Release Type");
  tags["REMIXER"]                    = StringList("Remixer");
  tags["SCRIPT"]                     = StringList("Script");
  tags["SHOWSORT"]                   = StringList("Show Sort");
  tags["SHOWWORKMOVEMENT"]           = StringList("1");
  tags["SUBTITLE"]                   = StringList("Subtitle");
  tags["TITLE"]                      = StringList("Title");
  tags["TITLESORT"]                  = StringList("Title Sort");
  tags["TRACKNUMBER"]                = StringList("2/4");
  tags["TVEPISODE"]                  = StringList("3");
  tags["TVEPISODEID"]                = StringList("TV Episode ID");
  tags["TVNETWORK"]                  = StringList("TV Network");
  tags["TVSEASON"]                   = StringList("2");
  tags["TVSHOW"]                     = StringList("TV Show");
  tags["WORK"]                       = StringList("Work");

  ScopedFileCopy copy("no-tags", ".m4a");
  {
    MP4::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    ASSERT_TRUE(properties.isEmpty());
    f.setProperties(tags);
    f.save();
  }
  {
    const MP4::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    if(tags != properties) {
      ASSERT_EQ(tags.toString(), properties.toString());
    }
    ASSERT_EQ(tags, properties);
  }
}

TEST(MP4, testPropertiesMovement)
{
  MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));

  PropertyMap tags         = f.properties();

  tags["WORK"]             = StringList("Foo");
  tags["MOVEMENTNAME"]     = StringList("Bar");
  tags["MOVEMENTNUMBER"]   = StringList("2");
  tags["MOVEMENTCOUNT"]    = StringList("3");
  tags["SHOWWORKMOVEMENT"] = StringList("1");
  f.setProperties(tags);

  tags = f.properties();

  ASSERT_TRUE(f.tag()->contains("\251wrk"));
  ASSERT_EQ(StringList("Foo"), f.tag()->item("\251wrk").toStringList());
  ASSERT_EQ(StringList("Foo"), tags["WORK"]);

  ASSERT_TRUE(f.tag()->contains("\251mvn"));
  ASSERT_EQ(StringList("Bar"), f.tag()->item("\251mvn").toStringList());
  ASSERT_EQ(StringList("Bar"), tags["MOVEMENTNAME"]);

  ASSERT_TRUE(f.tag()->contains("\251mvi"));
  ASSERT_EQ(2, f.tag()->item("\251mvi").toInt());
  ASSERT_EQ(StringList("2"), tags["MOVEMENTNUMBER"]);

  ASSERT_TRUE(f.tag()->contains("\251mvc"));
  ASSERT_EQ(3, f.tag()->item("\251mvc").toInt());
  ASSERT_EQ(StringList("3"), tags["MOVEMENTCOUNT"]);

  ASSERT_TRUE(f.tag()->contains("shwm"));
  ASSERT_TRUE(f.tag()->item("shwm").toBool());
  ASSERT_EQ(StringList("1"), tags["SHOWWORKMOVEMENT"]);

  tags["SHOWWORKMOVEMENT"] = StringList("0");
  f.setProperties(tags);

  tags = f.properties();

  ASSERT_TRUE(f.tag()->contains("shwm"));
  ASSERT_FALSE(f.tag()->item("shwm").toBool());
  ASSERT_EQ(StringList("0"), tags["SHOWWORKMOVEMENT"]);

  tags["WORK"]             = StringList();
  tags["MOVEMENTNAME"]     = StringList();
  tags["MOVEMENTNUMBER"]   = StringList();
  tags["MOVEMENTCOUNT"]    = StringList();
  tags["SHOWWORKMOVEMENT"] = StringList();
  f.setProperties(tags);
}

TEST(MP4, testFuzzedFile)
{
  MP4::File f(TEST_FILE_PATH_C("infloop.m4a"));
  // The file has an invalid atom length of 2775 in the last atom
  // ("free", offset 0xc521, 00000ad7 66726565), whereas the remaining file
  // length is 2727 bytes, therefore the file is now considered invalid.
  ASSERT_FALSE(f.isValid());
}

TEST(MP4, testRepeatedSave)
{
  ScopedFileCopy copy("no-tags", ".m4a");

  MP4::File f(copy.fileName().c_str());
  f.tag()->setTitle("0123456789");
  f.save();
  f.save();
  ASSERT_EQ(static_cast<offset_t>(2862), f.find("0123456789"));
  ASSERT_EQ(static_cast<offset_t>(-1), f.find("0123456789", 2863));
}

TEST(MP4, testWithZeroLengthAtom)
{
  MP4::File f(TEST_FILE_PATH_C("zero-length-mdat.m4a"));
  ASSERT_TRUE(f.isValid());
  ASSERT_EQ(1115, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(22050, f.audioProperties()->sampleRate());
}

TEST(MP4, testEmptyValuesRemoveItems)
{
  const MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
  MP4::Tag *tag = f.tag();
  const String testTitle("Title");
  const String testArtist("Artist");
  const String testAlbum("Album");
  const String testComment("Comment");
  const String testGenre("Genre");
  const String nullString;
  const unsigned int testYear  = 2020;
  const unsigned int testTrack = 1;
  const unsigned int zeroUInt  = 0;

  tag->setTitle(testTitle);
  ASSERT_EQ(testTitle, tag->title());
  ASSERT_TRUE(tag->contains("\251nam"));
  tag->setArtist(testArtist);
  ASSERT_EQ(testArtist, tag->artist());
  ASSERT_TRUE(tag->contains("\251ART"));
  tag->setAlbum(testAlbum);
  ASSERT_EQ(testAlbum, tag->album());
  ASSERT_TRUE(tag->contains("\251alb"));
  tag->setComment(testComment);
  ASSERT_EQ(testComment, tag->comment());
  ASSERT_TRUE(tag->contains("\251cmt"));
  tag->setGenre(testGenre);
  ASSERT_EQ(testGenre, tag->genre());
  ASSERT_TRUE(tag->contains("\251gen"));
  tag->setYear(testYear);
  ASSERT_EQ(testYear, tag->year());
  ASSERT_TRUE(tag->contains("\251day"));
  tag->setTrack(testTrack);
  ASSERT_EQ(testTrack, tag->track());
  ASSERT_TRUE(tag->contains("trkn"));

  tag->setTitle(nullString);
  ASSERT_EQ(nullString, tag->title());
  ASSERT_FALSE(tag->contains("\251nam"));
  tag->setArtist(nullString);
  ASSERT_EQ(nullString, tag->artist());
  ASSERT_FALSE(tag->contains("\251ART"));
  tag->setAlbum(nullString);
  ASSERT_EQ(nullString, tag->album());
  ASSERT_FALSE(tag->contains("\251alb"));
  tag->setComment(nullString);
  ASSERT_EQ(nullString, tag->comment());
  ASSERT_FALSE(tag->contains("\251cmt"));
  tag->setGenre(nullString);
  ASSERT_EQ(nullString, tag->genre());
  ASSERT_FALSE(tag->contains("\251gen"));
  tag->setYear(zeroUInt);
  ASSERT_EQ(zeroUInt, tag->year());
  ASSERT_FALSE(tag->contains("\251day"));
  tag->setTrack(zeroUInt);
  ASSERT_EQ(zeroUInt, tag->track());
  ASSERT_FALSE(tag->contains("trkn"));
}

TEST(MP4, testRemoveMetadata)
{
  ScopedFileCopy copy("no-tags", ".m4a");

  {
    MP4::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_FALSE(f.hasMP4Tag());
    MP4::Tag *tag = f.tag();
    ASSERT_TRUE(tag->isEmpty());
    tag->setTitle("TITLE");
    f.save();
  }
  {
    MP4::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasMP4Tag());
    ASSERT_FALSE(f.tag()->isEmpty());
    f.strip();
  }
  {
    MP4::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_FALSE(f.hasMP4Tag());
    ASSERT_TRUE(f.tag()->isEmpty());
    ASSERT_TRUE(fileEqual(
      copy.fileName(),
      TEST_FILE_PATH_C("no-tags.m4a")));
  }
}

TEST(MP4, testNonFullMetaAtom)
{
  {
    MP4::File f(TEST_FILE_PATH_C("non-full-meta.m4a"));
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasMP4Tag());

    ASSERT_TRUE(f.tag()->contains("covr"));
    MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
    ASSERT_EQ(static_cast<unsigned int>(2), l.size());
    ASSERT_EQ(MP4::CoverArt::PNG, l[0].format());
    ASSERT_EQ(static_cast<unsigned int>(79), l[0].data().size());
    ASSERT_EQ(MP4::CoverArt::JPEG, l[1].format());
    ASSERT_EQ(static_cast<unsigned int>(287), l[1].data().size());

    PropertyMap properties = f.properties();
    ASSERT_EQ(StringList("Test Artist!!!!"), properties["ARTIST"]);
    ASSERT_EQ(StringList("FAAC 1.24"), properties["ENCODEDBY"]);
  }
}
