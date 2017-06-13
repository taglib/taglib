/***************************************************************************
    copyright           : (C) 2009 by Lukas Lalinsky
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
#include <flacfile.h>
#include <xiphcomment.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("FLAC File")
{
  SECTION("Read audio properties")
  {
    FLAC::File f(TEST_FILE_PATH_C("sinewave.flac"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3550);
    REQUIRE(f.audioProperties()->bitrate() == 145);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->sampleWidth() == 16);
    REQUIRE(f.audioProperties()->sampleFrames() == 156556);
    REQUIRE(f.audioProperties()->signature().toHex() == "cfe3d9dabadeab2cbf2ca235274b7f76");
  }
  SECTION("Skip and remove multiple comment blocks")
  {
    const ScopedFileCopy copy("multiple-vc", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->artist() == "Artist 1");
      f.tag()->setArtist("The Artist");
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->artist() == "The Artist");
      REQUIRE(f.find("Artist") == 69L);
      REQUIRE(f.find("Artist", 70) == -1);
    }
  }
  SECTION("Read picture")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");

    FLAC::File f(copy.fileName().c_str());
    const List<FLAC::Picture *> lst = f.pictureList();
    REQUIRE(lst.size() == 1);

    const FLAC::Picture *pic = lst.front();
    REQUIRE(pic->type() == FLAC::Picture::FrontCover);
    REQUIRE(pic->width() == 1);
    REQUIRE(pic->height() == 1);
    REQUIRE(pic->colorDepth() == 24);
    REQUIRE(pic->numColors() == 0);
    REQUIRE(pic->mimeType() == "image/png");
    REQUIRE(pic->description() == "A pixel.");
    REQUIRE(pic->data().size() == 150);
  }
  SECTION("Add picture")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      List<FLAC::Picture *> lst = f.pictureList();
      REQUIRE(lst.size() == 1);

      FLAC::Picture *newpic = new FLAC::Picture();
      newpic->setType(FLAC::Picture::BackCover);
      newpic->setWidth(5);
      newpic->setHeight(6);
      newpic->setColorDepth(16);
      newpic->setNumColors(7);
      newpic->setMimeType("image/jpeg");
      newpic->setDescription("new image");
      newpic->setData("JPEG data");
      f.addPicture(newpic);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      const List<FLAC::Picture *> lst = f.pictureList();
      REQUIRE(lst.size() == 2);

      REQUIRE(lst[0]->type() == FLAC::Picture::FrontCover);
      REQUIRE(lst[0]->width() == 1);
      REQUIRE(lst[0]->height() == 1);
      REQUIRE(lst[0]->colorDepth() == 24);
      REQUIRE(lst[0]->numColors() == 0);
      REQUIRE(lst[0]->mimeType() == "image/png");
      REQUIRE(lst[0]->description() == "A pixel.");
      REQUIRE(lst[0]->data().size() == 150);

      REQUIRE(lst[1]->type() == FLAC::Picture::BackCover);
      REQUIRE(lst[1]->width() == 5);
      REQUIRE(lst[1]->height() == 6);
      REQUIRE(lst[1]->colorDepth() == 16);
      REQUIRE(lst[1]->numColors() == 7);
      REQUIRE(lst[1]->mimeType() == "image/jpeg");
      REQUIRE(lst[1]->description() == "new image");
      REQUIRE(lst[1]->data() == "JPEG data");
    }
  }
  SECTION("Replace picture")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      List<FLAC::Picture *> lst = f.pictureList();
      REQUIRE(lst.size() == 1);

      FLAC::Picture *newpic = new FLAC::Picture();
      newpic->setType(FLAC::Picture::BackCover);
      newpic->setWidth(5);
      newpic->setHeight(6);
      newpic->setColorDepth(16);
      newpic->setNumColors(7);
      newpic->setMimeType("image/jpeg");
      newpic->setDescription("new image");
      newpic->setData("JPEG data");
      f.removePictures();
      f.addPicture(newpic);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      const List<FLAC::Picture *> lst = f.pictureList();
      REQUIRE(lst.size() == 1);

      REQUIRE(lst[0]->type() == FLAC::Picture::BackCover);
      REQUIRE(lst[0]->width() == 5);
      REQUIRE(lst[0]->height() == 6);
      REQUIRE(lst[0]->colorDepth() == 16);
      REQUIRE(lst[0]->numColors() == 7);
      REQUIRE(lst[0]->mimeType() == "image/jpeg");
      REQUIRE(lst[0]->description() == "new image");
      REQUIRE(lst[0]->data() == "JPEG data");
    }
  }
  SECTION("Remove all pictures")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.pictureList().size() == 1);

      f.removePictures();
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.pictureList().isEmpty());
    }
  }
  SECTION("Save tags repeatedly without breaking file (1)")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->title() == "Silence");
      f.tag()->setTitle("NEW TITLE");
      f.save();
      REQUIRE(f.tag()->title() == "NEW TITLE");
      f.tag()->setTitle("NEW TITLE 2");
      f.save();
      REQUIRE(f.tag()->title() == "NEW TITLE 2");
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->title() == "NEW TITLE 2");
    }
  }
  SECTION("Save tags repeatedly without breaking file (2)")
  {
    const ScopedFileCopy copy("no-tags", ".flac");

    FLAC::File f(copy.fileName().c_str());
    f.ID3v2Tag(true)->setTitle("0123456789");
    f.save();
    REQUIRE(f.length() == 5735);
    f.save();
    REQUIRE(f.length() == 5735);
    REQUIRE(f.find("fLaC") >= 0);
  }
  SECTION("Save tags repeatedly without breaking file (3)")
  {
    const ScopedFileCopy copy("no-tags", ".flac");

    FLAC::File f(copy.fileName().c_str());
    f.xiphComment()->setTitle(longText(8 * 1024));
    f.save();
    REQUIRE(f.length() == 12862);
    f.save();
    REQUIRE(f.length() == 12862);
  }
  SECTION("Save multiple values")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      f.xiphComment(true)->addField("ARTIST", "artist 1", true);
      f.xiphComment(true)->addField("ARTIST", "artist 2", false);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      const Ogg::FieldListMap m = f.xiphComment()->fieldListMap();
      REQUIRE(m["ARTIST"].size() == 2);
      REQUIRE(m["ARTIST"][0] == "artist 1");
      REQUIRE(m["ARTIST"][1] == "artist 2");
    }
  }
  SECTION("Read and write property map")
  {
    // test unicode & multiple values with dict interface
    const ScopedFileCopy copy("silence-44-s", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      PropertyMap dict;
      dict["ARTIST"].append("artøst 1");
      dict["ARTIST"].append("artöst 2");
      f.setProperties(dict);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      const PropertyMap dict = f.properties();
      REQUIRE(dict["ARTIST"].size() == 2);
      REQUIRE(dict["ARTIST"][0] == "artøst 1");
      REQUIRE(dict["ARTIST"][1] == "artöst 2");
    }
  }
  SECTION("Skip invalid property map fields")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    FLAC::File f(copy.fileName().c_str());

    PropertyMap map;
    map[L"H\x00c4\x00d6"] = String("bla");
    PropertyMap invalid = f.setProperties(map);
    REQUIRE(invalid.size() == 1);
    REQUIRE(f.properties().isEmpty());
  }
  SECTION("Don't treat zero-sized padding as error")
  {
    const ScopedFileCopy copy("zero-sized-padding", ".flac");

    FLAC::File f(copy.fileName().c_str());
    REQUIRE(f.isValid());
  }
  SECTION("Don't create zero-sized padding")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    const String text = longText(3067);
    {
      FLAC::File f(copy.fileName().c_str());
      f.xiphComment()->setTitle("ABC");
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      f.xiphComment()->setTitle(text);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.xiphComment()->title() == text);
    }
  }
  SECTION("Shrink too large padding")
  {
    const ScopedFileCopy copy("no-tags", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      f.xiphComment()->setTitle(longText(128 * 1024));
      f.save();
      REQUIRE(f.length() > 128 * 1024);
    }
    {
      FLAC::File f(copy.fileName().c_str());
      f.xiphComment()->setTitle("0123456789");
      f.save();
      REQUIRE(f.length() < 8 * 1024);
    }
  }
  SECTION("Don't treat empty seektable as error and keep it untouched")
  {
    const ScopedFileCopy copy("empty-seektable", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      f.xiphComment(true)->setTitle("XiphComment Title");
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      f.seek(42);
      const ByteVector data = f.readBlock(4);
      REQUIRE(data == ByteVector("\x03\x00\x00\x00", 4));
    }
  }
  SECTION("Save ID3v1 tag without breaking the audio stream")
  {
    const ScopedFileCopy copy("no-tags", ".flac");

    ByteVector audioStream;
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v1Tag());
      REQUIRE(f.length() == (long)4692);

      f.seek(0x0100);
      audioStream = f.readBlock(4436);

      f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
      f.save();
      REQUIRE(f.hasID3v1Tag());
      REQUIRE(f.length() == (long)4820);

      f.seek(0x0100);
      REQUIRE(f.readBlock(4436) == audioStream);
    }
  }
  SECTION("Update ID3v2 tag")
  {
    const ScopedFileCopy copy("no-tags", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      f.ID3v2Tag(true)->setTitle("0123456789");
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      f.ID3v2Tag()->setTitle("ABCDEFGHIJ");
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.ID3v2Tag()->title() == "ABCDEFGHIJ");
    }
  }
  SECTION("Don't create empty ID3v2 tag")
  {
    const ScopedFileCopy copy("no-tags", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      f.ID3v2Tag(true);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.hasID3v2Tag());
    }
  }
  SECTION("Create and strip tags")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      f.xiphComment(true)->setTitle("XiphComment Title");
      f.ID3v1Tag(true)->setTitle("ID3v1 Title");
      f.ID3v2Tag(true)->setTitle("ID3v2 Title");
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.hasXiphComment());
      REQUIRE(f.hasID3v1Tag());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.xiphComment()->title() == "XiphComment Title");
      REQUIRE(f.ID3v1Tag()->title() == "ID3v1 Title");
      REQUIRE(f.ID3v2Tag()->title() == "ID3v2 Title");
      f.strip(FLAC::File::ID3v2);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.hasXiphComment());
      REQUIRE(f.hasID3v1Tag());
      REQUIRE_FALSE(f.hasID3v2Tag());
      REQUIRE(f.xiphComment()->title() == "XiphComment Title");
      REQUIRE(f.ID3v1Tag()->title() == "ID3v1 Title");
      f.strip(FLAC::File::ID3v1);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.hasXiphComment());
      REQUIRE_FALSE(f.hasID3v1Tag());
      REQUIRE_FALSE(f.hasID3v2Tag());
      REQUIRE(f.xiphComment()->title() == "XiphComment Title");
      f.strip(FLAC::File::XiphComment);
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.hasXiphComment());
      REQUIRE_FALSE(f.hasID3v1Tag());
      REQUIRE_FALSE(f.hasID3v2Tag());
      REQUIRE(f.xiphComment()->isEmpty());
      REQUIRE(f.xiphComment()->vendorID() == "reference libFLAC 1.1.0 20030126");
    }
  }
  SECTION("Remove fields from Xiph comment")
  {
    const ScopedFileCopy copy("silence-44-s", ".flac");
    {
      FLAC::File f(copy.fileName().c_str());
      f.xiphComment(true)->setTitle("XiphComment Title");
      f.ID3v2Tag(true)->setTitle("ID3v2 Title");
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.xiphComment()->title() == "XiphComment Title");
      f.xiphComment()->removeFields("TITLE");
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str());
      REQUIRE(f.xiphComment()->title().isEmpty());
    }
  }
}
