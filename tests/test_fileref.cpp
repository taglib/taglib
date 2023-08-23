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

#include "aifffile.h"
#include "apefile.h"
#include "asffile.h"
#include "fileref.h"
#include "flacfile.h"
#include "mp4file.h"
#include "mpcfile.h"
#include "mpegfile.h"
#include "oggflacfile.h"
#include "opusfile.h"
#include "speexfile.h"
#include "tag.h"
#include "tbytevectorstream.h"
#include "tfilestream.h"
#include "trueaudiofile.h"
#include "utils.h"
#include "vorbisfile.h"
#include "wavfile.h"
#include "wavpackfile.h"
#include "xmfile.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

namespace {
  class DummyResolver : public FileRef::FileTypeResolver
  {
  public:
    File *createFile(FileName fileName, bool, AudioProperties::ReadStyle) const override
    {
      return new Ogg::Vorbis::File(fileName);
    }
  };

  class DummyStreamResolver : public FileRef::StreamTypeResolver
  {
  public:
    File *createFile(FileName, bool, AudioProperties::ReadStyle) const override
    {
      return nullptr;
    }

    File *createFileFromStream(IOStream *s, bool, AudioProperties::ReadStyle) const override
    {
      return new MP4::File(s);
    }
  };
} // namespace

template <typename T>
void fileRefSave(const string &filename, const string &ext)
{
  ScopedFileCopy copy(filename, ext);
  string newname = copy.fileName();

  {
    FileRef f(newname.c_str());
    ASSERT_TRUE(dynamic_cast<T *>(f.file()));
    ASSERT_FALSE(f.isNull());
    f.tag()->setArtist("test artist");
    f.tag()->setTitle("test title");
    f.tag()->setGenre("Test!");
    f.tag()->setAlbum("albummmm");
    f.tag()->setComment("a comment");
    f.tag()->setTrack(5);
    f.tag()->setYear(2020);
    f.save();
  }
  {
    FileRef f(newname.c_str());
    ASSERT_FALSE(f.isNull());
    ASSERT_EQ(f.tag()->artist(), String("test artist"));
    ASSERT_EQ(f.tag()->title(), String("test title"));
    ASSERT_EQ(f.tag()->genre(), String("Test!"));
    ASSERT_EQ(f.tag()->album(), String("albummmm"));
    ASSERT_EQ(f.tag()->comment(), String("a comment"));
    ASSERT_EQ(f.tag()->track(), static_cast<unsigned int>(5));
    ASSERT_EQ(f.tag()->year(), static_cast<unsigned int>(2020));
    f.tag()->setArtist("ttest artist");
    f.tag()->setTitle("ytest title");
    f.tag()->setGenre("uTest!");
    f.tag()->setAlbum("ialbummmm");
    f.tag()->setComment("another comment");
    f.tag()->setTrack(7);
    f.tag()->setYear(2080);
    f.save();
  }
  {
    FileRef f(newname.c_str());
    ASSERT_FALSE(f.isNull());
    ASSERT_EQ(f.tag()->artist(), String("ttest artist"));
    ASSERT_EQ(f.tag()->title(), String("ytest title"));
    ASSERT_EQ(f.tag()->genre(), String("uTest!"));
    ASSERT_EQ(f.tag()->album(), String("ialbummmm"));
    ASSERT_EQ(f.tag()->comment(), String("another comment"));
    ASSERT_EQ(f.tag()->track(), static_cast<unsigned int>(7));
    ASSERT_EQ(f.tag()->year(), static_cast<unsigned int>(2080));
  }

  {
    FileStream fs(newname.c_str());
    FileRef f(&fs);
    ASSERT_TRUE(dynamic_cast<T *>(f.file()));
    ASSERT_FALSE(f.isNull());
    ASSERT_EQ(f.tag()->artist(), String("ttest artist"));
    ASSERT_EQ(f.tag()->title(), String("ytest title"));
    ASSERT_EQ(f.tag()->genre(), String("uTest!"));
    ASSERT_EQ(f.tag()->album(), String("ialbummmm"));
    ASSERT_EQ(f.tag()->comment(), String("another comment"));
    ASSERT_EQ(f.tag()->track(), static_cast<unsigned int>(7));
    ASSERT_EQ(f.tag()->year(), static_cast<unsigned int>(2080));
    f.tag()->setArtist("test artist");
    f.tag()->setTitle("test title");
    f.tag()->setGenre("Test!");
    f.tag()->setAlbum("albummmm");
    f.tag()->setComment("a comment");
    f.tag()->setTrack(5);
    f.tag()->setYear(2020);
    f.save();
  }

  ByteVector fileContent;
  {
    FileStream fs(newname.c_str());
    FileRef f(&fs);
    ASSERT_TRUE(dynamic_cast<T *>(f.file()));
    ASSERT_FALSE(f.isNull());
    ASSERT_EQ(f.tag()->artist(), String("test artist"));
    ASSERT_EQ(f.tag()->title(), String("test title"));
    ASSERT_EQ(f.tag()->genre(), String("Test!"));
    ASSERT_EQ(f.tag()->album(), String("albummmm"));
    ASSERT_EQ(f.tag()->comment(), String("a comment"));
    ASSERT_EQ(f.tag()->track(), static_cast<unsigned int>(5));
    ASSERT_EQ(f.tag()->year(), static_cast<unsigned int>(2020));

    fs.seek(0);
    fileContent = fs.readBlock(fs.length());
  }

  {
    ByteVectorStream bs(fileContent);
    FileRef f(&bs);
    ASSERT_TRUE(dynamic_cast<T *>(f.file()));
    ASSERT_FALSE(f.isNull());
    ASSERT_EQ(f.tag()->artist(), String("test artist"));
    ASSERT_EQ(f.tag()->title(), String("test title"));
    ASSERT_EQ(f.tag()->genre(), String("Test!"));
    ASSERT_EQ(f.tag()->album(), String("albummmm"));
    ASSERT_EQ(f.tag()->comment(), String("a comment"));
    ASSERT_EQ(f.tag()->track(), static_cast<unsigned int>(5));
    ASSERT_EQ(f.tag()->year(), static_cast<unsigned int>(2020));
    f.tag()->setArtist("ttest artist");
    f.tag()->setTitle("ytest title");
    f.tag()->setGenre("uTest!");
    f.tag()->setAlbum("ialbummmm");
    f.tag()->setComment("another comment");
    f.tag()->setTrack(7);
    f.tag()->setYear(2080);
    f.save();

    fileContent = *bs.data();
  }
  {
    ByteVectorStream bs(fileContent);
    FileRef f(&bs);
    ASSERT_TRUE(dynamic_cast<T *>(f.file()));
    ASSERT_FALSE(f.isNull());
    ASSERT_EQ(f.tag()->artist(), String("ttest artist"));
    ASSERT_EQ(f.tag()->title(), String("ytest title"));
    ASSERT_EQ(f.tag()->genre(), String("uTest!"));
    ASSERT_EQ(f.tag()->album(), String("ialbummmm"));
    ASSERT_EQ(f.tag()->comment(), String("another comment"));
    ASSERT_EQ(f.tag()->track(), static_cast<unsigned int>(7));
    ASSERT_EQ(f.tag()->year(), static_cast<unsigned int>(2080));
  }
}

