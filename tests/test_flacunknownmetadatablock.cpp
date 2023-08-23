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

#include "flacunknownmetadatablock.h"
#include "tag.h"
#include "tbytevectorlist.h"
#include "tstringlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(FLACUnknownMetadataBlock, testAccessors)
{
  ByteVector data("abc\x01", 4);
  FLAC::UnknownMetadataBlock block(42, data);
  ASSERT_EQ(42, block.code());
  ASSERT_EQ(data, block.data());
  ASSERT_EQ(data, block.render());
  ByteVector data2("xxx", 3);
  block.setCode(13);
  block.setData(data2);
  ASSERT_EQ(13, block.code());
  ASSERT_EQ(data2, block.data());
  ASSERT_EQ(data2, block.render());
}
