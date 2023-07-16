/***************************************************************************
    copyright           : (C) 2012 by Michael Helmling
    email               : helmling@mathematik.uni-kl.de
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

#include "tpropertymap.h"
#include "tag.h"
#include "apetag.h"
#include "asftag.h"
#include "id3v1tag.h"
#include "id3v2tag.h"
#include "infotag.h"
#include "mp4tag.h"
#include "xiphcomment.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace TagLib;

class TestPropertyMap : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestPropertyMap);
  CPPUNIT_TEST(testInvalidKeys);
  CPPUNIT_TEST(testGetSetApe);
  CPPUNIT_TEST(testGetSetAsf);
  CPPUNIT_TEST(testGetSetId3v1);
  CPPUNIT_TEST(testGetSetId3v2);
  CPPUNIT_TEST(testGetSetInfo);
  CPPUNIT_TEST(testGetSetMp4);
  CPPUNIT_TEST(testGetSetXiphComment);
  CPPUNIT_TEST(testGetSet);
  CPPUNIT_TEST_SUITE_END();

public:
  void testInvalidKeys()
  {
    PropertyMap map1;
    CPPUNIT_ASSERT(map1.isEmpty());
    map1[L"\x00c4\x00d6\x00dc"].append("test");
    CPPUNIT_ASSERT_EQUAL(map1.size(), 1u);

    PropertyMap map2;
    map2[L"\x00c4\x00d6\x00dc"].append("test");
    CPPUNIT_ASSERT(map1 == map2);
    CPPUNIT_ASSERT(map1.contains(map2));

    map2["ARTIST"] = String("Test Artist");
    CPPUNIT_ASSERT(map1 != map2);
    CPPUNIT_ASSERT(map2.contains(map1));

    map2[L"\x00c4\x00d6\x00dc"].append("test 2");
    CPPUNIT_ASSERT(!map2.contains(map1));

  }

  template <typename T>
  void tagGetSet()
  {
    T tag;

    tag.setTitle("Test Title");
    tag.setArtist("Test Artist");
    tag.setAlbum("Test Album");
    tag.setYear(2015);
    tag.setTrack(10);

    {
      PropertyMap prop = tag.properties();
      CPPUNIT_ASSERT_EQUAL(String("Test Title"),  prop["TITLE"      ].front());
      CPPUNIT_ASSERT_EQUAL(String("Test Artist"), prop["ARTIST"     ].front());
      CPPUNIT_ASSERT_EQUAL(String("Test Album"),  prop["ALBUM"      ].front());
      CPPUNIT_ASSERT_EQUAL(String("2015"),        prop["DATE"       ].front());
      CPPUNIT_ASSERT_EQUAL(String("10"),          prop["TRACKNUMBER"].front());

      prop["TITLE"      ].front() = "Test Title 2";
      prop["ARTIST"     ].front() = "Test Artist 2";
      prop["TRACKNUMBER"].front() = "5";

      tag.setProperties(prop);
    }

    CPPUNIT_ASSERT_EQUAL(String("Test Title 2"),  tag.title());
    CPPUNIT_ASSERT_EQUAL(String("Test Artist 2"), tag.artist());
    CPPUNIT_ASSERT_EQUAL(5U, tag.track());
    CPPUNIT_ASSERT(!tag.isEmpty());

    PropertyMap props = tag.properties();
    CPPUNIT_ASSERT_EQUAL(StringList("Test Artist 2"), props.find("ARTIST")->second);
    CPPUNIT_ASSERT(props.find("COMMENT") == props.end());
    props.replace("ARTIST", StringList("Test Artist 3"));
    CPPUNIT_ASSERT_EQUAL(StringList("Test Artist 3"), props["ARTIST"]);

    PropertyMap eraseMap;
    eraseMap.insert("ARTIST", StringList());
    eraseMap.insert("ALBUM", StringList());
    eraseMap.insert("TITLE", StringList());
    props.erase(eraseMap);
    CPPUNIT_ASSERT_EQUAL(String("DATE=2015\nTRACKNUMBER=5\n"), props.toString());

    tag.setProperties(PropertyMap());

    CPPUNIT_ASSERT(tag.isEmpty());
    CPPUNIT_ASSERT(tag.properties().isEmpty());
    CPPUNIT_ASSERT_EQUAL(String(""), tag.title());
    CPPUNIT_ASSERT_EQUAL(String(""), tag.artist());
    CPPUNIT_ASSERT_EQUAL(String(""), tag.album());
    CPPUNIT_ASSERT_EQUAL(String(""), tag.comment());
    CPPUNIT_ASSERT_EQUAL(String(""), tag.genre());
    CPPUNIT_ASSERT_EQUAL(0U, tag.track());
    CPPUNIT_ASSERT(tag.isEmpty());
    CPPUNIT_ASSERT(tag.properties().isEmpty());
  }

  void testGetSetId3v1()
  {
    tagGetSet<ID3v1::Tag>();
  }

  void testGetSetId3v2()
  {
    tagGetSet<ID3v2::Tag>();
  }

  void testGetSetXiphComment()
  {
    tagGetSet<Ogg::XiphComment>();
  }

  void testGetSetApe()
  {
    tagGetSet<APE::Tag>();
  }

  void testGetSetAsf()
  {
    tagGetSet<ASF::Tag>();
  }

  void testGetSetMp4()
  {
    tagGetSet<MP4::Tag>();
  }

  void testGetSetInfo()
  {
    tagGetSet<RIFF::Info::Tag>();
  }

  void testGetSet()
  {
    PropertyMap props;
    props["Title"] = String("Test Title");
    StringList artists("Artist 1");
    artists.append("Artist 2");
    props.insert("Artist", artists);
    CPPUNIT_ASSERT_EQUAL(StringList("Test Title"), props.value("TITLE"));
    CPPUNIT_ASSERT_EQUAL(StringList("Test Title"), props.value("Title"));
    CPPUNIT_ASSERT_EQUAL(StringList("Test Title"), props["TITLE"]);
    CPPUNIT_ASSERT_EQUAL(StringList("Test Title"), props["Title"]);
    CPPUNIT_ASSERT(props.contains("title"));
    CPPUNIT_ASSERT_EQUAL(StringList("Test Title"), props.find("TITLE")->second);
    CPPUNIT_ASSERT_EQUAL(2U, props.size());
    CPPUNIT_ASSERT(!props.isEmpty());
    props.clear();
    CPPUNIT_ASSERT(props.isEmpty());
    CPPUNIT_ASSERT_EQUAL(StringList(), props.value("TITLE"));
    CPPUNIT_ASSERT_EQUAL(StringList(), props.value("Title"));
    CPPUNIT_ASSERT_EQUAL(artists, props.value("Title", artists));
    CPPUNIT_ASSERT(!props.contains("title"));
    CPPUNIT_ASSERT(props.find("TITLE") == props.end());
    CPPUNIT_ASSERT_EQUAL(0U, props.size());
    CPPUNIT_ASSERT(props.isEmpty());
    CPPUNIT_ASSERT_EQUAL(StringList(), props["TITLE"]);
    CPPUNIT_ASSERT_EQUAL(StringList(), props["Title"]);
    CPPUNIT_ASSERT(props.contains("title"));
    CPPUNIT_ASSERT(props.find("TITLE") != props.end());
    CPPUNIT_ASSERT_EQUAL(1U, props.size());
    CPPUNIT_ASSERT(!props.isEmpty());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestPropertyMap);
