/***************************************************************************
    copyright           : (C) 2012 by Lukas Lalinsky
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
#include <flacunknownmetadatablock.h>
#include <flacpicture.h>
#include "utils.h"

using namespace TagLib;

namespace
{
  const ByteVector pictureData(
    "\x00\x00\x00\x03\x00\x00\x00\x09\x69\x6D\x61\x67\x65\x2F\x70\x6E"
    "\x67\x00\x00\x00\x08\x41\x20\x70\x69\x78\x65\x6C\x2E\x00\x00\x00"
    "\x01\x00\x00\x00\x01\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00"
    "\x96\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44"
    "\x52\x00\x00\x00\x01\x00\x00\x00\x01\x08\x02\x00\x00\x00\x90\x77"
    "\x53\xDE\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0B\x13\x00\x00"
    "\x0B\x13\x01\x00\x9A\x9C\x18\x00\x00\x00\x07\x74\x49\x4D\x45\x07"
    "\xD6\x0B\x1C\x0A\x36\x06\x08\x44\x3D\x32\x00\x00\x00\x1D\x74\x45"
    "\x58\x74\x43\x6F\x6D\x6D\x65\x6E\x74\x00\x43\x72\x65\x61\x74\x65"
    "\x64\x20\x77\x69\x74\x68\x20\x54\x68\x65\x20\x47\x49\x4D\x50\xEF"
    "\x64\x25\x6E\x00\x00\x00\x0C\x49\x44\x41\x54\x08\xD7\x63\xF8\xFF"
    "\xFF\x3F\x00\x05\xFE\x02\xFE\xDC\xCC\x59\xE7\x00\x00\x00\x00\x49"
    "\x45\x4E\x44\xAE\x42\x60\x82", 199);
}

TEST_CASE("FLAC metadata block")
{
  SECTION("Parse and render unknown block")
  {
    ByteVector data("abc\x01", 4);
    FLAC::UnknownMetadataBlock block(42, data);
    REQUIRE(block.code() == 42);
    REQUIRE(block.data() == data);
    REQUIRE(block.render() == data);
    ByteVector data2("xxx", 3);
    block.setCode(13);
    block.setData(data2);
    REQUIRE(block.code() == 13);
    REQUIRE(block.data() == data2);
    REQUIRE(block.render() == data2);
  }
  SECTION("Parse and render picture block")
  {
    const FLAC::Picture pic(pictureData);

    REQUIRE(pic.type() == FLAC::Picture::FrontCover);
    REQUIRE(pic.width() == 1);
    REQUIRE(pic.height() == 1);
    REQUIRE(pic.colorDepth() == 24);
    REQUIRE(pic.numColors() == 0);
    REQUIRE(pic.mimeType() == "image/png");
    REQUIRE(pic.description() == "A pixel.");
    REQUIRE(pic.data().size() == 150);
    REQUIRE(pic.render() == pictureData);
  }
}
