#include <cppunit/extensions/HelperMacros.h>
#include <tstring.h>
#include <tmap.h>

using namespace std;
using namespace TagLib;

class TestMap : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMap);
  CPPUNIT_TEST(testInsert);
  CPPUNIT_TEST_SUITE_END();

public:

  void testInsert()
  {
    Map<String, int> m1;
    m1.insert("foo", 3);
    CPPUNIT_ASSERT_EQUAL(3, m1["foo"]);
    m1.insert("foo", 7);
    CPPUNIT_ASSERT_EQUAL(7, m1["foo"]);

    m1.insert("alice",  5);
    m1.insert("bob",    9);
    m1.insert("carol", 11);

    Map<String, int> m2 = m1;
    Map<String, int>::Iterator it = m2.find("bob");
    (*it).second = 99;
    CPPUNIT_ASSERT_EQUAL(m1["bob"], 9);
    CPPUNIT_ASSERT_EQUAL(m2["bob"], 99);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMap);
