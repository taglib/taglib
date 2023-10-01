/***************************************************************************
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

#include <array>
#include "tbytevector.h"
#include "tvariant.h"
#include "tstringlist.h"
#include "tbytevectorlist.h"
#include <cppunit/extensions/HelperMacros.h>
#include <sstream>
#include "utils.h"

using namespace TagLib;

class TestVariant : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestVariant);
  CPPUNIT_TEST(testVariantTypes);
  CPPUNIT_TEST(testVariantToOStream);
  CPPUNIT_TEST_SUITE_END();

public:
  void testVariantTypes()
  {
    ByteVectorList bvl {"first", "second"};
    StringList sl {"el0", "el"};
    VariantList vl {"1st", "2nd"};
    VariantMap vm {{"key1", "value1"}, {"key2", "value2"}};

    Variant varVoid;
    Variant varBool(true);
    Variant varInt(-4);
    Variant varUInt(5U);
    Variant varLongLong(-6LL);
    Variant varULongLong(7ULL);
    Variant varDouble(1.23);
    Variant varString(String("test"));
    Variant varString2("charp");
    Variant varStringList(sl);
    Variant varByteVector(ByteVector("data"));
    Variant varByteVectorList(bvl);
    Variant varVariantList(vl);
    Variant varVariantMap(vm);

    static const std::array<std::tuple<Variant, Variant::Type>, 14> varTypes {
      std::tuple{varVoid, Variant::Void},
      std::tuple{varBool, Variant::Bool},
      std::tuple{varInt, Variant::Int},
      std::tuple{varUInt, Variant::UInt},
      std::tuple{varLongLong, Variant::LongLong},
      std::tuple{varULongLong, Variant::ULongLong},
      std::tuple{varDouble, Variant::Double},
      std::tuple{varString, Variant::String},
      std::tuple{varString2, Variant::String},
      std::tuple{varStringList, Variant::StringList},
      std::tuple{varByteVector, Variant::ByteVector},
      std::tuple{varByteVectorList, Variant::ByteVectorList},
      std::tuple{varVariantList, Variant::VariantList},
      std::tuple{varVariantMap, Variant::VariantMap}
    };

    for(const auto &t : varTypes) {
      CPPUNIT_ASSERT_EQUAL(std::get<1>(t), std::get<0>(t).type());
      if(std::get<0>(t).type() == Variant::Void) {
        CPPUNIT_ASSERT(std::get<0>(t).isEmpty());
      }
      else {
        CPPUNIT_ASSERT(!std::get<0>(t).isEmpty());
      }
    }

    bool ok;
    CPPUNIT_ASSERT_EQUAL(true, varBool.toBool(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(true, varBool.value<bool>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(-4, varInt.toInt(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(-4, varInt.value<int>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(5U, varUInt.toUInt(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(5U, varUInt.value<unsigned int>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(-6LL, varLongLong.toLongLong(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(-6LL, varLongLong.value<long long>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(7ULL, varULongLong.toULongLong(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(7ULL, varULongLong.value<unsigned long long>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(1.23, varDouble.toDouble(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(1.23, varDouble.value<double>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(String("test"), varString.toString(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(String("test"), varString.value<String>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(String("charp"), varString2.toString(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(String("charp"), varString2.value<String>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(sl, varStringList.toStringList(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(sl, varStringList.value<StringList>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(ByteVector("data"), varByteVector.toByteVector(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(ByteVector("data"), varByteVector.value<ByteVector>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(bvl, varByteVectorList.toByteVectorList(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(bvl, varByteVectorList.value<ByteVectorList>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(vl, varVariantList.toList(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(vl, varVariantList.value<VariantList>(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(vm, varVariantMap.toMap(&ok));
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(vm, varVariantMap.value<VariantMap>(&ok));
    CPPUNIT_ASSERT(ok);

    CPPUNIT_ASSERT_EQUAL(0, varBool.toInt(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(0U, varInt.toUInt(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(0LL, varUInt.toLongLong(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(0ULL, varLongLong.toULongLong(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(0.0, varULongLong.toDouble(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(String(), varDouble.toString(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(StringList(), varString.toStringList(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(ByteVector(), varStringList.toByteVector(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT(varByteVector.toByteVectorList(&ok).isEmpty());
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(VariantList(), varByteVectorList.toList(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(VariantMap(), varVariantList.toMap(&ok));
    CPPUNIT_ASSERT(!ok);
    CPPUNIT_ASSERT_EQUAL(false, varVariantMap.toBool(&ok));
    CPPUNIT_ASSERT(!ok);

    CPPUNIT_ASSERT(varUInt == varUInt);
    CPPUNIT_ASSERT(varUInt == Variant(5U));
    CPPUNIT_ASSERT(varUInt != Variant(6U));
    CPPUNIT_ASSERT(varUInt != Variant(5));
    CPPUNIT_ASSERT(varUInt != varInt);

    Variant varUInt2(varUInt);
    CPPUNIT_ASSERT(varUInt == varUInt2);
    varUInt2 = 6U;
    CPPUNIT_ASSERT(varUInt != varUInt2);
    CPPUNIT_ASSERT_EQUAL(5U, varUInt.toUInt());
    CPPUNIT_ASSERT_EQUAL(6U, varUInt2.toUInt());
  }

  void testVariantToOStream()
  {
    std::stringstream ss;
    VariantMap vm {
      {"strlist", StringList {"first", "second"}},
      {"varlist", VariantList {Variant(), 1U, -10LL, 4.32, false}},
      {"data", ByteVector("\xa9\x01\x7f", 3)}
    };
    ss << vm;

    CPPUNIT_ASSERT_EQUAL(
      R"({"data": "\xa9\x01\x7f", "strlist": ["first", "second"],)"
      R"( "varlist": [null, 1, -10, 4.32, false]})"s,
    ss.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestVariant);
