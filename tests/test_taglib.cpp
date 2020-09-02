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

#include <taglib.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace TagLib;

class TestTagLib : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestTagLib);
  CPPUNIT_TEST(testAtLeastVersionActualVersion);
  CPPUNIT_TEST(testAtLeastVersionMajorMinor);
  CPPUNIT_TEST(testAtLeastVersionMajorMinorPatch);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAtLeastVersionActualVersion()
  {
    int actualMajor = TAGLIB_MAJOR_VERSION;
    int actualMinor = TAGLIB_MINOR_VERSION;
    int actualPatch = TAGLIB_PATCH_VERSION;

    CPPUNIT_ASSERT(isTagLibVersionAtLeast(actualMajor, actualMinor));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(actualMajor, actualMinor, actualPatch));
  }

  void testAtLeastVersionMajorMinor()
  {
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(0, 9));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 0));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 9));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 10));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 11));

    // the following may need to be adjusted in the future :)
    CPPUNIT_ASSERT(!isTagLibVersionAtLeast(3, 0));
    CPPUNIT_ASSERT(!isTagLibVersionAtLeast(3, 7));
  }

  void testAtLeastVersionMajorMinorPatch()
  {
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(0, 9, 0));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(0, 9, 999));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(0, 9, 0));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 0, 0));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 9, 0));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 9, 1));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 10, 0));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 10, 999));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 11, 0));
    CPPUNIT_ASSERT(isTagLibVersionAtLeast(1, 11, 1));

    // the following may need to be adjusted in the future :)
    CPPUNIT_ASSERT(!isTagLibVersionAtLeast(2, 99, 0));
    CPPUNIT_ASSERT(!isTagLibVersionAtLeast(3, 0, 0));
    CPPUNIT_ASSERT(!isTagLibVersionAtLeast(3, 50, 0));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTagLib);
