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

#include "tdebug.h"
#include "tpropertymap.h"
#include "utils.h"
#include "vorbisfile.h"
#include "xiphcomment.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(XiphComment, testYear)
{
  Ogg::XiphComment cmt;
  ASSERT_EQ(static_cast<unsigned int>(0), cmt.year());
  cmt.addField("YEAR", "2009");
  ASSERT_EQ(static_cast<unsigned int>(2009), cmt.year());
  cmt.addField("DATE", "2008");
  ASSERT_EQ(static_cast<unsigned int>(2008), cmt.year());
}

TEST(XiphComment, testSetYear)
{
  Ogg::XiphComment cmt;
  cmt.addField("YEAR", "2009");
  cmt.addField("DATE", "2008");
  cmt.setYear(1995);
  ASSERT_TRUE(cmt.fieldListMap()["YEAR"].isEmpty());
  ASSERT_EQ(String("1995"), cmt.fieldListMap()["DATE"].front());
}

TEST(XiphComment, testTrack)
{
  Ogg::XiphComment cmt;
  ASSERT_EQ(static_cast<unsigned int>(0), cmt.track());
  cmt.addField("TRACKNUM", "7");
  ASSERT_EQ(static_cast<unsigned int>(7), cmt.track());
  cmt.addField("TRACKNUMBER", "8");
  ASSERT_EQ(static_cast<unsigned int>(8), cmt.track());
}

TEST(XiphComment, testSetTrack)
{
  Ogg::XiphComment cmt;
  cmt.addField("TRACKNUM", "7");
  cmt.addField("TRACKNUMBER", "8");
  cmt.setTrack(3);
  ASSERT_TRUE(cmt.fieldListMap()["TRACKNUM"].isEmpty());
  ASSERT_EQ(String("3"), cmt.fieldListMap()["TRACKNUMBER"].front());
}

TEST(XiphComment, testInvalidKeys1)
{
  PropertyMap map;
  map[""]    = String("invalid key: empty string");
  map["A=B"] = String("invalid key: contains '='");
  map["A~B"] = String("invalid key: contains '~'");
  map["A\x7F"
      "B"]
    = String("invalid key: contains '\x7F'");
  map[L"A\x3456"
      "B"]
    = String("invalid key: Unicode");

  Ogg::XiphComment cmt;
  PropertyMap unsuccessful = cmt.setProperties(map);
  ASSERT_EQ(static_cast<unsigned int>(5), unsuccessful.size());
  ASSERT_TRUE(cmt.properties().isEmpty());
}

TEST(XiphComment, testInvalidKeys2)
{
  Ogg::XiphComment cmt;
  cmt.addField("", "invalid key: empty string");
  cmt.addField("A=B", "invalid key: contains '='");
  cmt.addField("A~B", "invalid key: contains '~'");
  cmt.addField("A\x7F"
               "B",
               "invalid key: contains '\x7F'");
  cmt.addField(L"A\x3456"
               "B",
               "invalid key: Unicode");
  ASSERT_EQ(0U, cmt.fieldCount());
}

TEST(XiphComment, testClearComment)
{
  ScopedFileCopy copy("empty", ".ogg");

  {
    Ogg::Vorbis::File f(copy.fileName().c_str());
    f.tag()->addField("COMMENT", "Comment1");
    f.save();
  }
  {
    Ogg::Vorbis::File f(copy.fileName().c_str());
    f.tag()->setComment("");
    ASSERT_EQ(String(""), f.tag()->comment());
  }
}

TEST(XiphComment, testRemoveFields)
{
  Ogg::Vorbis::File f(TEST_FILE_PATH_C("empty.ogg"));
  f.tag()->addField("title", "Title1");
  f.tag()->addField("Title", "Title1", false);
  f.tag()->addField("titlE", "Title2", false);
  f.tag()->addField("TITLE", "Title3", false);
  f.tag()->addField("artist", "Artist1");
  f.tag()->addField("ARTIST", "Artist2", false);
  ASSERT_EQ(String("Title1 Title1 Title2 Title3"), f.tag()->title());
  ASSERT_EQ(String("Artist1 Artist2"), f.tag()->artist());

  f.tag()->removeFields("title", "Title1");
  ASSERT_EQ(String("Title2 Title3"), f.tag()->title());
  ASSERT_EQ(String("Artist1 Artist2"), f.tag()->artist());

  f.tag()->removeFields("Artist");
  ASSERT_EQ(String("Title2 Title3"), f.tag()->title());
  ASSERT_TRUE(f.tag()->artist().isEmpty());

  f.tag()->removeAllFields();
  ASSERT_TRUE(f.tag()->title().isEmpty());
  ASSERT_TRUE(f.tag()->artist().isEmpty());
  ASSERT_EQ(String("Xiph.Org libVorbis I 20050304"), f.tag()->vendorID());
}

TEST(XiphComment, testPicture)
{
  ScopedFileCopy copy("empty", ".ogg");
  string newname = copy.fileName();

  {
    Vorbis::File f(newname.c_str());
    auto newpic = new FLAC::Picture();
    newpic->setType(FLAC::Picture::BackCover);
    newpic->setWidth(5);
    newpic->setHeight(6);
    newpic->setColorDepth(16);
    newpic->setNumColors(7);
    newpic->setMimeType("image/jpeg");
    newpic->setDescription("new image");
    newpic->setData("JPEG data");
    f.tag()->addPicture(newpic);
    f.save();
  }
  {
    Vorbis::File f(newname.c_str());
    List<FLAC::Picture *> lst = f.tag()->pictureList();
    ASSERT_EQ(static_cast<unsigned int>(1), lst.size());
    ASSERT_EQ(static_cast<int>(5), lst[0]->width());
    ASSERT_EQ(static_cast<int>(6), lst[0]->height());
    ASSERT_EQ(static_cast<int>(16), lst[0]->colorDepth());
    ASSERT_EQ(static_cast<int>(7), lst[0]->numColors());
    ASSERT_EQ(String("image/jpeg"), lst[0]->mimeType());
    ASSERT_EQ(String("new image"), lst[0]->description());
    ASSERT_EQ(ByteVector("JPEG data"), lst[0]->data());
  }
}

TEST(XiphComment, testLowercaseFields)
{
  const ScopedFileCopy copy("lowercase-fields", ".ogg");
  {
    Vorbis::File f(copy.fileName().c_str());
    List<FLAC::Picture *> lst = f.tag()->pictureList();
    ASSERT_EQ(String("TEST TITLE"), f.tag()->title());
    ASSERT_EQ(String("TEST ARTIST"), f.tag()->artist());
    ASSERT_EQ(static_cast<unsigned int>(1), lst.size());
    f.save();
  }
  {
    Vorbis::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.find("METADATA_BLOCK_PICTURE") > 0);
  }
}
