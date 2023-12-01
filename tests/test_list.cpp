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

#include "tlist.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace TagLib;

class TestList : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestList);
  CPPUNIT_TEST(testAppend);
  CPPUNIT_TEST(testDetach);
  CPPUNIT_TEST(bracedInit);
  CPPUNIT_TEST(testSort);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAppend()
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
    CPPUNIT_ASSERT_EQUAL(4U, l1.size());
    CPPUNIT_ASSERT(l1 == l3);
  }

  void testDetach()
  {
    {
      List<int> l1;
      l1.append(1);
      l1.append(2);
      l1.append(3);
      l1.append(4);

      List<int> l2 = l1;
      auto it = l2.find(3);
      *it = 33;
      CPPUNIT_ASSERT_EQUAL(3,  l1[2]);
      CPPUNIT_ASSERT_EQUAL(33, l2[2]);
    }
    {
      List<int *> l1;
      List<int *> l2 = l1;
      CPPUNIT_ASSERT(!l1.autoDelete());
      CPPUNIT_ASSERT(!l2.autoDelete());
      l2.setAutoDelete(true);
      CPPUNIT_ASSERT(!l1.autoDelete());
      CPPUNIT_ASSERT(l2.autoDelete());
    }
    {
      List<int> l1;
      List<int> l2 = l1;
      l1.insert(l1.begin(), 1);
      CPPUNIT_ASSERT(!l1.isEmpty());
      CPPUNIT_ASSERT_EQUAL(1, l1.front());
      CPPUNIT_ASSERT(l2.isEmpty());
    }
  }

  void bracedInit()
  {
    List<int> l1 {
      1,
      2,
      3
    };
    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(l1.size()));
    CPPUNIT_ASSERT_EQUAL(1, l1[0]);
    CPPUNIT_ASSERT_EQUAL(2, l1[1]);
    CPPUNIT_ASSERT_EQUAL(3, l1[2]);

    List<int*> l2 {
      new int(1),
      new int(2),
      new int(3)
    };
    l2.setAutoDelete(true);
    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(l2.size()));
    CPPUNIT_ASSERT_EQUAL(1, *l2[0]);
    CPPUNIT_ASSERT_EQUAL(2, *l2[1]);
    CPPUNIT_ASSERT_EQUAL(3, *l2[2]);

    List<int> l3 = {
      1,
      2,
      3
    };
    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(l3.size()));
    CPPUNIT_ASSERT_EQUAL(1, l3[0]);
    CPPUNIT_ASSERT_EQUAL(2, l3[1]);
    CPPUNIT_ASSERT_EQUAL(3, l3[2]);

    List<int*> l4 = {
      new int(1),
      new int(2),
      new int(3)
    };
    l4.setAutoDelete(true);
    l4 = {
      new int(4),
      new int(5),
      new int(6)
    };
    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(l4.size()));
    CPPUNIT_ASSERT_EQUAL(4, *l4[0]);
    CPPUNIT_ASSERT_EQUAL(5, *l4[1]);
    CPPUNIT_ASSERT_EQUAL(6, *l4[2]);
  }

  void testSort()
  {
    List<int> list1 {
      3,
      2,
      1
    };
    list1.sort();
    CPPUNIT_ASSERT_EQUAL(list1[0], 1);
    CPPUNIT_ASSERT_EQUAL(list1[1], 2);
    CPPUNIT_ASSERT_EQUAL(list1[2], 3);

    List<int> list2 {
      1,
      2,
      3
    };
    list2.sort([](const auto &a, const auto &b) { return a > b; });
    CPPUNIT_ASSERT_EQUAL(list2[0], 3);
    CPPUNIT_ASSERT_EQUAL(list2[1], 2);
    CPPUNIT_ASSERT_EQUAL(list2[2], 1);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestList);
