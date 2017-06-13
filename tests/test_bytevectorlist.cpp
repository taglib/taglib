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
#include <tbytevectorlist.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("ByteVectorList")
{
  SECTION("Split by single char")
  {
    const ByteVector v1("a b");
    const ByteVectorList l1 = ByteVectorList::split(v1, " ");
    REQUIRE(l1.size() == 2);
    REQUIRE(l1[0] == "a");
    REQUIRE(l1[1] == "b");

    const ByteVector v2("a");
    const ByteVectorList l2 = ByteVectorList::split(v2, " ");
    REQUIRE(l2.size() == 1);
    REQUIRE(l2[0] == "a");
  }
  SECTION("Split by string")
  {
    const ByteVector v1("ab01cd");
    const ByteVectorList l1 = ByteVectorList::split(v1, "01");
    REQUIRE(l1.size() == 2);
    REQUIRE(l1[0] == "ab");
    REQUIRE(l1[1] == "cd");

    const ByteVector v2("ab");
    const ByteVectorList l2 = ByteVectorList::split(v2, "01");
    REQUIRE(l2.size() == 1);
    REQUIRE(l2[0] == "ab");
  }
}
