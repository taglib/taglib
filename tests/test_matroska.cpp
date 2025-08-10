#include <string>
#include <cstdio>

#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tag.h"
#include "matroskafile.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMatroska : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMatroska);
  CPPUNIT_TEST(testTags);
  CPPUNIT_TEST_SUITE_END();

public:
  void testTags()
  {
    // TODO implement
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMatroska);
