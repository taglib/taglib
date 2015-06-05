#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <acidinfo.h>
#include <tdebug.h>
#include <tbytevector.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestAcidInfo : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAcidInfo);
  CPPUNIT_TEST(testParseAcidChunk);
  CPPUNIT_TEST_SUITE_END();

  public:
  void testParseAcidChunk()
  {
    RIFF::WAV::AcidInfo acidInfo(ByteVector(
      "\x02\x00\x00\x00\x35\x00\x00\x80\x00\x00\x00\x00\x20\x00\x00\x00\x04"
      "\x00\x04\x00\x00\x00\x00\x43", 24));

    CPPUNIT_ASSERT_EQUAL((acidInfo.flags() & RIFF::WAV::AcidInfo::OneShot), 0);
    CPPUNIT_ASSERT_EQUAL((acidInfo.flags() & RIFF::WAV::AcidInfo::RootNote),
      (int)RIFF::WAV::AcidInfo::RootNote);
    CPPUNIT_ASSERT_EQUAL((acidInfo.flags() & RIFF::WAV::AcidInfo::Storage), 0);
    CPPUNIT_ASSERT_EQUAL(acidInfo.rootNote(),
      (short)RIFF::WAV::AcidInfo::RootNoteF);
    CPPUNIT_ASSERT_EQUAL(acidInfo.tempo(), (float)128.0);
    CPPUNIT_ASSERT_EQUAL(acidInfo.numberOfBeats(), 32);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAcidInfo);

