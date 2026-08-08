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

#include "tstringlist.h"
#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tag.h"
#include "asffile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestASF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestASF);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST(testLosslessProperties);
  CPPUNIT_TEST(testRead);
  CPPUNIT_TEST(testSaveMultipleValues);
  CPPUNIT_TEST(testSaveStream);
  CPPUNIT_TEST(testSaveLanguage);
  CPPUNIT_TEST(testDWordTrackNumber);
  CPPUNIT_TEST(testSaveLargeValue);
  CPPUNIT_TEST(testSavePicture);
  CPPUNIT_TEST(testSaveMultiplePictures);
  CPPUNIT_TEST(testProperties);
  CPPUNIT_TEST(testPropertiesAllSupported);
  CPPUNIT_TEST(testPropertiesRealFile);
  CPPUNIT_TEST(testCaseInsensitiveAttributeNames);
  CPPUNIT_TEST(testRepeatedSave);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioProperties()
  {
    ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3712, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(48000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(ASF::Properties::WMA2, f.audioProperties()->codec());
    CPPUNIT_ASSERT_EQUAL(String("Windows Media Audio 9.1"), f.audioProperties()->codecName());
    CPPUNIT_ASSERT_EQUAL(String("64 kbps, 48 kHz, stereo 2-pass CBR"), f.audioProperties()->codecDescription());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
  }

  void testLosslessProperties()
  {
    ASF::File f(TEST_FILE_PATH_C("lossless.wma"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3549, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(1152, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(ASF::Properties::WMA9Lossless, f.audioProperties()->codec());
    CPPUNIT_ASSERT_EQUAL(String("Windows Media Audio 9.2 Lossless"), f.audioProperties()->codecName());
    CPPUNIT_ASSERT_EQUAL(String("VBR Quality 100, 44 kHz, 2 channel 16 bit 1-pass VBR"), f.audioProperties()->codecDescription());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isEncrypted());
  }

  void testRead()
  {
    ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));
    CPPUNIT_ASSERT_EQUAL(String("test"), f.tag()->title());
  }

  void testSaveMultipleValues()
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
      CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(
        f.tag()->attributeListMap()["WM/AlbumTitle"].size()));
    }
  }

  void testDWordTrackNumber()
  {
    ScopedFileCopy copy("silence-1", ".wma");
    string newname = copy.fileName();

    {
      ASF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.tag()->contains("WM/TrackNumber"));
      f.tag()->setAttribute("WM/TrackNumber", static_cast<unsigned int>(123));
      f.save();
    }
    {
      ASF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.tag()->contains("WM/TrackNumber"));
      CPPUNIT_ASSERT_EQUAL(ASF::Attribute::DWordType,
                           f.tag()->attribute("WM/TrackNumber").front().type());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(123), f.tag()->track());
      f.tag()->setTrack(234);
      f.save();
    }
    {
      ASF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.tag()->contains("WM/TrackNumber"));
      CPPUNIT_ASSERT_EQUAL(ASF::Attribute::UnicodeType,
                           f.tag()->attribute("WM/TrackNumber").front().type());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(234), f.tag()->track());
    }
  }

  void testSaveStream()
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
      CPPUNIT_ASSERT_EQUAL(43, f.tag()->attribute("WM/AlbumTitle").front().stream());
    }
  }

  void testSaveLanguage()
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
      CPPUNIT_ASSERT_EQUAL(32, f.tag()->attribute("WM/AlbumTitle").front().stream());
      CPPUNIT_ASSERT_EQUAL(56, f.tag()->attribute("WM/AlbumTitle").front().language());
    }
  }

  void testSaveLargeValue()
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
      CPPUNIT_ASSERT_EQUAL(ByteVector(70000, 'x'),
                           f.tag()->attribute("WM/Blob").front().toByteVector());
    }
  }

  void testSavePicture()
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
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), values2.size());
      ASF::Attribute attr2 = values2.front();
      ASF::Picture picture2 = attr2.toPicture();
      CPPUNIT_ASSERT(picture2.isValid());
      CPPUNIT_ASSERT_EQUAL(String("image/jpeg"), picture2.mimeType());
      CPPUNIT_ASSERT_EQUAL(ASF::Picture::FrontCover, picture2.type());
      CPPUNIT_ASSERT_EQUAL(String("description"), picture2.description());
      CPPUNIT_ASSERT_EQUAL(ByteVector("data"), picture2.picture());
    }
  }

  void testSaveMultiplePictures()
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
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), values2.size());
      ASF::Picture picture3 = values2[1].toPicture();
      CPPUNIT_ASSERT(picture3.isValid());
      CPPUNIT_ASSERT_EQUAL(String("image/jpeg"), picture3.mimeType());
      CPPUNIT_ASSERT_EQUAL(ASF::Picture::FrontCover, picture3.type());
      CPPUNIT_ASSERT_EQUAL(String("description"), picture3.description());
      CPPUNIT_ASSERT_EQUAL(ByteVector("data"), picture3.picture());
      ASF::Picture picture4 = values2[0].toPicture();
      CPPUNIT_ASSERT(picture4.isValid());
      CPPUNIT_ASSERT_EQUAL(String("image/png"), picture4.mimeType());
      CPPUNIT_ASSERT_EQUAL(ASF::Picture::BackCover, picture4.type());
      CPPUNIT_ASSERT_EQUAL(String("back cover"), picture4.description());
      CPPUNIT_ASSERT_EQUAL(ByteVector("PNG data"), picture4.picture());
    }
  }

  void testProperties()
  {
    ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));

    PropertyMap tags = f.properties();

    tags["TRACKNUMBER"] = StringList("2");
    tags["DISCNUMBER"] = StringList("3");
    tags["BPM"] = StringList("123");
    tags["ARTIST"] = StringList("Foo Bar");
    f.setProperties(tags);

    tags = f.properties();

    CPPUNIT_ASSERT_EQUAL(String("Foo Bar"), f.tag()->artist());
    CPPUNIT_ASSERT_EQUAL(StringList("Foo Bar"), tags["ARTIST"]);

    CPPUNIT_ASSERT(f.tag()->contains("WM/BeatsPerMinute"));
    CPPUNIT_ASSERT_EQUAL(1u, f.tag()->attributeListMap()["WM/BeatsPerMinute"].size());
    CPPUNIT_ASSERT_EQUAL(String("123"), f.tag()->attribute("WM/BeatsPerMinute").front().toString());
    CPPUNIT_ASSERT_EQUAL(StringList("123"), tags["BPM"]);

    CPPUNIT_ASSERT(f.tag()->contains("WM/TrackNumber"));
    CPPUNIT_ASSERT_EQUAL(1u, f.tag()->attributeListMap()["WM/TrackNumber"].size());
    CPPUNIT_ASSERT_EQUAL(String("2"), f.tag()->attribute("WM/TrackNumber").front().toString());
    CPPUNIT_ASSERT_EQUAL(StringList("2"), tags["TRACKNUMBER"]);

    CPPUNIT_ASSERT(f.tag()->contains("WM/PartOfSet"));
    CPPUNIT_ASSERT_EQUAL(1u, f.tag()->attributeListMap()["WM/PartOfSet"].size());
    CPPUNIT_ASSERT_EQUAL(String("3"), f.tag()->attribute("WM/PartOfSet").front().toString());
    CPPUNIT_ASSERT_EQUAL(StringList("3"), tags["DISCNUMBER"]);
  }

  void testPropertiesAllSupported()
  {
    PropertyMap tags;
    tags["ACOUSTID_ID"] = StringList("Acoustid ID");
    tags["ACOUSTID_FINGERPRINT"] = StringList("Acoustid Fingerprint");
    tags["ALBUM"] = StringList("Album");
    tags["ALBUMARTIST"] = StringList("Album Artist");
    tags["ALBUMARTISTSORT"] = StringList("Album Artist Sort");
    tags["ALBUMSORT"] = StringList("Album Sort");
    tags["ARTIST"] = StringList("Artist");
    tags["ARTISTS"] = StringList("Artists");
    tags["ARTISTSORT"] = StringList("Artist Sort");
    tags["ARTISTWEBPAGE"] = StringList("Artist Webpage");
    tags["ASIN"] = StringList("ASIN");
    tags["BARCODE"] = StringList("Barcode");
    tags["BPM"] = StringList("123");
    tags["CATALOGNUMBER"] = StringList("Catalog Number");
    tags["COMMENT"] = StringList("Comment");
    tags["COMPOSER"] = StringList("Composer");
    tags["CONDUCTOR"] = StringList("Conductor");
    tags["COPYRIGHT"] = StringList("2021 Copyright");
    tags["DATE"] = StringList("2021-01-03 12:29:23");
    tags["DISCNUMBER"] = StringList("3/5");
    tags["DISCSUBTITLE"] = StringList("Disc Subtitle");
    tags["ENCODEDBY"] = StringList("Encoded by");
    tags["ENCODING"] = StringList("Encoding");
    tags["ENCODINGTIME"] = StringList("131452413620000000");
    tags["FILEWEBPAGE"] = StringList("File Webpage");
    tags["GENRE"] = StringList("Genre");
    tags["WORK"] = StringList("Grouping");
    tags["INITIALKEY"] = StringList("Initial Key");
    tags["ISRC"] = StringList("UKAAA0500001");
    tags["LABEL"] = StringList("Label");
    tags["LANGUAGE"] = StringList("eng");
    tags["LYRICIST"] = StringList("Lyricist");
    tags["LYRICS"] = StringList("Lyrics");
    tags["MEDIA"] = StringList("Media");
    tags["MOOD"] = StringList("Mood");
    tags["MUSICBRAINZ_ALBUMARTISTID"] = StringList("MusicBrainz_AlbumartistID");
    tags["MUSICBRAINZ_ALBUMID"] = StringList("MusicBrainz_AlbumID");
    tags["MUSICBRAINZ_ARTISTID"] = StringList("MusicBrainz_ArtistID");
    tags["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
    tags["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
    tags["MUSICBRAINZ_TRACKID"] = StringList("MusicBrainz_TrackID");
    tags["MUSICBRAINZ_WORKID"] = StringList("MusicBrainz_WorkID");
    tags["MUSICIP_PUID"] = StringList("MusicIP PUID");
    tags["ORIGINALALBUM"] = StringList("Original Album");
    tags["ORIGINALARTIST"] = StringList("Original Artist");
    tags["ORIGINALFILENAME"] = StringList("Original Filename");
    tags["ORIGINALLYRICIST"] = StringList("Original Lyricist");
    tags["ORIGINALDATE"] = StringList("2021-01-03 13:52:19");
    tags["PRODUCER"] = StringList("Producer");
    tags["RELEASECOUNTRY"] = StringList("Release Country");
    tags["RELEASESTATUS"] = StringList("Release Status");
    tags["RELEASETYPE"] = StringList("Release Type");
    tags["REMIXER"] = StringList("Remixer");
    tags["SCRIPT"] = StringList("Script");
    tags["SUBTITLE"] = StringList("Subtitle");
    tags["TITLE"] = StringList("Title");
    tags["TITLESORT"] = StringList("Title Sort");
    tags["TRACKNUMBER"] = StringList("2/4");
    tags["REPLAYGAIN_TRACK_GAIN"] = StringList("-4.25 dB");
    tags["REPLAYGAIN_TRACK_PEAK"] = StringList("0.985412");
    tags["REPLAYGAIN_ALBUM_GAIN"] = StringList("-3.80 dB");
    tags["REPLAYGAIN_ALBUM_PEAK"] = StringList("0.998201");
    tags["MEDIACLASSPRIMARYID"] = StringList("D1607DBC-E323-4BE2-86A1-48A42A28441E");
    tags["MEDIACLASSSECONDARYID"] = StringList("F24FF731-96FC-4D0F-A2F5-5A3483682B1A");
    tags["COLLECTIONGROUPID"] = StringList("A81F04B3-2B1B-4B52-8700-6F8091DF3B41");
    tags["COLLECTIONID"] = StringList("3F2504E0-4F89-11D3-9A0C-0305E82C3301");
    tags["CONTENTID"] = StringList("6B29FC40-CA47-1067-B31D-00DD010662DA");
    tags["CONTENTDISTRIBUTOR"] = StringList("Universal Music Group");
    tags["PARENTALRATING"] = StringList("Explicit");
    tags["PERIOD"] = StringList("Baroque");
    tags["PROMOTIONURL"] = StringList("https://www.artistwebsite.com/store/album-special-edition");
    tags["TOOLNAME"] = StringList("Windows Media Encoder");
    tags["TOOLVERSION"] = StringList("9.00.00.2980");
    tags["PROVIDER"] = StringList("AMG");
    tags["UNIQUEFILEIDENTIFIER"] = StringList("AMGI_0000000000012345");
    tags["WMFSDKVERSION"] = StringList("12.0.19041.1");
    tags["WMFSDKNEEDED"] = StringList("9.0.0.4503");
    tags["DEVICECONFORMANCETEMPLATE"] = StringList("L1");
    tags["MEDIAFOUNDATIONVERSION"] = StringList("2.0");
    tags["ISVBR"] = StringList("1");
    tags["PEAKVALUE"] = StringList("32104");
    tags["AVERAGELEVEL"] = StringList("2450");

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
      CPPUNIT_ASSERT(properties.isEmpty());
      f.setProperties(tags);
      f.save();
    }
    {
      const ASF::File f(copy.fileName().c_str());
      PropertyMap properties = f.properties();
      if(tags != properties) {
        CPPUNIT_ASSERT_EQUAL(tags.toString(), properties.toString());
      }
      CPPUNIT_ASSERT(tags == properties);

      const ASF::Tag *asfTag = f.tag();
      CPPUNIT_ASSERT(asfTag);
      std::map<String, ASF::Attribute::AttributeTypes> expected = {
        {"ASIN", ASF::Attribute::UnicodeType},
        {"Acoustid/Fingerprint", ASF::Attribute::UnicodeType},
        {"Acoustid/Id", ASF::Attribute::UnicodeType},
        {"AverageLevel", ASF::Attribute::DWordType},
        {"DeviceConformanceTemplate", ASF::Attribute::UnicodeType},
        {"IsVBR", ASF::Attribute::BoolType},
        {"MediaFoundationVersion", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Album Artist Id", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Album Id", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Album Release Country", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Album Status", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Album Type", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Artist Id", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Release Group Id", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Release Track Id", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Track Id", ASF::Attribute::UnicodeType},
        {"MusicBrainz/Work Id", ASF::Attribute::UnicodeType},
        {"MusicIP/PUID", ASF::Attribute::UnicodeType},
        {"PeakValue", ASF::Attribute::DWordType},
        {"WM/ARTISTS", ASF::Attribute::UnicodeType},
        {"WM/AlbumArtist", ASF::Attribute::UnicodeType},
        {"WM/AlbumArtistSortOrder", ASF::Attribute::UnicodeType},
        {"WM/AlbumSortOrder", ASF::Attribute::UnicodeType},
        {"WM/AlbumTitle", ASF::Attribute::UnicodeType},
        {"WM/ArtistSortOrder", ASF::Attribute::UnicodeType},
        {"WM/AudioFileURL", ASF::Attribute::UnicodeType},
        {"WM/AuthorURL", ASF::Attribute::UnicodeType},
        {"WM/Barcode", ASF::Attribute::UnicodeType},
        {"WM/BeatsPerMinute", ASF::Attribute::UnicodeType},
        {"WM/CatalogNo", ASF::Attribute::UnicodeType},
        {"WM/Composer", ASF::Attribute::UnicodeType},
        {"WM/Conductor", ASF::Attribute::UnicodeType},
        {"WM/ContentDistributor", ASF::Attribute::UnicodeType},
        {"WM/ContentGroupDescription", ASF::Attribute::UnicodeType},
        {"WM/EncodedBy", ASF::Attribute::UnicodeType},
        {"WM/EncodingSettings", ASF::Attribute::UnicodeType},
        {"WM/EncodingTime", ASF::Attribute::QWordType},
        {"WM/Genre", ASF::Attribute::UnicodeType},
        {"WM/ISRC", ASF::Attribute::UnicodeType},
        {"WM/InitialKey", ASF::Attribute::UnicodeType},
        {"WM/Language", ASF::Attribute::UnicodeType},
        {"WM/Lyrics", ASF::Attribute::UnicodeType},
        {"WM/Media", ASF::Attribute::UnicodeType},
        {"WM/MediaClassPrimaryID", ASF::Attribute::GuidType},
        {"WM/MediaClassSecondaryID", ASF::Attribute::GuidType},
        {"WM/ModifiedBy", ASF::Attribute::UnicodeType},
        {"WM/Mood", ASF::Attribute::UnicodeType},
        {"WM/OriginalAlbumTitle", ASF::Attribute::UnicodeType},
        {"WM/OriginalArtist", ASF::Attribute::UnicodeType},
        {"WM/OriginalFilename", ASF::Attribute::UnicodeType},
        {"WM/OriginalLyricist", ASF::Attribute::UnicodeType},
        {"WM/OriginalReleaseYear", ASF::Attribute::UnicodeType},
        {"WM/ParentalRating", ASF::Attribute::UnicodeType},
        {"WM/PartOfSet", ASF::Attribute::UnicodeType},
        {"WM/Period", ASF::Attribute::UnicodeType},
        {"WM/Producer", ASF::Attribute::UnicodeType},
        {"WM/PromotionURL", ASF::Attribute::UnicodeType},
        {"WM/Provider", ASF::Attribute::UnicodeType},
        {"WM/Publisher", ASF::Attribute::UnicodeType},
        {"WM/Script", ASF::Attribute::UnicodeType},
        {"WM/SetSubTitle", ASF::Attribute::UnicodeType},
        {"WM/SubTitle", ASF::Attribute::UnicodeType},
        {"WM/TitleSortOrder", ASF::Attribute::UnicodeType},
        {"WM/ToolName", ASF::Attribute::UnicodeType},
        {"WM/ToolVersion", ASF::Attribute::UnicodeType},
        {"WM/TrackNumber", ASF::Attribute::UnicodeType},
        {"WM/UniqueFileIdentifier", ASF::Attribute::UnicodeType},
        {"WM/WMCollectionGroupID", ASF::Attribute::GuidType},
        {"WM/WMCollectionID", ASF::Attribute::GuidType},
        {"WM/WMContentID", ASF::Attribute::GuidType},
        {"WM/Writer", ASF::Attribute::UnicodeType},
        {"WM/Year", ASF::Attribute::UnicodeType},
        {"WMFSDKNeeded", ASF::Attribute::UnicodeType},
        {"WMFSDKVersion", ASF::Attribute::UnicodeType},
        {"replaygain_album_gain", ASF::Attribute::UnicodeType},
        {"replaygain_album_peak", ASF::Attribute::UnicodeType},
        {"replaygain_track_gain", ASF::Attribute::UnicodeType},
        {"replaygain_track_peak", ASF::Attribute::UnicodeType},
      };
      std::map<String, ASF::Attribute::AttributeTypes> actual;
      for(const auto &[name, attr] : asfTag->attributeListMap()) {
        if(!attr.isEmpty()) {
          actual[name] = attr[0].type();
        }
      }
      if(expected != actual) {
        auto mapToStrings = [](const std::map<String, ASF::Attribute::AttributeTypes> &map){
          StringList result;
          for(const auto &[name, attr] : map) {
            result.append(name + ":" + String::number(attr));
          }
          return result;
        };
        const StringList expectedStrs = mapToStrings(expected);
        const StringList actualStrs = mapToStrings(actual);
        CPPUNIT_ASSERT_EQUAL(expectedStrs, actualStrs);
      }
      CPPUNIT_ASSERT(expected == actual);
    }
  }

  void testPropertiesRealFile()
  {
    ASF::File f(TEST_FILE_PATH_C("real_example.wma"));

    PropertyMap tags = f.properties();

    CPPUNIT_ASSERT_EQUAL(StringList("Wake Up, Get Up, Get Out There"), tags["TITLE"]);
    CPPUNIT_ASSERT_EQUAL(StringList("Shoji Meguro"), tags["ARTIST"]);
    CPPUNIT_ASSERT_EQUAL(StringList("Persona 5: Sounds of Rebellion"), tags["ALBUM"]);
    CPPUNIT_ASSERT_EQUAL(StringList("-8.27 dB"), tags["REPLAYGAIN_TRACK_GAIN"]);
    CPPUNIT_ASSERT_EQUAL(StringList("1.000000"), tags["REPLAYGAIN_TRACK_PEAK"]);
    CPPUNIT_ASSERT_EQUAL(StringList("131452413620000000"), tags["ENCODINGTIME"]);
    CPPUNIT_ASSERT_EQUAL(StringList("32673"), tags["PEAKVALUE"]);
    CPPUNIT_ASSERT_EQUAL(StringList("8315"), tags["AVERAGELEVEL"]);
    CPPUNIT_ASSERT_EQUAL(StringList("0"), tags["ISVBR"]);
    CPPUNIT_ASSERT_EQUAL(StringList("12.0.15063.332"), tags["WMFSDKVERSION"]);
    CPPUNIT_ASSERT_EQUAL(StringList("0.0.0.0000"), tags["WMFSDKNEEDED"]);
    CPPUNIT_ASSERT_EQUAL(StringList("User Feedback"), tags["PROVIDER"]);
    CPPUNIT_ASSERT_EQUAL(StringList("L1"), tags["DEVICECONFORMANCETEMPLATE"]);
    CPPUNIT_ASSERT_EQUAL(StringList("2.112"), tags["MEDIAFOUNDATIONVERSION"]);
    CPPUNIT_ASSERT_EQUAL(StringList("1"), tags["TRACKNUMBER"]);
    CPPUNIT_ASSERT_EQUAL(StringList("Alternative"), tags["GENRE"]);
    CPPUNIT_ASSERT_EQUAL(StringList("Shoji Meguro"), tags["ALBUMARTIST"]);
    CPPUNIT_ASSERT_EQUAL(StringList("00000000-0000-0000-0000-000000000000"), tags["COLLECTIONGROUPID"]);
    CPPUNIT_ASSERT_EQUAL(StringList("00000000-0000-0000-0000-000000000000"), tags["COLLECTIONID"]);
    CPPUNIT_ASSERT_EQUAL(StringList("00000000-0000-0000-0000-000000000000"), tags["CONTENTID"]);
    CPPUNIT_ASSERT_EQUAL(StringList("BC7D60D1-23E3-E24B-86A1-48A42A28441E"), tags["MEDIACLASSPRIMARYID"]);
    CPPUNIT_ASSERT_EQUAL(StringList("00000000-0000-0000-0000-000000000000"), tags["MEDIACLASSSECONDARYID"]);
    CPPUNIT_ASSERT_EQUAL(StringList(";"), tags["UNIQUEFILEIDENTIFIER"]);
  }

  void testCaseInsensitiveAttributeNames()
  {
    ScopedFileCopy copy("silence-1", ".wma");
    {
      ASF::File f(copy.fileName().c_str());
      f.tag()->setAttribute("REPLAYGAIN_TRACK_GAIN", String("-6.00 dB"));
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      PropertyMap tags = f.properties();
      CPPUNIT_ASSERT_EQUAL(StringList("-6.00 dB"), tags["REPLAYGAIN_TRACK_GAIN"]);

      // Replacing the value must not leave a second, differently cased
      // attribute behind.
      tags["REPLAYGAIN_TRACK_GAIN"] = StringList("-7.00 dB");
      f.setProperties(tags);
      CPPUNIT_ASSERT(!f.tag()->contains("REPLAYGAIN_TRACK_GAIN"));
      CPPUNIT_ASSERT_EQUAL(String("-7.00 dB"),
        f.tag()->attribute("replaygain_track_gain").front().toString());
      tags = f.properties();
      CPPUNIT_ASSERT_EQUAL(StringList("-7.00 dB"), tags["REPLAYGAIN_TRACK_GAIN"]);
    }
  }

  void testRepeatedSave()
  {
    ScopedFileCopy copy("silence-1", ".wma");
    ASF::File f(copy.fileName().c_str());
    f.tag()->setTitle(longText(128 * 1024));
    f.save();
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(297578), f.length());
    f.tag()->setTitle(longText(16 * 1024));
    f.save();
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(68202), f.length());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestASF);
