/***************************************************************************
    copyright           : (C) 2007 by Lukas Lalinsky
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

#include "id3v1genres.h"
#include "id3v1tag.h"
#include "mpegfile.h"
#include "tstring.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(ID3v1, testStripWhiteSpace)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  {
    MPEG::File f(newname.c_str());
    f.ID3v1Tag(true)->setArtist("Artist     ");
    f.save();
  }

  {
    MPEG::File f(newname.c_str());
    ASSERT_TRUE(f.ID3v1Tag(false));
    ASSERT_EQ(String("Artist"), f.ID3v1Tag(false)->artist());
  }
}

TEST(ID3v1, testGenres)
{
  ASSERT_EQ(String("Darkwave"), ID3v1::genre(50));
  ASSERT_EQ(100, ID3v1::genreIndex("Humour"));
  ASSERT_TRUE(ID3v1::genreList().contains("Heavy Metal"));
  ASSERT_EQ(79, ID3v1::genreMap()["Hard Rock"]);
}

TEST(ID3v1, testRenamedGenres)
{
  ASSERT_EQ(String("Bebop"), ID3v1::genre(85));
  ASSERT_EQ(85, ID3v1::genreIndex("Bebop"));
  ASSERT_EQ(85, ID3v1::genreIndex("Bebob"));

  ID3v1::Tag tag;
  tag.setGenre("Hardcore");
  ASSERT_EQ(String("Hardcore Techno"), tag.genre());
  ASSERT_EQ(129U, tag.genreNumber());
}