TEST(FileRef, testMusepack)
{
  fileRefSave<MPC::File>("click", ".mpc");
}

TEST(FileRef, testASF)
{
  fileRefSave<ASF::File>("silence-1", ".wma");
}

TEST(FileRef, testVorbis)
{
  fileRefSave<Ogg::Vorbis::File>("empty", ".ogg");
}

TEST(FileRef, testSpeex)
{
  fileRefSave<Ogg::Speex::File>("empty", ".spx");
}

TEST(FileRef, testFLAC)
{
  fileRefSave<FLAC::File>("no-tags", ".flac");
}

TEST(FileRef, testMP3)
{
  fileRefSave<MPEG::File>("xing", ".mp3");
}

TEST(FileRef, testTrueAudio)
{
  fileRefSave<TrueAudio::File>("empty", ".tta");
}

TEST(FileRef, testMP4_1)
{
  fileRefSave<MP4::File>("has-tags", ".m4a");
}

TEST(FileRef, testMP4_2)
{
  fileRefSave<MP4::File>("no-tags", ".m4a");
}

TEST(FileRef, testMP4_3)
{
  fileRefSave<MP4::File>("no-tags", ".3g2");
}

TEST(FileRef, testMP4_4)
{
  fileRefSave<MP4::File>("blank_video", ".m4v");
}

TEST(FileRef, testWav)
{
  fileRefSave<RIFF::WAV::File>("empty", ".wav");
}

