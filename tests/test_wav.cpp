/***************************************************************************
    copyright           : (C) 2010 by Lukas Lalinsky
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

#include "id3v2tag.h"
#include "infotag.h"
#include "tbytevectorlist.h"
#include "tbytevectorstream.h"
#include "tfilestream.h"
#include "tpropertymap.h"
#include "wavfile.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestWAV : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestWAV);
  CPPUNIT_TEST(testPCMProperties);
  CPPUNIT_TEST(testALAWProperties);
  CPPUNIT_TEST(testFloatProperties);
  CPPUNIT_TEST(testFloatWithoutFactChunkProperties);
  CPPUNIT_TEST(testZeroSizeDataChunk);
  CPPUNIT_TEST(testID3v2Tag);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testInfoTag);
  CPPUNIT_TEST(testStripTags);
  CPPUNIT_TEST(testDuplicateTags);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST(testFileWithGarbageAppended);
  CPPUNIT_TEST(testStripAndProperties);
  CPPUNIT_TEST(testPCMWithFactChunk);
  CPPUNIT_TEST(testWaveFormatExtensible);
  CPPUNIT_TEST(testInvalidChunk);
  CPPUNIT_TEST(testRIFFInfoProperties);
  CPPUNIT_TEST_SUITE_END();

public:

  void testPCMProperties()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("empty.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3675, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(32, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(1000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(3675U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->format());
  }

  void testALAWProperties()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("alaw.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3550, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(128, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(8000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(8, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(28400U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(6, f.audioProperties()->format());
  }

  void testFloatProperties()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("float64.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(97, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(5645, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(4281U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->format());
  }

  void testFloatWithoutFactChunkProperties()
  {
    ByteVector wavData = PlainFile(TEST_FILE_PATH_C("float64.wav")).readAll();
    CPPUNIT_ASSERT_EQUAL(ByteVector("fact"), wavData.mid(36, 4));
    // Remove the fact chunk by renaming it to fakt
    wavData[38] = 'k';
    ByteVectorStream wavStream(wavData);
    RIFF::WAV::File f(&wavStream);
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(97, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(5645, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(4281U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->format());
  }

  void testZeroSizeDataChunk()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("zero-size-chunk.wav"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testID3v2Tag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());

      f.ID3v2Tag()->setTitle(L"Title");
      f.ID3v2Tag()->setArtist(L"Artist");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L"Title"),  f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L"Artist"), f.ID3v2Tag()->artist());

      f.ID3v2Tag()->setTitle(L"");
      f.ID3v2Tag()->setArtist(L"");
      f.save();
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.ID3v2Tag()->artist());
    }
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("empty", ".wav");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      RIFF::WAV::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(RIFF::WAV::File::AllTags, File::StripOthers, ID3v2::v3);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      RIFF::WAV::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testInfoTag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasInfoTag());

      f.InfoTag()->setTitle(L"Title");
      f.InfoTag()->setArtist(L"Artist");
      f.save();
      CPPUNIT_ASSERT(f.hasInfoTag());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"Title"),  f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L"Artist"), f.InfoTag()->artist());

      f.InfoTag()->setTitle(L"");
      f.InfoTag()->setArtist(L"");
      f.save();
      CPPUNIT_ASSERT(!f.hasInfoTag());
    }

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.InfoTag()->artist());
    }
  }

  void testStripTags()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      f.ID3v2Tag()->setTitle("test title");
      f.InfoTag()->setTitle("test title");
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasInfoTag());
      f.save(RIFF::WAV::File::ID3v2, File::StripOthers);
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasInfoTag());
      f.ID3v2Tag()->setTitle("test title");
      f.InfoTag()->setTitle("test title");
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasInfoTag());
      f.save(RIFF::WAV::File::Info, File::StripOthers);
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasInfoTag());
    }
  }

  void testDuplicateTags()
  {
    ScopedFileCopy copy("duplicate_tags", ".wav");

    RIFF::WAV::File f(copy.fileName().c_str());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(17052), f.length());

    // duplicate_tags.wav has duplicate ID3v2/INFO tags.
    // title() returns "Title2" if can't skip the second tag.

    CPPUNIT_ASSERT(f.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.ID3v2Tag()->title());

    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.InfoTag()->title());

    f.save();
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(15898), f.length());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(-1), f.find("Title2"));
  }

  void testFuzzedFile1()
  {
    RIFF::WAV::File f1(TEST_FILE_PATH_C("infloop.wav"));
    CPPUNIT_ASSERT(f1.isValid());
    // The file has problems:
    // Chunk 'ISTt' has invalid size (larger than the file size).
    // Its properties can nevertheless be read.
    RIFF::WAV::Properties* properties = f1.audioProperties();
    CPPUNIT_ASSERT_EQUAL(1, properties->channels());
    CPPUNIT_ASSERT_EQUAL(88, properties->bitrate());
    CPPUNIT_ASSERT_EQUAL(8, properties->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(11025, properties->sampleRate());
    CPPUNIT_ASSERT(!f1.hasInfoTag());
    CPPUNIT_ASSERT(!f1.hasID3v2Tag());
  }

  void testFuzzedFile2()
  {
    RIFF::WAV::File f2(TEST_FILE_PATH_C("segfault.wav"));
    CPPUNIT_ASSERT(f2.isValid());
  }

  void testFileWithGarbageAppended()
  {
    ScopedFileCopy copy("empty", ".wav");
    ByteVector contentsBeforeModification;
    {
      FileStream stream(copy.fileName().c_str());
      stream.seek(0, IOStream::End);
      constexpr char garbage[] = "12345678";
      stream.writeBlock(ByteVector(garbage, sizeof(garbage) - 1));
      stream.seek(0);
      contentsBeforeModification = stream.readBlock(stream.length());
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      f.ID3v2Tag()->setTitle("ID3v2 Title");
      f.InfoTag()->setTitle("INFO Title");
      CPPUNIT_ASSERT(f.save());
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      f.strip();
    }
    {
      FileStream stream(copy.fileName().c_str());
      ByteVector contentsAfterModification = stream.readBlock(stream.length());
      CPPUNIT_ASSERT_EQUAL(contentsBeforeModification, contentsAfterModification);
    }
  }

  void testStripAndProperties()
  {
    ScopedFileCopy copy("empty", ".wav");

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      f.ID3v2Tag()->setTitle("ID3v2");
      f.InfoTag()->setTitle("INFO");
      f.save();
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL(String("ID3v2"), f.properties()["TITLE"].front());
      f.strip(RIFF::WAV::File::ID3v2);
      CPPUNIT_ASSERT_EQUAL(String("INFO"), f.properties()["TITLE"].front());
      f.strip(RIFF::WAV::File::Info);
      CPPUNIT_ASSERT(f.properties().isEmpty());
    }
  }

  void testPCMWithFactChunk()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("pcm_with_fact_chunk.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3675, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(32, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(1000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(3675U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->format());
  }

  void testWaveFormatExtensible()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("uint8we.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(2937, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(128, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(8000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(8, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(23493U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->format());
  }

  void testInvalidChunk()
  {
    ScopedFileCopy copy("invalid-chunk", ".wav");

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      f.ID3v2Tag()->setTitle("Title");
      f.save();
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
  }

  void testRIFFInfoProperties()
  {
    PropertyMap tags;
    tags["ALBUM"] = StringList("Album");
    tags["ARRANGER"] = StringList("Arranger");
    tags["ARTIST"] = StringList("Artist");
    tags["ARTISTWEBPAGE"] = StringList("Artist Webpage");
    tags["BPM"] = StringList("123");
    tags["COMMENT"] = StringList("Comment");
    tags["COMPOSER"] = StringList("Composer");
    tags["COPYRIGHT"] = StringList("2023 Copyright");
    tags["DATE"] = StringList("2023");
    tags["DISCSUBTITLE"] = StringList("Disc Subtitle");
    tags["ENCODEDBY"] = StringList("Encoded by");
    tags["ENCODING"] = StringList("Encoding");
    tags["ENCODINGTIME"] = StringList("2023-11-25 15:42:39");
    tags["GENRE"] = StringList("Genre");
    tags["ISRC"] = StringList("UKAAA0500001");
    tags["LABEL"] = StringList("Label");
    tags["LANGUAGE"] = StringList("eng");
    tags["LYRICIST"] = StringList("Lyricist");
    tags["MEDIA"] = StringList("Media");
    tags["PERFORMER"] = StringList("Performer");
    tags["RELEASECOUNTRY"] = StringList("Release Country");
    tags["REMIXER"] = StringList("Remixer");
    tags["TITLE"] = StringList("Title");
    tags["TRACKNUMBER"] = StringList("2/4");

    ScopedFileCopy copy("empty", ".wav");
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      RIFF::Info::Tag *infoTag = f.InfoTag();
      CPPUNIT_ASSERT(infoTag->isEmpty());
      PropertyMap properties = infoTag->properties();
      CPPUNIT_ASSERT(properties.isEmpty());
      infoTag->setProperties(tags);
      f.save();
    }
    {
      const RIFF::WAV::File f(copy.fileName().c_str());
      RIFF::Info::Tag *infoTag = f.InfoTag();
      CPPUNIT_ASSERT(!infoTag->isEmpty());
      PropertyMap properties = infoTag->properties();
      if (tags != properties) {
        CPPUNIT_ASSERT_EQUAL(tags.toString(), properties.toString());
      }
      CPPUNIT_ASSERT(tags == properties);

      const RIFF::Info::FieldListMap expectedFields = {
        {"IPRD", "Album"},
        {"IENG", "Arranger"},
        {"IART", "Artist"},
        {"IBSU", "Artist Webpage"},
        {"IBPM", "123"},
        {"ICMT", "Comment"},
        {"IMUS", "Composer"},
        {"ICOP", "2023 Copyright"},
        {"ICRD", "2023"},
        {"PRT1", "Disc Subtitle"},
        {"ITCH", "Encoded by"},
        {"ISFT", "Encoding"},
        {"IDIT", "2023-11-25 15:42:39"},
        {"IGNR", "Genre"},
        {"ISRC", "UKAAA0500001"},
        {"IPUB", "Label"},
        {"ILNG", "eng"},
        {"IWRI", "Lyricist"},
        {"IMED", "Media"},
        {"ISTR", "Performer"},
        {"ICNT", "Release Country"},
        {"IEDT", "Remixer"},
        {"INAM", "Title"},
        {"IPRT", "2/4"}
      };
      CPPUNIT_ASSERT(expectedFields == infoTag->fieldListMap());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
