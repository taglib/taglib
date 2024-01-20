/***************************************************************************
    copyright           : (C) 2013-2023 Stephen F. Booth
    email               : me@sbooth.org
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

#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "dsdifffile.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestDSDIFF : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestDSDIFF);
  CPPUNIT_TEST(testProperties);
  CPPUNIT_TEST(testTags);
  CPPUNIT_TEST(testSaveID3v2);
  CPPUNIT_TEST(testSaveID3v23);
  CPPUNIT_TEST(testStrip);
  CPPUNIT_TEST(testRepeatedSave);
  CPPUNIT_TEST(testId3InProp);
  CPPUNIT_TEST_SUITE_END();

public:

  void testProperties()
  {
    DSDIFF::File f(TEST_FILE_PATH_C("empty10ms.dff"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(10, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(5644, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(2822400, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->bitsPerSample());
    CPPUNIT_ASSERT_EQUAL(static_cast<long long>(28224), f.audioProperties()->sampleCount());
  }

  void testTags()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String(""), f.tag()->artist());
      f.tag()->setArtist("The Artist");
      f.save();
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String("The Artist"), f.tag()->artist());
    }
  }

  void testSaveID3v2()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());

      f.tag()->setTitle(L"TitleXXX");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L"TitleXXX"), f.tag()->title());

      f.tag()->setTitle("");
      f.save();
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      f.ID3v2Tag(true)->setTitle(L"TitleXXX");
      f.save();
      CPPUNIT_ASSERT(f.hasID3v2Tag());
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String(L"TitleXXX"), f.tag()->title());
    }
  }

  void testSaveID3v23()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    String xxx = ByteVector(254, 'X');
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(false, f.hasID3v2Tag());

      f.tag()->setTitle(xxx);
      f.tag()->setArtist("Artist A");
      f.save(DSDIFF::File::AllTags, File::StripOthers, ID3v2::v3);
      CPPUNIT_ASSERT_EQUAL(true, f.hasID3v2Tag());
    }
    {
      DSDIFF::File f2(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), f2.ID3v2Tag()->header()->majorVersion());
      CPPUNIT_ASSERT_EQUAL(String("Artist A"), f2.tag()->artist());
      CPPUNIT_ASSERT_EQUAL(xxx, f2.tag()->title());
    }
  }

  void testStrip()
  {
    {
      ScopedFileCopy copy("empty10ms", ".dff");
      {
        DSDIFF::File f(copy.fileName().c_str());
        f.ID3v2Tag(true)->setArtist("X");
        f.DIINTag(true)->setArtist("Y");
        f.save();
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.hasID3v2Tag());
        CPPUNIT_ASSERT(f.hasDIINTag());
        f.strip();
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(!f.hasID3v2Tag());
        CPPUNIT_ASSERT(!f.hasDIINTag());
      }
    }

    {
      ScopedFileCopy copy("empty10ms", ".dff");
      {
        DSDIFF::File f(copy.fileName().c_str());
        f.ID3v2Tag(true);
        f.DIINTag(true);
        f.tag()->setArtist("X");
        f.save();
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.hasID3v2Tag());
        CPPUNIT_ASSERT(f.hasDIINTag());
        f.strip(DSDIFF::File::ID3v2);
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(!f.hasID3v2Tag());
        CPPUNIT_ASSERT(f.hasDIINTag());
      }
    }

    {
      ScopedFileCopy copy("empty10ms", ".dff");
      {
        DSDIFF::File f(copy.fileName().c_str());
        f.tag()->setArtist("X");
        f.save();
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.hasID3v2Tag());
        CPPUNIT_ASSERT(f.hasDIINTag());
        f.strip(DSDIFF::File::DIIN);
      }
      {
        DSDIFF::File f(copy.fileName().c_str());
        CPPUNIT_ASSERT(f.hasID3v2Tag());
        CPPUNIT_ASSERT(!f.hasDIINTag());
      }
    }
  }

  void testRepeatedSave()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(7186), f.length());
      CPPUNIT_ASSERT_EQUAL(String(""), f.tag()->title());
      f.tag()->setTitle("NEW TITLE");
      f.save();
      CPPUNIT_ASSERT_EQUAL(String("NEW TITLE"), f.tag()->title());
      f.tag()->setTitle("NEW TITLE 2");
      f.save();
      CPPUNIT_ASSERT_EQUAL(String("NEW TITLE 2"), f.tag()->title());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(8292), f.length());
      f.save();
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(8292), f.length());
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT_EQUAL(String("NEW TITLE 2"), f.tag()->title());
      f.strip();
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(7186), f.length());
    }

    // Check if file without tags is same as original empty file
    const ByteVector dsfData = PlainFile(TEST_FILE_PATH_C("empty10ms.dff")).readAll();
    const ByteVector fileData = PlainFile(newname.c_str()).readAll();
    CPPUNIT_ASSERT(dsfData == fileData);
  }

  void testId3InProp()
  {
    PropertyMap props;
    props["ALBUM"] = StringList("Album");
    props["ALBUMARTIST"] = StringList("Album Artist");
    props["ARTIST"] = StringList("Artist");
    props["COMMENT"] = StringList("Comment");
    props["DATE"] = StringList("2023-10-20");
    props["DISCNUMBER"] = StringList("1/2");
    props["GENRE"] = StringList("Genre");
    props["TITLE"] = StringList("Title");
    props["TRACKNUMBER"] = StringList("2/3");

    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    {
      PlainFile id3PropFile(newname.c_str());
      id3PropFile.insert("\x28", 0x0b, 1); // modify FRM8 length
      id3PropFile.insert("\x6c", 0x2b, 1); // modify PROP length
      // insert ID3 chunk into PROP
      id3PropFile.insert(ByteVector(
        "ID3 \x00\x00\x00\x00\x00\x00\x00\x16ID3\x04\x00\x00\x00\x00\x00\x0c"
        "TIT2\x00\x00\x00\x02\x00\x00\x00X", 34), 0x76, 0);
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(f.hasID3v2Tag());
      CPPUNIT_ASSERT_EQUAL(String("X"), f.tag()->title());
      f.setProperties(props);
      f.DIINTag(true)->setArtist("DIINArtist");
      f.save();
    }
    {
      DSDIFF::File f(newname.c_str());
      ID3v2::Tag *tag = f.ID3v2Tag();
      CPPUNIT_ASSERT(tag);
      CPPUNIT_ASSERT_EQUAL(String("Title"), tag->title());
      CPPUNIT_ASSERT_EQUAL(String("Artist"), tag->artist());
      CPPUNIT_ASSERT_EQUAL(String("Album"), tag->album());
      PropertyMap properties = f.properties();
      if (props != properties) {
        CPPUNIT_ASSERT_EQUAL(props.toString(), properties.toString());
      }
      CPPUNIT_ASSERT(props == properties);
      CPPUNIT_ASSERT_EQUAL(String("DIINArtist"), f.DIINTag()->artist());
      f.strip();
    }
    {
      // Check if file without tags is same as original empty file
      const ByteVector dsfData = PlainFile(TEST_FILE_PATH_C("empty10ms.dff")).readAll();
      const ByteVector fileData = PlainFile(newname.c_str()).readAll();
      CPPUNIT_ASSERT(dsfData == fileData);
    }
  }

  void testSaveDiin()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    string newname = copy.fileName();

    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasDIINTag());

      DSDIFF::DIIN::Tag *tag = f.DIINTag(true);
      CPPUNIT_ASSERT(tag);
      tag->setArtist("DIIN Artist");
      tag->setTitle("DIIN Title");
      tag->setAlbum("not supported");
      f.save();
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(f.hasDIINTag());

      DSDIFF::DIIN::Tag *tag = f.DIINTag(false);
      CPPUNIT_ASSERT(tag);
      CPPUNIT_ASSERT_EQUAL(String("DIIN Artist"), tag->artist());
      CPPUNIT_ASSERT_EQUAL(String("DIIN Title"), tag->title());
      CPPUNIT_ASSERT_EQUAL(String(), tag->album());
      CPPUNIT_ASSERT_EQUAL(String(), tag->comment());
      CPPUNIT_ASSERT_EQUAL(String(), tag->genre());
      CPPUNIT_ASSERT_EQUAL(0U, tag->year());
      CPPUNIT_ASSERT_EQUAL(0U, tag->track());

      PropertyMap properties = f.properties();
      CPPUNIT_ASSERT_EQUAL(2U, properties.size());
      CPPUNIT_ASSERT_EQUAL(StringList("DIIN Artist"), properties.value("ARTIST"));
      CPPUNIT_ASSERT_EQUAL(StringList("DIIN Title"), properties.value("TITLE"));

      tag->setArtist("");
      tag->setTitle("");
      f.save();
    }
    {
      DSDIFF::File f(newname.c_str());
      CPPUNIT_ASSERT(!f.hasID3v2Tag());
      CPPUNIT_ASSERT(!f.hasDIINTag());
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDSDIFF);
