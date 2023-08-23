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

#include "flacfile.h"
#include "id3v1tag.h"
#include "id3v2tag.h"
#include "plainfile.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "utils.h"
#include "xiphcomment.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(FLAC, testSignature)
{
  FLAC::File f(TEST_FILE_PATH_C("no-tags.flac"), nullptr);
  ASSERT_EQ(ByteVector("a1b141f766e9849ac3db1030a20a3c77"), f.audioProperties()->signature().toHex());
}

TEST(FLAC, testMultipleCommentBlocks)
{
  ScopedFileCopy copy("multiple-vc", ".flac");
  string newname = copy.fileName();

  {
    FLAC::File f(newname.c_str(), nullptr);
    ASSERT_EQ(String("Artist 1"), f.tag()->artist());
    f.tag()->setArtist("The Artist");
    f.save();
  }
  {
    FLAC::File f(newname.c_str(), nullptr);
    ASSERT_EQ(String("The Artist"), f.tag()->artist());
    ASSERT_EQ(static_cast<offset_t>(69), f.find("Artist"));
    ASSERT_EQ(static_cast<offset_t>(-1), f.find("Artist", 70));
  }
}

TEST(FLAC, testReadPicture)
{
  ScopedFileCopy copy("silence-44-s", ".flac");
  string newname = copy.fileName();

  FLAC::File f(newname.c_str(), nullptr);
  List<FLAC::Picture *> lst = f.pictureList();
  ASSERT_EQ(static_cast<unsigned int>(1), lst.size());

  FLAC::Picture *pic = lst.front();
  ASSERT_EQ(FLAC::Picture::FrontCover, pic->type());
  ASSERT_EQ(1, pic->width());
  ASSERT_EQ(1, pic->height());
  ASSERT_EQ(24, pic->colorDepth());
  ASSERT_EQ(0, pic->numColors());
  ASSERT_EQ(String("image/png"), pic->mimeType());
  ASSERT_EQ(String("A pixel."), pic->description());
  ASSERT_EQ(static_cast<unsigned int>(150), pic->data().size());
}

TEST(FLAC, testAddPicture)
{
  ScopedFileCopy copy("silence-44-s", ".flac");
  string newname = copy.fileName();

  {
    FLAC::File f(newname.c_str(), nullptr);
    List<FLAC::Picture *> lst = f.pictureList();
    ASSERT_EQ(static_cast<unsigned int>(1), lst.size());

    auto newpic = new FLAC::Picture();
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
    FLAC::File f(newname.c_str(), nullptr);
    List<FLAC::Picture *> lst = f.pictureList();
    ASSERT_EQ(static_cast<unsigned int>(2), lst.size());

    FLAC::Picture *pic = lst[0];
    ASSERT_EQ(FLAC::Picture::FrontCover, pic->type());
    ASSERT_EQ(1, pic->width());
    ASSERT_EQ(1, pic->height());
    ASSERT_EQ(24, pic->colorDepth());
    ASSERT_EQ(0, pic->numColors());
    ASSERT_EQ(String("image/png"), pic->mimeType());
    ASSERT_EQ(String("A pixel."), pic->description());
    ASSERT_EQ(static_cast<unsigned int>(150), pic->data().size());

    pic = lst[1];
    ASSERT_EQ(FLAC::Picture::BackCover, pic->type());
    ASSERT_EQ(5, pic->width());
    ASSERT_EQ(6, pic->height());
    ASSERT_EQ(16, pic->colorDepth());
    ASSERT_EQ(7, pic->numColors());
    ASSERT_EQ(String("image/jpeg"), pic->mimeType());
    ASSERT_EQ(String("new image"), pic->description());
    ASSERT_EQ(ByteVector("JPEG data"), pic->data());
  }
}

