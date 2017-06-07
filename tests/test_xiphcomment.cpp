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
#include <xiphcomment.h>
#include <vorbisfile.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Xiph comment")
{
  SECTION("Year")
  {
    Ogg::XiphComment cmt;
    REQUIRE(cmt.year() == 0);
    cmt.addField("YEAR", "2009");
    REQUIRE(cmt.year() == 2009);
    cmt.addField("DATE", "2008");
    REQUIRE(cmt.year() == 2008);
  }
  SECTION("Set year")
  {
    Ogg::XiphComment cmt;
    cmt.addField("YEAR", "2009");
    cmt.addField("DATE", "2008");
    cmt.setYear(1995);
    REQUIRE(cmt.fieldListMap()["YEAR"].isEmpty());
    REQUIRE(cmt.fieldListMap()["DATE"].front() == "1995");
  }
  SECTION("Track")
  {
    Ogg::XiphComment cmt;
    REQUIRE(cmt.track() == 0);
    cmt.addField("TRACKNUM", "7");
    REQUIRE(cmt.track() == 7);
    cmt.addField("TRACKNUMBER", "8");
    REQUIRE(cmt.track() == 8);
  }
  SECTION("Set track")
  {
    Ogg::XiphComment cmt;
    cmt.addField("TRACKNUM", "7");
    cmt.addField("TRACKNUMBER", "8");
    cmt.setTrack(3);
    REQUIRE(cmt.fieldListMap()["TRACKNUM"].isEmpty());
    REQUIRE(cmt.fieldListMap()["TRACKNUMBER"].front() == "3");
  }
  SECTION("Skip invalid keys")
  {
    Ogg::XiphComment cmt;
    cmt.addField("", "invalid key: empty string");
    cmt.addField("A=B", "invalid key: contains '='");
    cmt.addField("A~B", "invalid key: contains '~'");
    cmt.addField("A\x7F" "B", "invalid key: contains '\x7F'");
    cmt.addField(L"A\x3456" "B", "invalid key: Unicode");
    REQUIRE(cmt.fieldCount() == 0);
    REQUIRE(cmt.isEmpty());
  }
  SECTION("Skip invalid keys in PropertyMap")
  {
    PropertyMap map;
    map[""] = String("invalid key: empty string");
    map["A=B"] = String("invalid key: contains '='");
    map["A~B"] = String("invalid key: contains '~'");
    map["A\x7F" "B"] = String("invalid key: contains '\x7F'");
    map[L"A\x3456" "B"] = String("invalid key: Unicode");
    
    Ogg::XiphComment cmt;
    PropertyMap unsuccessful = cmt.setProperties(map);
    REQUIRE(unsuccessful.size() == 5);
    REQUIRE(cmt.properties().isEmpty());
  }
  SECTION("Clear comment")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    {
      Ogg::Vorbis::File f(copy.fileName().c_str());
      f.tag()->addField("COMMENT", "Comment1");
      f.save();
    }
    {
      Ogg::Vorbis::File f(copy.fileName().c_str());
      f.tag()->setComment("");
      REQUIRE(f.tag()->comment().isEmpty());
    }
  }
  SECTION("Remove fields")
  {
    Ogg::Vorbis::File f(TEST_FILE_PATH_C("empty.ogg"));
    f.tag()->addField("title", "Title1");
    f.tag()->addField("Title", "Title1", false);
    f.tag()->addField("titlE", "Title2", false);
    f.tag()->addField("TITLE", "Title3", false);
    f.tag()->addField("artist", "Artist1");
    f.tag()->addField("ARTIST", "Artist2", false);
    REQUIRE(f.tag()->title() == "Title1 Title1 Title2 Title3");
    REQUIRE(f.tag()->artist() == "Artist1 Artist2");
    
    f.tag()->removeFields("title", "Title1");
    REQUIRE(f.tag()->title() == "Title2 Title3");
    REQUIRE(f.tag()->artist() == "Artist1 Artist2");
    
    f.tag()->removeFields("Artist");
    REQUIRE(f.tag()->title() == "Title2 Title3");
    REQUIRE(f.tag()->artist().isEmpty());
    
    f.tag()->removeAllFields();
    REQUIRE(f.tag()->title().isEmpty());
    REQUIRE(f.tag()->artist().isEmpty());
    REQUIRE(f.tag()->vendorID() == "Xiph.Org libVorbis I 20050304");
  }
  SECTION("Remove fields")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    {
      Ogg::Vorbis::File f(copy.fileName().c_str());
      FLAC::Picture *newpic = new FLAC::Picture();
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
      Ogg::Vorbis::File f(copy.fileName().c_str());
      const List<FLAC::Picture *> lst = f.tag()->pictureList();
      REQUIRE(lst.size() == 1);
      REQUIRE(lst[0]->width() == 5);
      REQUIRE(lst[0]->height() == 6);
      REQUIRE(lst[0]->colorDepth() == 16);
      REQUIRE(lst[0]->numColors() == 7);
      REQUIRE(lst[0]->mimeType() == "image/jpeg");
      REQUIRE(lst[0]->description() == "new image");
      REQUIRE(lst[0]->data() == "JPEG data");
    }
  }
  SECTION("Field name in lower case")
  {
    const ScopedFileCopy copy("lowercase-fields", ".ogg");
    {
      Vorbis::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->title() == "TEST TITLE");
      REQUIRE(f.tag()->artist() == "TEST ARTIST");
      REQUIRE(f.tag()->pictureList().size() == 1);
      f.save();
    }
    {
      Vorbis::File f(copy.fileName().c_str());
      REQUIRE(f.find("METADATA_BLOCK_PICTURE") > 0);
    }
  }
}
