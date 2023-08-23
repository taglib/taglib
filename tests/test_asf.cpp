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

#include "asffile.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(ASF, testAudioProperties)
{
  ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3712, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(64, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(48000, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(ASF::Properties::WMA2, f.audioProperties()->codec());
  ASSERT_EQ(String("Windows Media Audio 9.1"), f.audioProperties()->codecName());
  ASSERT_EQ(String("64 kbps, 48 kHz, stereo 2-pass CBR"), f.audioProperties()->codecDescription());
  ASSERT_FALSE(f.audioProperties()->isEncrypted());
}

TEST(ASF, testLosslessProperties)
{
  ASF::File f(TEST_FILE_PATH_C("lossless.wma"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3549, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(1152, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(ASF::Properties::WMA9Lossless, f.audioProperties()->codec());
  ASSERT_EQ(String("Windows Media Audio 9.2 Lossless"), f.audioProperties()->codecName());
  ASSERT_EQ(String("VBR Quality 100, 44 kHz, 2 channel 16 bit 1-pass VBR"), f.audioProperties()->codecDescription());
  ASSERT_FALSE(f.audioProperties()->isEncrypted());
}

TEST(ASF, testRead)
{
  ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));
  ASSERT_EQ(String("test"), f.tag()->title());
}

TEST(ASF, testSaveMultipleValues)
{
  ScopedFileCopy copy("silence-1", ".wma");
  string newname = copy.fileName();

  {
    ASF::File f(newname.c_str());
    ASF::AttributeList values;
    values.append("Foo");
    values.append("Bar");
    f.tag()->setAttribute("WM/AlbumTitle", values);
    f.save();
  }
  {
    ASF::File f(newname.c_str());
    ASSERT_EQ(2, static_cast<int>(f.tag()->attributeListMap()["WM/AlbumTitle"].size()));
  }
}

TEST(ASF, testDWordTrackNumber)
{
  ScopedFileCopy copy("silence-1", ".wma");
  string newname = copy.fileName();

  {
    ASF::File f(newname.c_str());
    ASSERT_FALSE(f.tag()->contains("WM/TrackNumber"));
    f.tag()->setAttribute("WM/TrackNumber", static_cast<unsigned int>(123));
    f.save();
  }
  {
    ASF::File f(newname.c_str());
    ASSERT_TRUE(f.tag()->contains("WM/TrackNumber"));
    ASSERT_EQ(ASF::Attribute::DWordType,
              f.tag()->attribute("WM/TrackNumber").front().type());
    ASSERT_EQ(static_cast<unsigned int>(123), f.tag()->track());
    f.tag()->setTrack(234);
    f.save();
  }
  {
    ASF::File f(newname.c_str());
    ASSERT_TRUE(f.tag()->contains("WM/TrackNumber"));
    ASSERT_EQ(ASF::Attribute::UnicodeType,
              f.tag()->attribute("WM/TrackNumber").front().type());
    ASSERT_EQ(static_cast<unsigned int>(234), f.tag()->track());
  }
}

TEST(ASF, testSaveStream)
{
  ScopedFileCopy copy("silence-1", ".wma");
  string newname = copy.fileName();

  {
    ASF::File f(newname.c_str());
    ASF::Attribute attr("Foo");
    attr.setStream(43);
    f.tag()->setAttribute("WM/AlbumTitle", attr);
    f.save();
  }

  {
    ASF::File f(newname.c_str());
    ASSERT_EQ(43, f.tag()->attribute("WM/AlbumTitle").front().stream());
  }
}

TEST(ASF, testSaveLanguage)
{
  ScopedFileCopy copy("silence-1", ".wma");
  string newname = copy.fileName();

  {
    ASF::File f(newname.c_str());
    ASF::Attribute attr("Foo");
    attr.setStream(32);
    attr.setLanguage(56);
    f.tag()->setAttribute("WM/AlbumTitle", attr);
    f.save();
  }
  {
    ASF::File f(newname.c_str());
    ASSERT_EQ(32, f.tag()->attribute("WM/AlbumTitle").front().stream());
    ASSERT_EQ(56, f.tag()->attribute("WM/AlbumTitle").front().language());
  }
}

TEST(ASF, testSaveLargeValue)
{
  ScopedFileCopy copy("silence-1", ".wma");
  string newname = copy.fileName();

  {
    ASF::File f(newname.c_str());
    ASF::Attribute attr(ByteVector(70000, 'x'));
    f.tag()->setAttribute("WM/Blob", attr);
    f.save();
  }
  {
    ASF::File f(newname.c_str());
    ASSERT_EQ(ByteVector(70000, 'x'),
              f.tag()->attribute("WM/Blob").front().toByteVector());
  }
}

TEST(ASF, testSavePicture)
{
  ScopedFileCopy copy("silence-1", ".wma");
  string newname = copy.fileName();

  {
    ASF::File f(newname.c_str());
    ASF::Picture picture;
    picture.setMimeType("image/jpeg");
    picture.setType(ASF::Picture::FrontCover);
    picture.setDescription("description");
    picture.setPicture("data");
    f.tag()->setAttribute("WM/Picture", picture);
    f.save();
  }
  {
    ASF::File f(newname.c_str());
    ASF::AttributeList values2 = f.tag()->attribute("WM/Picture");
    ASSERT_EQ(static_cast<unsigned int>(1), values2.size());
    ASF::Attribute attr2  = values2.front();
    ASF::Picture picture2 = attr2.toPicture();
    ASSERT_TRUE(picture2.isValid());
    ASSERT_EQ(String("image/jpeg"), picture2.mimeType());
    ASSERT_EQ(ASF::Picture::FrontCover, picture2.type());
    ASSERT_EQ(String("description"), picture2.description());
    ASSERT_EQ(ByteVector("data"), picture2.picture());
  }
}

TEST(ASF, testSaveMultiplePictures)
{
  ScopedFileCopy copy("silence-1", ".wma");
  string newname = copy.fileName();

  {
    ASF::File f(newname.c_str());
    ASF::AttributeList values;
    ASF::Picture picture;
    picture.setMimeType("image/jpeg");
    picture.setType(ASF::Picture::FrontCover);
    picture.setDescription("description");
    picture.setPicture("data");
    values.append(ASF::Attribute(picture));
    ASF::Picture picture2;
    picture2.setMimeType("image/png");
    picture2.setType(ASF::Picture::BackCover);
    picture2.setDescription("back cover");
    picture2.setPicture("PNG data");
    values.append(ASF::Attribute(picture2));
    f.tag()->setAttribute("WM/Picture", values);
    f.save();
  }
  {
    ASF::File f(newname.c_str());
    ASF::AttributeList values2 = f.tag()->attribute("WM/Picture");
    ASSERT_EQ(static_cast<unsigned int>(2), values2.size());
    ASF::Picture picture3 = values2[1].toPicture();
    ASSERT_TRUE(picture3.isValid());
    ASSERT_EQ(String("image/jpeg"), picture3.mimeType());
    ASSERT_EQ(ASF::Picture::FrontCover, picture3.type());
    ASSERT_EQ(String("description"), picture3.description());
    ASSERT_EQ(ByteVector("data"), picture3.picture());
    ASF::Picture picture4 = values2[0].toPicture();
    ASSERT_TRUE(picture4.isValid());
    ASSERT_EQ(String("image/png"), picture4.mimeType());
    ASSERT_EQ(ASF::Picture::BackCover, picture4.type());
    ASSERT_EQ(String("back cover"), picture4.description());
    ASSERT_EQ(ByteVector("PNG data"), picture4.picture());
  }
}

TEST(ASF, testProperties)
{
  ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));

  PropertyMap tags    = f.properties();

  tags["TRACKNUMBER"] = StringList("2");
  tags["DISCNUMBER"]  = StringList("3");
  tags["BPM"]         = StringList("123");
  tags["ARTIST"]      = StringList("Foo Bar");
  f.setProperties(tags);

  tags = f.properties();

  ASSERT_EQ(String("Foo Bar"), f.tag()->artist());
  ASSERT_EQ(StringList("Foo Bar"), tags["ARTIST"]);

  ASSERT_TRUE(f.tag()->contains("WM/BeatsPerMinute"));
  ASSERT_EQ(1u, f.tag()->attributeListMap()["WM/BeatsPerMinute"].size());
  ASSERT_EQ(String("123"), f.tag()->attribute("WM/BeatsPerMinute").front().toString());
  ASSERT_EQ(StringList("123"), tags["BPM"]);

  ASSERT_TRUE(f.tag()->contains("WM/TrackNumber"));
  ASSERT_EQ(1u, f.tag()->attributeListMap()["WM/TrackNumber"].size());
  ASSERT_EQ(String("2"), f.tag()->attribute("WM/TrackNumber").front().toString());
  ASSERT_EQ(StringList("2"), tags["TRACKNUMBER"]);

  ASSERT_TRUE(f.tag()->contains("WM/PartOfSet"));
  ASSERT_EQ(1u, f.tag()->attributeListMap()["WM/PartOfSet"].size());
  ASSERT_EQ(String("3"), f.tag()->attribute("WM/PartOfSet").front().toString());
  ASSERT_EQ(StringList("3"), tags["DISCNUMBER"]);
}

TEST(ASF, testPropertiesAllSupported)
{
  PropertyMap tags;
  tags["ACOUSTID_ID"]                = StringList("Acoustid ID");
  tags["ACOUSTID_FINGERPRINT"]       = StringList("Acoustid Fingerprint");
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
  tags["COMPOSER"]                   = StringList("Composer");
  tags["CONDUCTOR"]                  = StringList("Conductor");
  tags["COPYRIGHT"]                  = StringList("2021 Copyright");
  tags["DATE"]                       = StringList("2021-01-03 12:29:23");
  tags["DISCNUMBER"]                 = StringList("3/5");
  tags["DISCSUBTITLE"]               = StringList("Disc Subtitle");
  tags["ENCODEDBY"]                  = StringList("Encoded by");
  tags["GENRE"]                      = StringList("Genre");
  tags["WORK"]                       = StringList("Grouping");
  tags["ISRC"]                       = StringList("UKAAA0500001");
  tags["LABEL"]                      = StringList("Label");
  tags["LANGUAGE"]                   = StringList("eng");
  tags["LYRICIST"]                   = StringList("Lyricist");
  tags["LYRICS"]                     = StringList("Lyrics");
  tags["MEDIA"]                      = StringList("Media");
  tags["MOOD"]                       = StringList("Mood");
  tags["MUSICBRAINZ_ALBUMARTISTID"]  = StringList("MusicBrainz_AlbumartistID");
  tags["MUSICBRAINZ_ALBUMID"]        = StringList("MusicBrainz_AlbumID");
  tags["MUSICBRAINZ_ARTISTID"]       = StringList("MusicBrainz_ArtistID");
  tags["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
  tags["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
  tags["MUSICBRAINZ_TRACKID"]        = StringList("MusicBrainz_TrackID");
  tags["MUSICBRAINZ_WORKID"]         = StringList("MusicBrainz_WorkID");
  tags["MUSICIP_PUID"]               = StringList("MusicIP PUID");
  tags["ORIGINALDATE"]               = StringList("2021-01-03 13:52:19");
  tags["PRODUCER"]                   = StringList("Producer");
  tags["RELEASECOUNTRY"]             = StringList("Release Country");
  tags["RELEASESTATUS"]              = StringList("Release Status");
  tags["RELEASETYPE"]                = StringList("Release Type");
  tags["REMIXER"]                    = StringList("Remixer");
  tags["SCRIPT"]                     = StringList("Script");
  tags["SUBTITLE"]                   = StringList("Subtitle");
  tags["TITLE"]                      = StringList("Title");
  tags["TITLESORT"]                  = StringList("Title Sort");
  tags["TRACKNUMBER"]                = StringList("2/4");

  ScopedFileCopy copy("silence-1", ".wma");
  {
    ASF::File f(copy.fileName().c_str());
    ASF::Tag *asfTag = f.tag();
    asfTag->setTitle("");
    asfTag->attributeListMap().clear();
    f.save();
  }
  {
    ASF::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    ASSERT_TRUE(properties.isEmpty());
    f.setProperties(tags);
    f.save();
  }
  {
    const ASF::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    if(tags != properties) {
      ASSERT_EQ(tags.toString(), properties.toString());
    }
    ASSERT_EQ(tags, properties);
  }
}

TEST(ASF, testRepeatedSave)
{
  ScopedFileCopy copy("silence-1", ".wma");

  {
    ASF::File f(copy.fileName().c_str());
    f.tag()->setTitle(longText(128 * 1024));
    f.save();
    ASSERT_EQ(static_cast<offset_t>(297578), f.length());
    f.tag()->setTitle(longText(16 * 1024));
    f.save();
    ASSERT_EQ(static_cast<offset_t>(68202), f.length());
  }
}
