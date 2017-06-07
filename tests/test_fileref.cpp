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

#include <catch/catch.hpp>
#include <fileref.h>
#include <tfilestream.h>
#include <tbytevectorstream.h>
#include <aifffile.h>
#include <apefile.h>
#include <asffile.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <oggflacfile.h>
#include <speexfile.h>
#include <trueaudiofile.h>
#include <wavfile.h>
#include <vorbisfile.h>
#include "utils.h"

using namespace TagLib;

namespace
{
  class DummyResolver : public FileRef::FileTypeResolver
  {
  public:
    virtual File *createFile(FileName fileName, bool, AudioProperties::ReadStyle) const
    {
      return new Ogg::Vorbis::File(fileName);
    }
  };
  DummyResolver resolver;

  template <typename T>
  void fileRefSave(const string &filename, const string &ext)
  {
    const ScopedFileCopy copy(filename, ext);
    {
      FileRef f(copy.fileName().c_str());
      REQUIRE(dynamic_cast<T*>(f.file()));
      REQUIRE_FALSE(f.isNull());
      f.tag()->setArtist("test artist");
      f.tag()->setTitle("test title");
      f.tag()->setGenre("Test!");
      f.tag()->setAlbum("albummmm");
      f.tag()->setTrack(5);
      f.tag()->setYear(2020);
      f.save();
    }
    {
      FileRef f(copy.fileName().c_str());
      REQUIRE_FALSE(f.isNull());
      REQUIRE(f.tag()->artist() == "test artist");
      REQUIRE(f.tag()->title() == "test title");
      REQUIRE(f.tag()->genre() == "Test!");
      REQUIRE(f.tag()->album() == "albummmm");
      REQUIRE(f.tag()->track() == 5);
      REQUIRE(f.tag()->year() == 2020);
      f.tag()->setArtist("ttest artist");
      f.tag()->setTitle("ytest title");
      f.tag()->setGenre("uTest!");
      f.tag()->setAlbum("ialbummmm");
      f.tag()->setTrack(7);
      f.tag()->setYear(2080);
      f.save();
    }
    {
      FileRef f(copy.fileName().c_str());
      REQUIRE_FALSE(f.isNull());
      REQUIRE(f.tag()->artist() == "ttest artist");
      REQUIRE(f.tag()->title() == "ytest title");
      REQUIRE(f.tag()->genre() == "uTest!");
      REQUIRE(f.tag()->album() == "ialbummmm");
      REQUIRE(f.tag()->track() == 7);
      REQUIRE(f.tag()->year() == 2080);
    }

    {
      FileStream fs(copy.fileName().c_str());
      FileRef f(&fs);
      REQUIRE(dynamic_cast<T*>(f.file()));
      REQUIRE_FALSE(f.isNull());
      REQUIRE(f.tag()->artist() == "ttest artist");
      REQUIRE(f.tag()->title() == "ytest title");
      REQUIRE(f.tag()->genre() == "uTest!");
      REQUIRE(f.tag()->album() == "ialbummmm");
      REQUIRE(f.tag()->track() == 7);
      REQUIRE(f.tag()->year() == 2080);
      f.tag()->setArtist("test artist");
      f.tag()->setTitle("test title");
      f.tag()->setGenre("Test!");
      f.tag()->setAlbum("albummmm");
      f.tag()->setTrack(5);
      f.tag()->setYear(2020);
      f.save();
    }

    ByteVector fileContent;
    {
      FileStream fs(copy.fileName().c_str());
      FileRef f(&fs);
      REQUIRE(dynamic_cast<T*>(f.file()));
      REQUIRE_FALSE(f.isNull());
      REQUIRE(f.tag()->artist() == "test artist");
      REQUIRE(f.tag()->title() == "test title");
      REQUIRE(f.tag()->genre() == "Test!");
      REQUIRE(f.tag()->album() == "albummmm");
      REQUIRE(f.tag()->track() == 5);
      REQUIRE(f.tag()->year() == 2020);

      fs.seek(0);
      fileContent = fs.readBlock(fs.length());
    }

    {
      ByteVectorStream bs(fileContent);
      FileRef f(&bs);
      REQUIRE(dynamic_cast<T*>(f.file()));
      REQUIRE_FALSE(f.isNull());
      REQUIRE(f.tag()->artist() == "test artist");
      REQUIRE(f.tag()->title() == "test title");
      REQUIRE(f.tag()->genre() == "Test!");
      REQUIRE(f.tag()->album() == "albummmm");
      REQUIRE(f.tag()->track() == 5);
      REQUIRE(f.tag()->year() == 2020);
      f.tag()->setArtist("ttest artist");
      f.tag()->setTitle("ytest title");
      f.tag()->setGenre("uTest!");
      f.tag()->setAlbum("ialbummmm");
      f.tag()->setTrack(7);
      f.tag()->setYear(2080);
      f.save();

      fileContent = *bs.data();
    }
    {
      ByteVectorStream bs(fileContent);
      FileRef f(&bs);
      REQUIRE(dynamic_cast<T*>(f.file()));
      REQUIRE_FALSE(f.isNull());
      REQUIRE(f.tag()->artist() == "ttest artist");
      REQUIRE(f.tag()->title() == "ytest title");
      REQUIRE(f.tag()->genre() == "uTest!");
      REQUIRE(f.tag()->album() == "ialbummmm");
      REQUIRE(f.tag()->track() == 7);
      REQUIRE(f.tag()->year() == 2080);
    }
  }
}

