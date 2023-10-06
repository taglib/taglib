/***************************************************************************
    copyright            : (C) 2020 by Kevin Andre
    email                : hyperquantum@gmail.com

    copyright            : (C) 2023 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

#include "tversionnumber.h"
#include "tstring.h"
#include "taglib.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace TagLib;

class TestTagLib : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestTagLib);
  CPPUNIT_TEST(testVersionNumber);
  CPPUNIT_TEST(testRuntimeVersion);
  CPPUNIT_TEST_SUITE_END();

public:

  void testVersionNumber()
  {
    VersionNumber v210(2, 1, 0);
    VersionNumber v211(2, 1, 1);
    VersionNumber v220(2, 2, 0);
    VersionNumber v300(3, 0, 0);
    CPPUNIT_ASSERT_EQUAL(0x020100U, v210.combinedVersion());
    CPPUNIT_ASSERT_EQUAL(2U, v210.majorVersion());
    CPPUNIT_ASSERT_EQUAL(1U, v210.minorVersion());
    CPPUNIT_ASSERT_EQUAL(0U, v210.patchVersion());
    CPPUNIT_ASSERT(v210 == VersionNumber(2, 1));
    CPPUNIT_ASSERT(!(v210 == v211));
    CPPUNIT_ASSERT(v210 != v211);
    CPPUNIT_ASSERT(!(v210 != v210));
    CPPUNIT_ASSERT(v210 < v211);
    CPPUNIT_ASSERT(!(v220 < v210));
    CPPUNIT_ASSERT(v220 > v211);
    CPPUNIT_ASSERT(!(v210 > v211));
    CPPUNIT_ASSERT(v210 <= v210);
    CPPUNIT_ASSERT(v210 <= v300);
    CPPUNIT_ASSERT(!(v300 <= v220));
    CPPUNIT_ASSERT(v210 >= v210);
    CPPUNIT_ASSERT(v220 >= v210);
    CPPUNIT_ASSERT(!(v210 >= v300));
    CPPUNIT_ASSERT_EQUAL(String("2.1.0"), v210.toString());
  }

  void testRuntimeVersion()
  {
    CPPUNIT_ASSERT(runtimeVersion() >= VersionNumber(
      TAGLIB_MAJOR_VERSION, TAGLIB_MINOR_VERSION));
    CPPUNIT_ASSERT(runtimeVersion() >= VersionNumber(
      TAGLIB_MAJOR_VERSION, TAGLIB_MINOR_VERSION, TAGLIB_PATCH_VERSION));
    CPPUNIT_ASSERT(runtimeVersion() == VersionNumber(
      TAGLIB_MAJOR_VERSION, TAGLIB_MINOR_VERSION, TAGLIB_PATCH_VERSION));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTagLib);
