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

#include "apetag.h"
#include "asftag.h"
#include "id3v1tag.h"
#include "id3v2tag.h"
#include "infotag.h"
#include "mp4tag.h"
#include "tag.h"
#include "tpropertymap.h"
#include "utils.h"
#include "xiphcomment.h"
#include <gtest/gtest.h>

using namespace TagLib;

TEST(PropertyMap, testInvalidKeys)
{
  PropertyMap map1;
  ASSERT_TRUE(map1.isEmpty());
  map1[L"\x00c4\x00d6\x00dc"].append("test");
  ASSERT_EQ(map1.size(), 1u);

  PropertyMap map2;
  map2[L"\x00c4\x00d6\x00dc"].append("test");
  ASSERT_EQ(map1, map2);
  ASSERT_TRUE(map1.contains(map2));

  map2["ARTIST"] = String("Test Artist");
  ASSERT_NE(map1, map2);
  ASSERT_TRUE(map2.contains(map1));

  map2[L"\x00c4\x00d6\x00dc"].append("test 2");
  ASSERT_FALSE(map2.contains(map1));
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
    ASSERT_EQ(String("Test Title"), prop["TITLE"].front());
    ASSERT_EQ(String("Test Artist"), prop["ARTIST"].front());
    ASSERT_EQ(String("Test Album"), prop["ALBUM"].front());
    ASSERT_EQ(String("2015"), prop["DATE"].front());
    ASSERT_EQ(String("10"), prop["TRACKNUMBER"].front());

    prop["TITLE"].front()       = "Test Title 2";
    prop["ARTIST"].front()      = "Test Artist 2";
    prop["TRACKNUMBER"].front() = "5";

    tag.setProperties(prop);
  }

  ASSERT_EQ(String("Test Title 2"), tag.title());
  ASSERT_EQ(String("Test Artist 2"), tag.artist());
  ASSERT_EQ(5U, tag.track());
  ASSERT_FALSE(tag.isEmpty());

  PropertyMap props = tag.properties();
  ASSERT_EQ(StringList("Test Artist 2"), props.find("ARTIST")->second);
  ASSERT_EQ(props.end(), props.find("COMMENT"));
  props.replace("ARTIST", StringList("Test Artist 3"));
  ASSERT_EQ(StringList("Test Artist 3"), props["ARTIST"]);

  PropertyMap eraseMap;
  eraseMap.insert("ARTIST", StringList());
  eraseMap.insert("ALBUM", StringList());
  eraseMap.insert("TITLE", StringList());
  props.erase(eraseMap);
  ASSERT_EQ(String("DATE=2015\nTRACKNUMBER=5\n"), props.toString());

  tag.setProperties(PropertyMap());

  ASSERT_TRUE(tag.isEmpty());
  ASSERT_TRUE(tag.properties().isEmpty());
  ASSERT_EQ(String(""), tag.title());
  ASSERT_EQ(String(""), tag.artist());
  ASSERT_EQ(String(""), tag.album());
  ASSERT_EQ(String(""), tag.comment());
  ASSERT_EQ(String(""), tag.genre());
  ASSERT_EQ(0U, tag.track());
  ASSERT_TRUE(tag.isEmpty());
  ASSERT_TRUE(tag.properties().isEmpty());
}

TEST(PropertyMap, testGetSetId3v1)
{
  tagGetSet<ID3v1::Tag>();
}

TEST(PropertyMap, testGetSetId3v2)
{
  tagGetSet<ID3v2::Tag>();
}

TEST(PropertyMap, testGetSetXiphComment)
{
  tagGetSet<Ogg::XiphComment>();
}

TEST(PropertyMap, testGetSetApe)
{
  tagGetSet<APE::Tag>();
}

TEST(PropertyMap, testGetSetAsf)
{
  tagGetSet<ASF::Tag>();
}

TEST(PropertyMap, testGetSetMp4)
{
  tagGetSet<MP4::Tag>();
}

TEST(PropertyMap, testGetSetInfo)
{
  tagGetSet<RIFF::Info::Tag>();
}

TEST(PropertyMap, testGetSet)
{
  PropertyMap props;
  props["Title"] = String("Test Title");
  StringList artists("Artist 1");
  artists.append("Artist 2");
  props.insert("Artist", artists);
  ASSERT_EQ(StringList("Test Title"), props.value("TITLE"));
  ASSERT_EQ(StringList("Test Title"), props.value("Title"));
  ASSERT_EQ(StringList("Test Title"), props["TITLE"]);
  ASSERT_EQ(StringList("Test Title"), props["Title"]);
  ASSERT_TRUE(props.contains("title"));
  ASSERT_EQ(StringList("Test Title"), props.find("TITLE")->second);
  ASSERT_EQ(2U, props.size());
  ASSERT_FALSE(props.isEmpty());
  props.clear();
  ASSERT_TRUE(props.isEmpty());
  ASSERT_EQ(StringList(), props.value("TITLE"));
  ASSERT_EQ(StringList(), props.value("Title"));
  ASSERT_EQ(artists, props.value("Title", artists));
  ASSERT_FALSE(props.contains("title"));
  ASSERT_EQ(props.end(), props.find("TITLE"));
  ASSERT_EQ(0U, props.size());
  ASSERT_TRUE(props.isEmpty());
  ASSERT_EQ(StringList(), props["TITLE"]);
  ASSERT_EQ(StringList(), props["Title"]);
  ASSERT_TRUE(props.contains("title"));
  ASSERT_NE(props.end(), props.find("TITLE"));
  ASSERT_EQ(1U, props.size());
  ASSERT_FALSE(props.isEmpty());
}
