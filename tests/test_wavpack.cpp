#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <wavpackfile.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestWavPack : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestWavPack);
  CPPUNIT_TEST(testLengthScan);
  CPPUNIT_TEST_SUITE_END();

public:

  void testLengthScan()
  {
    WavPack::File f("data/no_length.wv");
    CPPUNIT_ASSERT_EQUAL(4, f.audioProperties()->length());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWavPack);
