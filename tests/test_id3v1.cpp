/***************************************************************************
    copyright           : (C) 2007 by Lukas Lalinsky
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
#include <cstdlib>

#include "tstring.h"
#include "mpegfile.h"
#include "id3v1tag.h"
#include "id3v1genres.h"
#include <cppunit/extensions/HelperMacros.h>
#include "plainfile.h"
#include "utils.h"

using namespace std;
using namespace TagLib;

namespace
{
  // The bytes of a Windows code page an ID3v1 field cannot tell from ISO 8859-1
  // on disk, and the text they stand for. 0x80 is a Euro sign in every ANSI
  // code page in use and a control character in ISO 8859-1.
  const ByteVector kAnsiArtist("A \x80 B", 5);
  const String   kAnsiArtistText(L"A \x20AC B");

  // A Cyrillic byte, which ISO 8859-1 cannot represent at all and which is once
  // again a control character there.
  const ByteVector kCyrillicArtist("A \xC0 B", 5);
  const String   kCyrillicArtistText(L"A \x0410 B");

  // The same text in UTF-8, as software that writes UTF-8 into ID3v1 produces
  // it, plus the ISO 8859-1 reading of those same bytes.
  const ByteVector kUtf8Artist("Caf\xC3\xA9", 5);
  const String   kUtf8ArtistText(L"Caf\xE9");
  const String   kUtf8ArtistLatin1(L"Caf\xC3\xA9");
}

class TestID3v1 : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestID3v1);
  CPPUNIT_TEST(testStripWhiteSpace);
  CPPUNIT_TEST(testGenres);
  CPPUNIT_TEST(testRenamedGenres);
  CPPUNIT_TEST(testAssumedCodePageDefaultIsLatin1);
  CPPUNIT_TEST(testAssumedCodePage);
  CPPUNIT_TEST(testConstructorCodePage);
  CPPUNIT_TEST(testUnsupportedCodePage);
  CPPUNIT_TEST(testCommentUsesCodePage);
  CPPUNIT_TEST(testUserHandlerBeatsCodePage);
  CPPUNIT_TEST(testMp3ReadsWithIso88591);
  CPPUNIT_TEST(testMp3ReadsWithWindows1252);
  CPPUNIT_TEST_SUITE_END();

public:

  void testStripWhiteSpace()
  {
    ScopedFileCopy copy("xing", ".mp3");
    string newname = copy.fileName();

    {
      MPEG::File f(newname.c_str());
      f.ID3v1Tag(true)->setArtist("Artist     ");
      f.save();
    }

    {
      MPEG::File f(newname.c_str());
      CPPUNIT_ASSERT(f.ID3v1Tag(false));
      CPPUNIT_ASSERT_EQUAL(String("Artist"), f.ID3v1Tag(false)->artist());
    }
  }

  void testGenres()
  {
    CPPUNIT_ASSERT_EQUAL(String("Darkwave"), ID3v1::genre(50));
    CPPUNIT_ASSERT_EQUAL(100, ID3v1::genreIndex("Humour"));
    CPPUNIT_ASSERT(ID3v1::genreList().contains("Heavy Metal"));
    CPPUNIT_ASSERT_EQUAL(79, ID3v1::genreMap()["Hard Rock"]);
  }

  void testRenamedGenres()
  {
    CPPUNIT_ASSERT_EQUAL(String("Bebop"), ID3v1::genre(85));
    CPPUNIT_ASSERT_EQUAL(85, ID3v1::genreIndex("Bebop"));
    CPPUNIT_ASSERT_EQUAL(85, ID3v1::genreIndex("Bebob"));

    ID3v1::Tag tag;
    tag.setGenre("Hardcore");
    CPPUNIT_ASSERT_EQUAL(String("Hardcore Techno"), tag.genre());
    CPPUNIT_ASSERT_EQUAL(129U, tag.genreNumber());
  }

private:

  // Writes a tag whose raw artist bytes are \a artist, so that the reading of
  // the bytes is what the rest of the test can look at.  Nothing else is set,
  // as an empty field renders as a run of NULs.
  static string makeTag(const ByteVector &artist)
  {
    ScopedFileCopy copy("xing", ".mp3", false);
    const string name = copy.fileName();

    FILE *file = fopen(name.c_str(), "r+b");
    CPPUNIT_ASSERT(file);
    // The tag goes where a file already ending in one would carry it: the last
    // 128 bytes. The source file is longer than that, so a shorter tag would
    // not be found at all.
    CPPUNIT_ASSERT_EQUAL(0, fseek(file, -128, SEEK_END));

    ByteVector tag = ID3v1::Tag::fileIdentifier();
    // Leave an empty title behind, so the bytes land in the artist field.
    tag.resize(3 + 30, 0);
    tag.append(artist);
    tag.resize(128, 0);
    // 0xff in the genre byte is the ID3v1 value for "no genre".
    tag[127] = static_cast<char>(0xff);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(128), fwrite(tag.data(), 1, 128, file));
    fclose(file);
    return name;
  }

  // The offset makeTag() wrote at: a tag lives in the last 128 bytes.
  static offset_t tagOffset(File *file)
  {
    return file->length() - 128;
  }

  // Puts back the code page a tag is assumed to have when none is stated, so
  // that a test stating another one does not carry it into the next test.
  class RestoreAssumedCodePage
  {
  public:
    RestoreAssumedCodePage() :
      m_previous(ID3v1::Tag::assumedCodePage())
    {
    }

    ~RestoreAssumedCodePage()
    {
      ID3v1::Tag::setAssumedCodePage(m_previous);
    }

  private:
    const unsigned int m_previous;
  };

  // A handler the user installed is an explicit override, so it outranks any
  // code page, once again because installing one is a deliberate act.
  class FixedStringHandler : public ID3v1::StringHandler
  {
  public:
    String parse(const ByteVector &) const override { return String(L"from the handler"); }
  };

  // The ID3v1 specification says ISO 8859-1, and that is what an untouched
  // library assumes: the bytes of a file that holds another encoding are read
  // as the Latin-1 characters they also happen to be.
  void testAssumedCodePageDefaultIsLatin1()
  {
    CPPUNIT_ASSERT_EQUAL(0u, ID3v1::Tag::assumedCodePage());

    const string name = makeTag(kUtf8Artist);
    MPEG::File f(name.c_str());
    CPPUNIT_ASSERT(f.ID3v1Tag(false));
    CPPUNIT_ASSERT_EQUAL(kUtf8ArtistLatin1, f.ID3v1Tag(false)->artist());
    deleteFile(name);
  }

  // Software that writes UTF-8 or an ANSI code page without any way of saying
  // so is what the option is for.  Stated once, it is what every tag read
  // afterwards is decoded with.
  void testAssumedCodePage()
  {
    const RestoreAssumedCodePage restore;

    ID3v1::Tag::setAssumedCodePage(65001);
    CPPUNIT_ASSERT_EQUAL(65001u, ID3v1::Tag::assumedCodePage());

    {
      const string name = makeTag(kUtf8Artist);
      MPEG::File f(name.c_str());
      CPPUNIT_ASSERT_EQUAL(kUtf8ArtistText, f.ID3v1Tag(false)->artist());
      deleteFile(name);
    }

    ID3v1::Tag::setAssumedCodePage(1252);
    CPPUNIT_ASSERT_EQUAL(1252u, ID3v1::Tag::assumedCodePage());

    {
      const string name = makeTag(kAnsiArtist);
      MPEG::File f(name.c_str());
      CPPUNIT_ASSERT_EQUAL(kAnsiArtistText, f.ID3v1Tag(false)->artist());
      deleteFile(name);
    }

    ID3v1::Tag::setAssumedCodePage(1251);
    CPPUNIT_ASSERT_EQUAL(1251u, ID3v1::Tag::assumedCodePage());

    {
      const string name = makeTag(kCyrillicArtist);
      MPEG::File f(name.c_str());
      CPPUNIT_ASSERT_EQUAL(kCyrillicArtistText, f.ID3v1Tag(false)->artist());
      deleteFile(name);
    }

    // Stating 0 again is going back to what the specification says.
    ID3v1::Tag::setAssumedCodePage(0);
    CPPUNIT_ASSERT_EQUAL(0u, ID3v1::Tag::assumedCodePage());

    const string name = makeTag(kUtf8Artist);
    MPEG::File f(name.c_str());
    CPPUNIT_ASSERT_EQUAL(kUtf8ArtistLatin1, f.ID3v1Tag(false)->artist());
    deleteFile(name);
  }

  // A tag can be given its own code page as well, for callers that read files
  // from more than one source and cannot state a single process wide default.
  void testConstructorCodePage()
  {
    const RestoreAssumedCodePage restore;

    // Even with another assumption in place, the code page handed to the
    // constructor is the one that decides.
    ID3v1::Tag::setAssumedCodePage(65001);

    {
      const string name = makeTag(kAnsiArtist);
      PlainFile file(name.c_str());
      CPPUNIT_ASSERT(file.isValid());
      ID3v1::Tag tag(&file, tagOffset(&file), 1252);
      CPPUNIT_ASSERT_EQUAL(kAnsiArtistText, tag.artist());
      deleteFile(name);
    }

    {
      const string name = makeTag(kCyrillicArtist);
      PlainFile file(name.c_str());
      ID3v1::Tag tag(&file, tagOffset(&file), 1251);
      CPPUNIT_ASSERT_EQUAL(kCyrillicArtistText, tag.artist());
      deleteFile(name);
    }

    // Code page 0 is the "not stated" value, which means the assumption.
    {
      const string name = makeTag(kUtf8Artist);
      PlainFile file(name.c_str());
      ID3v1::Tag tag(&file, tagOffset(&file), 0);
      CPPUNIT_ASSERT_EQUAL(kUtf8ArtistText, tag.artist());
      deleteFile(name);
    }
  }

  // A code page this library cannot decode falls back to what is assumed: the
  // number is a hint about how the text was written, and dropping the whole tag
  // over a hint that could not be followed would lose the fields as well.
  void testUnsupportedCodePage()
  {
    const RestoreAssumedCodePage restore;

    // A code page this library has no table for, here a Japanese one, is
    // refused as the assumption and the previous one stays.
    ID3v1::Tag::setAssumedCodePage(1252);
    ID3v1::Tag::setAssumedCodePage(932);
    CPPUNIT_ASSERT_EQUAL(1252u, ID3v1::Tag::assumedCodePage());

    {
      const string name = makeTag(kAnsiArtist);
      PlainFile file(name.c_str());
      // Handing the same unsupported number to the constructor falls back to
      // the assumption rather than refusing to read the tag.
      ID3v1::Tag tag(&file, tagOffset(&file), 932);
      CPPUNIT_ASSERT_EQUAL(kAnsiArtistText, tag.artist());
      deleteFile(name);
    }
  }

  // The charset covers the whole tag, the ID3v1.0 comment included. Its 30
  // bytes are a single field there, so it goes through the same decoding even
  // though ID3v1.1 would have cut the track number out of it.
  void testCommentUsesCodePage()
  {
    const RestoreAssumedCodePage restore;
    ID3v1::Tag::setAssumedCodePage(1252);

    ScopedFileCopy copy("xing", ".mp3", false);
    const string name = copy.fileName();

    FILE *file = fopen(name.c_str(), "r+b");
    CPPUNIT_ASSERT(file);
    CPPUNIT_ASSERT_EQUAL(0, fseek(file, -128, SEEK_END));

    ByteVector tag = ID3v1::Tag::fileIdentifier();
    tag.resize(3 + 30 + 30 + 30 + 4, 0);
    tag.append(kAnsiArtist);
    tag.resize(128, 0);
    tag[127] = static_cast<char>(0xff);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(128), fwrite(tag.data(), 1, 128, file));
    fclose(file);

    MPEG::File f(name.c_str());
    CPPUNIT_ASSERT(f.ID3v1Tag(false));
    CPPUNIT_ASSERT_EQUAL(kAnsiArtistText, f.ID3v1Tag(false)->comment());
    deleteFile(name);
  }

  // A real MP3 carrying an ID3v1 tag whose bytes are only meaningful as ISO
  // 8859-1, which is what the ID3v1 specification prescribes and what a caller
  // states explicitly. The bytes are one of Latin-1's own characters, so the
  // reading is the text it spells; a code page that reassigns this range would
  // not produce it.
  void testMp3ReadsWithIso88591()
  {
    const RestoreAssumedCodePage restore;

    // Keep any assumption out of it: this tag is read as ISO 8859-1 because
    // the code page says so, not because the default happens to agree.
    ID3v1::Tag::setAssumedCodePage(65001);

    const ByteVector latin1Artist("Caf\xE9", 4);
    const String   latin1ArtistText(L"Caf\xE9");

    const string name = makeTag(latin1Artist);
    PlainFile file(name.c_str());
    CPPUNIT_ASSERT(file.isValid());
    ID3v1::Tag tag(&file, tagOffset(&file), 28591);
    CPPUNIT_ASSERT_EQUAL(latin1ArtistText, tag.artist());
    deleteFile(name);
  }

  // The same MP3 shape read as Windows-1252 instead. The bytes below are a
  // Euro sign and a pair of curly quotes in 1252, and C1 controls in ISO
  // 8859-1, so the two readings differ and only one of them is right.
  void testMp3ReadsWithWindows1252()
  {
    const RestoreAssumedCodePage restore;

    // An assumption the tag contradicts, so the reading has to come from the
    // code page handed to the constructor.
    ID3v1::Tag::setAssumedCodePage(28591);

    const ByteVector windows1252Artist("A \x80 \x93q" "\x94", 7);
    const String   windows1252ArtistText(L"A \x20AC \x201Cq\x201D");

    const string name = makeTag(windows1252Artist);
    PlainFile file(name.c_str());
    CPPUNIT_ASSERT(file.isValid());
    ID3v1::Tag tag(&file, tagOffset(&file), 1252);
    CPPUNIT_ASSERT_EQUAL(windows1252ArtistText, tag.artist());
    deleteFile(name);
  }

  // An installed handler is used for every tag, whatever code page is stated,
  // and removing it hands the tags back to that code page.
  void testUserHandlerBeatsCodePage()
  {
    const RestoreAssumedCodePage restore;

    class RestoreHandler
    {
    public:
      ~RestoreHandler() { ID3v1::Tag::setStringHandler(nullptr); }
    } restoreHandler;

    ID3v1::Tag::setAssumedCodePage(1252);

    {
      const string name = makeTag(kAnsiArtist);
      MPEG::File f(name.c_str());
      CPPUNIT_ASSERT_EQUAL(kAnsiArtistText, f.ID3v1Tag(false)->artist());
      deleteFile(name);
    }

    const FixedStringHandler handler;
    ID3v1::Tag::setStringHandler(&handler);

    {
      const string name = makeTag(kAnsiArtist);
      MPEG::File f(name.c_str());
      CPPUNIT_ASSERT_EQUAL(String(L"from the handler"), f.ID3v1Tag(false)->artist());
      deleteFile(name);
    }

    // Also for a tag whose code page was stated explicitly.
    {
      const string name = makeTag(kAnsiArtist);
      PlainFile file(name.c_str());
      ID3v1::Tag tag(&file, tagOffset(&file), 1252);
      CPPUNIT_ASSERT_EQUAL(String(L"from the handler"), tag.artist());
      deleteFile(name);
    }

    ID3v1::Tag::setStringHandler(nullptr);

    {
      const string name = makeTag(kAnsiArtist);
      MPEG::File f(name.c_str());
      CPPUNIT_ASSERT_EQUAL(kAnsiArtistText, f.ID3v1Tag(false)->artist());
      deleteFile(name);
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestID3v1);
