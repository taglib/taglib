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
#include <vorbisfile.h>
#include <oggpageheader.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Ogg Vorbis File")
{
  SECTION("Read audio properties")
  {
    Vorbis::File f(TEST_FILE_PATH_C("empty.ogg"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3685);
    REQUIRE(f.audioProperties()->bitrate() == 9);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->vorbisVersion() == 0);
    REQUIRE(f.audioProperties()->bitrateMaximum() == 0);
    REQUIRE(f.audioProperties()->bitrateNominal() == 112000);
    REQUIRE(f.audioProperties()->bitrateMinimum() == 0);
  }
  SECTION("Read and write simple values")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    {
      Vorbis::File f(copy.fileName().c_str());
      f.tag()->setArtist("The Artist");
      f.save();
    }
    {
      Vorbis::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->artist() == "The Artist");
    }
  }
  SECTION("Split and merge packets (1)")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    const String text = longText(128 * 1024, true);
    {
      Vorbis::File f(copy.fileName().c_str());
      f.tag()->setTitle(text);
      f.save();
    }
    {
      Vorbis::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.length() == 136383);
      REQUIRE(f.lastPageHeader()->pageSequenceNumber() == 19);
      REQUIRE(f.packet(0).size() == 30);
      REQUIRE(f.packet(1).size() == 131127);
      REQUIRE(f.packet(2).size() == 3832);
      REQUIRE(f.tag()->title() == text);

      REQUIRE(f.audioProperties());
      REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3685);

      f.tag()->setTitle("ABCDE");
      f.save();
    }
    {
      Vorbis::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.length() == 4370);
      REQUIRE(f.lastPageHeader()->pageSequenceNumber() == 3);
      REQUIRE(f.packet(0).size() == 30);
      REQUIRE(f.packet(1).size() == 60);
      REQUIRE(f.packet(2).size() == 3832);
      REQUIRE(f.tag()->title() == "ABCDE");

      REQUIRE(f.audioProperties());
      REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3685);
    }
  }
  SECTION("Split and merge packets (2)")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    const String text = longText(60890, true);
    {
      Vorbis::File f(copy.fileName().c_str());
      f.tag()->setTitle(text);
      f.save();
    }
    {
      Vorbis::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.tag()->title() == text);
    
      f.tag()->setTitle("ABCDE");
      f.save();
    }
    {
      Vorbis::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.tag()->title() == "ABCDE");
    }
  }
  SECTION("Update page checksum")
  {
    const ScopedFileCopy copy("empty", ".ogg");
    {
      Vorbis::File f(copy.fileName().c_str());
      f.tag()->setArtist("The Artist");
      f.save();
    
      f.seek(0x50);
      REQUIRE(f.readBlock(4).toUInt(0, true) == 0x3D3BD92DU);
    }
    {
      Vorbis::File f(copy.fileName().c_str());
      f.tag()->setArtist("The Artist 2");
      f.save();
    
      f.seek(0x50);
      REQUIRE(f.readBlock(4).toUInt(0, true) == 0xD985291CU);
    }
  }
  SECTION("Read and write property map (1)")
  {
    const ScopedFileCopy copy("empty", ".ogg");

    Vorbis::File f(copy.fileName().c_str());
    REQUIRE(f.tag()->properties().isEmpty());
    
    PropertyMap newTags;
    StringList values("value 1");
    values.append("value 2");
    newTags["ARTIST"] = values;
    f.tag()->setProperties(newTags);
    
    PropertyMap map = f.tag()->properties();
    REQUIRE(map.size() == 1);
    REQUIRE(map["ARTIST"].size() == 2);
    REQUIRE(map["ARTIST"][0] == "value 1");
    REQUIRE(map["ARTIST"][1] == "value 2");
  }
  SECTION("Read and write property map (2)")
  {
    const ScopedFileCopy copy("test", ".ogg");
    
    Vorbis::File f(copy.fileName().c_str());
    PropertyMap tags = f.tag()->properties();
    
    REQUIRE(tags["UNUSUALTAG"].size() == 2);
    REQUIRE(tags["UNUSUALTAG"][0] == "usual value");
    REQUIRE(tags["UNUSUALTAG"][1] == "another value");
    REQUIRE(tags["UNICODETAG"][0] == String("\xC3\xB6\xC3\xA4\xC3\xBC\x6F\xCE\xA3\xC3\xB8", String::UTF8));
          
    tags["UNICODETAG"][0] 
      = String("\xCE\xBD\xCE\xB5\xCF\x89\x20\xCE\xBD\xCE\xB1\xCE\xBB\xCF\x85\xCE\xB5", String::UTF8);
    tags.erase("UNUSUALTAG");
    f.tag()->setProperties(tags);
    REQUIRE(f.tag()->properties()["UNICODETAG"][0]
      == String("\xCE\xBD\xCE\xB5\xCF\x89\x20\xCE\xBD\xCE\xB1\xCE\xBB\xCF\x85\xCE\xB5", String::UTF8));
    REQUIRE_FALSE(f.tag()->properties().contains("UNUSUALTAG"));
  }
}
