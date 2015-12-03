#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <oggpageheader.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestOggFLAC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestOggFLAC);
  CPPUNIT_TEST(testFramingBit);
  CPPUNIT_TEST(testFuzzedFile);
  CPPUNIT_TEST(testSplitPackets);
  CPPUNIT_TEST_SUITE_END();

public:

  void testFramingBit()
  {
    ScopedFileCopy copy("empty_flac", ".oga");
    string newname = copy.fileName();

    {
      Ogg::FLAC::File f(newname.c_str());
      f.tag()->setArtist("The Artist");
      f.save();
    }
    {
      Ogg::FLAC::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String("The Artist"), f.tag()->artist());

      f.seek(0, File::End);
      CPPUNIT_ASSERT_EQUAL(9134LL, f.tell());
    }
  }

  void testFuzzedFile()
  {
    Ogg::FLAC::File f(TEST_FILE_PATH_C("segfault.oga"));
    CPPUNIT_ASSERT(!f.isValid());
  }

  void testSplitPackets()
  {
    ScopedFileCopy copy("empty_flac", ".oga");
    string newname = copy.fileName();

    String longText(std::string(128 * 1024, ' ').c_str());
    for(size_t i = 0; i < longText.length(); ++i)
      longText[i] = static_cast<wchar_t>(L'A' + (i % 26));

    {
      Ogg::FLAC::File f(newname.c_str());
      f.tag()->setTitle(longText);
      f.save();
    }
    {
      Ogg::FLAC::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(141141LL, f.length());
      CPPUNIT_ASSERT_EQUAL(21, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL((size_t)51, f.packet(0).size());
      CPPUNIT_ASSERT_EQUAL((size_t)131126, f.packet(1).size());
      CPPUNIT_ASSERT_EQUAL((size_t)22, f.packet(2).size());
      CPPUNIT_ASSERT_EQUAL((size_t)8196, f.packet(3).size());
      CPPUNIT_ASSERT_EQUAL(longText, f.tag()->title());

      CPPUNIT_ASSERT(f.audioProperties());
      CPPUNIT_ASSERT_EQUAL(3705, f.audioProperties()->lengthInMilliseconds());

      f.tag()->setTitle("ABCDE");
      f.save();
    }
    {
      Ogg::FLAC::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(9128LL, f.length());
      CPPUNIT_ASSERT_EQUAL(5, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL((size_t)51, f.packet(0).size());
      CPPUNIT_ASSERT_EQUAL((size_t)59, f.packet(1).size());
      CPPUNIT_ASSERT_EQUAL((size_t)22, f.packet(2).size());
      CPPUNIT_ASSERT_EQUAL((size_t)8196, f.packet(3).size());
      CPPUNIT_ASSERT_EQUAL(String("ABCDE"), f.tag()->title());

      CPPUNIT_ASSERT(f.audioProperties());
      CPPUNIT_ASSERT_EQUAL(3705, f.audioProperties()->lengthInMilliseconds());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestOggFLAC);
