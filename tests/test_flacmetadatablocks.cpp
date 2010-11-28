#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <tag.h>
#include <tstringlist.h>
#include <tbytevectorlist.h>
#include <flacfile.h>
#include <flacmetadatablock.h>
#include <flacmetadatablocks.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestFLACMetadataBlocks : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFLACMetadataBlocks);
  CPPUNIT_TEST(testRead);
  CPPUNIT_TEST_SUITE_END();

public:

  void testRead()
  {
    FLAC::File f("data/silence-44-s.flac");
    FLAC::MetadataBlocks b;
    f.seek(4);
    b.read(&f);
    List<FLAC::MetadataBlock *> blocks = b.metadataBlockList();
    CPPUNIT_ASSERT_EQUAL(TagLib::uint(5), blocks.size());
    CPPUNIT_ASSERT_EQUAL(0 + FLAC::MetadataBlock::StreamInfo, blocks[0]->code());
    CPPUNIT_ASSERT_EQUAL(0 + FLAC::MetadataBlock::SeekTable, blocks[1]->code());
    CPPUNIT_ASSERT_EQUAL(0 + FLAC::MetadataBlock::VorbisComment, blocks[2]->code());
    CPPUNIT_ASSERT_EQUAL(0 + FLAC::MetadataBlock::CueSheet, blocks[3]->code());
    CPPUNIT_ASSERT_EQUAL(0 + FLAC::MetadataBlock::Picture, blocks[4]->code());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFLACMetadataBlocks);

