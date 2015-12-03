#include <string>
#include <stdio.h>
#include <tag.h>
#include <tbytevectorlist.h>
#include <opusfile.h>
#include <oggpageheader.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestOpus : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestOpus);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST(testReadComments);
  CPPUNIT_TEST(testWriteComments);
  CPPUNIT_TEST(testSplitPackets);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioProperties()
  {
    Ogg::Opus::File f(TEST_FILE_PATH_C("correctness_gain_silent_output.opus"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(7, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(7, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(7737, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(37, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(48000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(48000, f.audioProperties()->inputSampleRate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->opusVersion());
  }

  void testReadComments()
  {
    Ogg::Opus::File f(TEST_FILE_PATH_C("correctness_gain_silent_output.opus"));
    CPPUNIT_ASSERT_EQUAL(StringList("Xiph.Org Opus testvectormaker"), f.tag()->fieldListMap()["ENCODER"]);
    CPPUNIT_ASSERT(f.tag()->fieldListMap().contains("TESTDESCRIPTION"));
    CPPUNIT_ASSERT(!f.tag()->fieldListMap().contains("ARTIST"));
    CPPUNIT_ASSERT_EQUAL(String("libopus 0.9.11-66-g64c2dd7"), f.tag()->vendorID());
  }

  void testWriteComments()
  {
    ScopedFileCopy copy("correctness_gain_silent_output", ".opus");
    string filename = copy.fileName();

    {
      Ogg::Opus::File f(filename.c_str());
      f.tag()->setArtist("Your Tester");
      f.save();
    }
    {
      Ogg::Opus::File f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(StringList("Xiph.Org Opus testvectormaker"), f.tag()->fieldListMap()["ENCODER"]);
      CPPUNIT_ASSERT(f.tag()->fieldListMap().contains("TESTDESCRIPTION"));
      CPPUNIT_ASSERT_EQUAL(StringList("Your Tester"), f.tag()->fieldListMap()["ARTIST"]);
      CPPUNIT_ASSERT_EQUAL(String("libopus 0.9.11-66-g64c2dd7"), f.tag()->vendorID());
    }
  }

  void testSplitPackets()
  {
    ScopedFileCopy copy("correctness_gain_silent_output", ".opus");
    string newname = copy.fileName();

    String longText(std::string(128 * 1024, ' ').c_str());
    for(size_t i = 0; i < longText.length(); ++i)
      longText[i] = static_cast<wchar_t>(L'A' + (i % 26));

    {
      Ogg::Opus::File f(newname.c_str());
      f.tag()->setTitle(longText);
      f.save();
    }
    {
      Ogg::Opus::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(167534LL, f.length());
      CPPUNIT_ASSERT_EQUAL(27, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL((size_t)19, f.packet(0).size());
      CPPUNIT_ASSERT_EQUAL((size_t)131380, f.packet(1).size());
      CPPUNIT_ASSERT_EQUAL((size_t)5, f.packet(2).size());
      CPPUNIT_ASSERT_EQUAL((size_t)5, f.packet(3).size());
      CPPUNIT_ASSERT_EQUAL(longText, f.tag()->title());

      CPPUNIT_ASSERT(f.audioProperties());
      CPPUNIT_ASSERT_EQUAL(7737, f.audioProperties()->lengthInMilliseconds());

      f.tag()->setTitle("ABCDE");
      f.save();
    }
    {
      Ogg::Opus::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(35521LL, f.length());
      CPPUNIT_ASSERT_EQUAL(11, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL((size_t)19, f.packet(0).size());
      CPPUNIT_ASSERT_EQUAL((size_t)313, f.packet(1).size());
      CPPUNIT_ASSERT_EQUAL((size_t)5, f.packet(2).size());
      CPPUNIT_ASSERT_EQUAL((size_t)5, f.packet(3).size());
      CPPUNIT_ASSERT_EQUAL(String("ABCDE"), f.tag()->title());

      CPPUNIT_ASSERT(f.audioProperties());
      CPPUNIT_ASSERT_EQUAL(7737, f.audioProperties()->lengthInMilliseconds());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestOpus);
