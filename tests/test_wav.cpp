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
#include <string>
#include <utility>
#include <cstdio>

#include "taglib_config.h"
#include "tcharset.h"
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

#ifdef TAGLIB_WITH_CHARSETTABLES
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

  // The payload of a conforming CSET chunk, which the RIFF specification lays
  // out as wCodePage, wCountryCode, wLanguageCode and wDialect, one WORD each.
  // Only the code page matters here, so the other three are the zero that means
  // "no opinion".
  ByteVector riffCsetPayload(unsigned int codePage)
  {
    return ByteVector::fromShort(static_cast<short>(codePage), false) + ByteVector(6, '\0');
  }

  // The same layout, with the country, language and dialect the file was written
  // for filled in as well. These are what a reader that took the code page for a
  // 32 bit number mixed into its result.
  ByteVector riffCsetPayloadWithLanguage(unsigned int codePage)
  {
    return ByteVector::fromShort(static_cast<short>(codePage), false) +
           ByteVector::fromShort(1, false) + ByteVector::fromShort(9, false) +
           ByteVector::fromShort(1, false);
  }

  // One field of an INFO list, as the reader will find it: an id and the text
  // in whatever encoding the file declares, without the NUL that ends it.
  struct InfoField
  {
    const char *id;
    ByteVector text;
  };

  // A WAV file with an INFO tag holding \a fields, and a CSET chunk whose raw
  // payload is \a cset. An empty \a cset means no CSET chunk at all, which is
  // what a file that declares no charset looks like. The CSET is written in
  // front of the LIST it describes, unless \a csetBehindList is set, which is
  // where a file TagLib did not write tends to put it.
  ByteVector riffWithInfoCset(std::initializer_list<InfoField> fields,
                              const ByteVector &cset, bool csetBehindList = false)
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

    if(!cset.isEmpty() && !csetBehindList)
      body.append(riffChunk("CSET", cset));
    body.append(riffChunk("LIST", list));
    if(!cset.isEmpty() && csetBehindList)
      body.append(riffChunk("CSET", cset));

    return ByteVector("RIFF", 4) + ByteVector::fromUInt(body.size() + 4, false) +
           ByteVector("WAVE", 4) + body;
  }

  // The same file, with the charset declared by \a codePage in the conforming
  // 8 byte CSET layout, or with no CSET chunk at all when \a codePage is 0.
  ByteVector riffWithInfo(std::initializer_list<InfoField> fields,
                          unsigned int codePage = 0, bool csetBehindList = false)
  {
    return riffWithInfoCset(fields, codePage ? riffCsetPayload(codePage) : ByteVector(),
                            csetBehindList);
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

  // A WAV file whose INFO text is in \a codePage, holding samples that a
  // different table would decode differently: 0x80 is the Euro sign in 1252
  // and U+0080 in Latin-1, 0x93/0x94 are curly quotes in 1252 and C1 controls
  // in Latin-1, 0xc0..0xc2 are Cyrillic capitals in 1251 and accented Latin
  // letters in Latin-1. So the expected strings below only come out right
  // when the page the CSET declares is the table actually used.
  ByteVector windows1252Riff()
  {
    return riffWithInfo({{"IART", ByteVector("A \x80 B", 5)},
                         {"IPRD", ByteVector("Caf\xE9 \x80", 6)},
                         {"ICMT", ByteVector("\x93quoted\x94", 10)}},
                        1252);
  }

  // The same file, with the country, language and dialect of the CSET chunk
  // filled in instead of left at their zero defaults.
  ByteVector windows1252WithLanguageRiff()
  {
    return riffWithInfoCset({{"IART", ByteVector("A \x80 B", 5)},
                             {"IPRD", ByteVector("Caf\xE9 \x80", 6)},
                             {"ICMT", ByteVector("\x93quoted\x94", 10)}},
                            riffCsetPayloadWithLanguage(1252));
  }

  ByteVector windows1251Riff()
  {
    return riffWithInfo({{"IART", ByteVector("A \xC0 B", 5)},
                         {"IPRD", ByteVector("Caf\xE9", 4)},
                         {"ICMT", ByteVector("\xC0\xC1\xC2", 3)}},
                        1251);
  }

  // The same text in a file whose CSET chunk is truncated: its payload holds a
  // single byte, where wCodePage alone needs 2, so it cannot hold the number the
  // chunk exists to declare and the file is malformed.
  ByteVector truncatedCsetRiff()
  {
    return riffWithInfoCset({{"IART", ByteVector("A \x80 B", 5)}},
                            ByteVector("\xA4", 1));
  }

  // The same text in a file whose CSET holds the code page and nothing else: the
  // three WORD fields behind wCodePage are what a conforming writer fills in, and
  // a file that leaves them out entirely still says all there is to say.
  ByteVector codePageOnlyCsetRiff()
  {
    return riffWithInfoCset({{"IART", ByteVector("A \x80 B", 5)}},
                            ByteVector::fromShort(1252, false));
  }

  // A file whose CSET declares a code page this library has no table for, here a
  // Japanese one. Its text is written in the RIFF default ISO 8859-1, so it reads
  // back as itself: the number is a hint, and a hint that cannot be followed must
  // not cost the tag its fields.
  ByteVector unsupportedCsetRiff()
  {
    return riffWithInfo({{"IART", ByteVector("A \xAE B", 5)},
                         {"ICMT", ByteVector("Caf\xE9", 4)}},
                        932);
  }

  // A file whose INFO text is UTF-8 and which declares no charset at all. The
  // specification cannot recognize this combination, because a file holding
  // ISO 8859-1 and one holding UTF-8 look the same without a CSET chunk; only
  // the reader's application knows which of them it wrote.
  ByteVector utf8WithoutCsetRiff()
  {
    return riffWithInfo({{"IART", ByteVector("Caf\xC3\xA9", 5)}}, 0);
  }

  // Puts back the code page an INFO tag assumes when a file declares none, so
  // that a test stating another one does not carry it into the next test.
  class RestoreAssumedCodePage
  {
  public:
    RestoreAssumedCodePage() :
      m_previous(RIFF::Info::Tag::assumedCodePage())
    {
    }

    ~RestoreAssumedCodePage()
    {
      RIFF::Info::Tag::setAssumedCodePage(m_previous);
    }

  private:
    const unsigned int m_previous;
  };

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
      const ByteVector chunkHeader = data.mid(static_cast<unsigned int>(offset), 8);
      CPPUNIT_ASSERT_EQUAL(8u, chunkHeader.size());

      const unsigned int size = chunkHeader.toUInt(4, false);
      const ByteVector payload = data.mid(static_cast<unsigned int>(offset + 8), size);
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
  // full 8 byte layout of four WORD fields: wCodePage is UTF-8's 65001, and the
  // country, language and dialect behind it are the zero that means the default.
  void assertRawCsetDeclaresUtf8(const ByteVector &data)
  {
    const ByteVector cset = checkedRawChunk(data, "CSET");
    CPPUNIT_ASSERT_EQUAL(8u, cset.size());
    CPPUNIT_ASSERT_EQUAL(65001u,
                         static_cast<unsigned int>(
                           static_cast<unsigned short>(cset.toShort(0, false))));
    CPPUNIT_ASSERT_EQUAL(ByteVector(6, '\0'), cset.mid(2, 6));
  }

  // Tags no single byte charset could have represented, as the strings and as
  // the UTF-8 bytes they have to end up as on disk.
  const String kNewTitle(L"Title \x4F60\x597D \x20AC");
  const ByteVector kNewTitleUtf8("Title \xE4\xBD\xA0\xE5\xA5\xBD \xE2\x82\xAC", 16);
  const String kNewComment(L"\x4F60\x597D");
  const ByteVector kNewCommentUtf8("\xE4\xBD\xA0\xE5\xA5\xBD", 6);
}
#endif // TAGLIB_WITH_CHARSETTABLES

