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
  CPPUNIT_TEST(testTags);
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

  void testTags()
  {
    ScopedFileCopy copy("empty", ".dsf");
    string newname = copy.fileName();

    DSF::File *f = new DSF::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String(""), f->tag()->artist());
    f->tag()->setArtist("The Artist");
    f->save();
    delete f;

    f = new DSF::File(newname.c_str());
    CPPUNIT_ASSERT_EQUAL(String("The Artist"), f->tag()->artist());
    delete f;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDSF);
