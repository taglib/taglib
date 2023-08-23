/***************************************************************************
    copyright           : (C) 2007 by Lukas Lalinsky
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

#include "apetag.h"
#include "id3v1tag.h"
#include "id3v2extendedheader.h"
#include "id3v2tag.h"
#include "mpegfile.h"
#include "mpegheader.h"
#include "mpegproperties.h"
#include "tpropertymap.h"
#include "tstring.h"
#include "utils.h"
#include "xingheader.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(MPEG, testAudioPropertiesXingHeaderCBR)
{
  MPEG::File f(TEST_FILE_PATH_C("lame_cbr.mp3"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(1887, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(1887164, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(64, f.audioProperties()->bitrate());
  ASSERT_EQ(1, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(MPEG::XingHeader::Xing, f.audioProperties()->xingHeader()->type());
}

TEST(MPEG, testAudioPropertiesXingHeaderVBR)
{
  MPEG::File f(TEST_FILE_PATH_C("lame_vbr.mp3"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(1887, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(1887164, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(70, f.audioProperties()->bitrate());
  ASSERT_EQ(1, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(MPEG::XingHeader::Xing, f.audioProperties()->xingHeader()->type());
}

TEST(MPEG, testAudioPropertiesVBRIHeader)
{
  MPEG::File f(TEST_FILE_PATH_C("rare_frames.mp3"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(222, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(222198, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(233, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(MPEG::XingHeader::VBRI, f.audioProperties()->xingHeader()->type());
}

TEST(MPEG, testAudioPropertiesNoVBRHeaders)
{
  MPEG::File f(TEST_FILE_PATH_C("bladeenc.mp3"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3553, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(64, f.audioProperties()->bitrate());
  ASSERT_EQ(1, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_FALSE(f.audioProperties()->xingHeader());

  const offset_t last = f.lastFrameOffset();
  const MPEG::Header lastHeader(&f, last, false);

  ASSERT_EQ(static_cast<offset_t>(28213), last);
  ASSERT_EQ(209, lastHeader.frameLength());
}

TEST(MPEG, testSkipInvalidFrames1)
{
  MPEG::File f(TEST_FILE_PATH_C("invalid-frames1.mp3"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(392, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(160, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_FALSE(f.audioProperties()->xingHeader());
}

TEST(MPEG, testSkipInvalidFrames2)
{
  MPEG::File f(TEST_FILE_PATH_C("invalid-frames2.mp3"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(314, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(192, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_FALSE(f.audioProperties()->xingHeader());
}

TEST(MPEG, testSkipInvalidFrames3)
{
  MPEG::File f(TEST_FILE_PATH_C("invalid-frames3.mp3"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(183, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(320, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_FALSE(f.audioProperties()->xingHeader());
}

TEST(MPEG, testVersion2DurationWithXingHeader)
{
  MPEG::File f(TEST_FILE_PATH_C("mpeg2.mp3"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(5387, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(5387285, f.audioProperties()->lengthInMilliseconds());
}

TEST(MPEG, testSaveID3v24)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  String xxx     = ByteVector(254, 'X');
  {
    MPEG::File f(newname.c_str());
    ASSERT_FALSE(f.hasID3v2Tag());

    f.tag()->setTitle(xxx);
    f.tag()->setArtist("Artist A");
    f.save(MPEG::File::AllTags, File::StripOthers, ID3v2::v4);
    ASSERT_TRUE(f.hasID3v2Tag());
  }
  {
    MPEG::File f2(newname.c_str());
    ASSERT_EQ(static_cast<unsigned int>(4), f2.ID3v2Tag()->header()->majorVersion());
    ASSERT_EQ(String("Artist A"), f2.tag()->artist());
    ASSERT_EQ(xxx, f2.tag()->title());
  }
}

TEST(MPEG, testSaveID3v23)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  String xxx     = ByteVector(254, 'X');
  {
    MPEG::File f(newname.c_str());
    ASSERT_FALSE(f.hasID3v2Tag());

    f.tag()->setTitle(xxx);
    f.tag()->setArtist("Artist A");
    f.save(MPEG::File::AllTags, File::StripOthers, ID3v2::v3);
    ASSERT_TRUE(f.hasID3v2Tag());
  }
  {
    MPEG::File f2(newname.c_str());
    ASSERT_EQ(static_cast<unsigned int>(3), f2.ID3v2Tag()->header()->majorVersion());
    ASSERT_EQ(String("Artist A"), f2.tag()->artist());
    ASSERT_EQ(xxx, f2.tag()->title());
  }
}

TEST(MPEG, testDuplicateID3v2)
{
  MPEG::File f(TEST_FILE_PATH_C("duplicate_id3v2.mp3"));

  // duplicate_id3v2.mp3 has duplicate ID3v2 tags.
  // Sample rate will be 32000 if can't skip the second tag.

  ASSERT_TRUE(f.hasID3v2Tag());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
}

TEST(MPEG, testFuzzedFile)
{
  MPEG::File f(TEST_FILE_PATH_C("excessive_alloc.mp3"));
  ASSERT_TRUE(f.isValid());
}

TEST(MPEG, testFrameOffset)
{
  {
    MPEG::File f(TEST_FILE_PATH_C("ape.mp3"));
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(0x0000), f.firstFrameOffset());
    ASSERT_EQ(static_cast<offset_t>(0x1FD6), f.lastFrameOffset());
  }
  {
    MPEG::File f(TEST_FILE_PATH_C("ape-id3v1.mp3"));
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(0x0000), f.firstFrameOffset());
    ASSERT_EQ(static_cast<offset_t>(0x1FD6), f.lastFrameOffset());
  }
  {
    MPEG::File f(TEST_FILE_PATH_C("ape-id3v2.mp3"));
    ASSERT_TRUE(f.isValid());
    ASSERT_EQ(static_cast<offset_t>(0x041A), f.firstFrameOffset());
    ASSERT_EQ(static_cast<offset_t>(0x23F0), f.lastFrameOffset());
  }
}

TEST(MPEG, testStripAndProperties)
{
  ScopedFileCopy copy("xing", ".mp3");

  {
    MPEG::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle("ID3v2");
    f.APETag(true)->setTitle("APE");
    f.ID3v1Tag(true)->setTitle("ID3v1");
    f.save();
  }
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_EQ(String("ID3v2"), f.properties()["TITLE"].front());
    f.strip(MPEG::File::ID3v2);
    ASSERT_EQ(String("APE"), f.properties()["TITLE"].front());
    f.strip(MPEG::File::APE);
    ASSERT_EQ(String("ID3v1"), f.properties()["TITLE"].front());
    f.strip(MPEG::File::ID3v1);
    ASSERT_TRUE(f.properties().isEmpty());
  }
}

TEST(MPEG, testProperties)
{
  PropertyMap tags;
  tags["ALBUM"]                      = StringList("Album");
  tags["ALBUMARTIST"]                = StringList("Album Artist");
  tags["ALBUMARTISTSORT"]            = StringList("Album Artist Sort");
  tags["ALBUMSORT"]                  = StringList("Album Sort");
  tags["ARRANGER"]                   = StringList("Arranger");
  tags["ARTIST"]                     = StringList("Artist");
  tags["ARTISTSORT"]                 = StringList("Artist Sort");
  tags["ARTISTWEBPAGE"]              = StringList("Artist Web Page");
  tags["ASIN"]                       = StringList("ASIN");
  tags["AUDIOSOURCEWEBPAGE"]         = StringList("Audio Source Web Page");
  tags["BARCODE"]                    = StringList("Barcode");
  tags["BPM"]                        = StringList("123");
  tags["CATALOGNUMBER"]              = StringList("Catalog Number");
  tags["COMMENT"]                    = StringList("Comment");
  tags["COMMENT:CDESC"]              = StringList("Comment with Description");
  tags["COMPILATION"]                = StringList("1");
  tags["COMPOSER"]                   = StringList("Composer");
  tags["COMPOSERSORT"]               = StringList("Composer Sort");
  tags["CONDUCTOR"]                  = StringList("Conductor");
  tags["WORK"]                       = StringList("Content Group");
  tags["COPYRIGHT"]                  = StringList("2021 Copyright");
  tags["COPYRIGHTURL"]               = StringList("Copyright URL");
  tags["DATE"]                       = StringList("2021-01-03 12:29:23");
  tags["DISCNUMBER"]                 = StringList("3/5");
  tags["DISCSUBTITLE"]               = StringList("Disc Subtitle");
  tags["DJMIXER"]                    = StringList("DJ Mixer");
  tags["ENCODEDBY"]                  = StringList("Encoded by");
  tags["ENCODING"]                   = StringList("Encoding");
  tags["ENCODINGTIME"]               = StringList("2021-01-03 13:48:44");
  tags["ENGINEER"]                   = StringList("Engineer");
  tags["FILETYPE"]                   = StringList("File Type");
  tags["FILEWEBPAGE"]                = StringList("File Web Page");
  tags["GENRE"]                      = StringList("Genre");
  tags["GROUPING"]                   = StringList("Grouping");
  tags["INITIALKEY"]                 = StringList("Dbm");
  tags["ISRC"]                       = StringList("UKAAA0500001");
  tags["LABEL"]                      = StringList("Label");
  tags["LANGUAGE"]                   = StringList("eng");
  tags["LENGTH"]                     = StringList("1234");
  tags["LYRICIST"]                   = StringList("Lyricist");
  tags["LYRICS:LDESC"]               = StringList("Lyrics");
  tags["MEDIA"]                      = StringList("Media");
  tags["MIXER"]                      = StringList("Mixer");
  tags["MOOD"]                       = StringList("Mood");
  tags["MOVEMENTNAME"]               = StringList("Movement Name");
  tags["MOVEMENTNUMBER"]             = StringList("2");
  tags["MUSICBRAINZ_ALBUMID"]        = StringList("MusicBrainz_AlbumID");
  tags["MUSICBRAINZ_ALBUMARTISTID"]  = StringList("MusicBrainz_AlbumartistID");
  tags["MUSICBRAINZ_ARTISTID"]       = StringList("MusicBrainz_ArtistID");
  tags["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
  tags["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
  tags["MUSICBRAINZ_TRACKID"]        = StringList("MusicBrainz_TrackID");
  tags["MUSICBRAINZ_WORKID"]         = StringList("MusicBrainz_WorkID");
  tags["ORIGINALALBUM"]              = StringList("Original Album");
  tags["ORIGINALARTIST"]             = StringList("Original Artist");
  tags["ORIGINALDATE"]               = StringList("2021-01-03 13:52:19");
  tags["ORIGINALFILENAME"]           = StringList("Original Filename");
  tags["ORIGINALLYRICIST"]           = StringList("Original Lyricist");
  tags["OWNER"]                      = StringList("Owner");
  tags["PAYMENTWEBPAGE"]             = StringList("Payment Web Page");
  tags["PERFORMER:DRUMS"]            = StringList("Drummer");
  tags["PERFORMER:GUITAR"]           = StringList("Guitarist");
  tags["PLAYLISTDELAY"]              = StringList("10");
  tags["PODCAST"]                    = StringList();
  tags["PODCASTCATEGORY"]            = StringList("Podcast Category");
  tags["PODCASTDESC"]                = StringList("Podcast Description");
  tags["PODCASTID"]                  = StringList("Podcast ID");
  tags["PODCASTURL"]                 = StringList("Podcast URL");
  tags["PRODUCEDNOTICE"]             = StringList("2021 Produced Notice");
  tags["PRODUCER"]                   = StringList("Producer");
  tags["PUBLISHERWEBPAGE"]           = StringList("Publisher Web Page");
  tags["RADIOSTATION"]               = StringList("Radio Station");
  tags["RADIOSTATIONOWNER"]          = StringList("Radio Station Owner");
  tags["RELEASECOUNTRY"]             = StringList("Release Country");
  tags["RELEASESTATUS"]              = StringList("Release Status");
  tags["RELEASETYPE"]                = StringList("Release Type");
  tags["REMIXER"]                    = StringList("Remixer");
  tags["SCRIPT"]                     = StringList("Script");
  tags["SUBTITLE"]                   = StringList("Subtitle");
  tags["TITLE"]                      = StringList("Title");
  tags["TITLESORT"]                  = StringList("Title Sort");
  tags["TRACKNUMBER"]                = StringList("2/4");
  tags["URL:UDESC"]                  = StringList("URL");

  ScopedFileCopy copy("xing", ".mp3");
  {
    MPEG::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    ASSERT_TRUE(properties.isEmpty());
    f.setProperties(tags);
    f.save();
  }
  {
    const MPEG::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    if(tags != properties) {
      ASSERT_EQ(tags.toString(), properties.toString());
    }
    ASSERT_EQ(tags, properties);
  }
}

TEST(MPEG, testRepeatedSave1)
{
  ScopedFileCopy copy("xing", ".mp3");

  {
    MPEG::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle(std::string(4096, 'X').c_str());
    f.save();
  }
  {
    MPEG::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle("");
    f.save();
    f.ID3v2Tag(true)->setTitle(std::string(4096, 'X').c_str());
    f.save();
    ASSERT_EQ(static_cast<offset_t>(5141), f.firstFrameOffset());
  }
}

TEST(MPEG, testRepeatedSave2)
{
  ScopedFileCopy copy("xing", ".mp3");

  MPEG::File f(copy.fileName().c_str());
  f.ID3v2Tag(true)->setTitle("0123456789");
  f.save();
  f.save();
  ASSERT_EQ(static_cast<offset_t>(-1), f.find("ID3", 3));
}

TEST(MPEG, testRepeatedSave3)
{
  ScopedFileCopy copy("xing", ".mp3");

  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasAPETag());
    ASSERT_FALSE(f.hasID3v1Tag());

    f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.save();
    f.APETag()->setTitle("0");
    f.save();
    f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.APETag()->setTitle("01234 56789 ABCDE FGHIJ 01234 56789 ABCDE FGHIJ 01234 56789");
    f.save();
  }
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasAPETag());
    ASSERT_TRUE(f.hasID3v1Tag());
  }
}

TEST(MPEG, testEmptyID3v2)
{
  ScopedFileCopy copy("xing", ".mp3");

  {
    MPEG::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle("0123456789");
    f.save(MPEG::File::ID3v2);
  }
  {
    MPEG::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle("");
    f.save(MPEG::File::ID3v2, File::StripNone);
  }
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasID3v2Tag());
  }
}

TEST(MPEG, testEmptyID3v1)
{
  ScopedFileCopy copy("xing", ".mp3");

  {
    MPEG::File f(copy.fileName().c_str());
    f.ID3v1Tag(true)->setTitle("0123456789");
    f.save(MPEG::File::ID3v1);
  }
  {
    MPEG::File f(copy.fileName().c_str());
    f.ID3v1Tag(true)->setTitle("");
    f.save(MPEG::File::ID3v1, File::StripNone);
  }
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasID3v1Tag());
  }
}

TEST(MPEG, testEmptyAPE)
{
  ScopedFileCopy copy("xing", ".mp3");

  {
    MPEG::File f(copy.fileName().c_str());
    f.APETag(true)->setTitle("0123456789");
    f.save(MPEG::File::APE);
  }
  {
    MPEG::File f(copy.fileName().c_str());
    f.APETag(true)->setTitle("");
    f.save(MPEG::File::APE, File::StripNone);
  }
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasAPETag());
  }
}

TEST(MPEG, testIgnoreGarbage)
{
  const ScopedFileCopy copy("garbage", ".mp3");
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(static_cast<offset_t>(2255), f.firstFrameOffset());
    ASSERT_EQ(static_cast<offset_t>(6015), f.lastFrameOffset());
    ASSERT_EQ(String("Title A"), f.ID3v2Tag()->title());
    f.ID3v2Tag()->setTitle("Title B");
    f.save();
  }
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(String("Title B"), f.ID3v2Tag()->title());
  }
}

TEST(MPEG, testExtendedHeader)
{
  const ScopedFileCopy copy("extended-header", ".mp3");
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasID3v2Tag());
    ID3v2::Tag *tag            = f.ID3v2Tag();
    ID3v2::ExtendedHeader *ext = tag->extendedHeader();
    ASSERT_TRUE(ext);
    ASSERT_EQ(12U, ext->size());
    ASSERT_EQ(String("Druids"), tag->title());
    ASSERT_EQ(String("Excelsis"), tag->artist());
    ASSERT_EQ(String("Vo Chrieger U Drache"), tag->album());
    ASSERT_EQ(2013U, tag->year());
    ASSERT_EQ(String("Folk/Power Metal"), tag->genre());
    ASSERT_EQ(3U, tag->track());
    ASSERT_EQ(String("2013"),
              f.properties().value("ORIGINALDATE").front());
  }
}
