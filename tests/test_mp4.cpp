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
#include <mp4atom.h>
#include <mp4file.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("MP4 File")
{
  SECTION("Read audio properties (AAC)")
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3708);
    REQUIRE(f.audioProperties()->bitrate() == 3);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE_FALSE(f.audioProperties()->isEncrypted());
    REQUIRE(f.audioProperties()->codec() == MP4::Properties::AAC);
  }
  SECTION("Read audio properties (ALAC)")
  {
    MP4::File f(TEST_FILE_PATH_C("empty_alac.m4a"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 3);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 3);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 3705);
    REQUIRE(f.audioProperties()->bitrate() == 3);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE_FALSE(f.audioProperties()->isEncrypted());
    REQUIRE(f.audioProperties()->codec() == MP4::Properties::ALAC);
  }
  SECTION("Read audio properties (M4V)")
  {
    MP4::File f(TEST_FILE_PATH_C("blank_video.m4v"));
    REQUIRE(f.audioProperties());
    REQUIRE(f.audioProperties()->length() == 0);
    REQUIRE(f.audioProperties()->lengthInSeconds() == 0);
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 975);
    REQUIRE(f.audioProperties()->bitrate() == 96);
    REQUIRE(f.audioProperties()->channels() == 2);
    REQUIRE(f.audioProperties()->sampleRate() == 44100);
    REQUIRE(f.audioProperties()->bitsPerSample() == 16);
    REQUIRE_FALSE(f.audioProperties()->isEncrypted());
    REQUIRE(f.audioProperties()->codec() == MP4::Properties::AAC);
  }
  SECTION("Check invalid file")
  {
    MP4::File f(TEST_FILE_PATH_C("empty.aiff"));
    REQUIRE_FALSE(f.isValid());
  }
  SECTION("Check if file has tag")
  {
    {
      MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
      REQUIRE(f.isValid());
      REQUIRE(f.hasMP4Tag());
    }
    const ScopedFileCopy copy("no-tags", ".m4a");
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE_FALSE(f.hasMP4Tag());
      f.tag()->setTitle("TITLE");
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.isValid());
      REQUIRE(f.hasMP4Tag());
    }
  }
  SECTION("Check if tag is empty")
  {
    MP4::Tag t1;
    REQUIRE(t1.isEmpty());
    t1.setArtist("Foo");
    REQUIRE_FALSE(t1.isEmpty());

    MP4::Tag t2;
    t2.setItem("foo", "bar");
    REQUIRE_FALSE(t2.isEmpty());
  }
  SECTION("Update 'stco' atom")
  {
    const ScopedFileCopy copy("no-tags", ".3g2");

    ByteVectorList data1;
    {
      MP4::File f(copy.fileName().c_str());
      f.tag()->setArtist(ByteVector(3000, 'x'));

      MP4::Atoms a(&f);
      MP4::Atom *stco = a.find("moov")->findall("stco", true)[0];
      f.seek(stco->offset + 12);
      ByteVector data = f.readBlock(stco->length - 12);
      unsigned int count = data.mid(0, 4).toUInt();
      int pos = 4;
      while (count--) {
        unsigned int offset = data.mid(pos, 4).toUInt();
        f.seek(offset);
        data1.append(f.readBlock(20));
        pos += 4;
      }

      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());

      MP4::Atoms a(&f);
      MP4::Atom *stco = a.find("moov")->findall("stco", true)[0];
      f.seek(stco->offset + 12);
      ByteVector data = f.readBlock(stco->length - 12);
      unsigned int count = data.mid(0, 4).toUInt();
      int pos = 4, i = 0;
      while (count--) {
        unsigned int offset = data.mid(pos, 4).toUInt();
        f.seek(offset);
        REQUIRE(f.readBlock(20) == data1[i]);
        pos += 4;
        i++;
      }
    }
  }
  SECTION("Update free-form atom")
  {
    const ScopedFileCopy copy("has-tags", ".m4a");
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->contains("----:com.apple.iTunes:iTunNORM"));
      f.tag()->setItem("----:org.kde.TagLib:Foo", StringList("Bar"));
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->contains("----:org.kde.TagLib:Foo"));
      REQUIRE(f.tag()->item("----:org.kde.TagLib:Foo").toStringList().front() == "Bar");
      f.save();
    }
  }
  SECTION("Save exsting item when 'ilst' atom is at last")
  {
    const ScopedFileCopy copy("ilst-is-last", ".m4a");
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->item("----:com.apple.iTunes:replaygain_track_minmax").toStringList().front() == "82,164");
      REQUIRE(f.tag()->artist() == "Pearl Jam");
      f.tag()->setComment("foo");
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->item("----:com.apple.iTunes:replaygain_track_minmax").toStringList().front() == "82,164");
      REQUIRE(f.tag()->artist() == "Pearl Jam");
      REQUIRE(f.tag()->comment() == "foo");
    }
  }
  SECTION("Update atom with 64-bit length")
  {
    const ScopedFileCopy copy("64bit", ".mp4");
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->itemMap()["cpil"].toBool());

      MP4::Atoms atoms(&f);
      MP4::Atom *moov = atoms.atoms[0];
      REQUIRE(moov->length == 77);

      f.tag()->setItem("pgap", true);
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->item("cpil").toBool());
      REQUIRE(f.tag()->item("pgap").toBool());

      MP4::Atoms atoms(&f);
      MP4::Atom *moov = atoms.atoms[0];
      // original size + 'pgap' size + padding
      REQUIRE(moov->length == 77 + 25 + 974);
    }
  }
  SECTION("Read genre")
  {
    MP4::File f(TEST_FILE_PATH_C("gnre.m4a"));
    REQUIRE(f.tag()->genre() == "Ska");
  }
  SECTION("Read cover art (1)")
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    REQUIRE(f.tag()->contains("covr"));
    MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
    REQUIRE(l.size() == 2);
    REQUIRE(l[0].format() == MP4::CoverArt::PNG);
    REQUIRE(l[0].data().size() == 79);
    REQUIRE(l[1].format() == MP4::CoverArt::JPEG);
    REQUIRE(l[1].data().size() == 287);
  }
  SECTION("Read cover art (2)")
  {
    MP4::File f(TEST_FILE_PATH_C("covr-junk.m4a"));
    REQUIRE(f.tag()->contains("covr"));
    MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
    REQUIRE(l.size() == 2);
    REQUIRE(l[0].format() == MP4::CoverArt::PNG);
    REQUIRE(l[0].data().size() == 79);
    REQUIRE(l[1].format() == MP4::CoverArt::JPEG);
    REQUIRE(l[1].data().size() == 287);
  }
  SECTION("Write cover art")
  {
    const ScopedFileCopy copy("has-tags", ".m4a");
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->contains("covr"));
      MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
      l.append(MP4::CoverArt(MP4::CoverArt::PNG, "foo"));
      f.tag()->setItem("covr", l);
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      REQUIRE(f.tag()->contains("covr"));
      MP4::CoverArtList l = f.tag()->item("covr").toCoverArtList();
      REQUIRE(l.size() == 3);
      REQUIRE(l[0].format() == MP4::CoverArt::PNG);
      REQUIRE(l[0].data().size() == 79);
      REQUIRE(l[1].format() == MP4::CoverArt::JPEG);
      REQUIRE(l[1].data().size() == 287);
      REQUIRE(l[2].format() == MP4::CoverArt::PNG);
      REQUIRE(l[2].data().size() == 3);
    }
  }
  SECTION("Read and write property map (1)")
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    
    PropertyMap tags = f.properties();
    
    REQUIRE(tags["ARTIST"] == StringList("Test Artist"));
    
    tags["TRACKNUMBER"] = StringList("2/4");
    tags["DISCNUMBER"] = StringList("3/5");
    tags["BPM"] = StringList("123");
    tags["ARTIST"] = StringList("Foo Bar");
    tags["COMPILATION"] = StringList("1");
    f.setProperties(tags);
    
    tags = f.properties();
    
    REQUIRE(f.tag()->contains("trkn"));
    REQUIRE(f.tag()->item("trkn").toIntPair().first == 2);
    REQUIRE(f.tag()->item("trkn").toIntPair().second == 4);
    REQUIRE(tags["TRACKNUMBER"] == StringList("2/4"));
    
    REQUIRE(f.tag()->contains("disk"));
    REQUIRE(f.tag()->item("disk").toIntPair().first == 3);
    REQUIRE(f.tag()->item("disk").toIntPair().second == 5);
    REQUIRE(tags["DISCNUMBER"] == StringList("3/5"));
    
    REQUIRE(f.tag()->contains("tmpo"));
    REQUIRE(f.tag()->item("tmpo").toInt() == 123);
    REQUIRE(tags["BPM"] == StringList("123"));
    
    REQUIRE(f.tag()->contains("\251ART"));
    REQUIRE(f.tag()->item("\251ART").toStringList() == StringList("Foo Bar"));
    REQUIRE(tags["ARTIST"] == StringList("Foo Bar"));
    
    REQUIRE(f.tag()->contains("cpil"));
    REQUIRE(f.tag()->item("cpil").toBool());
    REQUIRE(tags["COMPILATION"] == StringList("1"));
    
    tags["COMPILATION"] = StringList("0");
    f.setProperties(tags);
    
    tags = f.properties();
    
    REQUIRE(f.tag()->contains("cpil"));
    REQUIRE_FALSE(f.tag()->item("cpil").toBool());
    REQUIRE(tags["COMPILATION"] == StringList("0"));
    
    // Empty properties do not result in access violations
    // when converting integers
    tags["TRACKNUMBER"] = StringList();
    tags["DISCNUMBER"] = StringList();
    tags["BPM"] = StringList();
    tags["COMPILATION"] = StringList();
    f.setProperties(tags);
  }
  SECTION("Read and write property map (2)")
  {
    MP4::File f(TEST_FILE_PATH_C("has-tags.m4a"));
    
    PropertyMap tags = f.properties();
    
    tags["WORK"] = StringList("Foo");
    tags["MOVEMENTNAME"] = StringList("Bar");
    tags["MOVEMENTNUMBER"] = StringList("2");
    tags["MOVEMENTCOUNT"] = StringList("3");
    tags["SHOWWORKMOVEMENT"] = StringList("1");
    f.setProperties(tags);
    
    tags = f.properties();
    
    REQUIRE(f.tag()->contains("\251wrk"));
    REQUIRE(f.tag()->item("\251wrk").toStringList() == StringList("Foo"));
    REQUIRE(tags["WORK"] == StringList("Foo"));
    
    REQUIRE(f.tag()->contains("\251mvn"));
    REQUIRE(f.tag()->item("\251mvn").toStringList() == StringList("Bar"));
    REQUIRE(tags["MOVEMENTNAME"] == StringList("Bar"));
    
    REQUIRE(f.tag()->contains("\251mvi"));
    REQUIRE(f.tag()->item("\251mvi").toInt() == 2);
    REQUIRE(tags["MOVEMENTNUMBER"] == StringList("2"));
    
    REQUIRE(f.tag()->contains("\251mvc"));
    REQUIRE(f.tag()->item("\251mvc").toInt() == 3);
    REQUIRE(tags["MOVEMENTCOUNT"] == StringList("3"));
    
    REQUIRE(f.tag()->contains("shwm"));
    REQUIRE(f.tag()->item("shwm").toBool());
    REQUIRE(tags["SHOWWORKMOVEMENT"] == StringList("1"));
    
    tags["SHOWWORKMOVEMENT"] = StringList("0");
    f.setProperties(tags);
    
    tags = f.properties();
    
    REQUIRE(f.tag()->contains("shwm"));
    REQUIRE_FALSE(f.tag()->item("shwm").toBool());
    REQUIRE(tags["SHOWWORKMOVEMENT"] == StringList("0"));
    
    tags["WORK"] = StringList();
    tags["MOVEMENTNAME"] = StringList();
    tags["MOVEMENTNUMBER"] = StringList();
    tags["MOVEMENTCOUNT"] = StringList();
    tags["SHOWWORKMOVEMENT"] = StringList();
    f.setProperties(tags);
  }
  SECTION("Open fuzzed file without crashing")
  {
    MP4::File f(TEST_FILE_PATH_C("infloop.m4a"));
    REQUIRE(f.isValid());
  }
  SECTION("Save tags repeatedly without breaking file")
  {
    const ScopedFileCopy copy("no-tags", ".m4a");
    
    MP4::File f(copy.fileName().c_str());
    f.tag()->setTitle("0123456789");
    f.save();
    f.save();
    REQUIRE(f.find("0123456789") == 2862);
    REQUIRE(f.find("0123456789", 2863) == -1);
  }
  SECTION("Read atom with zero-length")
  {
    MP4::File f(TEST_FILE_PATH_C("zero-length-mdat.m4a"));
    REQUIRE(f.isValid());
    REQUIRE(f.audioProperties()->lengthInMilliseconds() == 1115);
    REQUIRE(f.audioProperties()->sampleRate() == 22050);
  }
}