TEST_CASE("FileRef")
{
  SECTION("Detect AIFF file")
  {
    fileRefSave<RIFF::AIFF::File>("empty", ".aiff");
  }
  SECTION("Detect AIFF-C file")
  {
    fileRefSave<RIFF::AIFF::File>("alaw", ".aifc");
  }
  SECTION("Detect APE file")
  {
    fileRefSave<APE::File>("mac-399", ".ape");
  }
  SECTION("Detect ASF file")
  {
    fileRefSave<ASF::File>("silence-1", ".wma");
  }
  SECTION("Detect FLAC file")
  {
    fileRefSave<FLAC::File>("no-tags", ".flac");
  }
  SECTION("Detect MP3 file")
  {
    fileRefSave<MPEG::File>("xing", ".mp3");
  }
  SECTION("Detect MP4 file (1)")
  {
    fileRefSave<MP4::File>("has-tags", ".m4a");
  }
  SECTION("Detect MP4 file (2)")
  {
    fileRefSave<MP4::File>("no-tags", ".m4a");
  }
  SECTION("Detect MP4 file (3)")
  {
    fileRefSave<MP4::File>("no-tags", ".3g2");
  }
  SECTION("Detect MP4 file (4)")
  {
    fileRefSave<MP4::File>("blank_video", ".m4v");
  }
  SECTION("Detect Musepak file")
  {
    fileRefSave<MPC::File>("click", ".mpc");
  }
  SECTION("Detect Ogg FLAC file")
  {
    fileRefSave<Ogg::FLAC::File>("empty_flac", ".oga");
  }
  SECTION("Detect Ogg Speex file")
  {
    fileRefSave<Ogg::Speex::File>("empty", ".spx");
  }
  SECTION("Detect Ogg Vorbis file (1)")
  {
    fileRefSave<Ogg::Vorbis::File>("empty", ".ogg");
  }
  SECTION("Detect Ogg Vorbis file (2)")
  {
    fileRefSave<Ogg::Vorbis::File>("empty_vorbis", ".oga");
  }
  SECTION("Detect TrueAudio file")
  {
    fileRefSave<TrueAudio::File>("empty", ".tta");
  }
  SECTION("Detect WAV file")
  {
    fileRefSave<RIFF::WAV::File>("empty", ".wav");
  }
  SECTION("Unsupported formats")
  {
    FileRef f1(TEST_FILE_PATH_C("no-extension"));
    REQUIRE(f1.isNull());
    
    FileRef f2(TEST_FILE_PATH_C("unsupported-extension.xx"));
    REQUIRE(f2.isNull());
  }
  SECTION("User defined file type resolver")
  {
    {
      FileRef f(TEST_FILE_PATH_C("xing.mp3"));
      REQUIRE(dynamic_cast<MPEG::File *>(f.file()));
    }
    
    FileRef::addFileTypeResolver(&resolver);
    {
      FileRef f(TEST_FILE_PATH_C("xing.mp3"));
      REQUIRE(dynamic_cast<Ogg::Vorbis::File *>(f.file()));
    }
  }
}

