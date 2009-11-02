#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <flacfile.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestFLAC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFLAC);
  CPPUNIT_TEST(testMultipleCommentBlocks);
  CPPUNIT_TEST_SUITE_END();

public:

  void testMultipleCommentBlocks()
  {
    ScopedFileCopy copy("multiple-vc", ".flac");
    string newname = copy.fileName();

    FLAC::File *f = new FLAC::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String("Artist 1"), f->tag()->artist());
    f->tag()->setArtist("The Artist");
    f->save();
    delete f;

    f = new FLAC::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String("The Artist"), f->tag()->artist());
    delete f;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFLAC);
