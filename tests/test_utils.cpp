#include <tutils.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestUtils : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestUtils);
  CPPUNIT_TEST(testByteSwap);
  CPPUNIT_TEST_SUITE_END();

public:

  void testByteSwap()
  {
    CPPUNIT_ASSERT_EQUAL((TagLib::ushort)0x3412, Utils::byteSwap((TagLib::ushort)0x1234));
    CPPUNIT_ASSERT_EQUAL(0x78563412U, Utils::byteSwap(0x12345678U));
    CPPUNIT_ASSERT_EQUAL(0xF0DEBC9A78563412ULL, Utils::byteSwap(0x123456789ABCDEF0ULL));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestUtils);
