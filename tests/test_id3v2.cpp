#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <textidentificationframe.h>

using namespace std;
using namespace TagLib;

class TestID3v2 : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestID3v2);
  CPPUNIT_TEST(testUnsynchDecode);
  CPPUNIT_TEST(testUTF16BEDelimiter);
  CPPUNIT_TEST(testUTF16Delimiter);
  CPPUNIT_TEST(testBrokenFrame1);
  //CPPUNIT_TEST(testItunes24FrameSize);
  CPPUNIT_TEST_SUITE_END();

public:

  void testUnsynchDecode()
  {
    MPEG::File f("data/unsynch.id3", false);
    CPPUNIT_ASSERT(f.tag());
    CPPUNIT_ASSERT_EQUAL(String("My babe just cares for me"), f.tag()->title());
  }

  void testUTF16BEDelimiter()
  {
    ID3v2::TextIdentificationFrame f(ByteVector("TPE1"), String::UTF16BE);
    StringList sl;
    sl.append("Foo");
    sl.append("Bar");
    f.setText(sl);
    CPPUNIT_ASSERT_EQUAL((unsigned int)(4+4+2+1+6+2+6), f.render().size());
  }

  void testUTF16Delimiter()
  {
    ID3v2::TextIdentificationFrame f(ByteVector("TPE1"), String::UTF16);
    StringList sl;
    sl.append("Foo");
    sl.append("Bar");
    f.setText(sl);
    CPPUNIT_ASSERT_EQUAL((unsigned int)(4+4+2+1+8+2+8), f.render().size());
  }

  void testBrokenFrame1()
  {
    MPEG::File f("data/broken-tenc.id3", false);
    CPPUNIT_ASSERT(f.tag());
    CPPUNIT_ASSERT(!f.ID3v2Tag()->frameListMap().contains("TENC"));
  }

  /*void testItunes24FrameSize()
  {
    MPEG::File f("data/005411.id3", false);
    CPPUNIT_ASSERT(f.tag());
    CPPUNIT_ASSERT(f.ID3v2Tag()->frameListMap().contains("TIT2"));
    CPPUNIT_ASSERT_EQUAL(String("Sunshine Superman"), f.ID3v2Tag()->frameListMap()["TIT2"].front()->toString());
  }*/

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestID3v2);
