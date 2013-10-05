#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <dsffile.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestDSF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestDSF);
  CPPUNIT_TEST(testBasic);
  CPPUNIT_TEST_SUITE_END();

public:

  void testBasic()
  {
    DSF::File f(TEST_FILE_PATH_C("empty.dsf"));
    DSF::AudioProperties *props = f.audioProperties();
    CPPUNIT_ASSERT_EQUAL(2822400, props->sampleRate());
    CPPUNIT_ASSERT_EQUAL(1, props->channels());
    CPPUNIT_ASSERT_EQUAL(1, props->bitsPerSample());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDSF);
