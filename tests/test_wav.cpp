/***************************************************************************
    copyright           : (C) 2010 by Lukas Lalinsky
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

#include <initializer_list>
#include <iterator>
#include <string>
#include <cstdio>

#include "charset.h"
#include "id3v2tag.h"
#include "infotag.h"
#include "tbytevectorlist.h"
#include "tbytevectorstream.h"
#include "tfilestream.h"
#include "tpropertymap.h"
#include "wavfile.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

namespace
{
  // The bytes of a chunk: its id, its size, its payload, and the pad byte that
  // brings an odd sized payload to an even length.
  ByteVector riffChunk(const char *id, const ByteVector &payload)
  {
    ByteVector chunk = ByteVector(id, 4) + ByteVector::fromUInt(payload.size(), false) + payload;
    if(payload.size() % 2)
      chunk.append(static_cast<char>(0));
    return chunk;
  }

  // A CSET chunk declares the code page of the LIST/INFO text: a 32 bit code
  // page followed by 4 reserved bytes.
  ByteVector riffCset(unsigned int codePage)
  {
    return riffChunk("CSET", ByteVector::fromUInt(codePage, false) + ByteVector(4, '\0'));
  }

  // One field of an INFO list, as the reader will find it: an id and the text
  // in whatever encoding the file declares, without the NUL that ends it.
  struct InfoField
  {
    const char *id;
    ByteVector text;
  };

  // A WAV file with an INFO tag holding \a fields, and, when \a codePage is not
  // 0, a CSET chunk declaring the encoding the text is in. The CSET is written
  // in front of the LIST it describes, unless \a csetBehindList is set, which is
  // where a file TagLib did not write tends to put it.
  ByteVector riffWithInfo(std::initializer_list<InfoField> fields,
                          unsigned int codePage = 0, bool csetBehindList = false)
  {
    ByteVector list("INFO", 4);
    for(const auto &field : fields)
      list.append(riffChunk(field.id, field.text + ByteVector(1, '\0')));

    // fmt: PCM, one channel, 22050 Hz, 8 bits, 22050 bytes per second, and the
    // 8 bytes of audio the data chunk below has to hold for the file to parse.
    ByteVector body =
      riffChunk("fmt ", ByteVector("\x01\x00\x01\x00\x3E\x56\x00\x00"
                                   "\x3E\x56\x00\x00\x01\x00\x08\x00", 16));
    body.append(riffChunk("data", ByteVector(8, static_cast<char>(0x80))));

    if(codePage && !csetBehindList)
      body.append(riffCset(codePage));
    body.append(riffChunk("LIST", list));
    if(codePage && csetBehindList)
      body.append(riffCset(codePage));

    return ByteVector("RIFF", 4) + ByteVector::fromUInt(body.size() + 4, false) +
           ByteVector("WAVE", 4) + body;
  }

  // An INFO tag with no CSET chunk, which is what a reader has to fall back to
  // Latin-1 on.
  ByteVector latin1Riff()
  {
    return riffWithInfo({{"IART", ByteVector("A \xAE B", 5)},
                         {"IPRD", ByteVector("A \xAE B", 5)},
                         {"ICMT", ByteVector("Caf\xE9", 4)},
                         {"ICRD", ByteVector("2024-01-01", 10)}});
  }

  // An INFO tag that declares UTF-8, with the CSET behind the LIST it describes.
  ByteVector utf8Riff()
  {
    return riffWithInfo({{"IPRD", ByteVector("Caf\xC3\xA9", 5)},
                         {"ICRD", ByteVector("2024-03-03", 10)}},
                        65001, true);
  }

  // The file the code page catalog is read from: a CSET chunk declaring the
  // page, and in each of three text fields the whole byte range, so that a table
  // that is wrong at any one of the 255 positions fails. Byte 0x00 is not in
  // there because it ends the string; testCharsetDecodeStopsAtNul() covers it.
  ByteVector codePageRiff(unsigned int codePage)
  {
    ByteVector wholeRange;
    for(unsigned int b = 1; b < 256; ++b)
      wholeRange.append(static_cast<char>(b));

    return riffWithInfo({{"IART", wholeRange}, {"IPRD", wholeRange}, {"ICMT", wholeRange}},
                        codePage);
  }

  // Validates the raw chunk layout of the RIFF file in \a data against the
  // format itself instead of against TagLib, so that a file TagLib has just
  // written is checked independently of how TagLib would read it back.  Returns
  // the data of the first top level chunk called \a name.
  ByteVector checkedRawChunk(const ByteVector &data, const char *name)
  {
    const ByteVector wanted(name);

    CPPUNIT_ASSERT(data.size() >= 12u);
    CPPUNIT_ASSERT(data.startsWith("RIFF"));

    // The size in the header covers everything after the size field itself,
    // so the file must be exactly that long: no gap and no trailing junk.
    const offset_t declaredEnd = static_cast<offset_t>(data.toUInt(4, false)) + 8;
    CPPUNIT_ASSERT_EQUAL(declaredEnd, static_cast<offset_t>(data.size()));

    ByteVector found;
    offset_t offset = 12;
    while(offset + 8 <= declaredEnd) {
      const ByteVector chunkHeader = data.mid(static_cast<uint>(offset), 8);
      CPPUNIT_ASSERT_EQUAL(8u, chunkHeader.size());

      const unsigned int size = chunkHeader.toUInt(4, false);
      const ByteVector payload = data.mid(static_cast<uint>(offset + 8), size);
      CPPUNIT_ASSERT_EQUAL(size, payload.size());

      if(found.isEmpty() && chunkHeader.startsWith(wanted))
        found = payload;

      // Chunks are padded to an even length, so an odd size is followed by a
      // pad byte which is not counted in the size.
      offset += 8 + size + (size % 2);
    }

    // The chunks must tile the file exactly, up to and including its end.
    CPPUNIT_ASSERT_EQUAL(declaredEnd, offset);

    return found;
  }

  // The CSET chunk of a saved file must declare UTF-8 and nothing else, in the
  // full 8 byte layout: a 32 bit code page followed by 4 reserved bytes whose contents we disregard.
  void assertRawCsetDeclaresUtf8(const ByteVector &data)
  {
    const ByteVector cset = checkedRawChunk(data, "CSET");
    CPPUNIT_ASSERT_EQUAL(8u, cset.size());
    CPPUNIT_ASSERT_EQUAL(65001u, cset.toUInt(0, false));
    // we do not care about this: CPPUNIT_ASSERT_EQUAL(ByteVector(4, '\0'), cset.mid(4, 4));
  }

  // Fixed width hex, so that a byte or code point in a failure message lines up
  // with the table in test_wav_charset.inc.
  String hex(unsigned int value, unsigned int digits)
  {
    static const char *const d = "0123456789abcdef";
    String s;
    for(unsigned int i = digits; i > 0; --i)
      s += d[(value >> ((i - 1) * 4)) & 0xf];
    return s;
  }

  // Turns a repertoire from test_wav_charset.inc into a string: four hex digits
  // per byte, in byte order. Building it from code points rather than from a wide
  // string literal keeps this identical whether wchar_t is 16 or 32 bits.
  String repertoireToString(const char *digits)
  {
    std::wstring w;
    for(const char *p = digits; *p; p += 4) {
      unsigned int cp = 0;
      for(int i = 0; i < 4; ++i) {
        const char c = p[i];
        cp = (cp << 4) | static_cast<unsigned int>(c <= '9' ? c - '0' : (c | 0x20) - 'a' + 10);
      }
      w += static_cast<wchar_t>(cp);
    }
    return String(w);
  }

  // A code page's repertoire is 255 bytes of control codes and legacy symbols,
  // so a mismatch cannot be read off a failure message that prints both sides.
  // Compare quietly, then report the first byte that went wrong, which is the
  // only part of the difference worth seeing.
  void assertDecodedRepertoire(const String &field, const char *digits,
                               unsigned int codePage, const char *fieldName)
  {
    const String expected = repertoireToString(digits);
    const String id = String::number(codePage) + String(" ") + String(fieldName) + String(" ");

    // Checked first, so that indexing below is in range and a truncated field
    // is reported as such rather than as a difference at its last character.
    CPPUNIT_ASSERT_EQUAL(id + String::number(expected.size()),
                         id + String::number(field.size()));

    unsigned int i = 0;
    while(i < expected.size() && expected[i] == field[i])
      ++i;

    const bool identical = (i == expected.size());
    const String detail = identical
      ? id + String("matches the repertoire")
      : id + String("byte 0x") + hex(i + 1, 2)
        + String(" should be U+") + hex(expected[i], 4)
        + String(" but is U+") + hex(field[i], 4);
    CPPUNIT_ASSERT_EQUAL(id + String("matches the repertoire"), detail);
  }

  // Tags no single byte charset could have represented, as the strings and as
  // the UTF-8 bytes they have to end up as on disk.
  const String kNewTitle(L"Title \x4F60\x597D \x20AC");
  const ByteVector kNewTitleUtf8("Title \xE4\xBD\xA0\xE5\xA5\xBD \xE2\x82\xAC", 16);
  const String kNewComment(L"\x4F60\x597D");
  const ByteVector kNewCommentUtf8("\xE4\xBD\xA0\xE5\xA5\xBD", 6);

  // The expectations for every code page, hand maintained: see the header of the
  // file. They are the other side of every assertion below, so they must not come
  // from the code they check.
#include "test_wav_charset.inc"

  // The repertoire test_wav_charset.inc records for one code page, for the tests
  // that name a single page instead of walking the whole catalog.
  const char *repertoireFor(unsigned int codePage)
  {
    for(const auto &c : charsetCases) {
      if(c.codePage == codePage)
        return c.repertoire;
    }
    CPPUNIT_FAIL("no repertoire for that code page");
    return nullptr;
  }
}

class TestWAV : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestWAV);
  CPPUNIT_TEST(testPCMProperties);
  CPPUNIT_TEST(testALAWProperties);
  CPPUNIT_TEST(testFloatProperties);
  CPPUNIT_TEST(testFloatWithoutFactChunkProperties);
  CPPUNIT_TEST(testZeroSizeDataChunk);
  CPPUNIT_TEST(testID3v2Tag);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testInfoTag);
  CPPUNIT_TEST(testStripTags);
  CPPUNIT_TEST(testDuplicateTags);
  CPPUNIT_TEST(testFuzzedFile1);
  CPPUNIT_TEST(testFuzzedFile2);
  CPPUNIT_TEST(testFileWithGarbageAppended);
  CPPUNIT_TEST(testStripAndProperties);
  CPPUNIT_TEST(testPCMWithFactChunk);
  CPPUNIT_TEST(testWaveFormatExtensible);
  CPPUNIT_TEST(testInvalidChunk);
  CPPUNIT_TEST(testRF64IsSupported);
  CPPUNIT_TEST(testRF64Properties);
  CPPUNIT_TEST(testRF64Save);
  CPPUNIT_TEST(testRF64SaveRepairsClobberedSize);
  CPPUNIT_TEST(testRIFFInfoProperties);
  CPPUNIT_TEST(testBEXTTag);
  CPPUNIT_TEST(testBEXTTagWithOtherTags);
  CPPUNIT_TEST(testiXMLTag);
  CPPUNIT_TEST(testiXMLTagWithOtherTags);
  CPPUNIT_TEST(testInfoLatin1Default);
  CPPUNIT_TEST(testInfoWindows1252);
  CPPUNIT_TEST(testInfoUtf8);
  CPPUNIT_TEST(testInfoLatin1SaveAsUtf8);
  CPPUNIT_TEST(testInfoWindows1252SaveAsUtf8);
  CPPUNIT_TEST(testInfoUtf8SaveAsUtf8);
  CPPUNIT_TEST(testInfoSaveWithoutFieldsWritesNoCset);
  CPPUNIT_TEST(testInfoSaveWithoutModificationPreservesTag);
  CPPUNIT_TEST(testInfoStripRemovesCset);
  CPPUNIT_TEST(testInfoCodePageCatalog);
  CPPUNIT_TEST(testInfoCodePageCatalogSaveAsUtf8);
  CPPUNIT_TEST(testCharsetTablesRoundTrip);
  CPPUNIT_TEST(testCharsetDecodeStopsAtNul);
  CPPUNIT_TEST(testInfoUserHandlerBeatsCodePage);
  CPPUNIT_TEST(testInfoCodePageReturnsWithoutUserHandler);
  CPPUNIT_TEST_SUITE_END();

public:

  void testPCMProperties()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("empty.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3675, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(32, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(1000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(3675U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->format());
  }

  void testALAWProperties()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("alaw.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3550, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(128, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(8000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(8, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(28400U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(6, f.audioProperties()->format());
  }

  void testFloatProperties()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("float64.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(97, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(5645, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(4281U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->format());
  }

  void testFloatWithoutFactChunkProperties()
  {
    ByteVector wavData = PlainFile(TEST_FILE_PATH_C("float64.wav")).readAll();
    CPPUNIT_ASSERT_EQUAL(ByteVector("fact"), wavData.mid(36, 4));
    // Remove the fact chunk by renaming it to fakt
    wavData[38] = 'k';
    ByteVectorStream wavStream(wavData);
    RIFF::WAV::File f(&wavStream);
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(97, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(5645, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(64, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(4281U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->format());
  }

  void testZeroSizeDataChunk()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("zero-size-chunk.wav"));
    CPPUNIT_ASSERT(f.isValid());
  }

  void testID3v2Tag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());

      f.ID3v2Tag()->setTitle(L"Title");
      f.ID3v2Tag()->setArtist(L"Artist");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L"Title"),  f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L"Artist"), f.ID3v2Tag()->artist());

      f.ID3v2Tag()->setTitle(L"");
      f.ID3v2Tag()->setArtist(L"");
      f.save();
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.ID3v2Tag()->artist());
    }
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("empty", ".wav");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      RIFF::WAV::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(RIFF::WAV::File::AllTags, File::StripOthers, ID3v2::v3);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      RIFF::WAV::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testInfoTag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasInfoTag());

      f.InfoTag()->setTitle(L"Title");
      f.InfoTag()->setArtist(L"Artist");
      f.save();
      CPPUNIT_ASSERT(f.hasInfoTag());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"Title"),  f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L"Artist"), f.InfoTag()->artist());

      f.InfoTag()->setTitle(L"");
      f.InfoTag()->setArtist(L"");
      f.save();
      CPPUNIT_ASSERT(!f.hasInfoTag());
    }

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(String(L""), f.InfoTag()->artist());
    }
  }

  void testInfoLatin1Default()
  {
    ByteVectorStream stream(latin1Riff());
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"),  f.InfoTag()->album());
    CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"),  f.InfoTag()->artist());
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"),   f.InfoTag()->comment());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2024), f.InfoTag()->year());
  }

  void testInfoWindows1252()
  {
    ByteVectorStream stream(codePageRiff(1252));
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasInfoTag());
    assertDecodedRepertoire(f.InfoTag()->album(), repertoireFor(1252), 1252, "IPRD");
  }

  void testInfoUtf8()
  {
    ByteVectorStream stream(utf8Riff());
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"), f.InfoTag()->album());
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2024), f.InfoTag()->year());
  }

  // Each of the three files is put through the same two steps.  First it must
  // parse, with the charset its own CSET chunk declares.  Then its tags are
  // changed and written back, which rewrites the INFO chunk as UTF-8 and
  // declares that in a CSET chunk, and the result must be read back correctly.

  void testInfoLatin1SaveAsUtf8()
  {
    // Step 1: the file has no CSET chunk, so it is Latin-1 by default.
    {
      ByteVectorStream stream(latin1Riff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"), f.InfoTag()->album());
      CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"), f.InfoTag()->artist());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"), f.InfoTag()->comment());
      CPPUNIT_ASSERT_EQUAL(2024u, f.InfoTag()->year());
    }

    // Step 2: change the tags and write it out.
    ByteVectorStream stream(latin1Riff());
    {
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());

      f.InfoTag()->setTitle(kNewTitle);
      f.InfoTag()->setComment(kNewComment);
      CPPUNIT_ASSERT(f.save());
      CPPUNIT_ASSERT(f.hasInfoTag());
    }
    const ByteVector saved = *stream.data();

    // What was written must be a well formed RIFF file that declares UTF-8.
    assertRawCsetDeclaresUtf8(saved);
    const ByteVector list = checkedRawChunk(saved, "LIST");
    CPPUNIT_ASSERT(list.startsWith("INFO"));
    CPPUNIT_ASSERT(list.find(kNewTitleUtf8) >= 0);
    CPPUNIT_ASSERT(list.find(kNewCommentUtf8) >= 0);

    // Reading it back must give the changed tags, plus the fields that were left
    // alone unchanged by the Latin-1 to UTF-8 rewrite.
    {
      ByteVectorStream savedStream(saved);
      RIFF::WAV::File f(&savedStream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(kNewTitle, f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(kNewComment, f.InfoTag()->comment());
      CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"), f.InfoTag()->album());
      CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"), f.InfoTag()->artist());
      CPPUNIT_ASSERT_EQUAL(2024u, f.InfoTag()->year());
    }
  }

  void testInfoWindows1252SaveAsUtf8()
  {
    // Step 1: the file declares code page 1252 in its CSET chunk.
    {
      ByteVectorStream stream(codePageRiff(1252));
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      assertDecodedRepertoire(f.InfoTag()->album(), repertoireFor(1252), 1252, "IPRD");
    }

    // Step 2: change the tags and write it out.
    ByteVectorStream stream(codePageRiff(1252));
    {
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());

      f.InfoTag()->setTitle(kNewTitle);
      f.InfoTag()->setComment(kNewComment);
      CPPUNIT_ASSERT(f.save());
    }
    const ByteVector saved = *stream.data();

    // It used to declare code page 1252 and now declares UTF-8.
    assertRawCsetDeclaresUtf8(saved);
    const ByteVector list = checkedRawChunk(saved, "LIST");
    CPPUNIT_ASSERT(list.find(kNewTitleUtf8) >= 0);
    CPPUNIT_ASSERT(list.find(kNewCommentUtf8) >= 0);

    {
      ByteVectorStream savedStream(saved);
      RIFF::WAV::File f(&savedStream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(kNewTitle, f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(kNewComment, f.InfoTag()->comment());
      assertDecodedRepertoire(f.InfoTag()->album(), repertoireFor(1252), 1252, "IPRD");
    }
  }

  void testInfoUtf8SaveAsUtf8()
  {
    // Step 1: the file already declares UTF-8, and its CSET chunk comes
    // after the LIST chunk it applies to.
    {
      ByteVectorStream stream(utf8Riff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"), f.InfoTag()->album());
      CPPUNIT_ASSERT_EQUAL(2024u, f.InfoTag()->year());
    }

    // Step 2: change the tags and write it out.
    ByteVectorStream stream(utf8Riff());
    {
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());

      f.InfoTag()->setTitle(kNewTitle);
      f.InfoTag()->setComment(kNewComment);
      CPPUNIT_ASSERT(f.save());
    }
    const ByteVector saved = *stream.data();

    assertRawCsetDeclaresUtf8(saved);
    const ByteVector list = checkedRawChunk(saved, "LIST");
    CPPUNIT_ASSERT(list.find(kNewTitleUtf8) >= 0);
    CPPUNIT_ASSERT(list.find(kNewCommentUtf8) >= 0);

    {
      ByteVectorStream savedStream(saved);
      RIFF::WAV::File f(&savedStream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(kNewTitle, f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(kNewComment, f.InfoTag()->comment());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"), f.InfoTag()->album());
      CPPUNIT_ASSERT_EQUAL(2024u, f.InfoTag()->year());
    }
  }

  void testInfoSaveWithoutFieldsWritesNoCset()
  {
    ByteVectorStream stream(latin1Riff());
    {
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());

      for(const auto &[id, value] : f.InfoTag()->fieldListMap())
        f.InfoTag()->removeField(id);

      CPPUNIT_ASSERT(f.save());
      // Nothing is left to describe, so no CSET chunk is written either.
      CPPUNIT_ASSERT(!f.hasInfoTag());
    }

    const ByteVector saved = *stream.data();
    CPPUNIT_ASSERT(checkedRawChunk(saved, "CSET").isEmpty());
    CPPUNIT_ASSERT(checkedRawChunk(saved, "LIST").isEmpty());
  }

  // Saving a file that already has an INFO tag must keep that tag. It comes
  // back in UTF-8 with a CSET chunk saying so, whatever it was read as.
  void testInfoSaveWithoutModificationPreservesTag()
  {
    struct {
      ByteVector file;
      const wchar_t *album;   // for the files that are not one code page
      unsigned int codePage;  // or the code page, checked against its repertoire
    } cases[] = {
      {latin1Riff(),         L"A \xAE B", 0u},
      {codePageRiff(1251),   nullptr,      1251u},
      {codePageRiff(1252),   nullptr,      1252u},
      {utf8Riff(),           L"Caf\xE9",  0u}
    };

    for(const auto &c : cases) {
      // Save without touching a single field.
      ByteVectorStream stream(c.file);
      {
        RIFF::WAV::File f(&stream);
        CPPUNIT_ASSERT(f.isValid());
        CPPUNIT_ASSERT(f.hasInfoTag());
        CPPUNIT_ASSERT(f.save());
      }

      // The tag survived, and the file now declares UTF-8 for it.
      const ByteVector saved = *stream.data();
      assertRawCsetDeclaresUtf8(saved);
      CPPUNIT_ASSERT(checkedRawChunk(saved, "LIST").startsWith("INFO"));

      ByteVectorStream savedStream(saved);
      RIFF::WAV::File f(&savedStream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      if(c.codePage)
        assertDecodedRepertoire(f.InfoTag()->album(), repertoireFor(c.codePage), c.codePage, "IPRD");
      else
        CPPUNIT_ASSERT_EQUAL(String(c.album), f.InfoTag()->album());
    }
  }

  // Stripping the INFO tag takes the CSET chunk with it. A CSET that outlived
  // its LIST would describe a charset for text the file no longer has.
  void testInfoStripRemovesCset()
  {
    const ByteVector files[] = {latin1Riff(), utf8Riff(), codePageRiff(1251), codePageRiff(1252)};

    for(const ByteVector &file : files) {
      ByteVectorStream stream(file);
      {
        RIFF::WAV::File f(&stream);
        CPPUNIT_ASSERT(f.isValid());
        CPPUNIT_ASSERT(f.hasInfoTag());

        f.strip(RIFF::WAV::File::Info);
        CPPUNIT_ASSERT(f.InfoTag()->isEmpty());

        CPPUNIT_ASSERT(f.save(RIFF::WAV::File::Info));
        CPPUNIT_ASSERT(!f.hasInfoTag());
      }

      const ByteVector saved = *stream.data();
      CPPUNIT_ASSERT(checkedRawChunk(saved, "LIST").isEmpty());
      CPPUNIT_ASSERT(checkedRawChunk(saved, "CSET").isEmpty());
    }
  }

  // Step 1 for every single byte code page Charset supports, driven off one
  // table so a code page cannot be added to the tables without also being read
  // back through a file. Each file holds the whole byte range in each of its
  // text fields and all of it is compared, so a table that is wrong at any one
  // of the 255 positions fails instead of only where a sample differed.
  void testInfoCodePageCatalog()
  {
    for(const auto &c : charsetCases) {
      ByteVectorStream stream(codePageRiff(c.codePage));
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      assertDecodedRepertoire(f.InfoTag()->artist(), c.repertoire, c.codePage, "IART");
      assertDecodedRepertoire(f.InfoTag()->album(), c.repertoire, c.codePage, "IPRD");
      assertDecodedRepertoire(f.InfoTag()->comment(), c.repertoire, c.codePage, "ICMT");
    }
  }

  // Step 2 for every code page in that table. Each file is retitled and written
  // back as UTF-8, and the untouched fields are then read back from the result:
  // they can only survive that rewrite if the code page was decoded correctly
  // in the first place. ICMT is the one field the rewrite replaces.
  void testInfoCodePageCatalogSaveAsUtf8()
  {
    for(const auto &c : charsetCases) {
      ByteVectorStream stream(codePageRiff(c.codePage));
      {
        RIFF::WAV::File f(&stream);
        CPPUNIT_ASSERT(f.isValid());
        CPPUNIT_ASSERT(f.hasInfoTag());
        assertDecodedRepertoire(f.InfoTag()->artist(), c.repertoire, c.codePage, "IART");

        f.InfoTag()->setTitle(kNewTitle);
        f.InfoTag()->setComment(kNewComment);
        CPPUNIT_ASSERT(f.save());
      }

      // Whatever it was read as, it is UTF-8 on disk now.
      const ByteVector saved = *stream.data();
      assertRawCsetDeclaresUtf8(saved);
      const ByteVector list = checkedRawChunk(saved, "LIST");
      CPPUNIT_ASSERT(list.find(kNewTitleUtf8) >= 0);
      CPPUNIT_ASSERT(list.find(kNewCommentUtf8) >= 0);

      {
        ByteVectorStream savedStream(saved);
        RIFF::WAV::File f(&savedStream);
        CPPUNIT_ASSERT(f.isValid());
        CPPUNIT_ASSERT(f.hasInfoTag());
        CPPUNIT_ASSERT_EQUAL(kNewTitle, f.InfoTag()->title());
        CPPUNIT_ASSERT_EQUAL(kNewComment, f.InfoTag()->comment());
        // The 255 characters that were never touched are still all there.
        assertDecodedRepertoire(f.InfoTag()->artist(), c.repertoire, c.codePage, "IART");
        assertDecodedRepertoire(f.InfoTag()->album(), c.repertoire, c.codePage, "IPRD");
      }
    }
  }

  // Byte 0x00 is the one byte of the repertoire that cannot be part of a field's
  // text, because decoding stops at the first NUL: that is how a string ends.
  // It has to end the string and not turn into a character of its own.
  void testCharsetDecodeStopsAtNul()
  {
    const ByteVector data("\x41\x00\x42", 3);
    CPPUNIT_ASSERT_EQUAL(String(L"A"), Charset::decode(data, Charset::Type::Windows1251));
    CPPUNIT_ASSERT_EQUAL(String(), Charset::decode(ByteVector("\x00\x41", 2), Charset::Type::Windows1251));
  }

  // The generated tables have to cover every code page that can be declared, and
  // the internal representation of a string is UTF-8 from the moment it is read.
  // There is no way to encode back into a legacy code page, precisely so that
  // text a code page cannot hold is never quietly replaced.
  void testCharsetTablesRoundTrip()
  {
    unsigned int count = 0;
    const Charset::detail::CodePageEntry *entries =
      Charset::detail::codePageEntries(&count);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(std::size(charsetCases)), count);

    // Every code page in the table is one the catalog knows how to read back,
    // so a page cannot be added to Charset without a fixture to prove it.
    for(unsigned int i = 0; i < count; ++i) {
      bool covered = false;
      for(const auto &c : charsetCases)
        covered = covered || c.codePage == entries[i].codePage;
      CPPUNIT_ASSERT(covered);
      CPPUNIT_ASSERT(Charset::typeForCodePage(entries[i].codePage));
    }
    CPPUNIT_ASSERT_EQUAL(Charset::Type::UTF8, Charset::typeForCodePage(65001).value());

    // Encoding is UTF-8 and nothing else, so text no single byte code page could
    // ever represent survives instead of turning into '?'.
    CPPUNIT_ASSERT_EQUAL(ByteVector("\xE4\xBD\xA0", 3), Charset::encode(String(L"\x4F60")));
    CPPUNIT_ASSERT_EQUAL(kNewTitleUtf8, Charset::encode(kNewTitle));

    // And it is a plain UTF-8 round trip, so nothing is lost on the way through.
    const String sample(L"\x0416\x0490\x05D0\x0625\x0E01\x20A7");
    CPPUNIT_ASSERT_EQUAL(sample, Charset::decode(Charset::encode(sample), Charset::Type::UTF8));
  }

  // A handler the user installed is an explicit override, so it outranks the
  // code page the file declares. Otherwise setStringHandler() would do nothing
  // at all for every file TagLib itself writes, since those all carry a CSET.
  class FixedStringHandler : public RIFF::Info::StringHandler
  {
  public:
    String parse(const ByteVector &) const override { return String(L"from the handler"); }
    ByteVector render(const String &) const override { return ByteVector("from the handler"); }
  };

  void testInfoUserHandlerBeatsCodePage()
  {
    class Restore
    {
    public:
      ~Restore() { RIFF::Info::Tag::setStringHandler(nullptr); }
    } restore;

    // The file declares 1252, and the default path reads it with that table.
    {
      ByteVectorStream stream(codePageRiff(1252));
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT(f.InfoTag()->artist() != String(L"from the handler"));
    }

    const FixedStringHandler handler;
    RIFF::Info::Tag::setStringHandler(&handler);

    // With a handler installed it is used even though the file declares 1252,
    // on the way in ...
    {
      ByteVectorStream stream(codePageRiff(1252));
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"from the handler"), f.InfoTag()->artist());
    }

    // ... and on the way out, so the handler decides the bytes as well.
    ByteVectorStream stream(codePageRiff(1252));
    {
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"from the handler"), f.InfoTag()->artist());
      CPPUNIT_ASSERT(f.save());
    }
    CPPUNIT_ASSERT(checkedRawChunk(*stream.data(), "LIST")
                     .find(ByteVector("from the handler")) >= 0);
  }

  // Removing the handler again must put the file's own code page back in charge.
  void testInfoCodePageReturnsWithoutUserHandler()
  {
    ByteVectorStream stream(codePageRiff(1251));
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT(f.InfoTag()->artist() != String(L"from the handler"));
    assertDecodedRepertoire(f.InfoTag()->artist(), repertoireFor(1251), 1251, "IART");
  }

  void testStripTags()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      f.ID3v2Tag()->setTitle("test title");
      f.InfoTag()->setTitle("test title");
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasInfoTag());
      f.save(RIFF::WAV::File::ID3v2, File::StripOthers);
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasInfoTag());
      f.ID3v2Tag()->setTitle("test title");
      f.InfoTag()->setTitle("test title");
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasInfoTag());
      f.save(RIFF::WAV::File::Info, File::StripOthers);
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasInfoTag());
    }
  }

  void testDuplicateTags()
  {
    ScopedFileCopy copy("duplicate_tags", ".wav");

    RIFF::WAV::File f(copy.fileName().c_str());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(17052), f.length());

    // duplicate_tags.wav has duplicate ID3v2/INFO tags.
    // title() returns "Title2" if can't skip the second tag.

    CPPUNIT_ASSERT(f.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.ID3v2Tag()->title());

    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT_EQUAL(String("Title1"), f.InfoTag()->title());

    f.save();
    // Saving the INFO tag also writes the 16 byte CSET chunk (8 byte header
    // plus the 8 byte payload) which declares that the tag is UTF-8.
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(15914), f.length());
    CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(-1), f.find("Title2"));
  }

  void testFuzzedFile1()
  {
    RIFF::WAV::File f1(TEST_FILE_PATH_C("infloop.wav"));
    CPPUNIT_ASSERT(f1.isValid());
    // The file has problems:
    // Chunk 'ISTt' has invalid size (larger than the file size).
    // Its properties can nevertheless be read.
    RIFF::WAV::Properties* properties = f1.audioProperties();
    CPPUNIT_ASSERT_EQUAL(1, properties->channels());
    CPPUNIT_ASSERT_EQUAL(88, properties->bitrate());
    CPPUNIT_ASSERT_EQUAL(8, properties->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(11025, properties->sampleRate());
    CPPUNIT_ASSERT(!f1.hasInfoTag());
    CPPUNIT_ASSERT(!f1.hasID3v2Tag());
  }

  void testFuzzedFile2()
  {
    RIFF::WAV::File f2(TEST_FILE_PATH_C("segfault.wav"));
    CPPUNIT_ASSERT(f2.isValid());
  }

  void testFileWithGarbageAppended()
  {
    ScopedFileCopy copy("empty", ".wav");
    ByteVector contentsBeforeModification;
    {
      FileStream stream(copy.fileName().c_str());
      stream.seek(0, IOStream::End);
      constexpr char garbage[] = "\r2345678";
      stream.writeBlock(ByteVector(garbage, sizeof(garbage) - 1));
      stream.seek(0);
      contentsBeforeModification = stream.readBlock(stream.length());
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      f.ID3v2Tag()->setTitle("ID3v2 Title");
      f.InfoTag()->setTitle("INFO Title");
      CPPUNIT_ASSERT(f.save());
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      f.strip();
    }
    {
      FileStream stream(copy.fileName().c_str());
      ByteVector contentsAfterModification = stream.readBlock(stream.length());
      CPPUNIT_ASSERT_EQUAL(contentsBeforeModification, contentsAfterModification);
    }
  }

  void testStripAndProperties()
  {
    ScopedFileCopy copy("empty", ".wav");

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      f.ID3v2Tag()->setTitle("ID3v2");
      f.InfoTag()->setTitle("INFO");
      f.save();
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL(String("ID3v2"), f.properties()["TITLE"].front());
      f.strip(RIFF::WAV::File::ID3v2);
      CPPUNIT_ASSERT_EQUAL(String("INFO"), f.properties()["TITLE"].front());
      f.strip(RIFF::WAV::File::Info);
      CPPUNIT_ASSERT(f.properties().isEmpty());
    }
  }

  void testPCMWithFactChunk()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("pcm_with_fact_chunk.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(3675, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(32, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(1000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(16, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(3675U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->format());
  }

  void testWaveFormatExtensible()
  {
    RIFF::WAV::File f(TEST_FILE_PATH_C("uint8we.wav"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(2937, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(128, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(8000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(8, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(23493U, f.audioProperties()->sampleFrames());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->format());
  }

  void testInvalidChunk()
  {
    ScopedFileCopy copy("invalid-chunk", ".wav");

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      f.ID3v2Tag()->setTitle("Title");
      f.save();
    }
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
  }

  // rf64.wav is a 50 ms RF64: 0xffffffff sentinels in the 32-bit size fields at offset 4 and
  // in the "data" chunk header, with the real sizes in a leading "ds64" chunk. That is what a
  // WAVE file becomes past 4 GB; the sentinels behave the same at any size, so the fixture is
  // small.

  static void setMagic(const std::string &fileName, const ByteVector &magic)
  {
    FileStream stream(fileName.c_str());
    stream.seek(0);
    stream.writeBlock(magic);
  }

  void testRF64IsSupported()
  {
    ScopedFileCopy copy("rf64", ".wav");
    {
      FileStream stream(copy.fileName().c_str(), true);
      CPPUNIT_ASSERT(RIFF::WAV::File::isSupported(&stream));
    }
    setMagic(copy.fileName(), "BW64");
    {
      FileStream stream(copy.fileName().c_str(), true);
      CPPUNIT_ASSERT(RIFF::WAV::File::isSupported(&stream));
    }
    setMagic(copy.fileName(), "XX64");
    {
      FileStream stream(copy.fileName().c_str(), true);
      CPPUNIT_ASSERT(!RIFF::WAV::File::isSupported(&stream));
    }
  }

  void testRF64Properties()
  {
    ScopedFileCopy copy("rf64", ".wav");

    // Bytes past the audio, so that clamping the sentinel to what is available gives a
    // different answer from "ds64" and the test can tell which one was used.
    {
      FileStream stream(copy.fileName().c_str());
      stream.seek(0, IOStream::End);
      stream.writeBlock(ByteVector("junk", 4) + ByteVector::fromUInt(1000, false) +
                        ByteVector(1000, '\0'));
    }

    RIFF::WAV::File f(copy.fileName().c_str());
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT_EQUAL(50, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(48000, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
  }

  void testRF64Save()
  {
    ScopedFileCopy copy("rf64", ".wav");

    offset_t originalLength = 0;
    {
      FileStream stream(copy.fileName().c_str(), true);
      originalLength = stream.length();
    }

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      PropertyMap properties;
      properties["TITLE"] = StringList("Title");
      properties["ARTIST"] = StringList("Artist");
      CPPUNIT_ASSERT(f.setProperties(properties).isEmpty());
      CPPUNIT_ASSERT(f.save());
    }

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      const PropertyMap properties = f.properties();
      CPPUNIT_ASSERT(properties.contains("TITLE"));
      CPPUNIT_ASSERT(properties.contains("ARTIST"));
      CPPUNIT_ASSERT_EQUAL(String("Title"), properties["TITLE"].front());
      CPPUNIT_ASSERT_EQUAL(String("Artist"), properties["ARTIST"].front());
      CPPUNIT_ASSERT_EQUAL(50, f.audioProperties()->lengthInMilliseconds());
    }

    {
      FileStream stream(copy.fileName().c_str(), true);
      const offset_t length = stream.length();
      CPPUNIT_ASSERT(length > originalLength);

      // The 32-bit field has to stay a sentinel: a real number there makes readers stop
      // consulting "ds64", which past 4 GB is the only place the size fits.
      stream.seek(4);
      CPPUNIT_ASSERT_EQUAL(0xffffffffU, stream.readBlock(4).toUInt(false));

      // "ds64" carries the real size, so it is what has to track the file's growth.
      stream.seek(20);
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned long long>(length - 8),
                           stream.readBlock(8).toULongLong(false));

      // The audio's own extent is untouched.
      stream.seek(28);
      CPPUNIT_ASSERT_EQUAL(9600ULL, stream.readBlock(8).toULongLong(false));
    }
  }

  void testRF64SaveRepairsClobberedSize()
  {
    ScopedFileCopy copy("rf64", ".wav");

    // A real total where the sentinel belongs, as an earlier version of this code left it. The
    // value is malformed in a long-form file at any size, and past 4 GB it is also truncated,
    // which is what makes readers report milliseconds for hours of audio.
    {
      FileStream stream(copy.fileName().c_str());
      stream.seek(4);
      stream.writeBlock(ByteVector::fromUInt(5230, false));
    }

    {
      RIFF::WAV::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      f.InfoTag()->setTitle("Title");
      CPPUNIT_ASSERT(f.save());
    }

    {
      FileStream stream(copy.fileName().c_str(), true);
      const offset_t length = stream.length();

      stream.seek(4);
      CPPUNIT_ASSERT_EQUAL(0xffffffffU, stream.readBlock(4).toUInt(false));

      stream.seek(20);
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned long long>(length - 8),
                           stream.readBlock(8).toULongLong(false));

      stream.seek(28);
      CPPUNIT_ASSERT_EQUAL(9600ULL, stream.readBlock(8).toULongLong(false));
    }
  }

  void testRIFFInfoProperties()
  {
    PropertyMap tags;
    tags["ALBUM"] = StringList("Album");
    tags["ARRANGER"] = StringList("Arranger");
    tags["ARTIST"] = StringList("Artist");
    tags["ARTISTWEBPAGE"] = StringList("Artist Webpage");
    tags["BPM"] = StringList("123");
    tags["COMMENT"] = StringList("Comment");
    tags["COMPOSER"] = StringList("Composer");
    tags["COPYRIGHT"] = StringList("2023 Copyright");
    tags["DATE"] = StringList("2023");
    tags["DISCSUBTITLE"] = StringList("Disc Subtitle");
    tags["ENCODEDBY"] = StringList("Encoded by");
    tags["ENCODING"] = StringList("Encoding");
    tags["ENCODINGTIME"] = StringList("2023-11-25 15:42:39");
    tags["GENRE"] = StringList("Genre");
    tags["ISRC"] = StringList("UKAAA0500001");
    tags["LABEL"] = StringList("Label");
    tags["LANGUAGE"] = StringList("eng");
    tags["LYRICIST"] = StringList("Lyricist");
    tags["MEDIA"] = StringList("Media");
    tags["PERFORMER"] = StringList("Performer");
    tags["RELEASECOUNTRY"] = StringList("Release Country");
    tags["REMIXER"] = StringList("Remixer");
    tags["TITLE"] = StringList("Title");
    tags["TRACKNUMBER"] = StringList("2/4");

    ScopedFileCopy copy("empty", ".wav");
    {
      RIFF::WAV::File f(copy.fileName().c_str());
      RIFF::Info::Tag *infoTag = f.InfoTag();
      CPPUNIT_ASSERT(infoTag->isEmpty());
      PropertyMap properties = infoTag->properties();
      CPPUNIT_ASSERT(properties.isEmpty());
      infoTag->setProperties(tags);
      f.save();
    }
    {
      const RIFF::WAV::File f(copy.fileName().c_str());
      RIFF::Info::Tag *infoTag = f.InfoTag();
      CPPUNIT_ASSERT(!infoTag->isEmpty());
      PropertyMap properties = infoTag->properties();
      if (tags != properties) {
        CPPUNIT_ASSERT_EQUAL(tags.toString(), properties.toString());
      }
      CPPUNIT_ASSERT(tags == properties);

      const RIFF::Info::FieldListMap expectedFields = {
        {"IPRD", "Album"},
        {"IENG", "Arranger"},
        {"IART", "Artist"},
        {"IBSU", "Artist Webpage"},
        {"IBPM", "123"},
        {"ICMT", "Comment"},
        {"IMUS", "Composer"},
        {"ICOP", "2023 Copyright"},
        {"ICRD", "2023"},
        {"PRT1", "Disc Subtitle"},
        {"ITCH", "Encoded by"},
        {"ISFT", "Encoding"},
        {"IDIT", "2023-11-25 15:42:39"},
        {"IGNR", "Genre"},
        {"ISRC", "UKAAA0500001"},
        {"IPUB", "Label"},
        {"ILNG", "eng"},
        {"IWRI", "Lyricist"},
        {"IMED", "Media"},
        {"ISTR", "Performer"},
        {"ICNT", "Release Country"},
        {"IEDT", "Remixer"},
        {"INAM", "Title"},
        {"IPRT", "2/4"}
      };
      CPPUNIT_ASSERT(expectedFields == infoTag->fieldListMap());
    }
  }

  void testBEXTTag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasBEXTData());
      CPPUNIT_ASSERT(f.BEXTData().isEmpty());

      f.setBEXTData(ByteVector("test bext data"));
      f.save();
      CPPUNIT_ASSERT(f.hasBEXTData());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasBEXTData());
      CPPUNIT_ASSERT_EQUAL(ByteVector("test bext data"), f.BEXTData());

      f.setBEXTData(ByteVector());
      f.save();
      CPPUNIT_ASSERT(!f.hasBEXTData());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasBEXTData());
      CPPUNIT_ASSERT(f.BEXTData().isEmpty());
    }

    // Check if file without BEXT is same as original empty file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("empty.wav")).readAll();
    const ByteVector fileData = PlainFile(filename.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

  void testBEXTTagWithOtherTags()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      f.ID3v2Tag()->setTitle("ID3v2 Title");
      f.InfoTag()->setTitle("INFO Title");
      f.setBEXTData(ByteVector("bext payload"));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT(f.hasBEXTData());
      CPPUNIT_ASSERT_EQUAL(String("ID3v2 Title"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("INFO Title"), f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(ByteVector("bext payload"), f.BEXTData());
    }
  }

  void testiXMLTag()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasiXMLData());
      CPPUNIT_ASSERT(f.iXMLData().isEmpty());

      f.setiXMLData("<BWFXML><IXML_VERSION>1.0</IXML_VERSION></BWFXML>");
      f.save();
      CPPUNIT_ASSERT(f.hasiXMLData());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasiXMLData());
      CPPUNIT_ASSERT_EQUAL(
        String("<BWFXML><IXML_VERSION>1.0</IXML_VERSION></BWFXML>"),
        f.iXMLData());

      f.setiXMLData(String());
      f.save();
      CPPUNIT_ASSERT(!f.hasiXMLData());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasiXMLData());
      CPPUNIT_ASSERT(f.iXMLData().isEmpty());
    }

    // Check if file without iXML is same as original empty file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("empty.wav")).readAll();
    const ByteVector fileData = PlainFile(filename.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

  void testiXMLTagWithOtherTags()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    {
      RIFF::WAV::File f(filename.c_str());
      f.ID3v2Tag()->setTitle("ID3v2 Title");
      f.setiXMLData("<BWFXML><SCENE>1</SCENE></BWFXML>");
      f.setBEXTData(ByteVector("bext data"));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasiXMLData());
      CPPUNIT_ASSERT(f.hasBEXTData());
      CPPUNIT_ASSERT_EQUAL(String("ID3v2 Title"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(
        String("<BWFXML><SCENE>1</SCENE></BWFXML>"),
        f.iXMLData());
      CPPUNIT_ASSERT_EQUAL(ByteVector("bext data"), f.BEXTData());

      f.setiXMLData(String());
      f.setBEXTData(ByteVector());
      f.strip();
      CPPUNIT_ASSERT(f.save());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasiXMLData());
      CPPUNIT_ASSERT(f.iXMLData().isEmpty());
      CPPUNIT_ASSERT(!f.hasBEXTData());
      CPPUNIT_ASSERT(f.BEXTData().isEmpty());
    }

    // Check if file without tags is same as original empty file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("empty.wav")).readAll();
    const ByteVector fileData = PlainFile(filename.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
