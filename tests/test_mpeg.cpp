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

#include <string>
#include <cstdio>
#include <array>

#include "tstring.h"
#include "tpropertymap.h"
#include "mpegfile.h"
#include "id3v2tag.h"
#include "id3v1tag.h"
#include "apetag.h"
#include "mpegproperties.h"
#include "xingheader.h"
#include "mpegheader.h"
#include "id3v2extendedheader.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMPEG : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMPEG);
  CPPUNIT_TEST(testAudioPropertiesXingHeaderCBR);
  CPPUNIT_TEST(testAudioPropertiesXingHeaderVBR);
  CPPUNIT_TEST(testAudioPropertiesVBRIHeader);
  CPPUNIT_TEST(testAudioPropertiesNoVBRHeaders);
  CPPUNIT_TEST(testAudioPropertiesADTS);
  CPPUNIT_TEST(testSkipInvalidFrames1);
  CPPUNIT_TEST(testSkipInvalidFrames2);
  CPPUNIT_TEST(testSkipInvalidFrames3);
  CPPUNIT_TEST(testVersion2DurationWithXingHeader);
  CPPUNIT_TEST(testSaveID3v24);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testDuplicateID3v2);
  CPPUNIT_TEST(testFuzzedFile);
  CPPUNIT_TEST(testFrameOffset);
  CPPUNIT_TEST(testStripAndProperties);
  CPPUNIT_TEST(testProperties);
  CPPUNIT_TEST(testRepeatedSave1);
  CPPUNIT_TEST(testRepeatedSave2);
  CPPUNIT_TEST(testRepeatedSave3);
  CPPUNIT_TEST(testEmptyID3v2);
  CPPUNIT_TEST(testEmptyID3v1);
  CPPUNIT_TEST(testEmptyAPE);
  CPPUNIT_TEST(testIgnoreGarbage);
  CPPUNIT_TEST(testExtendedHeader);
  CPPUNIT_TEST(testReadStyleFast);
  CPPUNIT_TEST(testID3v22Properties);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioPropertiesXingHeaderCBR()
  {
    MPEG::File f(TEST_FILE_PATH_C("lame_cbr.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(1887, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(1887164, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(MPEG::XingHeader::Xing, f.audioProperties()->xingHeader()->type());
    CPPUNIT_ASSERT(!f.audioProperties()->isADTS());
  }

  void testAudioPropertiesXingHeaderVBR()
  {
    MPEG::File f(TEST_FILE_PATH_C("lame_vbr.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(1887, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(1887164, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(70, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(MPEG::XingHeader::Xing, f.audioProperties()->xingHeader()->type());
    CPPUNIT_ASSERT(!f.audioProperties()->isADTS());
  }

  void testAudioPropertiesVBRIHeader()
  {
    MPEG::File f(TEST_FILE_PATH_C("rare_frames.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(222, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(222198, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(233, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(MPEG::XingHeader::VBRI, f.audioProperties()->xingHeader()->type());
    CPPUNIT_ASSERT(!f.audioProperties()->isADTS());
  }

  void testAudioPropertiesNoVBRHeaders()
  {
    MPEG::File f(TEST_FILE_PATH_C("bladeenc.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3553, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT(!f.audioProperties()->xingHeader());
    CPPUNIT_ASSERT(!f.audioProperties()->isADTS());

    const offset_t last = f.lastFrameOffset();
    const MPEG::Header lastHeader(&f, last, false);

    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(28213), last);
    CPPUNIT_ASSERT_EQUAL(209, lastHeader.frameLength());
  }

  void testAudioPropertiesADTS()
  {
    constexpr std::array readStyles = {
      MPEG::Properties::Fast,
      MPEG::Properties::Average,
      MPEG::Properties::Accurate
    };
    for(auto readStyle : readStyles) {
      MPEG::File f(TEST_FILE_PATH_C("empty1s.aac"), true, readStyle);
      CPPUNIT_ASSERT(f.audioProperties());
      CPPUNIT_ASSERT_EQUAL(readStyle == MPEG::Properties::Fast ? 0 : 1,
        f.audioProperties()->lengthInSeconds());
      CPPUNIT_ASSERT_EQUAL(readStyle == MPEG::Properties::Fast ? 0 : 1176,
        f.audioProperties()->lengthInMilliseconds());
      CPPUNIT_ASSERT_EQUAL(readStyle == MPEG::Properties::Fast ? 0 : 1,
        f.audioProperties()->bitrate());
      CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
      CPPUNIT_ASSERT_EQUAL(MPEG::Header::FrontCenter,
        f.audioProperties()->channelConfiguration());
      CPPUNIT_ASSERT_EQUAL(11025, f.audioProperties()->sampleRate());
      CPPUNIT_ASSERT(!f.audioProperties()->xingHeader());
      CPPUNIT_ASSERT(f.audioProperties()->isADTS());

      const offset_t last = f.lastFrameOffset();
      const MPEG::Header lastHeader(&f, last, false);

      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(136), last);
      CPPUNIT_ASSERT_EQUAL(11, lastHeader.frameLength());
    }
  }

  void testSkipInvalidFrames1()
  {
    MPEG::File f(TEST_FILE_PATH_C("invalid-frames1.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(392, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(160, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT(!f.audioProperties()->xingHeader());
    CPPUNIT_ASSERT(!f.audioProperties()->isADTS());
  }

  void testSkipInvalidFrames2()
  {
    MPEG::File f(TEST_FILE_PATH_C("invalid-frames2.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(314, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(192, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT(!f.audioProperties()->xingHeader());
    CPPUNIT_ASSERT(!f.audioProperties()->isADTS());
  }

  void testSkipInvalidFrames3()
  {
    MPEG::File f(TEST_FILE_PATH_C("invalid-frames3.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(183, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(320, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT(!f.audioProperties()->xingHeader());
    CPPUNIT_ASSERT(!f.audioProperties()->isADTS());
  }

  void testVersion2DurationWithXingHeader()
  {
    MPEG::File f(TEST_FILE_PATH_C("mpeg2.mp3"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(5387, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(5387285, f.audioProperties()->lengthInMilliseconds());
  }

  void testSaveID3v24()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      MPEG::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(MPEG::File::AllTags, File::StripOthers, ID3v2::v4);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      MPEG::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(4), f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      MPEG::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(MPEG::File::AllTags, File::StripOthers, ID3v2::v3);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      MPEG::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testDuplicateID3v2()
  {
    MPEG::File f(TEST_FILE_PATH_C("duplicate_id3v2.mp3"));

    // duplicate_id3v2.mp3 has duplicate ID3v2 tags.
    // Sample rate will be 32000 if can't skip the second tag.

    CPPUNIT_ASSERT(f.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testFuzzedFile()
  {
    MPEG::File f(TEST_FILE_PATH_C("excessive_alloc.mp3"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFrameOffset()
  {
    {
      MPEG::File f(TEST_FILE_PATH_C("ape.mp3"));
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x0000), f.firstFrameOffset());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x1FD6), f.lastFrameOffset());
    }
    {
      MPEG::File f(TEST_FILE_PATH_C("ape-id3v1.mp3"));
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x0000), f.firstFrameOffset());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x1FD6), f.lastFrameOffset());
    }
    {
      MPEG::File f(TEST_FILE_PATH_C("ape-id3v2.mp3"));
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x041A), f.firstFrameOffset());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0x23F0), f.lastFrameOffset());
    }
  }

  void testStripAndProperties()
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
      CPPUNIT_ASSERT_EQUAL(String("ID3v2"), f.properties()["TITLE"].front());
      f.strip(MPEG::File::ID3v2);
      CPPUNIT_ASSERT_EQUAL(String("APE"), f.properties()["TITLE"].front());
      f.strip(MPEG::File::APE);
      CPPUNIT_ASSERT_EQUAL(String("ID3v1"), f.properties()["TITLE"].front());
      f.strip(MPEG::File::ID3v1);
      CPPUNIT_ASSERT(f.properties().isEmpty());
    }
  }

  void testProperties()
  {
    PropertyMap tags;
    tags["ALBUM"] = StringList("Album");
    tags["ALBUMARTIST"] = StringList("Album Artist");
    tags["ALBUMARTISTSORT"] = StringList("Album Artist Sort");
    tags["ALBUMSORT"] = StringList("Album Sort");
    tags["ARRANGER"] = StringList("Arranger");
    tags["ARTIST"] = StringList("Artist");
    tags["ARTISTSORT"] = StringList("Artist Sort");
    tags["ARTISTWEBPAGE"] = StringList("Artist Web Page");
    tags["ASIN"] = StringList("ASIN");
    tags["AUDIOSOURCEWEBPAGE"] = StringList("Audio Source Web Page");
    tags["BARCODE"] = StringList("Barcode");
    tags["BPM"] = StringList("123");
    tags["CATALOGNUMBER"] = StringList("Catalog Number");
    tags["COMMENT"] = StringList("Comment");
    tags["COMMENT:CDESC"] = StringList("Comment with Description");
    tags["COMPILATION"] = StringList("1");
    tags["COMPOSER"] = StringList("Composer");
    tags["COMPOSERSORT"] = StringList("Composer Sort");
    tags["CONDUCTOR"] = StringList("Conductor");
    tags["WORK"] = StringList("Content Group");
    tags["COPYRIGHT"] = StringList("2021 Copyright");
    tags["COPYRIGHTURL"] = StringList("Copyright URL");
    tags["DATE"] = StringList("2021-01-03 12:29:23");
    tags["DISCNUMBER"] = StringList("3/5");
    tags["DISCSUBTITLE"] = StringList("Disc Subtitle");
    tags["DJMIXER"] = StringList("DJ Mixer");
    tags["ENCODEDBY"] = StringList("Encoded by");
    tags["ENCODING"] = StringList("Encoding");
    tags["ENCODINGTIME"] = StringList("2021-01-03 13:48:44");
    tags["ENGINEER"] = StringList("Engineer");
    tags["FILETYPE"] = StringList("File Type");
    tags["FILEWEBPAGE"] = StringList("File Web Page");
    tags["GENRE"] = StringList("Genre");
    tags["GROUPING"] = StringList("Grouping");
    tags["INITIALKEY"] = StringList("Dbm");
    tags["ISRC"] = StringList("UKAAA0500001");
    tags["LABEL"] = StringList("Label");
    tags["LANGUAGE"] = StringList("eng");
    tags["LENGTH"] = StringList("1234");
    tags["LYRICIST"] = StringList("Lyricist");
    tags["LYRICS:LDESC"] = StringList("Lyrics");
    tags["MEDIA"] = StringList("Media");
    tags["MIXER"] = StringList("Mixer");
    tags["MOOD"] = StringList("Mood");
    tags["MOVEMENTNAME"] = StringList("Movement Name");
    tags["MOVEMENTNUMBER"] = StringList("2");
    tags["MUSICBRAINZ_ALBUMID"] = StringList("MusicBrainz_AlbumID");
    tags["MUSICBRAINZ_ALBUMARTISTID"] = StringList("MusicBrainz_AlbumartistID");
    tags["MUSICBRAINZ_ARTISTID"] = StringList("MusicBrainz_ArtistID");
    tags["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
    tags["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
    tags["MUSICBRAINZ_TRACKID"] = StringList("MusicBrainz_TrackID");
    tags["MUSICBRAINZ_WORKID"] = StringList("MusicBrainz_WorkID");
    tags["ORIGINALALBUM"] = StringList("Original Album");
    tags["ORIGINALARTIST"] = StringList("Original Artist");
    tags["ORIGINALDATE"] = StringList("2021-01-03 13:52:19");
    tags["ORIGINALFILENAME"] = StringList("Original Filename");
    tags["ORIGINALLYRICIST"] = StringList("Original Lyricist");
    tags["OWNER"] = StringList("Owner");
    tags["PAYMENTWEBPAGE"] = StringList("Payment Web Page");
    tags["PERFORMER:DRUMS"] = StringList("Drummer");
    tags["PERFORMER:GUITAR"] = StringList("Guitarist");
    tags["PLAYLISTDELAY"] = StringList("10");
    tags["PODCAST"] = StringList();
    tags["PODCASTCATEGORY"] = StringList("Podcast Category");
    tags["PODCASTDESC"] = StringList("Podcast Description");
    tags["PODCASTID"] = StringList("Podcast ID");
    tags["PODCASTURL"] = StringList("Podcast URL");
    tags["PRODUCEDNOTICE"] = StringList("2021 Produced Notice");
    tags["PRODUCER"] = StringList("Producer");
    tags["PUBLISHERWEBPAGE"] = StringList("Publisher Web Page");
    tags["RADIOSTATION"] = StringList("Radio Station");
    tags["RADIOSTATIONOWNER"] = StringList("Radio Station Owner");
    tags["RELEASECOUNTRY"] = StringList("Release Country");
    tags["RELEASESTATUS"] = StringList("Release Status");
    tags["RELEASETYPE"] = StringList("Release Type");
    tags["REMIXER"] = StringList("Remixer");
    tags["SCRIPT"] = StringList("Script");
    tags["SUBTITLE"] = StringList("Subtitle");
    tags["TITLE"] = StringList("Title");
    tags["TITLESORT"] = StringList("Title Sort");
    tags["TRACKNUMBER"] = StringList("2/4");
    tags["URL:UDESC"] = StringList("URL");

    ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      PropertyMap properties = f.properties();
      CPPUNIT_ASSERT(properties.isEmpty());
      f.setProperties(tags);
      f.save();
    }
    {
      const MPEG::File f(copy.fileName().c_str());
      PropertyMap properties = f.properties();
      if (tags != properties) {
        CPPUNIT_ASSERT_EQUAL(tags.toString(), properties.toString());
      }
      CPPUNIT_ASSERT(tags == properties);
    }
  }

  void testRepeatedSave1()
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
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(5141), f.firstFrameOffset());
    }
  }

  void testRepeatedSave2()
  {
    ScopedFileCopy copy("xing", ".mp3");

    MPEG::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle("0123456789");
    f.save();
    f.save();
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(-1), f.find("ID3", 3));
  }

  void testRepeatedSave3()
  {
    ScopedFileCopy copy("xing", ".mp3");

    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasAPETag());
      CPPUNIT_ASSERT(!f.hasID3v1Tag());

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
      CPPUNIT_ASSERT(f.hasAPETag());
      CPPUNIT_ASSERT(f.hasID3v1Tag());
    }
  }

  void testEmptyID3v2()
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
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
  }

  void testEmptyID3v1()
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
      CPPUNIT_ASSERT(!f.hasID3v1Tag());
    }
  }

  void testEmptyAPE()
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
      CPPUNIT_ASSERT(!f.hasAPETag());
    }
  }

  void testIgnoreGarbage()
  {
    const ScopedFileCopy copy("garbage", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(2255), f.firstFrameOffset());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(6015), f.lastFrameOffset());
      CPPUNIT_ASSERT_EQUAL(String("Title A"), f.ID3v2Tag()->title());
      f.ID3v2Tag()->setTitle("Title B");
      f.save();
    }
    {
      MPEG::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String("Title B"), f.ID3v2Tag()->title());
    }
  }

  void testExtendedHeader()
  {
    const ScopedFileCopy copy("extended-header", ".mp3");
    MPEG::File f(copy.fileName().c_str());
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasID3v2Tag());
    ID3v2::Tag *tag = f.ID3v2Tag();
    ID3v2::ExtendedHeader *ext = tag->extendedHeader();
    CPPUNIT_ASSERT(ext);
    CPPUNIT_ASSERT_EQUAL(12U, ext->size());
    CPPUNIT_ASSERT_EQUAL(String("Druids"), tag->title());
    CPPUNIT_ASSERT_EQUAL(String("Excelsis"), tag->artist());
    CPPUNIT_ASSERT_EQUAL(String("Vo Chrieger U Drache"), tag->album());
    CPPUNIT_ASSERT_EQUAL(2013U, tag->year());
    CPPUNIT_ASSERT_EQUAL(String("Folk/Power Metal"), tag->genre());
    CPPUNIT_ASSERT_EQUAL(3U, tag->track());
    CPPUNIT_ASSERT_EQUAL(String("2013"),
                         f.properties().value("ORIGINALDATE").front());
  }

  void testReadStyleFast()
  {
    const ScopedFileCopy copy("lame_cbr", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str(), true, MPEG::Properties::Fast);
      CPPUNIT_ASSERT(f.audioProperties());
      CPPUNIT_ASSERT_EQUAL(1887, f.audioProperties()->lengthInSeconds());
      CPPUNIT_ASSERT_EQUAL(1887164, f.audioProperties()->lengthInMilliseconds());
      CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitrate());
      CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
      CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(""), f.ID3v2Tag()->title());
      PropertyMap properties = f.properties();
      CPPUNIT_ASSERT_EQUAL(String("-1.020000 dB"), properties.value("REPLAYGAIN_TRACK_GAIN").front());
      CPPUNIT_ASSERT_EQUAL(String("0.920032"), properties.value("REPLAYGAIN_TRACK_PEAK").front());
      properties["TITLE"] = String("A Title");
      properties["Artist"] = String("An Artist");
      f.setProperties(properties);
      f.save();
    }
    {
      MPEG::File f(copy.fileName().c_str(), true, MPEG::Properties::Fast);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String("A Title"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("An Artist"), f.ID3v2Tag()->artist());
    }
    {
      MPEG::File f(TEST_FILE_PATH_C("garbage.mp3"), true, MPEG::Properties::Fast);
      CPPUNIT_ASSERT(f.isValid());
      // Garbage prevents detection of ID3v2 with fast read style
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(2255), f.firstFrameOffset());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(6015), f.lastFrameOffset());
    }
  }

  void testID3v22Properties()
  {
    ScopedFileCopy copy("itunes10", ".mp3");

    MPEG::File f(copy.fileName().c_str());
    PropertyMap expectedProperties(SimplePropertyMap{
      {"ALBUM", {"Album"}},
      {"ALBUMARTIST", {"Album Artist"}},
      {"ALBUMARTISTSORT", {"Sort Album Artist"}},
      {"ALBUMSORT", {"Sort Album"}},
      {"ARTIST", {"Artist"}},
      {"ARTISTSORT", {"Sort Artist"}},
      {"BPM", {"180"}},
      {"COMMENT", {"Comments"}},
      {"COMMENT:ITUNPGAP", {"1"}},
      {"COMPILATION", {"1"}},
      {"COMPOSER", {"Composer"}},
      {"COMPOSERSORT", {"Sort Composer"}},
      {"DATE", {"2011"}},
      {"DISCNUMBER", {"1/2"}},
      {"GENRE", {"Heavy Metal"}},
      {"LYRICS", {"Lyrics"}},
      {"SUBTITLE", {"Description"}},
      {"TITLE", {"iTunes10MP3"}},
      {"TITLESORT", {"Sort Name"}},
      {"TRACKNUMBER", {"1/10"}},
      {"WORK", {"Grouping"}}
    });
    expectedProperties.addUnsupportedData("APIC");
    expectedProperties.addUnsupportedData("UNKNOWN/RVA");

    PropertyMap properties = f.properties();
    if (expectedProperties != properties) {
      CPPUNIT_ASSERT_EQUAL(expectedProperties.toString(), properties.toString());
    }
    CPPUNIT_ASSERT(expectedProperties == properties);

    const String PICTURE_KEY("PICTURE");
    CPPUNIT_ASSERT_EQUAL(StringList(PICTURE_KEY), f.complexPropertyKeys());
    auto pictures = f.complexProperties(PICTURE_KEY);
    CPPUNIT_ASSERT_EQUAL(1U, pictures.size());
    auto picture = pictures.front();
    CPPUNIT_ASSERT_EQUAL(String("image/png"), picture.value("mimeType").toString());
    CPPUNIT_ASSERT(picture.value("description").toString().isEmpty());
    CPPUNIT_ASSERT_EQUAL(String("Other"), picture.value("pictureType").toString());
    auto data = picture.value("data").toByteVector();
    CPPUNIT_ASSERT(data.startsWith("\x89PNG\x0d\x0a\x1a\x0a"));
    CPPUNIT_ASSERT_EQUAL(2315U, data.size());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMPEG);
