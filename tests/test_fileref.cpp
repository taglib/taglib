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

#include "tfilestream.h"
#include "tbytevectorstream.h"
#include "tag.h"
#include "fileref.h"
#include "oggflacfile.h"
#include "vorbisfile.h"
#include "mpegfile.h"
#include "mpcfile.h"
#include "asffile.h"
#include "speexfile.h"
#include "flacfile.h"
#include "trueaudiofile.h"
#include "mp4file.h"
#include "wavfile.h"
#include "apefile.h"
#include "aifffile.h"
#include "wavpackfile.h"
#include "opusfile.h"
#include "xmfile.h"
#include "dsffile.h"
#include "dsdifffile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

namespace
{
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

class TestFileRef : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFileRef);
  CPPUNIT_TEST(testASF);
  CPPUNIT_TEST(testMusepack);
  CPPUNIT_TEST(testVorbis);
  CPPUNIT_TEST(testSpeex);
  CPPUNIT_TEST(testFLAC);
  CPPUNIT_TEST(testMP3);
  CPPUNIT_TEST(testOGA_FLAC);
  CPPUNIT_TEST(testOGA_Vorbis);
  CPPUNIT_TEST(testMP4_1);
  CPPUNIT_TEST(testMP4_2);
  CPPUNIT_TEST(testMP4_3);
  CPPUNIT_TEST(testMP4_4);
  CPPUNIT_TEST(testTrueAudio);
  CPPUNIT_TEST(testAPE);
  CPPUNIT_TEST(testWav);
  CPPUNIT_TEST(testAIFF_1);
  CPPUNIT_TEST(testAIFF_2);
  CPPUNIT_TEST(testWavPack);
  CPPUNIT_TEST(testOpus);
  CPPUNIT_TEST(testDSF);
  CPPUNIT_TEST(testDSDIFF);
  CPPUNIT_TEST(testUnsupported);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST(testDefaultFileExtensions);
  CPPUNIT_TEST(testFileResolver);
  CPPUNIT_TEST_SUITE_END();