namespace
{
  // A DISP payload: a clipboard format ID followed by data in that format.
  ByteVector dispPayload(unsigned int format, const ByteVector &data)
  {
    return ByteVector::fromUInt(format, false) + data;
  }

  // A CF_TEXT DISP payload: the NUL-terminated text, padded to an even size
  // inside the payload.
  ByteVector dispTextPayload(const char *text)
  {
    ByteVector payload = dispPayload(1, ByteVector(text) + ByteVector(1, '\0'));
    if(payload.size() % 2)
      payload.append('\0');
    return payload;
  }

  // The names and payload offsets of the top-level chunks of a RIFF file, or
  // an empty list if the chunks do not exactly fill the size in the header.
  List<std::pair<ByteVector, unsigned int>> riffChunkLayout(const ByteVector &file)
  {
    List<std::pair<ByteVector, unsigned int>> chunks;
    if(file.size() < 12 || file.toUInt(4, false) != file.size() - 8)
      return {};

    unsigned int offset = 12;
    while(offset + 8 <= file.size()) {
      const unsigned int size = file.toUInt(offset + 4, false);
      chunks.append({file.mid(offset, 4), offset + 8});
      offset += 8 + size + size % 2;
    }
    if(offset != file.size())
      return {};
    return chunks;
  }

