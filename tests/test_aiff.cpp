/***************************************************************************
    copyright           : (C) 2009 by Lukas Lalinsky
    email               : lukas@oxygene.sk
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <string>
#include <cstdio>

#include "tbytevectorlist.h"
#include "tag.h"
#include "taglib_config.h"
#include "aifffile.h"
#include "aiffnativetag.h"
#include "tbytevectorstream.h"
#include "tpropertymap.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

#ifdef TAGLIB_WITH_CHARSETTABLES
namespace
{
  // Puts back the code page a native tag assumes, so that a test stating
  // another one does not carry it into the next test.
  class RestoreAssumedCodePage
  {
  public:
    RestoreAssumedCodePage() :
      m_previous(RIFF::AIFF::NativeTag::assumedCodePage())
    {
    }

    ~RestoreAssumedCodePage()
    {
      RIFF::AIFF::NativeTag::setAssumedCodePage(m_previous);
    }

  private:
    const unsigned int m_previous;
  };

  // One chunk of an AIFF file: its id, its size, its payload, and the pad
  // byte that brings an odd sized payload to an even length.
  ByteVector aiffChunk(const char *id, const ByteVector &payload)
  {
    ByteVector chunk = ByteVector(id, 4) + ByteVector::fromUInt(payload.size()) +
                       payload;
    if(payload.size() % 2)
      chunk.append(static_cast<char>(0));
    return chunk;
  }

  // A minimal AIFF file: a COMM chunk describing one silent channel, and the
  // NAME chunk holding \a name unless it is empty.
  ByteVector aiffWithName(const ByteVector &name)
  {
    ByteVector body = aiffChunk("COMM",
                                ByteVector("\x00\x01\x00\x00\x00\x00\x00\x10"
                                           "\x40\x0E\xAC\x44\x00\x00\x00\x00"
                                           "\x00\x00", 18));
    if(!name.isEmpty())
      body.append(aiffChunk("NAME", name));

    return ByteVector("FORM") + ByteVector::fromUInt(body.size() + 4) +
           ByteVector("AIFF") + body;
  }

  // The payload of the first chunk called \a name in the AIFF file \a data,
  // checked against the format itself instead of against TagLib: the FORM
  // header's size has to cover exactly the file, and the chunks have to tile
  // it, so a file TagLib has just written is validated independently of how
  // TagLib would read it back.
  ByteVector checkedRawChunk(const ByteVector &data, const char *name)
  {
    const ByteVector wanted(name);

    CPPUNIT_ASSERT(data.size() >= 12u);
    CPPUNIT_ASSERT(data.startsWith("FORM"));

    const offset_t declaredEnd = static_cast<offset_t>(data.toUInt(4u)) + 8;
    CPPUNIT_ASSERT_EQUAL(declaredEnd, static_cast<offset_t>(data.size()));

    ByteVector found;
    offset_t offset = 12;
    while(offset + 8 <= declaredEnd) {
      const ByteVector header = data.mid(static_cast<unsigned int>(offset), 8);
      CPPUNIT_ASSERT_EQUAL(8u, header.size());

      const unsigned int size = header.toUInt(4u);
      const ByteVector payload = data.mid(static_cast<unsigned int>(offset + 8), size);
      CPPUNIT_ASSERT_EQUAL(size, payload.size());

      if(found.isEmpty() && header.startsWith(wanted))
        found = payload;

      // Chunks are padded to an even length, so an odd size is followed by a
      // pad byte which is not counted in the size.
      offset += 8 + size + (size % 2);
    }
    CPPUNIT_ASSERT_EQUAL(declaredEnd, offset);

    return found;
  }
}  // namespace
#endif // TAGLIB_WITH_CHARSETTABLES

class TestAIFF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAIFF);
  CPPUNIT_TEST(testAiffProperties);
  CPPUNIT_TEST(testAiffCProperties);
  CPPUNIT_TEST(testSaveID3v2);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testDuplicateID3v2);
  CPPUNIT_TEST(testNativeFields);
  CPPUNIT_TEST(testSaveNativeFields);
#ifdef TAGLIB_WITH_CHARSETTABLES
  CPPUNIT_TEST(testNativeMacRomanRead);
  CPPUNIT_TEST(testNativeMacRomanWrite);
  CPPUNIT_TEST(testNativeCodePageOverride);
#endif // TAGLIB_WITH_CHARSETTABLES
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST_SUITE_END();

public:

  void testAiffProperties()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("empty.aiff"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(67, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(706, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(2941U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(false, f.audioProperties()->isAiffC());
  }

  void testAiffCProperties()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("alaw.aifc"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(37, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(355, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(1622U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(true, f.audioProperties()->isAiffC());
    CPPUNIT_ASSERT_EQUAL(ByteVector("ALAW"), f.audioProperties()->compressionType());
    CPPUNIT_ASSERT_EQUAL(String("SGI CCITT G.711 A-law"), f.audioProperties()->compressionName());
  }

  void testSaveID3v2()
  {
    ScopedFileCopy copy("empty", ".aiff");
    string newname = copy.fileName();

    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());

      f.tag()->setTitle(L"TitleXXX");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }
    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L"TitleXXX"), f.tag()->title());

      f.tag()->setTitle("");
      f.save();
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("empty", ".aiff");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(ID3v2::v3);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      RIFF::AIFF::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), f2.tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testDuplicateID3v2()
  {
    ScopedFileCopy copy("duplicate_id3v2", ".aiff");

    // duplicate_id3v2.aiff has duplicate ID3v2 tag chunks.
    // title() returns "Title2" if can't skip the second tag.

    RIFF::AIFF::File f(copy.fileName().c_str());
    CPPUNIT_ASSERT(f.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.tag()->title());

    f.save();
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(7030), f.length());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(-1), f.find("Title2"));
  }

  void testNativeFields()
  {
    // The four text chunks of a file that predates ID3v2.
    RIFF::AIFF::File f(TEST_FILE_PATH_C("alaw.aifc"));
    CPPUNIT_ASSERT(f.hasNativeTag());
    CPPUNIT_ASSERT(!f.nativeTag()->isEmpty());
    CPPUNIT_ASSERT_EQUAL(String("woodblock"), f.nativeTag()->title());
    CPPUNIT_ASSERT_EQUAL(String("Prosonus"), f.nativeTag()->artist());
    CPPUNIT_ASSERT_EQUAL(String("Copyright 1991, Prosonus"),
                         f.nativeTag()->fieldText("(c) "));
    CPPUNIT_ASSERT(f.nativeTag()->comment().isEmpty());

    // The native tag has no field behind these.
    CPPUNIT_ASSERT(f.nativeTag()->album().isEmpty());
    CPPUNIT_ASSERT_EQUAL(0u, f.nativeTag()->year());
    f.nativeTag()->setAlbum("Album");
    f.nativeTag()->setYear(2026);
    CPPUNIT_ASSERT(f.nativeTag()->album().isEmpty());
    CPPUNIT_ASSERT_EQUAL(0u, f.nativeTag()->year());

    // The four chunks with their property keys, and nothing unsupported.
    const PropertyMap props = f.nativeTag()->properties();
    CPPUNIT_ASSERT_EQUAL(3u, props.size());
    CPPUNIT_ASSERT_EQUAL(String("woodblock"), props.value("TITLE").front());
    CPPUNIT_ASSERT_EQUAL(String("Prosonus"), props.value("ARTIST").front());
    CPPUNIT_ASSERT_EQUAL(String("Copyright 1991, Prosonus"),
                         props.value("COPYRIGHT").front());

    // A file with no native chunks holds an empty tag and says so.
    RIFF::AIFF::File empty(TEST_FILE_PATH_C("empty.aiff"));
    CPPUNIT_ASSERT(!empty.hasNativeTag());
    CPPUNIT_ASSERT(empty.nativeTag()->isEmpty());

    // Only the four text chunks are fields of this class: COMM is the audio
    // layout, so neither reading nor writing it as a field does anything.
    RIFF::AIFF::NativeTag::FieldDataMap fields;
    fields["NAME"] = ByteVector("Title");
    fields["COMM"] = ByteVector("not a field");
    RIFF::AIFF::NativeTag tag(fields);
    CPPUNIT_ASSERT_EQUAL(String("Title"), tag.title());
    CPPUNIT_ASSERT(tag.fieldText("COMM").isEmpty());
    tag.setFieldText("COMM", "ignored");
    CPPUNIT_ASSERT(tag.fieldText("COMM").isEmpty());
    CPPUNIT_ASSERT_EQUAL(1u, tag.properties().size());
  }

  void testSaveNativeFields()
  {
    ScopedFileCopy copy("empty", ".aiff");
    const string newname = copy.fileName();

    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasNativeTag());

      f.nativeTag()->setTitle("TitleXXX");
      f.nativeTag()->setArtist("Artist XXX");
      f.nativeTag()->setFieldText("(c) ", "Copyright XXX");
      f.nativeTag()->setComment("Comment XXX");
      CPPUNIT_ASSERT(f.save());
      CPPUNIT_ASSERT(f.hasNativeTag());

      // The native tag is its own tag: writing it writes no ID3v2 chunk.
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasNativeTag());
      CPPUNIT_ASSERT_EQUAL(String("TitleXXX"), f.nativeTag()->title());
      CPPUNIT_ASSERT_EQUAL(String("Artist XXX"), f.nativeTag()->artist());
      CPPUNIT_ASSERT_EQUAL(String("Copyright XXX"),
                           f.nativeTag()->fieldText("(c) "));
      CPPUNIT_ASSERT_EQUAL(String("Comment XXX"), f.nativeTag()->comment());

      // A field that loses its text loses its chunk; the others stay.
      f.nativeTag()->setTitle("");
      CPPUNIT_ASSERT(f.save());
    }
    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasNativeTag());
      CPPUNIT_ASSERT(f.nativeTag()->title().isEmpty());
      CPPUNIT_ASSERT_EQUAL(String("Artist XXX"), f.nativeTag()->artist());
    }
    {
      RIFF::AIFF::File f(newname.c_str());
      f.nativeTag()->setArtist("");
      f.nativeTag()->setFieldText("(c) ", "");
      f.nativeTag()->setComment("");
      CPPUNIT_ASSERT(f.save());
      CPPUNIT_ASSERT(!f.hasNativeTag());
    }
    {
      RIFF::AIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasNativeTag());
      CPPUNIT_ASSERT(f.nativeTag()->isEmpty());
    }

    // Saving a file whose native chunks one never touched keeps them.
    ScopedFileCopy aifcCopy("alaw", ".aifc");
    {
      RIFF::AIFF::File f(aifcCopy.fileName().c_str());
      CPPUNIT_ASSERT(f.save());
    }
    {
      RIFF::AIFF::File f(aifcCopy.fileName().c_str());
      CPPUNIT_ASSERT(f.hasNativeTag());
      CPPUNIT_ASSERT_EQUAL(String("woodblock"), f.nativeTag()->title());
      CPPUNIT_ASSERT_EQUAL(String("Prosonus"), f.nativeTag()->artist());
    }
  }

#ifdef TAGLIB_WITH_CHARSETTABLES
  void testNativeMacRomanRead()
  {
    RestoreAssumedCodePage restore;

    // No code page stated is what a native tag assumes when the caller has
    // said nothing, which is what this build starts with: the tag then reads
    // Mac OS Roman.
    CPPUNIT_ASSERT_EQUAL(0u, RIFF::AIFF::NativeTag::assumedCodePage());
    {
      ByteVectorStream stream(aiffWithName(ByteVector("Caf\x8E", 4)));
      RIFF::AIFF::File f(&stream, false);
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\u00E9"), f.nativeTag()->title());
    }

    // Stating the Mac OS Roman code page itself reads the very same byte.
    RIFF::AIFF::NativeTag::setAssumedCodePage(10000);
    {
      ByteVectorStream stream(aiffWithName(ByteVector("Caf\x8E", 4)));
      RIFF::AIFF::File f(&stream, false);
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\u00E9"), f.nativeTag()->title());
    }

    // The caller saying its files are ISO 8859-1 reads that byte as that
    // charset's character instead.
    RIFF::AIFF::NativeTag::setAssumedCodePage(28591);
    {
      ByteVectorStream stream(aiffWithName(ByteVector("Caf\x8E", 4)));
      RIFF::AIFF::File f(&stream, false);
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\u008E"), f.nativeTag()->title());
    }

    // Going back to no code page stated is going back to Mac OS Roman.
    RIFF::AIFF::NativeTag::setAssumedCodePage(0);
    CPPUNIT_ASSERT_EQUAL(0u, RIFF::AIFF::NativeTag::assumedCodePage());
    {
      ByteVectorStream stream(aiffWithName(ByteVector("Caf\x8E", 4)));
      RIFF::AIFF::File f(&stream, false);
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\u00E9"), f.nativeTag()->title());
    }
  }

  void testNativeMacRomanWrite()
  {
    RestoreAssumedCodePage restore;

    ByteVectorStream stream(aiffWithName(ByteVector()));
    {
      RIFF::AIFF::File f(&stream, false);
      CPPUNIT_ASSERT(!f.hasNativeTag());

      f.nativeTag()->setTitle(L"Caf\u00E9");
      CPPUNIT_ASSERT(f.save());
      CPPUNIT_ASSERT(f.hasNativeTag());
    }

    // The bytes the assumed charset writes are Mac OS Roman, not ISO 8859-1.
    CPPUNIT_ASSERT_EQUAL(ByteVector("Caf\x8E", 4),
                         checkedRawChunk(*stream.data(), "NAME"));

    // A tag read and written under one code page keeps its text as it was.
    ByteVectorStream reopened(*stream.data());
    RIFF::AIFF::File f(&reopened, false);
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\u00E9"), f.nativeTag()->title());
  }

  void testNativeCodePageOverride()
  {
    RestoreAssumedCodePage restore;

    // The caller says its files are UTF-8, so that is what both the writing
    // and the reading of the tag does.
    RIFF::AIFF::NativeTag::setAssumedCodePage(65001);
    ByteVectorStream stream(aiffWithName(ByteVector()));
    {
      RIFF::AIFF::File f(&stream, false);
      f.nativeTag()->setTitle(L"Caf\u00E9");
      CPPUNIT_ASSERT(f.save());
    }
    CPPUNIT_ASSERT_EQUAL(ByteVector("Caf\xC3\xA9", 5),
                         checkedRawChunk(*stream.data(), "NAME"));

    ByteVectorStream reopened(*stream.data());
    RIFF::AIFF::File f(&reopened, false);
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\u00E9"), f.nativeTag()->title());

    // A code page this library cannot convert is refused, so the one in
    // effect stays.
    RIFF::AIFF::NativeTag::setAssumedCodePage(999999);
    CPPUNIT_ASSERT_EQUAL(65001u, RIFF::AIFF::NativeTag::assumedCodePage());
  }
#endif // TAGLIB_WITH_CHARSETTABLES

  void testFuzzedFile1()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("segfault.aif"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testFuzzedFile2()
  {
    RIFF::AIFF::File f(TEST_FILE_PATH_C("excessive_alloc.aif"));
    CPPUNIT_ASSERT(f.isValid());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestAIFF);