TEST(FLAC, testReplacePicture)
{
  ScopedFileCopy copy("silence-44-s", ".flac");
  string newname = copy.fileName();

  {
    FLAC::File f(newname.c_str(), nullptr);
    List<FLAC::Picture *> lst = f.pictureList();
    ASSERT_EQ(static_cast<unsigned int>(1), lst.size());

    auto newpic = new FLAC::Picture();
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
    FLAC::File f(newname.c_str(), nullptr);
    List<FLAC::Picture *> lst = f.pictureList();
    ASSERT_EQ(static_cast<unsigned int>(1), lst.size());

    FLAC::Picture *pic = lst[0];
    ASSERT_EQ(FLAC::Picture::BackCover, pic->type());
    ASSERT_EQ(5, pic->width());
    ASSERT_EQ(6, pic->height());
    ASSERT_EQ(16, pic->colorDepth());
    ASSERT_EQ(7, pic->numColors());
    ASSERT_EQ(String("image/jpeg"), pic->mimeType());
    ASSERT_EQ(String("new image"), pic->description());
    ASSERT_EQ(ByteVector("JPEG data"), pic->data());
  }
}

TEST(FLAC, testRemoveAllPictures)
{
  ScopedFileCopy copy("silence-44-s", ".flac");
  string newname = copy.fileName();

  {
    FLAC::File f(newname.c_str(), nullptr);
    List<FLAC::Picture *> lst = f.pictureList();
    ASSERT_EQ(static_cast<unsigned int>(1), lst.size());

    f.removePictures();
    f.save();
  }
  {
    FLAC::File f(newname.c_str(), nullptr);
    List<FLAC::Picture *> lst = f.pictureList();
    ASSERT_EQ(static_cast<unsigned int>(0), lst.size());
  }
}

TEST(FLAC, testRepeatedSave1)
{
  ScopedFileCopy copy("silence-44-s", ".flac");
  string newname = copy.fileName();

  {
    FLAC::File f(newname.c_str(), nullptr);
    ASSERT_EQ(String("Silence"), f.tag()->title());
    f.tag()->setTitle("NEW TITLE");
    f.save();
    ASSERT_EQ(String("NEW TITLE"), f.tag()->title());
    f.tag()->setTitle("NEW TITLE 2");
    f.save();
    ASSERT_EQ(String("NEW TITLE 2"), f.tag()->title());
  }
  {
    FLAC::File f(newname.c_str(), nullptr);
    ASSERT_EQ(String("NEW TITLE 2"), f.tag()->title());
  }
}

TEST(FLAC, testRepeatedSave2)
{
  ScopedFileCopy copy("no-tags", ".flac");

  FLAC::File f(copy.fileName().c_str(), nullptr);
  f.ID3v2Tag(true)->setTitle("0123456789");
  f.save();
  ASSERT_EQ(static_cast<offset_t>(5735), f.length());
  f.save();
  ASSERT_EQ(static_cast<offset_t>(5735), f.length());
  ASSERT_TRUE(f.find("fLaC") >= 0);
}

TEST(FLAC, testRepeatedSave3)
{
  ScopedFileCopy copy("no-tags", ".flac");

  FLAC::File f(copy.fileName().c_str());
  f.xiphComment()->setTitle(longText(8 * 1024));
  f.save();
  ASSERT_EQ(static_cast<offset_t>(12862), f.length());
  f.save();
  ASSERT_EQ(static_cast<offset_t>(12862), f.length());
}

TEST(FLAC, testSaveMultipleValues)
{
  ScopedFileCopy copy("silence-44-s", ".flac");
  string newname = copy.fileName();

  {
    FLAC::File f(newname.c_str());
    f.xiphComment(true)->addField("ARTIST", "artist 1", true);
    f.xiphComment(true)->addField("ARTIST", "artist 2", false);
    f.save();
  }
  {
    FLAC::File f(newname.c_str());
    Ogg::FieldListMap m = f.xiphComment()->fieldListMap();
    ASSERT_EQ(static_cast<unsigned int>(2), m["ARTIST"].size());
    ASSERT_EQ(String("artist 1"), m["ARTIST"][0]);
    ASSERT_EQ(String("artist 2"), m["ARTIST"][1]);
  }
}

TEST(FLAC, testDict)
{
  // test unicode & multiple values with dict interface
  ScopedFileCopy copy("silence-44-s", ".flac");
  string newname = copy.fileName();

  {
    FLAC::File f(newname.c_str());
    PropertyMap dict;
    dict["ARTIST"].append("artøst 1");
    dict["ARTIST"].append("artöst 2");
    f.setProperties(dict);
    f.save();
  }
  {
    FLAC::File f(newname.c_str());
    PropertyMap dict = f.properties();
    ASSERT_EQ(static_cast<unsigned int>(2), dict["ARTIST"].size());
    ASSERT_EQ(String("artøst 1"), dict["ARTIST"][0]);
    ASSERT_EQ(String("artöst 2"), dict["ARTIST"][1]);
  }
}

