/***************************************************************************
    copyright           : (C) 2012 by Tsuda Kageyu
    email               : tsuda.kageyu@gmail.com
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

#include "infotag.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(Info, testTitle)
{
  RIFF::Info::Tag tag;

  ASSERT_EQ(String(""), tag.title());
  tag.setTitle("Test title 1");
  tag.setFieldText("TEST", "Dummy Text");

  ASSERT_EQ(String("Test title 1"), tag.title());

  RIFF::Info::FieldListMap map = tag.fieldListMap();
  ASSERT_EQ(String("Test title 1"), map["INAM"]);
  ASSERT_EQ(String("Dummy Text"), map["TEST"]);
}

TEST(Info, testNumericFields)
{
  RIFF::Info::Tag tag;

  ASSERT_EQ(static_cast<unsigned int>(0), tag.track());
  tag.setTrack(1234);
  ASSERT_EQ(static_cast<unsigned int>(1234), tag.track());
  ASSERT_EQ(String("1234"), tag.fieldText("IPRT"));

  ASSERT_EQ(static_cast<unsigned int>(0), tag.year());
  tag.setYear(1234);
  ASSERT_EQ(static_cast<unsigned int>(1234), tag.year());
  ASSERT_EQ(String("1234"), tag.fieldText("ICRD"));
}
