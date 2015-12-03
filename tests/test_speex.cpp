#include <speexfile.h>
#include <oggpageheader.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestSpeex : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestSpeex);
  CPPUNIT_TEST(testAudioProperties);
  CPPUNIT_TEST(testSplitPackets);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAudioProperties()
  {
    Ogg::Speex::File f(TEST_FILE_PATH_C("empty.spx"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(53, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(-1, f.audioProperties()->bitrateNominal());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
  }

  void testSplitPackets()
  {
    ScopedFileCopy copy("empty", ".spx");
    string newname = copy.fileName();

    String longText(std::string(128 * 1024, ' ').c_str());
    for (size_t i = 0; i < longText.length(); ++i)
      longText[i] = static_cast<wchar_t>(L'A' + (i % 26));

    {
      Ogg::Speex::File f(newname.c_str());
      f.tag()->setTitle(longText);
      f.save();
    }
    {
      Ogg::Speex::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(156330LL, f.length());
      CPPUNIT_ASSERT_EQUAL(23, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL((size_t)80, f.packet(0).size());
      CPPUNIT_ASSERT_EQUAL((size_t)131116, f.packet(1).size());
      CPPUNIT_ASSERT_EQUAL((size_t)93, f.packet(2).size());
      CPPUNIT_ASSERT_EQUAL((size_t)93, f.packet(3).size());
      CPPUNIT_ASSERT_EQUAL(longText, f.tag()->title());

      CPPUNIT_ASSERT(f.audioProperties());
      CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());

      f.tag()->setTitle("ABCDE");
      f.save();
    }
    {
      Ogg::Speex::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(24317LL, f.length());
      CPPUNIT_ASSERT_EQUAL(7, f.lastPageHeader()->pageSequenceNumber());
      CPPUNIT_ASSERT_EQUAL((size_t)80, f.packet(0).size());
      CPPUNIT_ASSERT_EQUAL((size_t)49, f.packet(1).size());
      CPPUNIT_ASSERT_EQUAL((size_t)93, f.packet(2).size());
      CPPUNIT_ASSERT_EQUAL((size_t)93, f.packet(3).size());
      CPPUNIT_ASSERT_EQUAL(String("ABCDE"), f.tag()->title());

      CPPUNIT_ASSERT(f.audioProperties());
      CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSpeex);
