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
#include <tmap.h>
#include <tstring.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Generic Map")
{
  SECTION("Insert elements")
  {
    Map<String, int> m1;
    m1.insert("foo", 3);
    m1.insert("bar", 5);
    REQUIRE(m1.size() == 2);
    REQUIRE(m1["foo"] == 3);
    REQUIRE(m1["bar"] == 5);
    m1.insert("foo", 7);
    REQUIRE(m1.size() == 2);
    REQUIRE(m1["foo"] == 7);
    REQUIRE(m1["bar"] == 5);
  }
  SECTION("Detach when returning non-const iterator")
  {
    Map<String, int> m1;
    m1.insert("alice", 5);
    m1.insert("bob", 9);
    m1.insert("carol", 11);
    
    Map<String, int> m2 = m1;
    Map<String, int>::Iterator it = m2.find("bob");
    (*it).second = 99;
    REQUIRE(m1["bob"] == 9);
    REQUIRE(m2["bob"] == 99);
  }
}
