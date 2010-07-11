#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <wavfile.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestWAV : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestWAV);
  CPPUNIT_TEST(testLength);
  CPPUNIT_TEST_SUITE_END();

public:

  void testLength()
  {
    RIFF::WAV::File f("data/empty.wav");
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