TEST(FileRef, testOGA_FLAC)
{
  fileRefSave<Ogg::FLAC::File>("empty_flac", ".oga");
}

TEST(FileRef, testOGA_Vorbis)
{
  fileRefSave<Ogg::Vorbis::File>("empty_vorbis", ".oga");
}

TEST(FileRef, testAPE)
{
  fileRefSave<APE::File>("mac-399", ".ape");
}

TEST(FileRef, testAIFF_1)
{
  fileRefSave<RIFF::AIFF::File>("empty", ".aiff");
}

TEST(FileRef, testAIFF_2)
{
  fileRefSave<RIFF::AIFF::File>("alaw", ".aifc");
}

TEST(FileRef, testWavPack)
{
  fileRefSave<WavPack::File>("click", ".wv");
}

TEST(FileRef, testOpus)
{
  fileRefSave<Ogg::Opus::File>("correctness_gain_silent_output", ".opus");
}

TEST(FileRef, testUnsupported)
{
  FileRef f1(TEST_FILE_PATH_C("no-extension"));
  ASSERT_TRUE(f1.isNull());

  FileRef f2(TEST_FILE_PATH_C("unsupported-extension.xx"));
  ASSERT_TRUE(f2.isNull());
}

TEST(FileRef, testCreate)
{
  // This is deprecated. But worth it to test.

  File *f = FileRef::create(TEST_FILE_PATH_C("empty_vorbis.oga"));
  ASSERT_TRUE(dynamic_cast<Ogg::Vorbis::File *>(f));
  delete f;

  f = FileRef::create(TEST_FILE_PATH_C("xing.mp3"));
  ASSERT_TRUE(dynamic_cast<MPEG::File *>(f));
  delete f;

  f = FileRef::create(TEST_FILE_PATH_C("test.xm"));
  ASSERT_TRUE(dynamic_cast<XM::File *>(f));
  delete f;
}

TEST(FileRef, testAudioProperties)
{
  FileRef f(TEST_FILE_PATH_C("xing.mp3"));
  const AudioProperties *audioProperties = f.audioProperties();
  ASSERT_EQ(2, audioProperties->lengthInSeconds());
  ASSERT_EQ(2064, audioProperties->lengthInMilliseconds());
}

TEST(FileRef, testDefaultFileExtensions)
{
  const StringList extensions = FileRef::defaultFileExtensions();
  ASSERT_TRUE(extensions.contains("mpc"));
  ASSERT_TRUE(extensions.contains("wma"));
  ASSERT_TRUE(extensions.contains("ogg"));
  ASSERT_TRUE(extensions.contains("spx"));
  ASSERT_TRUE(extensions.contains("flac"));
  ASSERT_TRUE(extensions.contains("mp3"));
  ASSERT_TRUE(extensions.contains("tta"));
  ASSERT_TRUE(extensions.contains("m4a"));
  ASSERT_TRUE(extensions.contains("3g2"));
  ASSERT_TRUE(extensions.contains("m4v"));
  ASSERT_TRUE(extensions.contains("wav"));
  ASSERT_TRUE(extensions.contains("oga"));
  ASSERT_TRUE(extensions.contains("ape"));
  ASSERT_TRUE(extensions.contains("aiff"));
  ASSERT_TRUE(extensions.contains("aifc"));
  ASSERT_TRUE(extensions.contains("wv"));
  ASSERT_TRUE(extensions.contains("opus"));
  ASSERT_TRUE(extensions.contains("xm"));
}

TEST(FileRef, testFileResolver)
{
  {
    FileRef f(TEST_FILE_PATH_C("xing.mp3"));
    ASSERT_NE(nullptr, dynamic_cast<MPEG::File *>(f.file()));
  }

  DummyResolver resolver;
  FileRef::addFileTypeResolver(&resolver);

  {
    FileRef f(TEST_FILE_PATH_C("xing.mp3"));
    ASSERT_NE(nullptr, dynamic_cast<Ogg::Vorbis::File *>(f.file()));
  }

  DummyStreamResolver streamResolver;
  FileRef::addFileTypeResolver(&streamResolver);

  {
    FileStream s(TEST_FILE_PATH_C("xing.mp3"));
    FileRef f(&s);
    ASSERT_NE(nullptr, dynamic_cast<MP4::File *>(f.file()));
  }
}
