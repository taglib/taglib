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

#include "id3v2tag.h"
#include "infotag.h"
#include "plainfile.h"
#include "tbytevectorlist.h"
#include "tbytevectorstream.h"
#include "tfilestream.h"
#include "tpropertymap.h"
#include "utils.h"
#include "wavfile.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(Wav, testPCMProperties)
{
  RIFF::WAV::File f(TEST_FILE_PATH_C("empty.wav"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3675, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(32, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(1000, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(3675U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1, f.audioProperties()->format());
}

TEST(Wav, testALAWProperties)
{
  RIFF::WAV::File f(TEST_FILE_PATH_C("alaw.wav"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3550, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(128, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(8000, f.audioProperties()->sampleRate());
  ASSERT_EQ(8, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(28400U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(6, f.audioProperties()->format());
}

TEST(Wav, testFloatProperties)
{
  RIFF::WAV::File f(TEST_FILE_PATH_C("float64.wav"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(97, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(5645, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(64, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(4281U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(3, f.audioProperties()->format());
}

TEST(Wav, testFloatWithoutFactChunkProperties)
{
  ByteVector wavData = PlainFile(TEST_FILE_PATH_C("float64.wav")).readAll();
  ASSERT_EQ(ByteVector("fact"), wavData.mid(36, 4));
  // Remove the fact chunk by renaming it to fakt
  wavData[38] = 'k';
  ByteVectorStream wavStream(wavData);
  RIFF::WAV::File f(&wavStream);
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(0, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(97, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(5645, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(64, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(4281U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(3, f.audioProperties()->format());
}

TEST(Wav, testZeroSizeDataChunk)
{
  RIFF::WAV::File f(TEST_FILE_PATH_C("zero-size-chunk.wav"));
  ASSERT_TRUE(f.isValid());
}

TEST(Wav, testID3v2Tag)
{
  ScopedFileCopy copy("empty", ".wav");
  string filename = copy.fileName();

  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_FALSE(f.hasID3v2Tag());

    f.ID3v2Tag()->setTitle(L"Title");
    f.ID3v2Tag()->setArtist(L"Artist");
    f.save();
    ASSERT_TRUE(f.hasID3v2Tag());
  }
  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(String(L"Title"), f.ID3v2Tag()->title());
    ASSERT_EQ(String(L"Artist"), f.ID3v2Tag()->artist());

    f.ID3v2Tag()->setTitle(L"");
    f.ID3v2Tag()->setArtist(L"");
    f.save();
    ASSERT_FALSE(f.hasID3v2Tag());
  }
  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_EQ(String(L""), f.ID3v2Tag()->title());
    ASSERT_EQ(String(L""), f.ID3v2Tag()->artist());
  }
}

TEST(Wav, testSaveID3v23)
{
  ScopedFileCopy copy("empty", ".wav");
  string newname = copy.fileName();

  String xxx     = ByteVector(254, 'X');
  {
    RIFF::WAV::File f(newname.c_str());
    ASSERT_FALSE(f.hasID3v2Tag());

    f.tag()->setTitle(xxx);
    f.tag()->setArtist("Artist A");
    f.save(RIFF::WAV::File::AllTags, File::StripOthers, ID3v2::v3);
    ASSERT_TRUE(f.hasID3v2Tag());
  }
  {
    RIFF::WAV::File f2(newname.c_str());
    ASSERT_EQ(static_cast<unsigned int>(3), f2.ID3v2Tag()->header()->majorVersion());
    ASSERT_EQ(String("Artist A"), f2.tag()->artist());
    ASSERT_EQ(xxx, f2.tag()->title());
  }
}

TEST(Wav, testInfoTag)
{
  ScopedFileCopy copy("empty", ".wav");
  string filename = copy.fileName();

  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_FALSE(f.hasInfoTag());

    f.InfoTag()->setTitle(L"Title");
    f.InfoTag()->setArtist(L"Artist");
    f.save();
    ASSERT_TRUE(f.hasInfoTag());
  }
  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_TRUE(f.hasInfoTag());
    ASSERT_EQ(String(L"Title"), f.InfoTag()->title());
    ASSERT_EQ(String(L"Artist"), f.InfoTag()->artist());

    f.InfoTag()->setTitle(L"");
    f.InfoTag()->setArtist(L"");
    f.save();
    ASSERT_FALSE(f.hasInfoTag());
  }

  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_TRUE(f.isValid());
    ASSERT_FALSE(f.hasInfoTag());
    ASSERT_EQ(String(L""), f.InfoTag()->title());
    ASSERT_EQ(String(L""), f.InfoTag()->artist());
  }
}

TEST(Wav, testStripTags)
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
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_TRUE(f.hasInfoTag());
    f.save(RIFF::WAV::File::ID3v2, File::StripOthers);
  }
  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_FALSE(f.hasInfoTag());
    f.ID3v2Tag()->setTitle("test title");
    f.InfoTag()->setTitle("test title");
    f.save();
  }
  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_TRUE(f.hasInfoTag());
    f.save(RIFF::WAV::File::Info, File::StripOthers);
  }
  {
    RIFF::WAV::File f(filename.c_str());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_TRUE(f.hasInfoTag());
  }
}

TEST(Wav, testDuplicateTags)
{
  ScopedFileCopy copy("duplicate_tags", ".wav");

  RIFF::WAV::File f(copy.fileName().c_str());
  ASSERT_EQ(static_cast<offset_t>(17052), f.length());

  // duplicate_tags.wav has duplicate ID3v2/INFO tags.
  // title() returns "Title2" if can't skip the second tag.

  ASSERT_TRUE(f.hasID3v2Tag());
  ASSERT_EQ(String("Title1"), f.ID3v2Tag()->title());

  ASSERT_TRUE(f.hasInfoTag());
  ASSERT_EQ(String("Title1"), f.InfoTag()->title());

  f.save();
  ASSERT_EQ(static_cast<offset_t>(15898), f.length());
  ASSERT_EQ(static_cast<offset_t>(-1), f.find("Title2"));
}

TEST(Wav, testFuzzedFile1)
{
  RIFF::WAV::File f1(TEST_FILE_PATH_C("infloop.wav"));
  ASSERT_TRUE(f1.isValid());
  // The file has problems:
  // Chunk 'ISTt' has invalid size (larger than the file size).
  // Its properties can nevertheless be read.
  RIFF::WAV::Properties *properties = f1.audioProperties();
  ASSERT_EQ(1, properties->channels());
  ASSERT_EQ(88, properties->bitrate());
  ASSERT_EQ(8, properties->bitsPerSample());
  ASSERT_EQ(11025, properties->sampleRate());
  ASSERT_FALSE(f1.hasInfoTag());
  ASSERT_FALSE(f1.hasID3v2Tag());
}

TEST(Wav, testFuzzedFile2)
{
  RIFF::WAV::File f2(TEST_FILE_PATH_C("segfault.wav"));
  ASSERT_TRUE(f2.isValid());
}

TEST(Wav, testFileWithGarbageAppended)
{
  ScopedFileCopy copy("empty", ".wav");
  ByteVector contentsBeforeModification;
  {
    FileStream stream(copy.fileName().c_str());
    stream.seek(0, IOStream::End);
    const char garbage[] = "12345678";
    stream.writeBlock(ByteVector(garbage, sizeof(garbage) - 1));
    stream.seek(0);
    contentsBeforeModification = stream.readBlock(stream.length());
  }
  {
    RIFF::WAV::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    f.ID3v2Tag()->setTitle("ID3v2 Title");
    f.InfoTag()->setTitle("INFO Title");
    ASSERT_TRUE(f.save());
  }
  {
    RIFF::WAV::File f(copy.fileName().c_str());
    f.strip();
  }
  {
    FileStream stream(copy.fileName().c_str());
    ByteVector contentsAfterModification = stream.readBlock(stream.length());
    ASSERT_EQ(contentsBeforeModification, contentsAfterModification);
  }
}

TEST(Wav, testStripAndProperties)
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
    ASSERT_EQ(String("ID3v2"), f.properties()["TITLE"].front());
    f.strip(RIFF::WAV::File::ID3v2);
    ASSERT_EQ(String("INFO"), f.properties()["TITLE"].front());
    f.strip(RIFF::WAV::File::Info);
    ASSERT_TRUE(f.properties().isEmpty());
  }
}

TEST(Wav, testPCMWithFactChunk)
{
  RIFF::WAV::File f(TEST_FILE_PATH_C("pcm_with_fact_chunk.wav"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3675, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(32, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(1000, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(3675U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1, f.audioProperties()->format());
}

TEST(Wav, testWaveFormatExtensible)
{
  RIFF::WAV::File f(TEST_FILE_PATH_C("uint8we.wav"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(2, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(2937, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(128, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(8000, f.audioProperties()->sampleRate());
  ASSERT_EQ(8, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(23493U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(1, f.audioProperties()->format());
}