TEST(FLAC, testProperties)
{
  PropertyMap tags;
  tags["ALBUM"]                      = StringList("Album");
  tags["ALBUMARTIST"]                = StringList("Album Artist");
  tags["ALBUMARTISTSORT"]            = StringList("Album Artist Sort");
  tags["ALBUMSORT"]                  = StringList("Album Sort");
  tags["ARTIST"]                     = StringList("Artist");
  tags["ARTISTS"]                    = StringList("Artists");
  tags["ARTISTSORT"]                 = StringList("Artist Sort");
  tags["ASIN"]                       = StringList("ASIN");
  tags["BARCODE"]                    = StringList("Barcode");
  tags["CATALOGNUMBER"]              = StringList("Catalog Number 1").append("Catalog Number 2");
  tags["COMMENT"]                    = StringList("Comment");
  tags["DATE"]                       = StringList("2021-01-10");
  tags["DISCNUMBER"]                 = StringList("3");
  tags["DISCTOTAL"]                  = StringList("5");
  tags["GENRE"]                      = StringList("Genre");
  tags["ISRC"]                       = StringList("UKAAA0500001");
  tags["LABEL"]                      = StringList("Label 1").append("Label 2");
  tags["MEDIA"]                      = StringList("Media");
  tags["MUSICBRAINZ_ALBUMARTISTID"]  = StringList("MusicBrainz_AlbumartistID");
  tags["MUSICBRAINZ_ALBUMID"]        = StringList("MusicBrainz_AlbumID");
  tags["MUSICBRAINZ_ARTISTID"]       = StringList("MusicBrainz_ArtistID");
  tags["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
  tags["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
  tags["MUSICBRAINZ_TRACKID"]        = StringList("MusicBrainz_TrackID");
  tags["ORIGINALDATE"]               = StringList("2021-01-09");
  tags["RELEASECOUNTRY"]             = StringList("Release Country");
  tags["RELEASESTATUS"]              = StringList("Release Status");
  tags["RELEASETYPE"]                = StringList("Release Type");
  tags["SCRIPT"]                     = StringList("Script");
  tags["TITLE"]                      = StringList("Title");
  tags["TRACKNUMBER"]                = StringList("2");
  tags["TRACKTOTAL"]                 = StringList("4");

  ScopedFileCopy copy("no-tags", ".flac");
  {
    FLAC::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    ASSERT_TRUE(properties.isEmpty());
    f.setProperties(tags);
    f.save();
  }
  {
    const FLAC::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    if(tags != properties) {
      ASSERT_EQ(tags.toString(), properties.toString());
    }
    ASSERT_EQ(tags, properties);
  }
}

TEST(FLAC, testInvalid)
{
  ScopedFileCopy copy("silence-44-s", ".flac");
  PropertyMap map;
  map[L"H\x00c4\x00d6"] = String("bla");
  FLAC::File f(copy.fileName().c_str());
  PropertyMap invalid = f.setProperties(map);
  ASSERT_EQ(static_cast<unsigned int>(1), invalid.size());
  ASSERT_EQ(static_cast<unsigned int>(0), f.properties().size());
}

TEST(FLAC, testAudioProperties)
{
  FLAC::File f(TEST_FILE_PATH_C("sinewave.flac"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3550, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(145, f.audioProperties()->bitrate());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(156556ULL, f.audioProperties()->sampleFrames());
  ASSERT_EQ(
    ByteVector("\xcf\xe3\xd9\xda\xba\xde\xab\x2c\xbf\x2c\xa2\x35\x27\x4b\x7f\x76"),
    f.audioProperties()->signature());
}

TEST(FLAC, testZeroSizedPadding1)
{
  ScopedFileCopy copy("zero-sized-padding", ".flac");

  FLAC::File f(copy.fileName().c_str());
  ASSERT_TRUE(f.isValid());
}

TEST(FLAC, testZeroSizedPadding2)
{
  ScopedFileCopy copy("silence-44-s", ".flac");

  {
    FLAC::File f(copy.fileName().c_str());
    f.xiphComment()->setTitle("ABC");
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    f.xiphComment()->setTitle(std::string(3067, 'X').c_str());
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
  }
}

TEST(FLAC, testShrinkPadding)
{
  ScopedFileCopy copy("no-tags", ".flac");

  {
    FLAC::File f(copy.fileName().c_str());
    f.xiphComment()->setTitle(longText(128 * 1024));
    f.save();
    ASSERT_TRUE(f.length() > 128 * 1024);
  }
  {
    FLAC::File f(copy.fileName().c_str());
    f.xiphComment()->setTitle("0123456789");
    f.save();
    ASSERT_TRUE(f.length() < 8 * 1024);
  }
}

TEST(FLAC, testSaveID3v1)
{
  ScopedFileCopy copy("no-tags", ".flac");

  ByteVector audioStream;
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasID3v1Tag());
    ASSERT_EQ(static_cast<offset_t>(4692), f.length());

    f.seek(0x0100);
    audioStream = f.readBlock(4436);

    f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.save();
    ASSERT_TRUE(f.hasID3v1Tag());
    ASSERT_EQ(static_cast<offset_t>(4820), f.length());

    f.seek(0x0100);
    ASSERT_EQ(audioStream, f.readBlock(4436));
  }
}

TEST(FLAC, testUpdateID3v2)
{
  ScopedFileCopy copy("no-tags", ".flac");

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
    ASSERT_EQ(String("ABCDEFGHIJ"), f.ID3v2Tag()->title());
  }
}

TEST(FLAC, testEmptyID3v2)
{
  ScopedFileCopy copy("no-tags", ".flac");

  {
    FLAC::File f(copy.fileName().c_str());
    f.ID3v2Tag(true);
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasID3v2Tag());
  }
}

