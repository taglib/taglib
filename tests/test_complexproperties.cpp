/***************************************************************************
    copyright            : (C) 2023 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#include "asfpicture.h"
#include "flacpicture.h"
#include "flacfile.h"
#include "tbytevector.h"
#include "tvariant.h"
#include "tzlib.h"
#include "fileref.h"
#include "apetag.h"
#include "asftag.h"
#include "mp4tag.h"
#include "xiphcomment.h"
#include "id3v1tag.h"
#include "id3v2tag.h"
#include "attachedpictureframe.h"
#include "generalencapsulatedobjectframe.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace TagLib;

namespace {

const String GEOB_KEY("GENERALOBJECT");
const String PICTURE_KEY("PICTURE");

const VariantMap TEST_PICTURE {
  {"data", ByteVector(
    "\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49\x46\x00\x01\x01\x01\x00\x48\x00\x48"
    "\x00\x00\xff\xdb\x00\x43\x00\x03\x02\x02\x02\x02\x02\x03\x02\x02\x02\x03"
    "\x03\x03\x03\x04\x06\x04\x04\x04\x04\x04\x08\x06\x06\x05\x06\x09\x08\x0a"
    "\x0a\x09\x08\x09\x09\x0a\x0c\x0f\x0c\x0a\x0b\x0e\x0b\x09\x09\x0d\x11\x0d"
    "\x0e\x0f\x10\x10\x11\x10\x0a\x0c\x12\x13\x12\x10\x13\x0f\x10\x10\x10\xff"
    "\xc9\x00\x0b\x08\x00\x01\x00\x01\x01\x01\x11\x00\xff\xcc\x00\x06\x00\x10"
    "\x10\x05\xff\xda\x00\x08\x01\x01\x00\x00\x3f\x00\xd2\xcf\x20\xff\xd9",
    125)},
  {"mimeType", "image/jpeg"},
  {"description", "Embedded cover"},
  {"pictureType", "Front Cover"}
};

}  // namespace

class TestComplexProperties : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestComplexProperties);
  CPPUNIT_TEST(testReadMp3Picture);
  CPPUNIT_TEST(testReadM4aPicture);
  CPPUNIT_TEST(testReadOggPicture);
  CPPUNIT_TEST(testReadWriteFlacPicture);
  CPPUNIT_TEST(testReadWriteMultipleProperties);
  CPPUNIT_TEST(testSetGetId3Geob);
  CPPUNIT_TEST(testSetGetId3Picture);
  CPPUNIT_TEST(testSetGetApePicture);
  CPPUNIT_TEST(testSetGetAsfPicture);
  CPPUNIT_TEST(testSetGetMp4Picture);
  CPPUNIT_TEST(testSetGetXiphPicture);
  CPPUNIT_TEST(testNonExistent);
  CPPUNIT_TEST_SUITE_END();

public:
  void testReadMp3Picture()
  {
    if(zlib::isAvailable()) {
      FileRef f(TEST_FILE_PATH_C("compressed_id3_frame.mp3"), false);
      CPPUNIT_ASSERT_EQUAL(StringList(PICTURE_KEY),
        f.complexPropertyKeys());
      auto pictures = f.complexProperties(PICTURE_KEY);
      CPPUNIT_ASSERT_EQUAL(1U, pictures.size());
      auto picture = pictures.front();
      CPPUNIT_ASSERT_EQUAL(86414U,
        picture.value("data").value<ByteVector>().size());
      CPPUNIT_ASSERT_EQUAL(String(""),
        picture.value("description").value<String>());
      CPPUNIT_ASSERT_EQUAL(String("image/bmp"),
        picture.value("mimeType").value<String>());
      CPPUNIT_ASSERT_EQUAL(String("Other"),
        picture.value("pictureType").value<String>());
    }
  }

  void testReadM4aPicture()
  {
    const ByteVector expectedData1(
      "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52\x00\x00"
      "\x00\x02\x00\x00\x00\x02\x08\x02\x00\x00\x00\xfd\xd4\x9a\x73\x00\x00\x00"
      "\x16\x49\x44\x41\x54\x78\x9c\x63\x7c\x9f\xca\xc0\xc0\xc0\xc0\xc4\xc0\xc0"
      "\xc0\xc0\xc0\x00\x00\x11\x09\x01\x58\xab\x88\xdb\x6f\x00\x00\x00\x00\x49"
      "\x45\x4e\x44\xae\x42\x60\x82", 79);
    const ByteVector expectedData2(
      "\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49\x46\x00\x01\x01\x01\x00\x64\x00\x64"
      "\x00\x00\xff\xdb\x00\x43\x00\x09\x06\x07\x08\x07\x06\x09\x08\x08\x08\x0a"
      "\x0a\x09\x0b\x0e\x17\x0f\x0e\x0d\x0d\x0e\x1c\x14\x15\x11\x17\x22\x1e\x23"
      "\x23\x21\x1e\x20\x20\x25\x2a\x35\x2d\x25\x27\x32\x28\x20\x20\x2e\x3f\x2f"
      "\x32\x37\x39\x3c\x3c\x3c\x24\x2d\x42\x46\x41\x3a\x46\x35\x3b\x3c\x39\xff"
      "\xdb\x00\x43\x01\x0a\x0a\x0a\x0e\x0c\x0e\x1b\x0f\x0f\x1b\x39\x26\x20\x26"
      "\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39"
      "\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39"
      "\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\x39\xff\xc0\x00\x11"
      "\x08\x00\x02\x00\x02\x03\x01\x22\x00\x02\x11\x01\x03\x11\x01\xff\xc4\x00"
      "\x15\x00\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x07\xff\xc4\x00\x14\x10\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xff\xc4\x00\x15\x01\x01\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x06\xff\xc4\x00\x14\x11\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xda\x00"
      "\x0c\x03\x01\x00\x02\x11\x03\x11\x00\x3f\x00\x8d\x80\xb8\x19\xff\xd9", 287);

    FileRef f(TEST_FILE_PATH_C("has-tags.m4a"), false);
    CPPUNIT_ASSERT_EQUAL(StringList(PICTURE_KEY),
      f.complexPropertyKeys());
    auto pictures = f.complexProperties(PICTURE_KEY);
    CPPUNIT_ASSERT_EQUAL(2U, pictures.size());
    auto picture = pictures.front();
    CPPUNIT_ASSERT_EQUAL(expectedData1,
      picture.value("data").value<ByteVector>());
    CPPUNIT_ASSERT_EQUAL(String("image/png"),
      picture.value("mimeType").value<String>());
    picture = pictures.back();
    CPPUNIT_ASSERT_EQUAL(expectedData2,
      picture.value("data").value<ByteVector>());
    CPPUNIT_ASSERT_EQUAL(String("image/jpeg"),
      picture.value("mimeType").value<String>());
  }

  void testReadOggPicture()
  {
    FileRef f(TEST_FILE_PATH_C("lowercase-fields.ogg"), false);
    CPPUNIT_ASSERT_EQUAL(StringList(PICTURE_KEY),
      f.complexPropertyKeys());
    auto pictures = f.complexProperties(PICTURE_KEY);
    CPPUNIT_ASSERT_EQUAL(1U, pictures.size());
    auto picture = pictures.front();
    CPPUNIT_ASSERT_EQUAL(ByteVector("JPEG data"),
      picture.value("data").value<ByteVector>());
    CPPUNIT_ASSERT_EQUAL(String("image/jpeg"),
      picture.value("mimeType").value<String>());
    CPPUNIT_ASSERT_EQUAL(String("Back Cover"),
      picture.value("pictureType").value<String>());
    CPPUNIT_ASSERT_EQUAL(String("new image"),
      picture.value("description").value<String>());
    CPPUNIT_ASSERT_EQUAL(16, picture.value("colorDepth").value<int>());
    CPPUNIT_ASSERT_EQUAL(7, picture.value("numColors").value<int>());
    CPPUNIT_ASSERT_EQUAL(5, picture.value("width").value<int>());
    CPPUNIT_ASSERT_EQUAL(6, picture.value("height").value<int>());
  }

  void testReadWriteFlacPicture()
  {
    VariantMap picture(TEST_PICTURE);
    picture.insert("colorDepth", 8);
    picture.insert("numColors", 1);
    picture.insert("width", 1);
    picture.insert("height", 1);

    ScopedFileCopy copy("no-tags", ".flac");

    {
      FLAC::File f(copy.fileName().c_str(), false);
      CPPUNIT_ASSERT(f.complexPropertyKeys().isEmpty());
      CPPUNIT_ASSERT(f.pictureList().isEmpty());
      CPPUNIT_ASSERT(f.setComplexProperties(PICTURE_KEY, {picture}));
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str(), false);
      CPPUNIT_ASSERT_EQUAL(StringList(PICTURE_KEY), f.complexPropertyKeys());
      CPPUNIT_ASSERT_EQUAL(picture, f.complexProperties(PICTURE_KEY).front());
      auto flacPictures = f.pictureList();
      CPPUNIT_ASSERT_EQUAL(1U, flacPictures.size());
      auto flacPicture = flacPictures.front();
      CPPUNIT_ASSERT_EQUAL(picture.value("data").value<ByteVector>(),
        flacPicture->data());
      CPPUNIT_ASSERT_EQUAL(picture.value("mimeType").value<String>(),
        flacPicture->mimeType());
      CPPUNIT_ASSERT_EQUAL(FLAC::Picture::FrontCover, flacPicture->type());
      CPPUNIT_ASSERT_EQUAL(picture.value("description").value<String>(),
        flacPicture->description());
      CPPUNIT_ASSERT_EQUAL(picture.value("colorDepth").value<int>(),
        flacPicture->colorDepth());
      CPPUNIT_ASSERT_EQUAL(picture.value("numColors").value<int>(),
        flacPicture->numColors());
      CPPUNIT_ASSERT_EQUAL(picture.value("width").value<int>(),
        flacPicture->width());
      CPPUNIT_ASSERT_EQUAL(picture.value("height").value<int>(),
        flacPicture->height());

      CPPUNIT_ASSERT(f.setComplexProperties(PICTURE_KEY, {}));
      f.save();
    }
    {
      FLAC::File f(copy.fileName().c_str(), false);
      CPPUNIT_ASSERT(f.complexPropertyKeys().isEmpty());
      CPPUNIT_ASSERT(f.pictureList().isEmpty());
    }
  }

  void testReadWriteMultipleProperties()
  {
    const VariantMap picture2 {
      {"data", ByteVector("PNG data")},
      {"mimeType", "image/png"},
      {"description", ""},
      {"pictureType", "Back Cover"}
    };
    const VariantMap geob1 {
      {"data", ByteVector("First")},
      {"mimeType", "text/plain"},
      {"description", "Object 1"},
      {"fileName", "test1.txt"}
    };
    const VariantMap geob2 {
      {"data", ByteVector("Second")},
      {"mimeType", "text/plain"},
      {"description", "Object 2"},
      {"fileName", "test2.txt"}
    };

    ScopedFileCopy copy("xing", ".mp3");

    {
      FileRef f(copy.fileName().c_str(), false);
      CPPUNIT_ASSERT(f.complexPropertyKeys().isEmpty());
      f.setComplexProperties(PICTURE_KEY, {TEST_PICTURE, picture2});
      f.setComplexProperties(GEOB_KEY, {geob1, geob2});
      f.save();
    }
    {
      FileRef f(copy.fileName().c_str(), false);
      CPPUNIT_ASSERT_EQUAL(StringList({PICTURE_KEY, GEOB_KEY}),
        f.complexPropertyKeys());
      CPPUNIT_ASSERT(List<VariantMap>({TEST_PICTURE, picture2}) ==
        f.complexProperties(PICTURE_KEY));
      CPPUNIT_ASSERT(List<VariantMap>({geob1, geob2}) ==
        f.complexProperties(GEOB_KEY));
    }
  }

  void testSetGetId3Geob()
  {
    const VariantMap geob {
      {"data", ByteVector("Just a test")},
      {"mimeType", "text/plain"},
      {"description", "Embedded object"},
      {"fileName", "test.txt"}
    };
    ID3v2::Tag tag;
    CPPUNIT_ASSERT(!tag.frameListMap().contains("GEOB"));
    CPPUNIT_ASSERT(tag.complexPropertyKeys().isEmpty());
    CPPUNIT_ASSERT(tag.complexProperties(GEOB_KEY).isEmpty());
    CPPUNIT_ASSERT(tag.setComplexProperties(GEOB_KEY, {geob}));
    CPPUNIT_ASSERT_EQUAL(StringList(GEOB_KEY), tag.complexPropertyKeys());
    CPPUNIT_ASSERT_EQUAL(geob, tag.complexProperties(GEOB_KEY).front());
    auto frames = tag.frameListMap().value("GEOB");
    CPPUNIT_ASSERT_EQUAL(1U, frames.size());
    auto frame =
      dynamic_cast<ID3v2::GeneralEncapsulatedObjectFrame *>(frames.front());
    CPPUNIT_ASSERT(frame);
    CPPUNIT_ASSERT_EQUAL(geob.value("data").value<ByteVector>(), frame->object());
    CPPUNIT_ASSERT_EQUAL(geob.value("mimeType").value<String>(), frame->mimeType());
    CPPUNIT_ASSERT_EQUAL(geob.value("description").value<String>(), frame->description());
    CPPUNIT_ASSERT_EQUAL(geob.value("fileName").value<String>(), frame->fileName());
  }

  void tagSetGetPicture(Tag &tag, const VariantMap &picture)
  {
    CPPUNIT_ASSERT(tag.complexPropertyKeys().isEmpty());
    CPPUNIT_ASSERT(tag.complexProperties(PICTURE_KEY).isEmpty());
    CPPUNIT_ASSERT(tag.setComplexProperties(PICTURE_KEY, {picture}));
    CPPUNIT_ASSERT_EQUAL(StringList(PICTURE_KEY), tag.complexPropertyKeys());
    CPPUNIT_ASSERT_EQUAL(picture, tag.complexProperties(PICTURE_KEY).front());
  }

  void testSetGetId3Picture()
  {
    const VariantMap picture(TEST_PICTURE);
    ID3v2::Tag tag;
    CPPUNIT_ASSERT(!tag.frameListMap().contains("APIC"));
    tagSetGetPicture(tag, picture);
    auto frames = tag.frameListMap().value("APIC");
    CPPUNIT_ASSERT_EQUAL(1U, frames.size());
    auto frame =
      dynamic_cast<ID3v2::AttachedPictureFrame *>(frames.front());
    CPPUNIT_ASSERT(frame);
    CPPUNIT_ASSERT_EQUAL(picture.value("data").value<ByteVector>(), frame->picture());
    CPPUNIT_ASSERT_EQUAL(picture.value("mimeType").value<String>(), frame->mimeType());
    CPPUNIT_ASSERT_EQUAL(picture.value("description").value<String>(), frame->description());
    CPPUNIT_ASSERT_EQUAL(ID3v2::AttachedPictureFrame::FrontCover, frame->type());
  }

  void testSetGetApePicture()
  {
    const String FRONT_COVER("COVER ART (FRONT)");
    VariantMap picture(TEST_PICTURE);
    picture.erase("mimeType");
    APE::Tag tag;
    CPPUNIT_ASSERT(!tag.itemListMap().contains(FRONT_COVER));
    tagSetGetPicture(tag, picture);
    auto item = tag.itemListMap().value(FRONT_COVER);
    CPPUNIT_ASSERT_EQUAL(
      picture.value("description").value<String>().data(String::UTF8)
      .append('\0')
      .append(picture.value("data").value<ByteVector>()),
      item.binaryData());
  }

  void testSetGetAsfPicture()
  {
    VariantMap picture(TEST_PICTURE);
    ASF::Tag tag;
    CPPUNIT_ASSERT(!tag.attributeListMap().contains("WM/Picture"));
    tagSetGetPicture(tag, picture);
    auto attributes = tag.attribute("WM/Picture");
    CPPUNIT_ASSERT_EQUAL(1U, attributes.size());
    auto asfPicture = attributes.front().toPicture();
    CPPUNIT_ASSERT_EQUAL(picture.value("data").value<ByteVector>(),
      asfPicture.picture());
    CPPUNIT_ASSERT_EQUAL(picture.value("mimeType").value<String>(),
      asfPicture.mimeType());
    CPPUNIT_ASSERT_EQUAL(picture.value("description").value<String>(),
      asfPicture.description());
    CPPUNIT_ASSERT_EQUAL(ASF::Picture::FrontCover, asfPicture.type());
  }

  void testSetGetMp4Picture()
  {
    VariantMap picture(TEST_PICTURE);
    picture.erase("description");
    picture.erase("pictureType");
    MP4::Tag tag;
    CPPUNIT_ASSERT(!tag.itemMap().contains("covr"));
    tagSetGetPicture(tag, picture);
    auto covrs = tag.item("covr").toCoverArtList();
    CPPUNIT_ASSERT_EQUAL(1U, covrs.size());
    auto covr = covrs.front();
    CPPUNIT_ASSERT_EQUAL(picture.value("data").value<ByteVector>(),
      covr.data());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, covr.format());
  }

  void testSetGetXiphPicture()
  {
    VariantMap picture(TEST_PICTURE);
    picture.insert("colorDepth", 8);
    picture.insert("numColors", 1);
    picture.insert("width", 1);
    picture.insert("height", 1);
    Ogg::XiphComment tag;
    CPPUNIT_ASSERT(tag.pictureList().isEmpty());
    tagSetGetPicture(tag, picture);
    auto pics = tag.pictureList();
    CPPUNIT_ASSERT_EQUAL(1U, pics.size());
    auto pic = pics.front();
    CPPUNIT_ASSERT_EQUAL(picture.value("data").value<ByteVector>(),
      pic->data());
    CPPUNIT_ASSERT_EQUAL(picture.value("mimeType").value<String>(),
      pic->mimeType());
    CPPUNIT_ASSERT_EQUAL(picture.value("description").value<String>(),
      pic->description());
    CPPUNIT_ASSERT_EQUAL(FLAC::Picture::FrontCover, pic->type());
    CPPUNIT_ASSERT_EQUAL(8, pic->colorDepth());
    CPPUNIT_ASSERT_EQUAL(1, pic->numColors());
    CPPUNIT_ASSERT_EQUAL(1, pic->width());
    CPPUNIT_ASSERT_EQUAL(1, pic->height());
  }

  void testNonExistent()
  {
    {
      ID3v2::Tag tag;
      CPPUNIT_ASSERT(tag.complexPropertyKeys().isEmpty());
      CPPUNIT_ASSERT(tag.complexProperties(PICTURE_KEY).isEmpty());
      CPPUNIT_ASSERT(tag.complexProperties(GEOB_KEY).isEmpty());
      CPPUNIT_ASSERT(tag.complexProperties("NONEXISTENT").isEmpty());
      CPPUNIT_ASSERT(!tag.setComplexProperties("NONEXISTENT", {{{"description", "test"}}}));
      CPPUNIT_ASSERT(tag.complexProperties("NONEXISTENT").isEmpty());
      CPPUNIT_ASSERT(tag.setComplexProperties(PICTURE_KEY, {TEST_PICTURE}));
      CPPUNIT_ASSERT(!tag.complexProperties(PICTURE_KEY).isEmpty());
    }
    {
      ID3v1::Tag tag;
      CPPUNIT_ASSERT(tag.complexPropertyKeys().isEmpty());
      CPPUNIT_ASSERT(tag.complexProperties(PICTURE_KEY).isEmpty());
      CPPUNIT_ASSERT(tag.complexProperties(GEOB_KEY).isEmpty());
      CPPUNIT_ASSERT(tag.complexProperties("NONEXISTENT").isEmpty());
      CPPUNIT_ASSERT(!tag.setComplexProperties("NONEXISTENT", {{{"description", "test"}}}));
      CPPUNIT_ASSERT(tag.complexProperties("NONEXISTENT").isEmpty());
      CPPUNIT_ASSERT(!tag.setComplexProperties(PICTURE_KEY, {TEST_PICTURE}));
      CPPUNIT_ASSERT(tag.complexProperties(PICTURE_KEY).isEmpty());
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestComplexProperties);
