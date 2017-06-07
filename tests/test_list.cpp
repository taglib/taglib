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
#include <tlist.h>
#include "utils.h"

using namespace TagLib;

TEST_CASE("Generic List")
{
  SECTION("Append and prepend elements")
  {
    List<int> l1;
    List<int> l2;
    List<int> l3;
    l1.append(2);
    l2.append(3);
    l2.append(4);
    l1.append(l2);
    l1.prepend(1);
    l3.append(1);
    l3.append(2);
    l3.append(3);
    l3.append(4);
    REQUIRE(l1.size() == 4);
    REQUIRE(l1 == l3);
  }
  SECTION("Detach when returning non-const iterator")
  {
      List<int> l1;
      l1.append(1);
      l1.append(2);
      l1.append(3);
      l1.append(4);
    
      List<int> l2 = l1;
      List<int>::Iterator it = l2.find(3);
      *it = 33;
      REQUIRE(l1[2] == 3);
      REQUIRE(l2[2] == 33);
  }
}