  // The payload offsets of the chunks called \a name, in file order.
  List<unsigned int> riffChunkOffsets(const ByteVector &file, const ByteVector &name)
  {
    List<unsigned int> offsets;
    for(const auto &[chunkName, offset] : riffChunkLayout(file)) {
      if(chunkName == name)
        offsets.append(offset);
    }
    return offsets;
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
  CPPUNIT_TEST(testDISPChunk);
  CPPUNIT_TEST(testDISPChunkOddSize);
  CPPUNIT_TEST(testDISPChunkGrow);
  CPPUNIT_TEST(testDISPChunkShrink);
  CPPUNIT_TEST(testDISPChunkUntouchedOnSave);
  CPPUNIT_TEST(testDISPChunkWithOtherTags);
#ifdef TAGLIB_WITH_CHARSETTABLES
  CPPUNIT_TEST(testInfoLatin1Default);
  CPPUNIT_TEST(testInfoWindows1252);
  CPPUNIT_TEST(testInfoUtf8);
  CPPUNIT_TEST(testInfoLatin1SaveAsUtf8);
  CPPUNIT_TEST(testInfoWindows1252SaveAsUtf8);
  CPPUNIT_TEST(testInfoUtf8SaveAsUtf8);
  CPPUNIT_TEST(testInfoSaveWithoutFieldsWritesNoCset);
  CPPUNIT_TEST(testInfoSaveWithoutModificationPreservesTag);
  CPPUNIT_TEST(testInfoStripRemovesCset);
  CPPUNIT_TEST(testInfoTruncatedCsetIsFormatError);
  CPPUNIT_TEST(testInfoCsetWithLanguageFields);
  CPPUNIT_TEST(testInfoCodePageOnlyCset);
  CPPUNIT_TEST(testInfoUnsupportedCsetKeepsFields);
  CPPUNIT_TEST(testCharsetDecodeStopsAtNul);
  CPPUNIT_TEST(testCharsetEncodeIsUtf8Only);
  CPPUNIT_TEST(testInfoUserHandlerBeatsCodePage);
  CPPUNIT_TEST(testInfoCodePageReturnsWithoutUserHandler);
  CPPUNIT_TEST(testInfoAssumedCodePageDefaultIsLatin1);
  CPPUNIT_TEST(testInfoAssumedCodePageAcceptsUtf8);
  CPPUNIT_TEST(testInfoAssumedCodePageWindows1252);
  CPPUNIT_TEST(testInfoAssumedCodePageOnlyWhenUndeclared);
  CPPUNIT_TEST(testInfoAssumedUtf8FallsBackToLatin1);
  CPPUNIT_TEST(testInfoAssumedCodePageRejectsUnsupported);
#endif
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

#ifdef TAGLIB_WITH_CHARSETTABLES
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
    ByteVectorStream stream(windows1252Riff());
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9 \x20AC"), f.InfoTag()->album());
    CPPUNIT_ASSERT_EQUAL(String(L"A \x20AC B"),     f.InfoTag()->artist());
    CPPUNIT_ASSERT_EQUAL(String(L"\x201Cquoted\x201D"), f.InfoTag()->comment());
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
      ByteVectorStream stream(windows1252Riff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9 \x20AC"), f.InfoTag()->album());
    }

    // Step 2: change the tags and write it out.
    ByteVectorStream stream(windows1252Riff());
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
      // The album was never touched and survived the rewrite unchanged.
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9 \x20AC"), f.InfoTag()->album());
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
      const wchar_t *album;
    } cases[] = {
      {latin1Riff(),      L"A \xAE B"},
      {windows1251Riff(), L"Caf\x0439"},
      {windows1252Riff(), L"Caf\xE9 \x20AC"},
      {utf8Riff(),        L"Caf\xE9"}
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
      CPPUNIT_ASSERT_EQUAL(String(c.album), f.InfoTag()->album());
    }
  }

  // Stripping the INFO tag takes the CSET chunk with it. A CSET that outlived
  // its LIST would describe a charset for text the file no longer has.
  void testInfoStripRemovesCset()
  {
    const ByteVector files[] = {latin1Riff(), utf8Riff(), windows1251Riff(), windows1252Riff()};

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

  // A CSET chunk too short to hold its own wCodePage is a file format error.
  // Two bytes are the whole of wCodePage, so there is nothing shorter to read,
  // and the chunk is ignored instead of guessed at: the INFO text keeps the RIFF
  // default of Latin-1, under which 0x80 is U+0080 and not the Euro sign that the
  // page the chunk tried to declare would have made of it.
  void testInfoTruncatedCsetIsFormatError()
  {
    // Step 1: the chunk is ignored, and the text it failed to declare the
    // charset of decodes as the Latin-1 default.
    {
      ByteVectorStream stream(truncatedCsetRiff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"A \x80 B"), f.InfoTag()->artist());
    }

    // Step 2: the malformed chunk never declared a charset, so saving replaces
    // it with the one valid CSET chunk TagLib writes, instead of keeping it.
    ByteVectorStream stream(truncatedCsetRiff());
    {
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.save());
    }
    assertRawCsetDeclaresUtf8(*stream.data());
  }

  // The code page of a CSET chunk is wCodePage, one WORD of the four the layout
  // has. The country, language and dialect behind it are not part of that number:
  // a file that fills them in declares the page its first WORD holds, and has to
  // be read with that page.
  void testInfoCsetWithLanguageFields()
  {
    {
      ByteVectorStream stream(windows1252WithLanguageRiff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"A \x20AC B"),         f.InfoTag()->artist());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9 \x20AC"),     f.InfoTag()->album());
      CPPUNIT_ASSERT_EQUAL(String(L"\x201Cquoted\x201D"), f.InfoTag()->comment());
    }

    // Saving rewrites the chunk the way TagLib writes it: the code page, and the
    // three fields behind it back at their zero defaults.
    ByteVectorStream stream(windows1252WithLanguageRiff());
    {
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.save());
    }
    assertRawCsetDeclaresUtf8(*stream.data());
  }

  // wCodePage on its own is all the chunk has to say. A file that leaves the
  // three fields behind it out entirely still declares a charset.
  void testInfoCodePageOnlyCset()
  {
    ByteVectorStream stream(codePageOnlyCsetRiff());
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT_EQUAL(String(L"A \x20AC B"), f.InfoTag()->artist());
  }

  // A code page this library has no table for is a hint it cannot follow, not a
  // reason to throw the tag away. The text keeps the RIFF default of ISO 8859-1,
  // so every field is still read, and a save still writes it back.
  void testInfoUnsupportedCsetKeepsFields()
  {
    {
      ByteVectorStream stream(unsupportedCsetRiff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"), f.InfoTag()->artist());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"),  f.InfoTag()->comment());
    }

    ByteVectorStream stream(unsupportedCsetRiff());
    {
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      f.InfoTag()->setTitle(kNewTitle);
      CPPUNIT_ASSERT(f.save());
    }

    // The field was read as ISO 8859-1 and is written back as UTF-8, so the
    // character it held is in the saved LIST as its two UTF-8 bytes.
    const ByteVector saved = *stream.data();
    assertRawCsetDeclaresUtf8(saved);
    CPPUNIT_ASSERT(checkedRawChunk(saved, "LIST").find(ByteVector("A \xC2\xAE B", 6)) >= 0);

    ByteVectorStream savedStream(saved);
    RIFF::WAV::File f(&savedStream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT_EQUAL(kNewTitle,           f.InfoTag()->title());
    CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"), f.InfoTag()->artist());
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"),  f.InfoTag()->comment());
  }

  // Byte 0x00 is the one byte of a code page's repertoire that cannot be part
  // of a field's text, because decoding stops at the first NUL: that is how a
  // string ends. It has to end the string and not turn into a character.
  void testCharsetDecodeStopsAtNul()
  {
    const ByteVector data("\x41\x00\x42", 3);
    CPPUNIT_ASSERT_EQUAL(String(L"A"), Charset::decode(data, Charset::Type::Windows1251));
    CPPUNIT_ASSERT_EQUAL(String(), Charset::decode(ByteVector("\x00\x41", 2), Charset::Type::Windows1251));
  }

  // A string is decoded into TagLib's internal UTF-16 representation, which is
  // not a byte encoding at all, so there is no way to encode back into a legacy
  // code page. That is precisely what keeps text a code page cannot hold from
  // being quietly replaced.
  void testCharsetEncodeIsUtf8Only()
  {
    CPPUNIT_ASSERT_EQUAL(Charset::Type::UTF8, Charset::typeForCodePage(65001).value());

    // Encoding is UTF-8 and nothing else, so text no single byte code page could
    // ever represent survives instead of turning into '?'.
    CPPUNIT_ASSERT_EQUAL(ByteVector("\xE4\xBD\xA0", 3), Charset::encode(String(L"\x4F60"), Charset::Type::UTF8));
    CPPUNIT_ASSERT_EQUAL(kNewTitleUtf8, Charset::encode(kNewTitle, Charset::Type::UTF8));

    // And it is a plain UTF-8 round trip, so nothing is lost on the way through.
    const String sample(L"\x0416\x0490\x05D0\x0625\x0E01\x20A7");
    CPPUNIT_ASSERT_EQUAL(sample, Charset::decode(Charset::encode(sample, Charset::Type::UTF8), Charset::Type::UTF8));
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
      ByteVectorStream stream(windows1252Riff());
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
      ByteVectorStream stream(windows1252Riff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT_EQUAL(String(L"from the handler"), f.InfoTag()->artist());
    }

    // ... and on the way out, so the handler decides the bytes as well.
    ByteVectorStream stream(windows1252Riff());
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
    ByteVectorStream stream(windows1251Riff());
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT(f.InfoTag()->artist() != String(L"from the handler"));
    CPPUNIT_ASSERT_EQUAL(String(L"A \x0410 B"), f.InfoTag()->artist());
  }

  // The specification makes ISO 8859-1 the charset of a file that declares
  // none, and that is what an untouched library assumes: the UTF-8 bytes of a
  // file that never said what they are read as the Latin-1 characters they
  // also happen to be.
  void testInfoAssumedCodePageDefaultIsLatin1()
  {
    CPPUNIT_ASSERT_EQUAL(0u, RIFF::Info::Tag::assumedCodePage());

    ByteVectorStream stream(utf8WithoutCsetRiff());
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT(f.hasInfoTag());
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\xC3\xA9"), f.InfoTag()->artist());
  }

  // Software that writes UTF-8 without declaring it is what the option is for.
  // Stated once, it is what every file that declares nothing is read with.
  void testInfoAssumedCodePageAcceptsUtf8()
  {
    const RestoreAssumedCodePage restore;

    RIFF::Info::Tag::setAssumedCodePage(65001);
    CPPUNIT_ASSERT_EQUAL(65001u, RIFF::Info::Tag::assumedCodePage());

    {
      ByteVectorStream stream(utf8WithoutCsetRiff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"), f.InfoTag()->artist());
    }

    // An INFO list read without its file, as in Tag(const ByteVector &data),
    // declares nothing either, so it is read the same way.
    const ByteVector list = ByteVector("INFO", 4) +
      riffChunk("IART", ByteVector("Caf\xC3\xA9", 5) + ByteVector(1, '\0'));
    RIFF::Info::Tag tag(list);
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"), tag.artist());

    // Stating 0 again is going back to what the specification says.
    RIFF::Info::Tag::setAssumedCodePage(0);
    CPPUNIT_ASSERT_EQUAL(0u, RIFF::Info::Tag::assumedCodePage());
    ByteVectorStream stream(utf8WithoutCsetRiff());
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT_EQUAL(String(L"Caf\xC3\xA9"), f.InfoTag()->artist());
  }

  // The other real world answer is an ANSI code page: such a file keeps its
  // typographic characters in 0x80-0x9f, where ISO 8859-1 has control
  // characters that no amount of faithful decoding turns into text.
  void testInfoAssumedCodePageWindows1252()
  {
    const RestoreAssumedCodePage restore;

    RIFF::Info::Tag::setAssumedCodePage(1252);
    CPPUNIT_ASSERT_EQUAL(1252u, RIFF::Info::Tag::assumedCodePage());

    ByteVectorStream stream(riffWithInfo({{"IART", ByteVector("A \x80 B", 5)}}));
    RIFF::WAV::File f(&stream);
    CPPUNIT_ASSERT(f.isValid());
    CPPUNIT_ASSERT_EQUAL(String(L"A \x20AC B"), f.InfoTag()->artist());
  }

  // The option is for files that declare nothing. A file declaring a code page
  // keeps it, whether or not this library has a table for it, and a CSET whose
  // wCodePage is zero is the specification's way of declaring nothing, so that
  // is where the assumption applies.
  void testInfoAssumedCodePageOnlyWhenUndeclared()
  {
    const RestoreAssumedCodePage restore;

    RIFF::Info::Tag::setAssumedCodePage(65001);

    {
      ByteVectorStream stream(windows1252Riff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9 \x20AC"), f.InfoTag()->album());
      CPPUNIT_ASSERT_EQUAL(String(L"A \x20AC B"), f.InfoTag()->artist());
    }

    {
      ByteVectorStream stream(unsupportedCsetRiff());
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L"A \xAE B"), f.InfoTag()->artist());
    }

    {
      ByteVectorStream stream(riffWithInfoCset(
        {{"IART", ByteVector("Caf\xC3\xA9", 5)}}, ByteVector(8, '\0')));
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"), f.InfoTag()->artist());
    }
  }

  // An assumed UTF-8 is a hint, and a field that is not valid UTF-8 contradicts
  // it. Such a field is read as the RIFF default ISO 8859-1 instead of as empty,
  // field by field, while a file that declares UTF-8 itself is taken at its word.
  void testInfoAssumedUtf8FallsBackToLatin1()
  {
    const RestoreAssumedCodePage restore;

    RIFF::Info::Tag::setAssumedCodePage(65001);

    {
      ByteVectorStream stream(riffWithInfo({{"IART", ByteVector("Caf\xC3\xA9", 5)},
                                            {"ICOP", ByteVector("\xA9 2019 BOOM", 11)}}));
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String(L"Caf\xE9"), f.InfoTag()->artist());
      CPPUNIT_ASSERT_EQUAL(String(L"\xA9 2019 BOOM"), f.InfoTag()->fieldText("ICOP"));
    }

    {
      ByteVectorStream stream(riffWithInfo({{"ICOP", ByteVector("\xA9 2019 BOOM", 11)}},
                                           65001));
      RIFF::WAV::File f(&stream);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.InfoTag()->fieldText("ICOP").isEmpty());
    }
  }

  // A code page this library cannot decode would quietly do nothing if it were
  // accepted as the assumption, so it is refused and the previous one stays.
  void testInfoAssumedCodePageRejectsUnsupported()
  {
    const RestoreAssumedCodePage restore;

    RIFF::Info::Tag::setAssumedCodePage(1252);
    CPPUNIT_ASSERT_EQUAL(1252u, RIFF::Info::Tag::assumedCodePage());

    RIFF::Info::Tag::setAssumedCodePage(932);
    CPPUNIT_ASSERT_EQUAL(1252u, RIFF::Info::Tag::assumedCodePage());
  }
