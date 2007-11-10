#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <id3v2frame.h>
#include <uniquefileidentifierframe.h>
#include <textidentificationframe.h>
#include <attachedpictureframe.h>
#include <generalencapsulatedobjectframe.h>
#include <relativevolumeframe.h>

using namespace std;
using namespace TagLib;

class PublicFrame : public ID3v2::Frame
{
  public:
    PublicFrame() : ID3v2::Frame(ByteVector("XXXX\0\0\0\0\0\0", 10)) {}
    String readStringField(const ByteVector &data, String::Type encoding,
                           int *positon = 0)
      { return ID3v2::Frame::readStringField(data, encoding, positon); }
    virtual String toString() const { return String::null; }
    virtual void parseFields(const ByteVector &) {}
    virtual ByteVector renderFields() const { return ByteVector::null; }
};

class TestID3v2 : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestID3v2);
  CPPUNIT_TEST(testUnsynchDecode);
  CPPUNIT_TEST(testUTF16BEDelimiter);
  CPPUNIT_TEST(testUTF16Delimiter);
  CPPUNIT_TEST(testReadStringField);
  CPPUNIT_TEST(testParseAPIC);
  CPPUNIT_TEST(testParseGEOB);
  CPPUNIT_TEST(testParseRelativeVolumeFrame);
  CPPUNIT_TEST(testParseUniqueFileIdentifierFrame);
  CPPUNIT_TEST(testParseEmptyUniqueFileIdentifierFrame);
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

  void testReadStringField()
  {
    PublicFrame f;
    ByteVector data("abc\0", 4);
    String str = f.readStringField(data, String::Latin1);
    CPPUNIT_ASSERT_EQUAL(String("abc"), str);
  }

  // http://bugs.kde.org/show_bug.cgi?id=151078
  void testParseAPIC()
  {
    ID3v2::AttachedPictureFrame f(ByteVector("APIC"
                                             "\x00\x00\x00\x07"
                                             "\x00\x00"
                                             "\x00"
                                             "m\x00"
                                             "\x01"
                                             "d\x00"
                                             "\x00", 17));
    CPPUNIT_ASSERT_EQUAL(String("m"), f.mimeType());
    CPPUNIT_ASSERT_EQUAL(ID3v2::AttachedPictureFrame::FileIcon, f.type());
    CPPUNIT_ASSERT_EQUAL(String("d"), f.description());
  }

  // http://bugs.kde.org/show_bug.cgi?id=151078
  void testParseGEOB()
  {
    ID3v2::GeneralEncapsulatedObjectFrame f(ByteVector("GEOB"
                                             "\x00\x00\x00\x08"
                                             "\x00\x00"
                                             "\x00"
                                             "m\x00"
                                             "f\x00"
                                             "d\x00"
                                             "\x00", 18));
    CPPUNIT_ASSERT_EQUAL(String("m"), f.mimeType());
    CPPUNIT_ASSERT_EQUAL(String("f"), f.fileName());
    CPPUNIT_ASSERT_EQUAL(String("d"), f.description());
  }

  // http://bugs.kde.org/show_bug.cgi?id=150481
  void testParseRelativeVolumeFrame()
  {
    ID3v2::RelativeVolumeFrame f(
      ByteVector("RVA2"              // Frame ID
                 "\x00\x00\x00\x0B"  // Frame size
                 "\x00\x00"          // Frame flags
                 "ident\x00"         // Identification
                 "\x02"              // Type of channel
                 "\x00\x0F"          // Volume adjustment
                 "\x08"              // Bits representing peak
                 "\x45", 21));       // Peak volume
    CPPUNIT_ASSERT_EQUAL(String("ident"), f.identification());
    CPPUNIT_ASSERT_EQUAL(15.0f / 512.0f,
                         f.volumeAdjustment(ID3v2::RelativeVolumeFrame::FrontRight));
    CPPUNIT_ASSERT_EQUAL((uchar)8,
                         f.peakVolume(ID3v2::RelativeVolumeFrame::FrontRight).bitsRepresentingPeak);
    CPPUNIT_ASSERT_EQUAL(ByteVector("\x45"),
                         f.peakVolume(ID3v2::RelativeVolumeFrame::FrontRight).peakVolume);
  }

  void testParseUniqueFileIdentifierFrame()
  {
    ID3v2::UniqueFileIdentifierFrame f(
      ByteVector("UFID"                 // Frame ID
                 "\x00\x00\x00\x09"     // Frame size
                 "\x00\x00"             // Frame flags
                 "owner\x00"            // Owner identifier
                 "\x00\x01\x02", 19));  // Identifier
    CPPUNIT_ASSERT_EQUAL(String("owner"),
                         f.owner());
    CPPUNIT_ASSERT_EQUAL(ByteVector("\x00\x01\x02", 3),
                         f.identifier());
  }

  void testParseEmptyUniqueFileIdentifierFrame()
  {
    ID3v2::UniqueFileIdentifierFrame f(
      ByteVector("UFID"                 // Frame ID
                 "\x00\x00\x00\x01"     // Frame size
                 "\x00\x00"             // Frame flags
                 "\x00"                 // Owner identifier
                 "", 11));              // Identifier
    CPPUNIT_ASSERT_EQUAL(String(),
                         f.owner());
    CPPUNIT_ASSERT_EQUAL(ByteVector(),
                         f.identifier());
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
