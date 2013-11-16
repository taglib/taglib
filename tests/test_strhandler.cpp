#include <string.h>
#include <tstring.h>
#include <tstringhandler.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <infotag.h>
#include <mpegfile.h>
#include <wavfile.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

namespace
{
  class CeasarStringHandler : public StringHandler
  {
  public:
    CeasarStringHandler() : key(0) {}

    virtual String parse(const ByteVector &data) const
    {
      ByteVector tmp = data;
      tmp.resize(::strlen(data.data()));

      for(ByteVector::Iterator it = tmp.begin(); it != tmp.end(); ++it)
        *it = static_cast<char>(*it - key);

      return String(tmp).stripWhiteSpace();
    }

    virtual ByteVector render(const String &s) const
    {
      ByteVector tmp = s.stripWhiteSpace().data(String::Latin1);
      for(ByteVector::Iterator it = tmp.begin(); it != tmp.end(); ++it)
        *it = static_cast<char>(*it + key);

      return tmp;
    }

    void setKey(int k) { key = k; }

  private:
    int key;
  };
}

class TestStringHandler : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestStringHandler);
  CPPUNIT_TEST(testID3v1Tag);
  CPPUNIT_TEST(testID3v2Tag);
  CPPUNIT_TEST(testInfoTag);
  CPPUNIT_TEST_SUITE_END();

public:

  void testID3v1Tag()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    CeasarStringHandler sh;
    ID3v1::Tag::setStringHandler(&sh);

    dynamic_cast<CeasarStringHandler*>(ID3v1::Tag::stringHandler())->setKey(2);

    {
      MPEG::File f(newname.c_str());
      f.ID3v1Tag(true)->setTitle("ABC");
      f.save();
    }

    dynamic_cast<CeasarStringHandler*>(ID3v1::Tag::stringHandler())->setKey(1);

    {
      MPEG::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String("BCD"), f.ID3v1Tag(false)->title());
    }

    ID3v1::Tag::setStringHandler(0);
  }

  void testID3v2Tag()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    CeasarStringHandler sh;
    ID3v2::Tag::setLatin1StringHandler(&sh);

    dynamic_cast<CeasarStringHandler*>(ID3v2::Tag::latin1StringHandler())->setKey(2);

    // StringHandler is not used for rendering.
    {
      MPEG::File f(newname.c_str());
      f.ID3v2Tag(true)->setTitle("ABC");
      f.save();
    }

    dynamic_cast<CeasarStringHandler*>(ID3v2::Tag::latin1StringHandler())->setKey(1);

    {
      MPEG::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String("@AB"), f.ID3v2Tag(false)->title());
    }

    ID3v2::Tag::setLatin1StringHandler(0);
  }

  void testInfoTag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string newname = copy.fileName();

    CeasarStringHandler sh;
    RIFF::Info::Tag::setStringHandler(&sh);

    dynamic_cast<CeasarStringHandler*>(RIFF::Info::Tag::stringHandler())->setKey(2);

    {
      RIFF::WAV::File f(newname.c_str());
      f.InfoTag()->setTitle("ABC");
      f.save();
    }

    dynamic_cast<CeasarStringHandler*>(RIFF::Info::Tag::stringHandler())->setKey(1);

    {
      RIFF::WAV::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String("BCD"), f.InfoTag()->title());
    }

    RIFF::Info::Tag::setStringHandler(0);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestStringHandler);