public:

  template <typename T>
  void fileRefSave(const string &filename, const string &ext)
  {
    ScopedFileCopy copy(filename, ext);
    string newname = copy.fileName();

    {
      FileRef f(newname.c_str());
      CPPUNIT_ASSERT(dynamic_cast<T*>(f.file()));
      CPPUNIT_ASSERT(!f.isNull());
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
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("test artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("test title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("Test!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("albummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->comment(), String("a comment"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), static_cast<unsigned int>(5));
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), static_cast<unsigned int>(2020));
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
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("ttest artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("ytest title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("uTest!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("ialbummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->comment(), String("another comment"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), static_cast<unsigned int>(7));
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), static_cast<unsigned int>(2080));
    }

    {
      FileStream fs(newname.c_str());
      FileRef f(&fs);
      CPPUNIT_ASSERT(dynamic_cast<T*>(f.file()));
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("ttest artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("ytest title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("uTest!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("ialbummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->comment(), String("another comment"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), static_cast<unsigned int>(7));
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), static_cast<unsigned int>(2080));
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
      CPPUNIT_ASSERT(dynamic_cast<T*>(f.file()));
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("test artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("test title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("Test!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("albummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->comment(), String("a comment"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), static_cast<unsigned int>(5));
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), static_cast<unsigned int>(2020));

      fs.seek(0);
      fileContent = fs.readBlock(fs.length());
    }

    {
      ByteVectorStream bs(fileContent);
      FileRef f(&bs);
      CPPUNIT_ASSERT(dynamic_cast<T*>(f.file()));
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("test artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("test title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("Test!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("albummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->comment(), String("a comment"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), static_cast<unsigned int>(5));
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), static_cast<unsigned int>(2020));
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
      CPPUNIT_ASSERT(dynamic_cast<T*>(f.file()));
      CPPUNIT_ASSERT(!f.isNull());
      CPPUNIT_ASSERT_EQUAL(f.tag()->artist(), String("ttest artist"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->title(), String("ytest title"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->genre(), String("uTest!"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->album(), String("ialbummmm"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->comment(), String("another comment"));
      CPPUNIT_ASSERT_EQUAL(f.tag()->track(), static_cast<unsigned int>(7));
      CPPUNIT_ASSERT_EQUAL(f.tag()->year(), static_cast<unsigned int>(2080));
    }
  }

  void testMusepack()
  {
    fileRefSave<MPC::File>("click", ".mpc");
  }

  void testASF()
  {
    fileRefSave<ASF::File>("silence-1", ".wma");
  }

  void testVorbis()
  {
    fileRefSave<Ogg::Vorbis::File>("empty", ".ogg");
  }

  void testSpeex()
  {
    fileRefSave<Ogg::Speex::File>("empty", ".spx");
  }

  void testFLAC()
  {
    fileRefSave<FLAC::File>("no-tags", ".flac");
  }

  void testMP3()
  {
    fileRefSave<MPEG::File>("xing", ".mp3");
  }

  void testTrueAudio()
  {
    fileRefSave<TrueAudio::File>("empty", ".tta");
  }

  void testMP4_1()
  {
    fileRefSave<MP4::File>("has-tags", ".m4a");
  }

  void testMP4_2()
  {
    fileRefSave<MP4::File>("no-tags", ".m4a");
  }

  void testMP4_3()
  {
    fileRefSave<MP4::File>("no-tags", ".3g2");
  }

  void testMP4_4()
  {
    fileRefSave<MP4::File>("blank_video", ".m4v");
  }

  void testWav()
  {
    fileRefSave<RIFF::WAV::File>("empty", ".wav");
  }

  void testOGA_FLAC()
  {
    fileRefSave<Ogg::FLAC::File>("empty_flac", ".oga");
  }

  void testOGA_Vorbis()
  {
    fileRefSave<Ogg::Vorbis::File>("empty_vorbis", ".oga");
  }

  void testAPE()
  {
    fileRefSave<APE::File>("mac-399", ".ape");
  }

  void testAIFF_1()
  {
    fileRefSave<RIFF::AIFF::File>("empty", ".aiff");
  }

  void testAIFF_2()
  {
    fileRefSave<RIFF::AIFF::File>("alaw", ".aifc");
  }

  void testWavPack()
  {
    fileRefSave<WavPack::File>("click", ".wv");
  }

  void testOpus()
  {
    fileRefSave<Ogg::Opus::File>("correctness_gain_silent_output", ".opus");
  }

  void testDSF()
  {
    fileRefSave<DSF::File>("empty10ms",".dsf");
  }

  void testDSDIFF()
  {
    fileRefSave<DSDIFF::File>("empty10ms",".dff");
  }

  void testUnsupported()
  {
    FileRef f1(TEST_FILE_PATH_C("no-extension"));
    CPPUNIT_ASSERT(f1.isNull());

    FileRef f2(TEST_FILE_PATH_C("unsupported-extension.xx"));
    CPPUNIT_ASSERT(f2.isNull());
  }

  void testAudioProperties()
  {
    FileRef f(TEST_FILE_PATH_C("xing.mp3"));
    const AudioProperties *audioProperties = f.audioProperties();
    CPPUNIT_ASSERT_EQUAL(2, audioProperties->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(2064, audioProperties->lengthInMilliseconds());
  }

  void testDefaultFileExtensions()
  {
    const StringList extensions = FileRef::defaultFileExtensions();
    CPPUNIT_ASSERT(extensions.contains("mpc"));
    CPPUNIT_ASSERT(extensions.contains("wma"));
    CPPUNIT_ASSERT(extensions.contains("ogg"));
    CPPUNIT_ASSERT(extensions.contains("spx"));
    CPPUNIT_ASSERT(extensions.contains("flac"));
    CPPUNIT_ASSERT(extensions.contains("mp3"));
    CPPUNIT_ASSERT(extensions.contains("tta"));
    CPPUNIT_ASSERT(extensions.contains("m4a"));
    CPPUNIT_ASSERT(extensions.contains("3g2"));
    CPPUNIT_ASSERT(extensions.contains("m4v"));
    CPPUNIT_ASSERT(extensions.contains("wav"));
    CPPUNIT_ASSERT(extensions.contains("oga"));
    CPPUNIT_ASSERT(extensions.contains("ape"));
    CPPUNIT_ASSERT(extensions.contains("aiff"));
    CPPUNIT_ASSERT(extensions.contains("aifc"));
    CPPUNIT_ASSERT(extensions.contains("wv"));
    CPPUNIT_ASSERT(extensions.contains("opus"));
    CPPUNIT_ASSERT(extensions.contains("xm"));
    CPPUNIT_ASSERT(extensions.contains("dsf"));
    CPPUNIT_ASSERT(extensions.contains("dff"));
    CPPUNIT_ASSERT(extensions.contains("dsdiff"));
    CPPUNIT_ASSERT(extensions.contains("shn"));
  }

  void testFileResolver()
  {
    {
      FileRef f(TEST_FILE_PATH_C("xing.mp3"));
      CPPUNIT_ASSERT(dynamic_cast<MPEG::File *>(f.file()) != nullptr);
    }

    DummyResolver resolver;
    FileRef::addFileTypeResolver(&resolver);

    {
      FileRef f(TEST_FILE_PATH_C("xing.mp3"));
      CPPUNIT_ASSERT(dynamic_cast<Ogg::Vorbis::File *>(f.file()) != nullptr);
    }

    DummyStreamResolver streamResolver;
    FileRef::addFileTypeResolver(&streamResolver);

    {
      FileStream s(TEST_FILE_PATH_C("xing.mp3"));
      FileRef f(&s);
      CPPUNIT_ASSERT(dynamic_cast<MP4::File *>(f.file()) != nullptr);
    }

    FileRef::clearFileTypeResolvers();
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFileRef);