TEST(FLAC, testStripTags)
{
  ScopedFileCopy copy("silence-44-s", ".flac");

  {
    FLAC::File f(copy.fileName().c_str());
    f.xiphComment(true)->setTitle("XiphComment Title");
    f.ID3v1Tag(true)->setTitle("ID3v1 Title");
    f.ID3v2Tag(true)->setTitle("ID3v2 Title");
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasXiphComment());
    ASSERT_TRUE(f.hasID3v1Tag());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(String("XiphComment Title"), f.xiphComment()->title());
    ASSERT_EQ(String("ID3v1 Title"), f.ID3v1Tag()->title());
    ASSERT_EQ(String("ID3v2 Title"), f.ID3v2Tag()->title());
    f.strip(FLAC::File::ID3v2);
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasXiphComment());
    ASSERT_TRUE(f.hasID3v1Tag());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_EQ(String("XiphComment Title"), f.xiphComment()->title());
    ASSERT_EQ(String("ID3v1 Title"), f.ID3v1Tag()->title());
    f.strip(FLAC::File::ID3v1);
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasXiphComment());
    ASSERT_FALSE(f.hasID3v1Tag());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_EQ(String("XiphComment Title"), f.xiphComment()->title());
    f.strip(FLAC::File::XiphComment);
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasXiphComment());
    ASSERT_FALSE(f.hasID3v1Tag());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_TRUE(f.xiphComment()->isEmpty());
    ASSERT_EQ(String("reference libFLAC 1.1.0 20030126"), f.xiphComment()->vendorID());
  }
}

TEST(FLAC, testRemoveXiphField)
{
  ScopedFileCopy copy("silence-44-s", ".flac");

  {
    FLAC::File f(copy.fileName().c_str());
    f.xiphComment(true)->setTitle("XiphComment Title");
    f.ID3v2Tag(true)->setTitle("ID3v2 Title");
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_EQ(String("XiphComment Title"), f.xiphComment()->title());
    f.xiphComment()->removeFields("TITLE");
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_EQ(String(), f.xiphComment()->title());
  }
}

TEST(FLAC, testEmptySeekTable)
{
  ScopedFileCopy copy("empty-seektable", ".flac");
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    f.xiphComment(true)->setTitle("XiphComment Title");
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.isValid());
    f.seek(42);
    const ByteVector data = f.readBlock(4);
    ASSERT_EQ(ByteVector("\x03\x00\x00\x00", 4), data);
  }
}

