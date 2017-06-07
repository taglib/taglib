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

#include <catch/catch.hpp>
#include <id3v1tag.h>
#include <id3v1genres.h>
#include <mpegfile.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("ID3v1 Tag")
{
  SECTION("Strip whitespaces")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      f.ID3v1Tag(true)->setArtist("Artist     ");
      f.save();
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v1Tag());
      REQUIRE(f.ID3v1Tag(false)->artist() == "Artist");
    }
  }
  SECTION("Convert between genre names and numbers")
  {
    REQUIRE(ID3v1::genre(50) == "Darkwave");
    REQUIRE(ID3v1::genreIndex("Humour") == 100);
  }
}
