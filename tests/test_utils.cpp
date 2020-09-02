/***************************************************************************
    copyright           : (C) 2020 by Kevin Andre
    email               : hyperquantum@gmail.com
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

#include <tutils.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace TagLib;

class TestUtils : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestUtils);
  CPPUNIT_TEST(testCompareInts);
  CPPUNIT_TEST(testCompareVersions);
  CPPUNIT_TEST_SUITE_END();

public:

  void testCompareInts()
  {
    CPPUNIT_ASSERT(Utils::compareInts(6, 7) < 0);
    CPPUNIT_ASSERT_EQUAL(0, Utils::compareInts(7, 7));
    CPPUNIT_ASSERT(Utils::compareInts(8, 7) > 0);
  }

  void testCompareVersions()
  {
    CPPUNIT_ASSERT(Utils::compareVersions(0, 9, 99, 0, 9, 99) == 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 0, 1, 0, 0) == 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 1, 1, 1, 1, 1) == 0);
    CPPUNIT_ASSERT(Utils::compareVersions(5, 4, 3, 5, 4, 3) == 0);

    CPPUNIT_ASSERT(Utils::compareVersions(0, 9, 99, 1, 0, 0) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 0, 0, 9, 99) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 0, 1, 0, 1) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 1, 1, 0, 0) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 1, 1, 1, 0) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 1, 0, 1, 0, 1) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 0, 1, 1, 0) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 1, 0, 1, 0, 0) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 0, 1, 1, 1) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 1, 1, 1, 0, 0) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 1, 1, 1, 1) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 1, 1, 1, 0, 1) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(1, 1, 0, 1, 1, 1) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 1, 1, 1, 1, 0) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(1, 1, 100, 1, 2, 0) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(1, 2, 0, 1, 1, 100) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(1, 0, 0, 2, 0, 0) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(2, 0, 0, 1, 0, 0) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(4, 6, 2, 4, 6, 3) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(4, 6, 3, 4, 6, 2) > 0);

    CPPUNIT_ASSERT(Utils::compareVersions(4, 8, 0, 4, 9, 0) < 0);
    CPPUNIT_ASSERT(Utils::compareVersions(4, 9, 0, 4, 8, 0) > 0);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestUtils);
