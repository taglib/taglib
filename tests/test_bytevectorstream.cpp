/***************************************************************************
    copyright           : (C) 2011 by Lukas Lalinsky
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
#include <tbytevectorstream.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("ByteVectorStream")
{
  SECTION("Initial data")
  {
    ByteVectorStream stream("abcd");
    REQUIRE(*stream.data() == "abcd");
  }
  SECTION("Write block")
  {
    ByteVectorStream stream("abcd");
    stream.seek(1);
    stream.writeBlock("xx");
    REQUIRE(*stream.data() == "axxd");
  }
  SECTION("Write block entails resizing")
  {
    ByteVectorStream stream("abcd");
    stream.seek(3);
    stream.writeBlock("xx");
    REQUIRE(*stream.data() == "abcxx");
    stream.seek(5);
    stream.writeBlock("yy");
    REQUIRE(*stream.data() == "abcxxyy");
  }
  SECTION("Read block")
  {
    ByteVectorStream stream("abcd");
    REQUIRE(stream.readBlock(1) == "a");
    REQUIRE(stream.readBlock(2) == "bc");
    REQUIRE(stream.readBlock(3) == "d");
    REQUIRE(stream.readBlock(3) == "");
  }
  SECTION("Remove block")
  {
    ByteVectorStream stream("abcd");
    stream.removeBlock(1, 1);
    REQUIRE(*stream.data() == "acd");
    stream.removeBlock(0, 2);
    REQUIRE(*stream.data() == "d");
    stream.removeBlock(0, 2);
    REQUIRE(*stream.data() == "");
  }
  SECTION("Insert block")
  {
    ByteVectorStream stream("abcd");
    stream.insert("xx", 1, 1);
    REQUIRE(*stream.data() == "axxcd");
    stream.insert("yy", 0, 2);
    REQUIRE(*stream.data() == "yyxcd");
    stream.insert("foa", 3, 2);
    REQUIRE(*stream.data() == "yyxfoa");
    stream.insert("123", 3, 0);
    REQUIRE(*stream.data() == "yyx123foa");
  }
  SECTION("Seek from the end")
  {
    ByteVectorStream stream("abcdefghijklmnopqrstuvwxyz");
    REQUIRE(stream.length() == 26);
    stream.seek(-4, IOStream::End);
    REQUIRE(stream.readBlock(2) == "wx");
    stream.seek(-25, IOStream::End);
    REQUIRE(stream.readBlock(2) == "bc");
  }
}
