#include <string>
#include <stdio.h>
#include <tstring.h>
#include <tfilestream.h>
#include <rmp3file.h>
#include <infotag.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestRMP3 : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestRMP3);
  CPPUNIT_TEST(testVersion2DurationWithXingHeader);
  CPPUNIT_TEST(testIID3Creation);
  CPPUNIT_TEST_SUITE_END();

public:

  void testVersion2DurationWithXingHeader()
  {
    RIFF::RMP3::File f(TEST_FILE_PATH_C("mpeg2.rmp"));
    CPPUNIT_ASSERT_EQUAL(5387, f.audioProperties()->length());
  }
  
  void testIID3Creation()
  {
    ScopedFileCopy copy("mpeg2", ".rmp");
    string newname = copy.fileName();
    
    RIFF::RMP3::File *f = new RIFF::RMP3::File(newname.c_str());
    f->tag()->setTitle("Test Title");
    f->save();
    delete f;
    
    FileStream fs(newname.c_str());
    fs.seek(-128, IOStream::End);
    CPPUNIT_ASSERT_EQUAL(ByteVector("TAG"), fs.readBlock(3));
    CPPUNIT_ASSERT_EQUAL(ByteVector("Test Title"), fs.readBlock(10));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestRMP3);
