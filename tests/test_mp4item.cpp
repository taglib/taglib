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

#include <string>
#include <cstdio>

#include "tag.h"
#include "mp4coverart.h"
#include "mp4item.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMP4Item : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMP4Item);
  CPPUNIT_TEST(testCoverArtList);
  CPPUNIT_TEST(testItemOperations);
  CPPUNIT_TEST_SUITE_END();

public:

  void testCoverArtList()
  {
    MP4::CoverArtList l;
    l.append(MP4::CoverArt(MP4::CoverArt::PNG, "foo"));
    l.append(MP4::CoverArt(MP4::CoverArt::JPEG, "bar"));

    MP4::Item i(l);
    MP4::CoverArtList l2 = i.toCoverArtList();

    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, l[0].format());
    CPPUNIT_ASSERT_EQUAL(ByteVector("foo"), l[0].data());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::JPEG, l[1].format());
    CPPUNIT_ASSERT_EQUAL(ByteVector("bar"), l[1].data());
  }

  void testItemOperations()
  {
    MP4::Item e;
    MP4::Item i1(1);
    MP4::Item i2(1);
    MP4::Item i3(-1);
    MP4::Item c1(static_cast<unsigned char>('A'));
    MP4::Item c2(static_cast<unsigned char>('A'));
    MP4::Item c3(static_cast<unsigned char>('Z'));
    MP4::Item u1(2U);
    MP4::Item u2(2U);
    MP4::Item u3(0U);
    MP4::Item l1(3LL);
    MP4::Item l2(3LL);
    MP4::Item l3(-7LL);
    MP4::Item b1(true);
    MP4::Item b2(true);
    MP4::Item b3(false);
    MP4::Item p1(4, 5);
    MP4::Item p2(4, 5);
    MP4::Item p3(-4, -5);
    MP4::Item s1(StringList{"abc", "de"});
    MP4::Item s2(StringList{"abc", "de"});
    MP4::Item s3(StringList{"abc"});
    MP4::Item v1(ByteVectorList{"f", "gh"});
    MP4::Item v2(ByteVectorList{"f", "gh"});
    MP4::Item v3(ByteVectorList{});
    MP4::Item a1(MP4::CoverArtList{
      MP4::CoverArt(MP4::CoverArt::PNG, "foo"),
      MP4::CoverArt(MP4::CoverArt::JPEG, "bar")
    });
    MP4::Item a2(MP4::CoverArtList{
      MP4::CoverArt(MP4::CoverArt::PNG, "foo"),
      MP4::CoverArt(MP4::CoverArt::JPEG, "bar")
    });
    MP4::Item a3(MP4::CoverArtList{
      MP4::CoverArt(MP4::CoverArt::JPEG, "bar")
    });

    CPPUNIT_ASSERT(i1 == i2);
    CPPUNIT_ASSERT(i2 != i3);
    CPPUNIT_ASSERT(i3 != c1);
    CPPUNIT_ASSERT(c1 == c1);
    CPPUNIT_ASSERT(c1 == c2);
    CPPUNIT_ASSERT(c2 != c3);
    CPPUNIT_ASSERT(c3 != u1);
    CPPUNIT_ASSERT(u1 == u2);
    CPPUNIT_ASSERT(u2 != u3);
    CPPUNIT_ASSERT(u3 != l1);
    CPPUNIT_ASSERT(l1 == l2);
    CPPUNIT_ASSERT(l2 != l3);
    CPPUNIT_ASSERT(l3 != b1);
    CPPUNIT_ASSERT(b1 == b2);
    CPPUNIT_ASSERT(b2 != b3);
    CPPUNIT_ASSERT(b3 != p1);
    CPPUNIT_ASSERT(p1 == p2);
    CPPUNIT_ASSERT(p2 != p3);
    CPPUNIT_ASSERT(p3 != s1);
    CPPUNIT_ASSERT(s1 == s2);
    CPPUNIT_ASSERT(s2 != s3);
    CPPUNIT_ASSERT(s3 != v1);
    CPPUNIT_ASSERT(v1 == v2);
    CPPUNIT_ASSERT(v2 != v3);
    CPPUNIT_ASSERT(v3 != a1);
    CPPUNIT_ASSERT(a1 == a2);
    CPPUNIT_ASSERT(a2 != a3);
    CPPUNIT_ASSERT(a3 != e);

    CPPUNIT_ASSERT(!e.isValid());
    CPPUNIT_ASSERT(i1.isValid());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::Void, e.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::Int, i1.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::Byte, c1.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::UInt, u1.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::LongLong, l1.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::Bool, b1.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::IntPair, p1.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::StringList, s1.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::ByteVectorList, v1.type());
    CPPUNIT_ASSERT_EQUAL(MP4::Item::Type::CoverArtList, a1.type());

    CPPUNIT_ASSERT_EQUAL(1, i1.toInt());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>('A'), c1.toByte());
    CPPUNIT_ASSERT_EQUAL(2U, u1.toUInt());
    CPPUNIT_ASSERT_EQUAL(3LL, l1.toLongLong());
    CPPUNIT_ASSERT_EQUAL(true, b1.toBool());
    CPPUNIT_ASSERT_EQUAL(4, p1.toIntPair().first);
    CPPUNIT_ASSERT_EQUAL((StringList{"abc", "de"}), s1.toStringList());
    CPPUNIT_ASSERT_EQUAL((ByteVectorList{"f", "gh"}), v1.toByteVectorList());
    CPPUNIT_ASSERT_EQUAL(MP4::CoverArt::PNG, a1.toCoverArtList().front().format());

    s3.swap(s1);
    CPPUNIT_ASSERT_EQUAL((StringList{"abc"}), s1.toStringList());
    CPPUNIT_ASSERT_EQUAL((StringList{"abc", "de"}), s3.toStringList());
    CPPUNIT_ASSERT_EQUAL(MP4::AtomDataType::TypeUndefined, s1.atomDataType());
    s1.setAtomDataType(MP4::AtomDataType::TypeUTF8);
    CPPUNIT_ASSERT_EQUAL(MP4::AtomDataType::TypeUTF8, s1.atomDataType());
    s1 = s3;
    CPPUNIT_ASSERT_EQUAL((StringList{"abc", "de"}), s1.toStringList());

    MP4::ItemMap m1{{"key1", i1}, {"key2", p1}};
    MP4::ItemMap m2{{"key1", i2}, {"key2", p2}};
    MP4::ItemMap m3{{"key1", i2}, {"key2", p3}};
    CPPUNIT_ASSERT(m1 == m2);
    CPPUNIT_ASSERT(m1 != m3);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMP4Item);
