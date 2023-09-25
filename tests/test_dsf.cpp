#include <string>
#include <cstdio>

#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tag.h"
#include "dsffile.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
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
    DSF::File f(TEST_FILE_PATH_C("empty10ms.dsf"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(10, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(5645, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(2822400, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->formatVersion());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->formatID());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channelType());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(static_cast<long long>(28224), f.audioProperties()->sampleCount());
    CPPUNIT_ASSERT_EQUAL(4096, f.audioProperties()->blockSizePerChannel());
  }

  void testTags()
  {
    ScopedFileCopy copy("empty10ms", ".dsf");
    string newname = copy.fileName();

    {
      DSF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.properties().isEmpty());
      CPPUNIT_ASSERT_EQUAL(String(""), f.tag()->artist());
      f.tag()->setArtist("The Artist");
      PropertyMap properties = f.properties();
      properties["ALBUMARTIST"] = StringList("Album Artist");
      f.setProperties(properties);
      f.save();
    }

    {
      DSF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String("The Artist"), f.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(String("The Artist"), f.properties()["ARTIST"].front());
      CPPUNIT_ASSERT_EQUAL(String("Album Artist"), f.properties()["ALBUMARTIST"].front());
      f.setProperties(PropertyMap());
      f.save();
    }

    // Check if file without tags is same as original empty file
    const ByteVector dsfData = PlainFile(TEST_FILE_PATH_C("empty10ms.dsf")).readAll();
    const ByteVector fileData = PlainFile(newname.c_str()).readAll();
    CPPUNIT_ASSERT(dsfData == fileData);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDSF);