TEST(FLAC, testPictureStoredAfterComment)
{
  // Blank.png from https://commons.wikimedia.org/wiki/File:Blank.png
  const unsigned char blankPngData[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x9d, 0x74, 0x66, 0x1a, 0x00, 0x00, 0x00,
    0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xae, 0xce, 0x1c, 0xe9, 0x00, 0x00,
    0x00, 0x04, 0x67, 0x41, 0x4d, 0x41, 0x00, 0x00, 0xb1, 0x8f, 0x0b, 0xfc,
    0x61, 0x05, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00,
    0x0e, 0xc3, 0x00, 0x00, 0x0e, 0xc3, 0x01, 0xc7, 0x6f, 0xa8, 0x64, 0x00,
    0x00, 0x00, 0x0c, 0x49, 0x44, 0x41, 0x54, 0x18, 0x57, 0x63, 0xc0, 0x01,
    0x18, 0x18, 0x00, 0x00, 0x1a, 0x00, 0x01, 0x82, 0x92, 0x4d, 0x60, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
  };
  const ByteVector picData(reinterpret_cast<const char *>(blankPngData),
                           sizeof(blankPngData));

  ScopedFileCopy copy("no-tags", ".flac");
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasID3v1Tag());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_FALSE(f.hasXiphComment());
    ASSERT_TRUE(f.pictureList().isEmpty());

    auto pic = new FLAC::Picture;
    pic->setData(picData);
    pic->setType(FLAC::Picture::FrontCover);
    pic->setMimeType("image/png");
    pic->setDescription("blank.png");
    pic->setWidth(3);
    pic->setHeight(2);
    pic->setColorDepth(32);
    pic->setNumColors(0);
    f.addPicture(pic);
    f.xiphComment(true)->setTitle("Title");
    f.save();
  }
  {
    FLAC::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasID3v1Tag());
    ASSERT_FALSE(f.hasID3v2Tag());
    ASSERT_TRUE(f.hasXiphComment());
    const List<FLAC::Picture *> pictures = f.pictureList();
    ASSERT_EQ(1U, pictures.size());
    ASSERT_EQ(picData, pictures[0]->data());
    ASSERT_EQ(FLAC::Picture::FrontCover, pictures[0]->type());
    ASSERT_EQ(String("image/png"), pictures[0]->mimeType());
    ASSERT_EQ(String("blank.png"), pictures[0]->description());
    ASSERT_EQ(3, pictures[0]->width());
    ASSERT_EQ(2, pictures[0]->height());
    ASSERT_EQ(32, pictures[0]->colorDepth());
    ASSERT_EQ(0, pictures[0]->numColors());
    ASSERT_EQ(String("Title"), f.xiphComment(false)->title());
  }

  const unsigned char expectedHeadData[] = {
    'f', 'L', 'a', 'C', 0x00, 0x00, 0x00, 0x22, 0x12, 0x00, 0x12, 0x00,
    0x00, 0x00, 0x0e, 0x00, 0x00, 0x10, 0x0a, 0xc4, 0x42, 0xf0, 0x00, 0x02,
    0x7a, 0xc0, 0xa1, 0xb1, 0x41, 0xf7, 0x66, 0xe9, 0x84, 0x9a, 0xc3, 0xdb,
    0x10, 0x30, 0xa2, 0x0a, 0x3c, 0x77, 0x04, 0x00, 0x00, 0x17, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 'T', 'I',
    'T', 'L', 'E', '=', 'T', 'i', 't', 'l', 'e', 0x06, 0x00, 0x00,
    0xa9, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x09, 'i', 'm', 'a',
    'g', 'e', '/', 'p', 'n', 'g', 0x00, 0x00, 0x00, 0x09, 'b', 'l',
    'a', 'n', 'k', '.', 'p', 'n', 'g', 0x00, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x77
  };
  ByteVector expectedData(reinterpret_cast<const char *>(expectedHeadData),
                          sizeof(expectedHeadData));
  expectedData.append(picData);
  const ByteVector fileData = PlainFile(copy.fileName().c_str()).readAll();
  ASSERT_TRUE(fileData.startsWith(expectedData));
}
