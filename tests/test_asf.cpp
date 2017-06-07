/***************************************************************************
    copyright           : (C) 2008 by Lukas Lalinsky
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
#include <asffile.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("ASF File")
{
  SECTION("Read audio properties (WMA)")
  {
    ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3712);
    REQUIRE(f.audioProperties()->bitrate() == 64);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 48000);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->codec() == ASF::Properties::WMA2);
    REQUIRE(f.audioProperties()->codecName() == "Windows Media Audio 9.1");
    REQUIRE(f.audioProperties()->codecDescription() == "64 kbps, 48 kHz, stereo 2-pass CBR");
    REQUIRE_FALSE(f.audioProperties()->isEncrypted());
  }
  SECTION("Read audio properties (WMA Lossless)")
  {
    ASF::File f(TEST_FILE_PATH_C("lossless.wma"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3549);
    REQUIRE(f.audioProperties()->bitrate() == 1152);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE(f.audioProperties()->codec() == ASF::Properties::WMA9Lossless);
    REQUIRE(f.audioProperties()->codecName() == "Windows Media Audio 9.2 Lossless");
    REQUIRE(f.audioProperties()->codecDescription() == "VBR Quality 100, 44 kHz, 2 channel 16 bit 1-pass VBR");
    REQUIRE_FALSE(f.audioProperties()->isEncrypted());
  }
  SECTION("Read tag")
  {
    ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));
    REQUIRE(f.tag()->title() == "test");
  }
  SECTION("Save multiple values")
  {
    const ScopedFileCopy copy("silence-1", ".wma");
    {
      ASF::File f(copy.fileName().c_str());
      ASF::AttributeList values;
      values.append("Foo");
      values.append("Bar");
      f.tag()->setAttribute("WM/AlbumTitle", values);
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->attributeListMap()["WM/AlbumTitle"].size() == 2);
    }
  }
  SECTION("Read and write DWORD track number")
  {
    const ScopedFileCopy copy("silence-1", ".wma");
    {
      ASF::File f(copy.fileName().c_str());
      REQUIRE_FALSE(f.tag()->contains("WM/TrackNumber"));
      f.tag()->setAttribute("WM/TrackNumber", 123U);
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->contains("WM/TrackNumber"));
      REQUIRE(f.tag()->attribute("WM/TrackNumber").front().type() == ASF::Attribute::DWordType);
      REQUIRE(f.tag()->track() == 123);
      f.tag()->setTrack(234);
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->contains("WM/TrackNumber"));
      REQUIRE(f.tag()->attribute("WM/TrackNumber").front().type() == ASF::Attribute::UnicodeType);
      REQUIRE(f.tag()->track() == 234);
    }
  }
  SECTION("Save stream")
  {
    const ScopedFileCopy copy("silence-1", ".wma");
    {
      ASF::File f(copy.fileName().c_str());
      ASF::Attribute attr("Foo");
      attr.setStream(43);
      f.tag()->setAttribute("WM/AlbumTitle", attr);
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->attribute("WM/AlbumTitle").front().stream() == 43);
    }
  }
  SECTION("Save language")
  {
    const ScopedFileCopy copy("silence-1", ".wma");
    {
      ASF::File f(copy.fileName().c_str());
      ASF::Attribute attr("Foo");
      attr.setStream(32);
      attr.setLanguage(56);
      f.tag()->setAttribute("WM/AlbumTitle", attr);
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->attribute("WM/AlbumTitle").front().stream() == 32);
      REQUIRE(f.tag()->attribute("WM/AlbumTitle").front().language() == 56);
    }
  }
  SECTION("Save large value")
  {
    const ScopedFileCopy copy("silence-1", ".wma");
    const ByteVector blob(70000, 'x');
    {
      ASF::File f(copy.fileName().c_str());
      ASF::Attribute attr(blob);
      f.tag()->setAttribute("WM/Blob", attr);
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->attribute("WM/Blob").front().toByteVector() == blob);
    }
  }
  SECTION("Save picture")
  {
    const ScopedFileCopy copy("silence-1", ".wma");
    {
      ASF::File f(copy.fileName().c_str());
      ASF::Picture picture;
      picture.setMimeType("image/jpeg");
      picture.setType(ASF::Picture::FrontCover);
      picture.setDescription("description");
      picture.setPicture("data");
      f.tag()->setAttribute("WM/Picture", picture);
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      ASF::AttributeList values2 = f.tag()->attribute("WM/Picture");
      REQUIRE(values2.size() == (unsigned int)1);
      ASF::Attribute attr2 = values2.front();
      ASF::Picture picture2 = attr2.toPicture();
      REQUIRE(picture2.isValid());
      REQUIRE(picture2.mimeType() == "image/jpeg");
      REQUIRE(picture2.type() == ASF::Picture::FrontCover);
      REQUIRE(picture2.description() == "description");
      REQUIRE(picture2.picture() == "data");
    }
  }
  SECTION("Save multiple pictures")
  {
    const ScopedFileCopy copy("silence-1", ".wma");
    {
      ASF::File f(copy.fileName().c_str());
      ASF::AttributeList values;
      ASF::Picture picture;
      picture.setMimeType("image/jpeg");
      picture.setType(ASF::Picture::FrontCover);
      picture.setDescription("description");
      picture.setPicture("data");
      values.append(ASF::Attribute(picture));
      ASF::Picture picture2;
      picture2.setMimeType("image/png");
      picture2.setType(ASF::Picture::BackCover);
      picture2.setDescription("back cover");
      picture2.setPicture("PNG data");
      values.append(ASF::Attribute(picture2));
      f.tag()->setAttribute("WM/Picture", values);
      f.save();
    }
    {
      ASF::File f(copy.fileName().c_str());
      ASF::AttributeList values2 = f.tag()->attribute("WM/Picture");
      REQUIRE(values2.size() == (unsigned int)2);
      ASF::Picture picture3 = values2[1].toPicture();
      REQUIRE(picture3.isValid());
      REQUIRE(picture3.mimeType() == "image/jpeg");
      REQUIRE(picture3.type() == ASF::Picture::FrontCover);
      REQUIRE(picture3.description() == "description");
      REQUIRE(picture3.picture() == "data");
      ASF::Picture picture4 = values2[0].toPicture();
      REQUIRE(picture4.isValid());
      REQUIRE(picture4.mimeType() == "image/png");
      REQUIRE(picture4.type() == ASF::Picture::BackCover);
      REQUIRE(picture4.description() == "back cover");
      REQUIRE(picture4.picture() == "PNG data");
    }
  }
  SECTION("Read and write property map")
  {
    ASF::File f(TEST_FILE_PATH_C("silence-1.wma"));
    
    PropertyMap tags = f.properties();
    
    tags["TRACKNUMBER"] = StringList("2");
    tags["DISCNUMBER"] = StringList("3");
    tags["BPM"] = StringList("123");
    tags["ARTIST"] = StringList("Foo Bar");
    f.setProperties(tags);
    
    tags = f.properties();
    
    REQUIRE(f.tag()->artist() == String("Foo Bar"));
    REQUIRE(tags["ARTIST"] == StringList("Foo Bar"));
    
    REQUIRE(f.tag()->contains("WM/BeatsPerMinute"));
    REQUIRE(f.tag()->attributeListMap()["WM/BeatsPerMinute"].size() == 1);
    REQUIRE(f.tag()->attribute("WM/BeatsPerMinute").front().toString() == "123");
    REQUIRE(tags["BPM"] == StringList("123"));
    
    REQUIRE(f.tag()->contains("WM/TrackNumber"));
    REQUIRE(f.tag()->attributeListMap()["WM/TrackNumber"].size() == 1);
    REQUIRE(f.tag()->attribute("WM/TrackNumber").front().toString() == "2");
    REQUIRE(tags["TRACKNUMBER"] == StringList("2"));
    
    REQUIRE(f.tag()->contains("WM/PartOfSet"));
    REQUIRE(f.tag()->attributeListMap()["WM/PartOfSet"].size() == 1);
    REQUIRE(f.tag()->attribute("WM/PartOfSet").front().toString() == "3");
    REQUIRE(tags["DISCNUMBER"] == StringList("3"));
  }
  SECTION("Save tags repeatedly without breaking file")
  {
    const ScopedFileCopy copy("silence-1", ".wma");
    {
      ASF::File f(copy.fileName().c_str());
      f.tag()->setTitle(longText(128 * 1024));
      f.save();
      REQUIRE(f.length() == 297578);
      f.tag()->setTitle(longText(16 * 1024));
      f.save();
      REQUIRE(f.length() == 68202);
    }
  }
}