#endif // TAGLIB_WITH_CHARSETTABLES

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

  void testDISPChunk()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    const ByteVector text = dispTextPayload("Title");
    const ByteVector dib = dispPayload(8, ByteVector("\x28\x00\x00\x00", 4));

    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasDISPData());
      CPPUNIT_ASSERT(f.DISPData().isEmpty());

      f.setDISPData(ByteVectorList({text, dib}));
      f.save();
      CPPUNIT_ASSERT(f.hasDISPData());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.hasDISPData());
      CPPUNIT_ASSERT_EQUAL(2U, f.DISPData().size());
      CPPUNIT_ASSERT_EQUAL(text, f.DISPData()[0]);
      CPPUNIT_ASSERT_EQUAL(dib, f.DISPData()[1]);

      f.setDISPData(ByteVectorList());
      f.save();
      CPPUNIT_ASSERT(!f.hasDISPData());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasDISPData());
      CPPUNIT_ASSERT(f.DISPData().isEmpty());
    }

    // Check if file without DISP is same as original empty file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("empty.wav")).readAll();
    const ByteVector fileData = PlainFile(filename.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

  void testDISPChunkOddSize()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    const ByteVector odd = dispPayload(1, ByteVector("odd", 3));
    CPPUNIT_ASSERT_EQUAL(7U, odd.size());

    {
      RIFF::WAV::File f(filename.c_str());
      f.setDISPData(ByteVectorList({odd}));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      f.setBEXTData(ByteVector("bext data"));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(1U, f.DISPData().size());
      CPPUNIT_ASSERT_EQUAL(odd, f.DISPData()[0]);
      CPPUNIT_ASSERT_EQUAL(ByteVector("bext data"), f.BEXTData());
    }

    const ByteVector fileData = PlainFile(filename.c_str()).readAll();
    const auto layout = riffChunkLayout(fileData);
    CPPUNIT_ASSERT_EQUAL(4U, layout.size());
    CPPUNIT_ASSERT_EQUAL(ByteVector("DISP"), layout[2].first);
    CPPUNIT_ASSERT_EQUAL(ByteVector("bext"), layout[3].first);
    CPPUNIT_ASSERT_EQUAL(layout[2].second + 8, layout[3].second - 8);
  }

  void testDISPChunkGrow()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    const ByteVector first = dispTextPayload("First");
    const ByteVector second = dispTextPayload("Second");
    const ByteVector third = dispPayload(8, ByteVector("\x28\x00\x00\x00", 4));

    {
      RIFF::WAV::File f(filename.c_str());
      f.setDISPData(ByteVectorList({dispTextPayload("Original")}));
      f.save();
    }

    const List<unsigned int> before =
      riffChunkOffsets(PlainFile(filename.c_str()).readAll(), "DISP");
    CPPUNIT_ASSERT_EQUAL(1U, before.size());

    {
      RIFF::WAV::File f(filename.c_str());
      f.setDISPData(ByteVectorList({first, second, third}));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(3U, f.DISPData().size());
      CPPUNIT_ASSERT_EQUAL(first, f.DISPData()[0]);
      CPPUNIT_ASSERT_EQUAL(second, f.DISPData()[1]);
      CPPUNIT_ASSERT_EQUAL(third, f.DISPData()[2]);
    }

    const List<unsigned int> after =
      riffChunkOffsets(PlainFile(filename.c_str()).readAll(), "DISP");
    CPPUNIT_ASSERT_EQUAL(3U, after.size());
    CPPUNIT_ASSERT_EQUAL(before[0], after[0]);
  }

  void testDISPChunkShrink()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    const ByteVector kept = dispTextPayload("Kept");

    {
      RIFF::WAV::File f(filename.c_str());
      f.setDISPData(ByteVectorList({dispTextPayload("First"), dispTextPayload("Second"),
                                    dispTextPayload("Third")}));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      f.setBEXTData(ByteVector("bext data"));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT_EQUAL(3U, f.DISPData().size());
      f.setDISPData(ByteVectorList({kept}));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(1U, f.DISPData().size());
      CPPUNIT_ASSERT_EQUAL(kept, f.DISPData()[0]);
      CPPUNIT_ASSERT(f.hasBEXTData());
      CPPUNIT_ASSERT_EQUAL(ByteVector("bext data"), f.BEXTData());
    }

    const ByteVector fileData = PlainFile(filename.c_str()).readAll();
    CPPUNIT_ASSERT_EQUAL(1U, riffChunkOffsets(fileData, "DISP").size());
    CPPUNIT_ASSERT_EQUAL(1U, riffChunkOffsets(fileData, "bext").size());
  }

  void testDISPChunkUntouchedOnSave()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    const ByteVector text = dispTextPayload("Display Title");

    {
      RIFF::WAV::File f(filename.c_str());
      f.setDISPData(ByteVectorList({text}));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      f.setBEXTData(ByteVector("bext data"));
      f.ID3v2Tag()->setTitle("ID3v2 Title");
      f.InfoTag()->setTitle("INFO Title");
      f.save();
    }

    const List<unsigned int> before =
      riffChunkOffsets(PlainFile(filename.c_str()).readAll(), "DISP");
    CPPUNIT_ASSERT_EQUAL(1U, before.size());

    {
      RIFF::WAV::File f(filename.c_str());
      f.InfoTag()->setTitle("New INFO Title");
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String("New INFO Title"), f.InfoTag()->title());
      CPPUNIT_ASSERT_EQUAL(1U, f.DISPData().size());
      CPPUNIT_ASSERT_EQUAL(text, f.DISPData()[0]);
    }

    const List<unsigned int> after =
      riffChunkOffsets(PlainFile(filename.c_str()).readAll(), "DISP");
    CPPUNIT_ASSERT(before == after);
  }

  void testDISPChunkWithOtherTags()
  {
    ScopedFileCopy copy("empty", ".wav");
    string filename = copy.fileName();

    const ByteVector text = dispTextPayload("Display Title");

    {
      RIFF::WAV::File f(filename.c_str());
      f.ID3v2Tag()->setTitle("ID3v2 Title");
      f.InfoTag()->setTitle("INFO Title");
      f.setDISPData(ByteVectorList({text}));
      f.save();
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasInfoTag());
      CPPUNIT_ASSERT(f.hasDISPData());
      CPPUNIT_ASSERT_EQUAL(String("ID3v2 Title"), f.ID3v2Tag()->title());
      CPPUNIT_ASSERT_EQUAL(String("INFO Title"), f.InfoTag()->title());
      CPPUNIT_ASSERT(ByteVectorList({text}) == f.DISPData());

      f.setDISPData(ByteVectorList());
      f.strip();
      CPPUNIT_ASSERT(f.save());
    }
    {
      RIFF::WAV::File f(filename.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasInfoTag());
      CPPUNIT_ASSERT(!f.hasDISPData());
    }

    // Check if file without tags is same as original empty file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("empty.wav")).readAll();
    const ByteVector fileData = PlainFile(filename.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestWAV);
