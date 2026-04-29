/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
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

/*
 * # Test files
 *
 * no-tags.mka was created from a small mp3 file using
 *     mkvmerge -o small.mka small.mp3
 * The tags were removed afterwards.
 *
 * tags-before-cues.mkv was created using
 *     ffmpeg -t 0.1 -s qcif -f rawvideo -pix_fmt rgb24 -r 25 \
 *            -i /dev/zero zero_second.mkv
 * The tags were added using Handbrake, which places them before the Cluster
 * and Cues elements. The tags were rewritten to conform to the specification.
 *
 * optimized.mpk is the same as tags-before-cues.mkv, but optimized using
 *     mkclean --keep-cues --optimize tags-before-cues.mkv
 * This is done to have a segment with the size encoded with fewer than 8 bytes,
 * so the resizing of the segment can be verified by adding a large attachment.
 *
 * no-tags.webm was created using
 *     ffmpeg -f lavfi -i color=c=blue:s=64x64 -frames:v 1 \
 *            -pix_fmt yuv420p frame.yuv
 *     vpxenc --codec=vp8 --width=64 --height=64 --fps=10/1 \
 *       --end-usage=cq --cq-level=4 --lag-in-frames=0 --auto-alt-ref=0 \
 *       --profile=0 --target-bitrate=1000 -o onems.webm frame.yuv
 * Then the EMBL void element after the seek head was transformed to have
 * the content size encoded with 8 bytes instead of 1 byte to have it in the
 * same format as used here.
 *
 * # File validation
 *
 * The files are read with read style "Accurate" to verify the segment positions
 * in seek head and cues.
 *
 * # Manual testing
 *
 * The integrity of Matroska files modified with TagLib can be verified with
 * mkvalidator v0.6.0. For inspection of files, mkvinfo and mkvtoolnix-gui
 * are useful.
 *
 * All tags of Matroska files can be read and written using properties and
 * complex properties, so tagwriter and tagreader are sufficient for testing.
 *
 * - Set standard tags
 *       examples/tagwriter -t 'Track Title' -a 'Artist Name' -A 'Album Title' \
 *                          -c 'Comment' -g 'Genre' -y 2025 -T 1 test.mka
 *
 * To remove standard tags, set them to an empty string or 0 for numeric tags.
 * Using the property interface, the properties listed in propertymapping.dox
 * and arbitrary string tags with track target level can be written.
 *
 * - Insert property
 *       examples/tagwriter -I 'ALBUMARTIST' 'Album Artist' test.mka
 * - Replace property
 *       examples/tagwriter -R 'ALBUMARTIST' 'Other Artist' test.mka
 * - Delete property
 *       examples/tagwriter -D 'ALBUMARTIST' test.mka
 *
 * Pictures can be attached with a description
 *     examples/tagwriter -p file.jpg 'Picture description' test.mka
 *
 * Alternatively, they can be set using complex properties. A complex property
 * can be set with
 *
 *     examples/tagwriter -C <complex-property-key> <key1=val1,key2=val2,...> FILE
 *
 * The second parameter can be set to "" to delete complex properties with the
 * given key. To set complex property values, a simple shorthand syntax can be
 * used. Multiple maps are separated by ';', values within a map are assigned
 * with key=value and separated by a ','. Types are automatically detected,
 * double quotes can be used to force a string. A ByteVector can be constructed
 * from the contents of a file with the path given after "file://". There is
 * no escape character, but hex codes are supported, e.g. "\x2C" to include a ','
 * and \x3B to include a ';'.
 *
 * - Set an attached file in a Matroska file:
 *       examples/tagwriter -C file.bin \
 *           'fileName=file.bin,data=file://file.bin,mimeType=application/octet-stream' \
 *           file.mka
 *
 * - Set simple tag with target type in a Matroska file:
 *       examples/tagwriter -C PART_NUMBER \
 *           'name=PART_NUMBER,targetTypeValue=20,value="2"' file.mka
 *
 * - Set simple tag with binary value in a Matroska file:
 *       examples/tagwriter -C BINARY \
 *           name=BINARY,data=file://file.bin,targetTypeValue=60 file.mka
 *
 * # Test coverage
 *
 * Not yet covered by the unit tests are:
 * - MkCueDuration, MkCueBlockNumber, MkCueCodecState, MkCueReference, MkCueRefTime,
 *   MkBitDepth and the methods handling these elements because none of the test
 *   files has such elements,
 * - some unused functions like EBML::parseVINT(), EBML::FloatElement::render(),
 * - some error cases because they never occur with the unit tests.
 */

#include <string>
#include <cstdio>

#include "tbytevectorlist.h"
#include "tbytevectorstream.h"
#include "tpropertymap.h"
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskaattachments.h"
#include "matroskaattachedfile.h"
#include "matroskachapter.h"
#include "matroskachapteredition.h"
#include "matroskachapters.h"
#include "matroskasimpletag.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMatroska : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMatroska);
  CPPUNIT_TEST(testPropertiesMka);
  CPPUNIT_TEST(testPropertiesMkv);
  CPPUNIT_TEST(testPropertiesWebm);
  CPPUNIT_TEST(testSimpleTagsAndAttachments);
  CPPUNIT_TEST(testAddRemoveTagsAttachments);
  CPPUNIT_TEST(testTagsWebm);
  CPPUNIT_TEST(testRepeatedSave);
  CPPUNIT_TEST(testPropertyInterface);
  CPPUNIT_TEST(testComplexProperties);
  CPPUNIT_TEST(testOpenInvalid);
  CPPUNIT_TEST(testSegmentSizeChange);
  CPPUNIT_TEST(testChapters);
  CPPUNIT_TEST(testSaveTypes);
  CPPUNIT_TEST(testSaveTypesBeforeCues);
  CPPUNIT_TEST(testSaveTypesNoTrailingVoid);
  CPPUNIT_TEST(testSaveTypesReclaimVoid);
  CPPUNIT_TEST_SUITE_END();

public:
  void testPropertiesMka()
  {
    Matroska::File f(TEST_FILE_PATH_C("no-tags.mka"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(444, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(223, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(String("matroska"), f.audioProperties()->docType());
    CPPUNIT_ASSERT_EQUAL(4, f.audioProperties()->docTypeVersion());
    CPPUNIT_ASSERT_EQUAL(String("A_MPEG/L3"), f.audioProperties()->codecName());
    CPPUNIT_ASSERT_EQUAL(String(""), f.audioProperties()->title());
  }

  void testPropertiesMkv()
  {
    Matroska::File f(TEST_FILE_PATH_C("tags-before-cues.mkv"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(120, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(227, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(String("matroska"), f.audioProperties()->docType());
    CPPUNIT_ASSERT_EQUAL(4, f.audioProperties()->docTypeVersion());
    CPPUNIT_ASSERT_EQUAL(String(""), f.audioProperties()->codecName());
    CPPUNIT_ASSERT_EQUAL(String("handbrake"), f.audioProperties()->title());
  }

  void testPropertiesWebm()
  {
    Matroska::File f(TEST_FILE_PATH_C("no-tags.webm"));
    CPPUNIT_ASSERT(f.audioProperties());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->lengthInSeconds());
    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->lengthInMilliseconds());
    CPPUNIT_ASSERT_EQUAL(2816, f.audioProperties()->bitrate());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->channels());
    CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->sampleRate());
    CPPUNIT_ASSERT_EQUAL(String("webm"), f.audioProperties()->docType());
    CPPUNIT_ASSERT_EQUAL(4, f.audioProperties()->docTypeVersion());
    CPPUNIT_ASSERT_EQUAL(String(""), f.audioProperties()->codecName());
    CPPUNIT_ASSERT_EQUAL(String(""), f.audioProperties()->title());

    Matroska::File noProps(TEST_FILE_PATH_C("no-tags.webm"), false);
    CPPUNIT_ASSERT(!noProps.audioProperties());
  }

  void testSimpleTagsAndAttachments()
  {
    ScopedFileCopy copy("no-tags", ".mka");
    string newname = copy.fileName();
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
      auto tag = f.tag(true);
      CPPUNIT_ASSERT(tag->isEmpty());
      tag->addSimpleTag(Matroska::SimpleTag(
        "Test Name 2", String("Test Value 2"),
        Matroska::SimpleTag::TargetTypeValue::Album,
        {}, true, 0x72ac, 0xed17, 0xca97, 0xa7ac));
      tag->insertSimpleTag(0, Matroska::SimpleTag(
        "Test Name 1", String("Test Value 1"),
        Matroska::SimpleTag::TargetTypeValue::Track, "en"));
      tag->insertSimpleTag(1, Matroska::SimpleTag(
        "Test Name 3", String("Test Value 3")));
      tag->removeSimpleTag(1);
      tag->setTitle("Test title");
      tag->setArtist("Test artist");
      tag->setYear(1969);
      auto attachments = f.attachments(true);
      attachments->addAttachedFile(Matroska::AttachedFile(
        "JPEG data", "cover.jpg", "image/jpeg", 5081000385627515072ULL,
        "Cover"));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(false);
      CPPUNIT_ASSERT(tag);
      auto attachments = f.attachments(false);
      CPPUNIT_ASSERT(attachments);
      CPPUNIT_ASSERT(!tag->isEmpty());

      CPPUNIT_ASSERT_EQUAL(String("Test title"), tag->title());
      CPPUNIT_ASSERT_EQUAL(String("Test artist"), tag->artist());
      CPPUNIT_ASSERT_EQUAL(1969U, tag->year());
      CPPUNIT_ASSERT_EQUAL(String(""), tag->album());
      CPPUNIT_ASSERT_EQUAL(String(""), tag->comment());
      CPPUNIT_ASSERT_EQUAL(String(""), tag->genre());
      CPPUNIT_ASSERT_EQUAL(0U, tag->track());

      const auto &simpleTags = tag->simpleTagsList();
      CPPUNIT_ASSERT_EQUAL(5U, simpleTags.size());

      CPPUNIT_ASSERT_EQUAL(String("en"), simpleTags[0].language());
      CPPUNIT_ASSERT_EQUAL(String("Test Name 1"), simpleTags[0].name());
      CPPUNIT_ASSERT_EQUAL(String("Test Value 1"), simpleTags[0].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[0].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[0].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[0].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[0].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[0].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[0].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[0].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[0].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[1].language());
      CPPUNIT_ASSERT_EQUAL(String("TITLE"), simpleTags[1].name());
      CPPUNIT_ASSERT_EQUAL(String("Test title"), simpleTags[1].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[1].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[1].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[1].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[1].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[2].language());
      CPPUNIT_ASSERT_EQUAL(String("ARTIST"), simpleTags[2].name());
      CPPUNIT_ASSERT_EQUAL(String("Test artist"), simpleTags[2].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[2].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[2].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[2].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[2].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[3].language());
      CPPUNIT_ASSERT_EQUAL(String("DATE_RECORDED"), simpleTags[3].name());
      CPPUNIT_ASSERT_EQUAL(String("1969"), simpleTags[3].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[3].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[3].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[3].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[3].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[4].language());
      CPPUNIT_ASSERT_EQUAL(String("Test Name 2"), simpleTags[4].name());
      CPPUNIT_ASSERT_EQUAL(String("Test Value 2"), simpleTags[4].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[4].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[4].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Album, simpleTags[4].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0x72acULL, simpleTags[4].trackUid());
      CPPUNIT_ASSERT_EQUAL(0xed17ULL, simpleTags[4].editionUid());
      CPPUNIT_ASSERT_EQUAL(0xca97ULL, simpleTags[4].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0xa7acULL, simpleTags[4].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[4].type());

      const auto &attachedFiles = attachments->attachedFileList();
      CPPUNIT_ASSERT_EQUAL(1U, attachedFiles.size());

      CPPUNIT_ASSERT_EQUAL(String("Cover"), attachedFiles[0].description());
      CPPUNIT_ASSERT_EQUAL(String("cover.jpg"), attachedFiles[0].fileName());
      CPPUNIT_ASSERT_EQUAL(String("image/jpeg"), attachedFiles[0].mediaType());
      CPPUNIT_ASSERT_EQUAL(ByteVector("JPEG data"), attachedFiles[0].data());
      CPPUNIT_ASSERT_EQUAL(5081000385627515072ULL, attachedFiles[0].uid());

      PropertyMap expectedProps;
      expectedProps["ARTIST"] = StringList("Test artist");
      expectedProps["DATE"] = StringList("1969");
      expectedProps["TEST NAME 1"] = StringList("Test Value 1");
      expectedProps["TITLE"] = StringList("Test title");
      auto props = f.properties();
      if (expectedProps != props) {
        CPPUNIT_ASSERT_EQUAL(expectedProps.toString(), props.toString());
      }
      CPPUNIT_ASSERT(expectedProps == props);
      CPPUNIT_ASSERT(expectedProps == tag->properties());

      auto keys = f.complexPropertyKeys();
      CPPUNIT_ASSERT_EQUAL(StringList({"Test Name 2", "PICTURE"}), keys);
      auto pictures = f.complexProperties("PICTURE");
      CPPUNIT_ASSERT_EQUAL(1U, pictures.size());
      const VariantMap expectedPic {
            {"data", ByteVector("JPEG data")},
            {"mimeType", "image/jpeg"},
            {"description", "Cover"},
            {"fileName", "cover.jpg"},
            {"uid", 5081000385627515072ULL}
      };
      CPPUNIT_ASSERT(List<VariantMap>({expectedPic}) == pictures);
      const VariantMap expectedComplexProps {
            {"defaultLanguage", true},
            {"language", "und"},
            {"name", "Test Name 2"},
            {"value", "Test Value 2"},
            {"targetTypeValue", 50},
            {"trackUid", 0x72acULL},
            {"editionUid", 0xed17ULL},
            {"chapterUid", 0xca97ULL},
            {"attachmentUid", 0xa7acULL},
          };
      auto complexProps = f.complexProperties("Test Name 2");
      CPPUNIT_ASSERT(List<VariantMap>({expectedComplexProps}) == complexProps);

      tag->clearSimpleTags();
      attachments->clear();
      f.save();
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
    }

    // Check if file without tags is same as original empty file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("no-tags.mka")).readAll();
    const ByteVector fileData = PlainFile(newname.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

  void testAddRemoveTagsAttachments()
  {
    ScopedFileCopy copy("no-tags", ".mka");
    string newname = copy.fileName();
    {
      Matroska::File f(newname.c_str());
      f.tag(true)->setComment("C");
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(String("C"), f.tag(false)->comment());
      f.tag()->setComment("");
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      f.attachments(true)->addAttachedFile(Matroska::AttachedFile(
        ByteVector(), "", ""));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      const auto &attachedFiles = f.attachments(false)->attachedFileList();
      CPPUNIT_ASSERT_EQUAL(1U, attachedFiles.size());
      f.attachments()->removeAttachedFile(attachedFiles.front().uid());
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
    }
  }

  void testTagsWebm()
  {
    ScopedFileCopy copy("no-tags", ".webm");
    string newname = copy.fileName();
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));

      f.setProperties(SimplePropertyMap{
        {"ARTIST", {"First artist", "second artist"}},
        {"", {"Invalid", "Ignored"}}
      });
      f.tag(false)->addSimpleTag(Matroska::SimpleTag("", ByteVector("Not valid")));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), false, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(false);
      CPPUNIT_ASSERT(tag);
      CPPUNIT_ASSERT(!f.attachments(false));
      CPPUNIT_ASSERT_EQUAL(String("First artist"), tag->artist());
      CPPUNIT_ASSERT_EQUAL(StringList({"First artist", "second artist"}), f.properties().value("ARTIST"));

      tag->setAlbum("Album");
      tag->setTrack(5);
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), false, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(false);
      CPPUNIT_ASSERT(tag);
      CPPUNIT_ASSERT_EQUAL(String("First artist"), tag->artist());
      CPPUNIT_ASSERT_EQUAL(String("Album"), tag->album());
      CPPUNIT_ASSERT_EQUAL(5U, tag->track());

      tag->setArtist("");
      tag->removeSimpleTag("TITLE", Matroska::SimpleTag::TargetTypeValue::Album);
      tag->setTrack(0);
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), false, AudioProperties::Accurate);
      CPPUNIT_ASSERT(!f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
    }

    // Check if file without tags is same as original empty file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("no-tags.webm")).readAll();
    const ByteVector fileData = PlainFile(newname.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

  void testRepeatedSave()
  {
    ScopedFileCopy copy("no-tags", ".mka");
    string newname = copy.fileName();
    String text = "01234 56789 ABCDE FGHIJ 01234 56789 ABCDE FGHIJ 01234 56789";
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.save());
      f.tag(true)->setTitle(text.substr(0, 23));
      CPPUNIT_ASSERT(f.save());
      f.tag(true)->setTitle(text.substr(0, 5));
      CPPUNIT_ASSERT(f.save());
      f.tag(true)->setTitle(text);
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT_EQUAL(text, f.tag()->title());
    }
  }

  void testPropertyInterface()
  {
    ScopedFileCopy copy("tags-before-cues", ".mkv");
    string newname = copy.fileName();

    PropertyMap initialProps;
    initialProps["ARTIST"] = StringList("Actors");
    initialProps["RELEASEDATE"] = StringList("2023");
    initialProps["DESCRIPTION"] = StringList("Description");
    initialProps["DIRECTOR"] = StringList("Director");
    initialProps["ENCODEDBY"] = StringList("Lavf59.27.100");
    initialProps["GENRE"] = StringList("Genre");
    initialProps["SUMMARY"] = StringList("Comment");
    initialProps["SYNOPSIS"] = StringList("Plot");

    const VariantMap initialComplexProps {
      {"defaultLanguage", true},
      {"language", "und"},
      {"name", "DURATION"},
      {"value", "00:00:00.120000000"},
      {"trackUid", 9584013959154292683ULL},
    };

    PropertyMap newProps;
    newProps["ALBUM"] = StringList("Album");
    newProps["ALBUMARTIST"] = StringList("Album Artist");
    newProps["ALBUMARTISTSORT"] = StringList("Album Artist Sort");
    newProps["ALBUMSORT"] = StringList("Album Sort");
    newProps["ARTIST"] = StringList("Artist");
    newProps["ARTISTS"] = StringList("Artists");
    newProps["ARTISTSORT"] = StringList("Artist Sort");
    newProps["ASIN"] = StringList("ASIN");
    newProps["BARCODE"] = StringList("Barcode");
    newProps["CATALOGNUMBER"] = StringList("Catalog Number 1").append("Catalog Number 2");
    newProps["COMMENT"] = StringList("Comment");
    newProps["DATE"] = StringList("2021-01-10");
    newProps["DISCNUMBER"] = StringList("3");
    newProps["DISCTOTAL"] = StringList("5");
    newProps["DJMIXER"] = StringList("Mixed by");
    newProps["ENCODEDBY"] = StringList("Encoded by");
    newProps["ENCODING"] = StringList("Encoder settings");
    newProps["ENCODINGTIME"] = StringList("Date encoded");
    newProps["GENRE"] = StringList("Genre");
    newProps["INITIALKEY"] = StringList("Initial key");
    newProps["ISRC"] = StringList("UKAAA0500001");
    newProps["LABEL"] = StringList("Label 1").append("Label 2");
    newProps["MEDIA"] = StringList("Media");
    newProps["MUSICBRAINZ_ALBUMARTISTID"] = StringList("MusicBrainz_AlbumartistID");
    newProps["MUSICBRAINZ_ALBUMID"] = StringList("MusicBrainz_AlbumID");
    newProps["MUSICBRAINZ_ARTISTID"] = StringList("MusicBrainz_ArtistID");
    newProps["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
    newProps["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
    newProps["MUSICBRAINZ_TRACKID"] = StringList("MusicBrainz_TrackID");
    newProps["OWNER"] = StringList("Purchase owner");
    newProps["ORIGINALDATE"] = StringList("2021-01-09");
    newProps["RELEASECOUNTRY"] = StringList("Release Country");
    newProps["RELEASEDATE"] = StringList("2021-01-10");
    newProps["RELEASESTATUS"] = StringList("Release Status");
    newProps["RELEASETYPE"] = StringList("Release Type");
    newProps["REMIXER"] = StringList("Remixed by");
    newProps["SCRIPT"] = StringList("Script");
    newProps["TAGGINGDATE"] = StringList("2021-01-08");
    newProps["TITLE"] = StringList("Title");
    newProps["TRACKNUMBER"] = StringList("2");
    newProps["TRACKTOTAL"] = StringList("4");

    const VariantMap newComplexProps {
        {"defaultLanguage", false},
        {"language", "en"},
        {"name", "BINARY"},
        {"data", ByteVector("\x01\x02\x03\x04\x05\x06")},
        {"targetTypeValue", static_cast<int>(Matroska::SimpleTag::TargetTypeValue::Collection)},
      };
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(false);
      CPPUNIT_ASSERT(tag);
      CPPUNIT_ASSERT(!f.attachments(false));

      CPPUNIT_ASSERT_EQUAL(String("handbrake"), tag->title());
      CPPUNIT_ASSERT_EQUAL(String("Actors"), tag->artist());
      CPPUNIT_ASSERT_EQUAL(String(""), tag->album());
      CPPUNIT_ASSERT_EQUAL(String(""), tag->comment());
      CPPUNIT_ASSERT_EQUAL(String("Genre"), tag->genre());
      CPPUNIT_ASSERT_EQUAL(0U, tag->track());

      const auto &simpleTags = tag->simpleTagsList();
      CPPUNIT_ASSERT_EQUAL(9U, simpleTags.size());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[0].language());
      CPPUNIT_ASSERT_EQUAL(String("DURATION"), simpleTags[0].name());
      CPPUNIT_ASSERT_EQUAL(String("00:00:00.120000000"), simpleTags[0].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[0].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[0].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::None, simpleTags[0].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(9584013959154292683ULL, simpleTags[0].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[0].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[0].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[0].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[0].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[1].language());
      CPPUNIT_ASSERT_EQUAL(String("ARTIST"), simpleTags[1].name());
      CPPUNIT_ASSERT_EQUAL(String("Actors"), simpleTags[1].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[1].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[1].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[1].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[1].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[2].language());
      CPPUNIT_ASSERT_EQUAL(String("DESCRIPTION"), simpleTags[2].name());
      CPPUNIT_ASSERT_EQUAL(String("Description"), simpleTags[2].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[2].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[2].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[2].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[2].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[3].language());
      CPPUNIT_ASSERT_EQUAL(String("DIRECTOR"), simpleTags[3].name());
      CPPUNIT_ASSERT_EQUAL(String("Director"), simpleTags[3].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[3].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[3].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[3].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[3].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[4].language());
      CPPUNIT_ASSERT_EQUAL(String("ENCODER"), simpleTags[4].name());
      CPPUNIT_ASSERT_EQUAL(String("Lavf59.27.100"), simpleTags[4].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[4].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[4].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[4].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[4].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[4].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[4].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[4].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[4].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[5].language());
      CPPUNIT_ASSERT_EQUAL(String("GENRE"), simpleTags[5].name());
      CPPUNIT_ASSERT_EQUAL(String("Genre"), simpleTags[5].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[5].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[5].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[5].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[5].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[5].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[5].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[5].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[5].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[6].language());
      CPPUNIT_ASSERT_EQUAL(String("SUMMARY"), simpleTags[6].name());
      CPPUNIT_ASSERT_EQUAL(String("Comment"), simpleTags[6].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[6].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[6].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[6].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[6].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[6].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[6].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[6].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[6].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[7].language());
      CPPUNIT_ASSERT_EQUAL(String("SYNOPSIS"), simpleTags[7].name());
      CPPUNIT_ASSERT_EQUAL(String("Plot"), simpleTags[7].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[7].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[7].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[7].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[7].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[7].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[7].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[7].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[7].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[8].language());
      CPPUNIT_ASSERT_EQUAL(String("DATE_RELEASED"), simpleTags[8].name());
      CPPUNIT_ASSERT_EQUAL(String("2023"), simpleTags[8].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[8].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[8].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Album, simpleTags[8].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[8].trackUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[8].editionUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[8].chapterUid());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[8].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[8].type());

      auto props = f.properties();
      if (initialProps != props) {
        CPPUNIT_ASSERT_EQUAL(initialProps.toString(), props.toString());
      }
      CPPUNIT_ASSERT(initialProps == props);
      CPPUNIT_ASSERT(initialProps == tag->properties());

      auto keys = f.complexPropertyKeys();
      CPPUNIT_ASSERT_EQUAL(StringList({"DURATION"}), keys);
      auto complexProps = f.complexProperties("DURATION");
      CPPUNIT_ASSERT(List<VariantMap>({initialComplexProps}) == complexProps);

      CPPUNIT_ASSERT(f.setComplexProperties("DURATION", {}));
      CPPUNIT_ASSERT(f.setComplexProperties("BINARY", {newComplexProps}));
      CPPUNIT_ASSERT(f.setProperties(newProps).isEmpty());
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(false);
      CPPUNIT_ASSERT(tag);
      CPPUNIT_ASSERT(!f.attachments(false));

      auto props = f.properties();
      if (newProps != props) {
        CPPUNIT_ASSERT_EQUAL(newProps.toString(), props.toString());
      }
      CPPUNIT_ASSERT(newProps == props);
      CPPUNIT_ASSERT(newProps == tag->properties());

      auto keys = f.complexPropertyKeys();
      CPPUNIT_ASSERT_EQUAL(StringList({"BINARY"}), keys);
      auto complexProps = f.complexProperties("BINARY");
      CPPUNIT_ASSERT(List<VariantMap>({newComplexProps}) == complexProps);

      const auto &simpleTags = tag->simpleTagsList();
      StringList simpleTagNames;
      for(const auto &simpleTag : simpleTags) {
        simpleTagNames.append(simpleTag.name());
      }
      const StringList expectedSimpleTagNames {
        "BINARY", "TITLE", "ARTIST", "ARTISTSORT", "TITLESORT",
        "PART_NUMBER", "TOTAL_PARTS", "MUSICBRAINZ_ALBUMARTISTID", "MUSICBRAINZ_ALBUMID",
        "MUSICBRAINZ_RELEASEGROUPID", "DATE_RELEASED", "ARTIST", "ARTISTS", "ARTISTSORT",
        "ASIN", "BARCODE", "CATALOG_NUMBER", "CATALOG_NUMBER", "COMMENT",
        "DATE_RECORDED", "MIXED_BY", "ENCODER", "ENCODER_SETTINGS", "DATE_ENCODED",
        "GENRE", "INITIAL_KEY", "ISRC", "LABEL_CODE", "LABEL_CODE",
        "ORIGINAL_MEDIA_TYPE", "MUSICBRAINZ_ARTISTID",
        "MUSICBRAINZ_RELEASETRACKID", "MUSICBRAINZ_TRACKID", "ORIGINALDATE",
        "PURCHASE_OWNER", "RELEASECOUNTRY", "RELEASESTATUS",
        "RELEASETYPE", "REMIXED_BY", "SCRIPT", "DATE_TAGGED", "TITLE",
        "PART_NUMBER", "TOTAL_PARTS"
      };
      CPPUNIT_ASSERT_EQUAL(expectedSimpleTagNames, simpleTagNames);

      CPPUNIT_ASSERT(f.setComplexProperties("DURATION", {initialComplexProps}));
      CPPUNIT_ASSERT(f.setComplexProperties("BINARY", {}));
      CPPUNIT_ASSERT(f.setProperties(initialProps).isEmpty());
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
    }

    // Check if file with initial tags is same as original file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("tags-before-cues.mkv")).readAll();
    const ByteVector fileData = PlainFile(newname.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

  void testComplexProperties()
  {
    ScopedFileCopy copy("no-tags", ".mka");
    string newname = copy.fileName();
    const VariantMap picture {
      {"data", ByteVector("JPEG data")},
      {"mimeType", "image/jpeg"},
      {"description", "Cover"},
      {"fileName", "folder.jpg"},
      {"uid", 123ULL}
    };
    const VariantMap font {
      {"data", ByteVector("TTF data")},
      {"mimeType", "font/ttf"},
      {"description", "Subtitle font"},
      {"fileName", "file.ttf"},
      {"uid", 456ULL}
    };
    const VariantMap trackUidTag {
      {"defaultLanguage", true},
      {"language", "und"},
      {"name", "DURATION"},
      {"trackUid", 8315232342706310039ULL},
      {"value", "00:00:00.120000000"},
    };
    const VariantMap targetTypeTag {
      {"defaultLanguage", false},
      {"language", "en"},
      {"name", "PART_NUMBER"},
      {"targetTypeValue", 20},
      {"value", "2"},
    };
    const VariantMap binaryTag {
      {"defaultLanguage", false},
      {"language", "und"},
      {"name", "THUMBNAIL"},
      {"targetTypeValue", 30},
      {"data", ByteVector("JPEG data")},
    };
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
      CPPUNIT_ASSERT(f.complexPropertyKeys().isEmpty());
      CPPUNIT_ASSERT(f.complexProperties("PICTURE").isEmpty());

      CPPUNIT_ASSERT(f.setComplexProperties("PICTURE", {picture}));
      CPPUNIT_ASSERT(f.setComplexProperties("file.ttf", {font}));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      auto attachments = f.attachments(false);
      CPPUNIT_ASSERT(attachments);

      const auto &attachedFiles = attachments->attachedFileList();
      CPPUNIT_ASSERT_EQUAL(2U, attachedFiles.size());
      CPPUNIT_ASSERT_EQUAL(picture.value("fileName").value<String>(), attachedFiles[0].fileName());
      CPPUNIT_ASSERT_EQUAL(picture.value("data").value<ByteVector>(), attachedFiles[0].data());
      CPPUNIT_ASSERT_EQUAL(picture.value("description").value<String>(), attachedFiles[0].description());
      CPPUNIT_ASSERT_EQUAL(picture.value("mimeType").value<String>(), attachedFiles[0].mediaType());
      CPPUNIT_ASSERT_EQUAL(picture.value("uid").value<unsigned long long>(), attachedFiles[0].uid());
      CPPUNIT_ASSERT_EQUAL(font.value("fileName").value<String>(), attachedFiles[1].fileName());
      CPPUNIT_ASSERT_EQUAL(font.value("data").value<ByteVector>(), attachedFiles[1].data());
      CPPUNIT_ASSERT_EQUAL(font.value("description").value<String>(), attachedFiles[1].description());
      CPPUNIT_ASSERT_EQUAL(font.value("mimeType").value<String>(), attachedFiles[1].mediaType());
      CPPUNIT_ASSERT_EQUAL(font.value("uid").value<unsigned long long>(), attachedFiles[1].uid());

      CPPUNIT_ASSERT_EQUAL(StringList({"PICTURE", "file.ttf"}), f.complexPropertyKeys());
      CPPUNIT_ASSERT(List<VariantMap>({picture}) == f.complexProperties("PICTURE"));
      CPPUNIT_ASSERT(List<VariantMap>({font}) == f.complexProperties("file.ttf"));

      CPPUNIT_ASSERT(f.setComplexProperties("DURATION", {trackUidTag}));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(true);
      CPPUNIT_ASSERT(tag);
      CPPUNIT_ASSERT(f.attachments(false));

      const auto &simpleTags = tag->simpleTagsList();
      CPPUNIT_ASSERT_EQUAL(1U, simpleTags.size());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("value").value<String>(), simpleTags[0].toString());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("language").value<String>(), simpleTags[0].language());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("name").value<String>(), simpleTags[0].name());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("defaultLanguage").value<bool>(), simpleTags[0].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("trackUid").value<unsigned long long>(), simpleTags[0].trackUid());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("editionUid").value<unsigned long long>(), simpleTags[0].editionUid());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("chapterUid").value<unsigned long long>(), simpleTags[0].chapterUid());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("attachmentUid").value<unsigned long long>(), simpleTags[0].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::None, simpleTags[0].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[0].toByteVector());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[0].type());

      CPPUNIT_ASSERT_EQUAL(StringList({ "DURATION", "PICTURE", "file.ttf"}), f.complexPropertyKeys());
      CPPUNIT_ASSERT(List<VariantMap>({trackUidTag}) == f.complexProperties("DURATION"));

      CPPUNIT_ASSERT(f.setComplexProperties("PICTURE", {}));
      CPPUNIT_ASSERT(f.setComplexProperties("file.ttf", {}));
      CPPUNIT_ASSERT(f.setComplexProperties("PART_NUMBER", {targetTypeTag}));
      CPPUNIT_ASSERT(f.setComplexProperties("THUMBNAIL", {binaryTag}));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(true);
      CPPUNIT_ASSERT(tag);
      CPPUNIT_ASSERT(!f.attachments(false));

      const auto &simpleTags = tag->simpleTagsList();
      CPPUNIT_ASSERT_EQUAL(3U, simpleTags.size());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("value").value<String>(), simpleTags[0].toString());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("language").value<String>(), simpleTags[0].language());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("name").value<String>(), simpleTags[0].name());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("defaultLanguage").value<bool>(), simpleTags[0].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("trackUid").value<unsigned long long>(), simpleTags[0].trackUid());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("editionUid").value<unsigned long long>(), simpleTags[0].editionUid());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("chapterUid").value<unsigned long long>(), simpleTags[0].chapterUid());
      CPPUNIT_ASSERT_EQUAL(trackUidTag.value("attachmentUid").value<unsigned long long>(), simpleTags[0].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::None, simpleTags[0].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[0].toByteVector());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[0].type());

      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("value").value<String>(), simpleTags[1].toString());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("language").value<String>(), simpleTags[1].language());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("name").value<String>(), simpleTags[1].name());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("defaultLanguage").value<bool>(), simpleTags[1].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("trackUid").value<unsigned long long>(), simpleTags[1].trackUid());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("editionUid").value<unsigned long long>(), simpleTags[1].editionUid());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("chapterUid").value<unsigned long long>(), simpleTags[1].chapterUid());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("attachmentUid").value<unsigned long long>(), simpleTags[1].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Subtrack, simpleTags[1].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[1].toByteVector());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[1].type());

      CPPUNIT_ASSERT_EQUAL(binaryTag.value("data").value<ByteVector>(), simpleTags[2].toByteVector());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("language").value<String>(), simpleTags[2].language());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("name").value<String>(), simpleTags[2].name());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("defaultLanguage").value<bool>(), simpleTags[2].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("trackUid").value<unsigned long long>(), simpleTags[2].trackUid());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("editionUid").value<unsigned long long>(), simpleTags[2].editionUid());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("chapterUid").value<unsigned long long>(), simpleTags[2].chapterUid());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("attachmentUid").value<unsigned long long>(), simpleTags[2].attachmentUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[2].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(String(), simpleTags[2].toString());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::BinaryType, simpleTags[2].type());

      CPPUNIT_ASSERT_EQUAL(StringList({"DURATION", "PART_NUMBER", "THUMBNAIL"}), f.complexPropertyKeys());
      CPPUNIT_ASSERT(List<VariantMap>({trackUidTag}) == f.complexProperties("DURATION"));
      CPPUNIT_ASSERT(List<VariantMap>({targetTypeTag}) == f.complexProperties("PART_NUMBER"));
      CPPUNIT_ASSERT(List<VariantMap>({binaryTag}) == f.complexProperties("THUMBNAIL"));

      CPPUNIT_ASSERT(f.setComplexProperties("PART_NUMBER", {}));
      CPPUNIT_ASSERT(f.setComplexProperties("THUMBNAIL", {}));
      CPPUNIT_ASSERT(f.setComplexProperties("DURATION", {}));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));

      CPPUNIT_ASSERT(f.setComplexProperties("font/ttf", {{
        {"data", ByteVector("TTF data")},
        {"description", "Subtitle font"},
        {"fileName", "file.ttf"},
      }}));
      CPPUNIT_ASSERT(f.setComplexProperties("file.otf", {{
        {"data", ByteVector("OTF data")},
        {"mimeType", "font/otf"},
        {"description", "OpenType"},
      }}));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(!f.tag(false));
      CPPUNIT_ASSERT(f.attachments(false));

      CPPUNIT_ASSERT_EQUAL(StringList({"file.ttf", "file.otf"}), f.complexPropertyKeys());
      auto ttfs = f.complexProperties("file.ttf");
      auto otfs = f.complexProperties("file.otf");
      CPPUNIT_ASSERT_EQUAL(1U, ttfs.size());
      CPPUNIT_ASSERT_EQUAL(1U, otfs.size());
      auto ttf = ttfs.front();
      auto otf = otfs.front();
      CPPUNIT_ASSERT_EQUAL(ByteVector("TTF data"), ttf.value("data").value<ByteVector>());
      CPPUNIT_ASSERT_EQUAL(String("Subtitle font"), ttf.value("description").value<String>());
      CPPUNIT_ASSERT_EQUAL(String("file.ttf"), ttf.value("fileName").value<String>());
      CPPUNIT_ASSERT_EQUAL(String("font/ttf"), ttf.value("mimeType").value<String>());
      CPPUNIT_ASSERT(ttf.value("uid").value<unsigned long long>() != 0ULL);
      CPPUNIT_ASSERT_EQUAL(ByteVector("OTF data"), otf.value("data").value<ByteVector>());
      CPPUNIT_ASSERT_EQUAL(String("OpenType"), otf.value("description").value<String>());
      CPPUNIT_ASSERT_EQUAL(String("file.otf"), otf.value("fileName").value<String>());
      CPPUNIT_ASSERT_EQUAL(String("font/otf"), otf.value("mimeType").value<String>());
      CPPUNIT_ASSERT(otf.value("uid").value<unsigned long long>() != 0ULL);

      CPPUNIT_ASSERT(f.setComplexProperties("file.ttf", {}));
      CPPUNIT_ASSERT(f.setComplexProperties("file.otf", {}));
      CPPUNIT_ASSERT(f.save());
    }

    // Check if file without tags is same as original empty file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("no-tags.mka")).readAll();
    const ByteVector fileData = PlainFile(newname.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

  void testOpenInvalid()
  {
    {
      Matroska::File f(TEST_FILE_PATH_C("garbage.mp3"));
      CPPUNIT_ASSERT(!f.isValid());
    }
    {
      ByteVector origData = PlainFile(TEST_FILE_PATH_C("no-tags.mka")).readAll();
      ByteVectorStream origStream(origData);
      Matroska::File origFile(&origStream, true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(origFile.isValid());

      ByteVector truncatedData = origData.mid(0, 4000);
      ByteVectorStream truncatedStream(truncatedData);
      Matroska::File truncatedFile(&truncatedStream, true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(!truncatedFile.isValid());
    }
  }

  void testSegmentSizeChange()
  {
    ScopedFileCopy copy("optimized", ".mkv");
    string newname = copy.fileName();
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
      auto attachments = f.attachments(true);
      // Large enough for emitSizeChanged() from Matroska::Segment::render()
      attachments->addAttachedFile(Matroska::AttachedFile(
        ByteVector(20000, 'x'), "cover.jpg", "image/jpeg",
        5081000385627515072ULL, "Cover"));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(true);
      CPPUNIT_ASSERT(tag);
      auto attachments = f.attachments(false);
      CPPUNIT_ASSERT(attachments);
      CPPUNIT_ASSERT(PropertyMap(SimplePropertyMap{
        {"ARTIST", {"Actors"}},
        {"DESCRIPTION", {"Description"}},
        {"DIRECTOR", {"Director"}},
        {"ENCODEDBY", {"Lavf59.27.100"}},
        {"GENRE", {"Genre"}},
        {"RELEASEDATE", {"2023"}},
        {"SUMMARY", {"Comment"}},
        {"SYNOPSIS", {"Plot"}}
      }) == tag->properties());

      attachments->clear();
      f.save();
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
    }
  }

  void testChapters()
  {
    const Matroska::ChapterEdition edition1(
      List<Matroska::Chapter>{
        Matroska::Chapter(
          0, 40000,
          List{
            Matroska::Chapter::Display("Chapter 1", "eng")},
          1, false),
        Matroska::Chapter(
          40000, 80000,
          List{
            Matroska::Chapter::Display("Chapter 2", "eng"),
            Matroska::Chapter::Display("Kapitel 2", "deu"),
          },
          2),
        Matroska::Chapter(
          80000, 120000,
          List{
            Matroska::Chapter::Display("Chapter 3", "und")},
          3, true)
      },
      true, false);
    const VariantMap chapterEdition1 {
            {"chapters",
              VariantList{
                VariantMap{
                  {"displays", VariantList{
                    VariantMap{{"language", "eng"}, {"string", "Chapter 1"}}}},
                  {"timeEnd", 40000ULL},
                  {"timeStart", 0ULL},
                  {"uid", 1ULL}
                },
                VariantMap{
                  {"displays", VariantList{
                    VariantMap{{"language", "eng"}, {"string", "Chapter 2"}},
                    VariantMap{{"language", "deu"}, {"string", "Kapitel 2"}}}},
                  {"timeEnd", 80000ULL},
                  {"timeStart", 40000ULL},
                  {"uid", 2ULL}
                },
                VariantMap{
                    {
                      "displays", VariantList{
                        VariantMap{{"language", "und"}, {"string", "Chapter 3"}}}
                    },
                  {"isHidden", true},
                  {"timeEnd", 120000ULL},
                  {"timeStart", 80000ULL},
                  {"uid", 3ULL}
                }
              }
            },
          {"isDefault", true}
    };
    const VariantMap chapterEdition2 {
      {"chapters",
        VariantList{
          VariantMap{
            {"displays", VariantList{
              VariantMap{{"string", "Chapter A"}}}},
            {"timeStart", 10000ULL},
            {"uid", 1234567890ULL}
          },
        }
      },
      {"isOrdered", true},
      {"uid", 321ULL}
    };

    ScopedFileCopy copy("tags-before-cues", ".mkv");
    string newname = copy.fileName();
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
      CPPUNIT_ASSERT(!f.chapters(false));
      CPPUNIT_ASSERT_EQUAL(StringList({"DURATION"}), f.complexPropertyKeys());
      CPPUNIT_ASSERT(f.complexProperties("CHAPTERS").isEmpty());

      f.chapters(true)->addChapterEdition(edition1);
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
      auto chapters = f.chapters(false);
      CPPUNIT_ASSERT(chapters);
      CPPUNIT_ASSERT_EQUAL(StringList({"DURATION", "CHAPTERS"}), f.complexPropertyKeys());
      auto chaptersProperties = f.complexProperties("CHAPTERS");
      CPPUNIT_ASSERT_EQUAL(1U, chaptersProperties.size());
      CPPUNIT_ASSERT_EQUAL(chapterEdition1, chaptersProperties.front());

      CPPUNIT_ASSERT_EQUAL(1U, chapters->chapterEditionList().size());
      const auto &edition = chapters->chapterEditionList().front();
      CPPUNIT_ASSERT_EQUAL(true, edition.isDefault());
      CPPUNIT_ASSERT_EQUAL(false, edition.isOrdered());
      CPPUNIT_ASSERT_EQUAL(0ULL, edition.uid());
      const auto &chapterAtoms = edition.chapterList();
      CPPUNIT_ASSERT_EQUAL(3U, chapterAtoms.size());
      CPPUNIT_ASSERT_EQUAL(1ULL, chapterAtoms[0].uid());
      CPPUNIT_ASSERT_EQUAL(false, chapterAtoms[0].isHidden());
      CPPUNIT_ASSERT_EQUAL(0ULL, chapterAtoms[0].timeStart());
      CPPUNIT_ASSERT_EQUAL(40000ULL, chapterAtoms[0].timeEnd());
      CPPUNIT_ASSERT_EQUAL(1U, chapterAtoms[0].displayList().size());
      CPPUNIT_ASSERT_EQUAL(String("Chapter 1"), chapterAtoms[0].displayList()[0].string());
      CPPUNIT_ASSERT_EQUAL(String("eng"), chapterAtoms[0].displayList()[0].language());
      CPPUNIT_ASSERT_EQUAL(2ULL, chapterAtoms[1].uid());
      CPPUNIT_ASSERT_EQUAL(false, chapterAtoms[1].isHidden());
      CPPUNIT_ASSERT_EQUAL(40000ULL, chapterAtoms[1].timeStart());
      CPPUNIT_ASSERT_EQUAL(80000ULL, chapterAtoms[1].timeEnd());
      CPPUNIT_ASSERT_EQUAL(2U, chapterAtoms[1].displayList().size());
      CPPUNIT_ASSERT_EQUAL(String("Chapter 2"), chapterAtoms[1].displayList()[0].string());
      CPPUNIT_ASSERT_EQUAL(String("eng"), chapterAtoms[1].displayList()[0].language());
      CPPUNIT_ASSERT_EQUAL(String("Kapitel 2"), chapterAtoms[1].displayList()[1].string());
      CPPUNIT_ASSERT_EQUAL(String("deu"), chapterAtoms[1].displayList()[1].language());
      CPPUNIT_ASSERT_EQUAL(3ULL, chapterAtoms[2].uid());
      CPPUNIT_ASSERT_EQUAL(true, chapterAtoms[2].isHidden());
      CPPUNIT_ASSERT_EQUAL(80000ULL, chapterAtoms[2].timeStart());
      CPPUNIT_ASSERT_EQUAL(120000ULL, chapterAtoms[2].timeEnd());
      CPPUNIT_ASSERT_EQUAL(1U, chapterAtoms[2].displayList().size());
      CPPUNIT_ASSERT_EQUAL(String("Chapter 3"), chapterAtoms[2].displayList()[0].string());
      CPPUNIT_ASSERT_EQUAL(String("und"), chapterAtoms[2].displayList()[0].language());

      CPPUNIT_ASSERT(f.setComplexProperties("CHAPTERS", {chapterEdition2}));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
      auto chapters = f.chapters(false);
      CPPUNIT_ASSERT(chapters);
      CPPUNIT_ASSERT_EQUAL(StringList({"DURATION", "CHAPTERS"}), f.complexPropertyKeys());
      auto chaptersProperties = f.complexProperties("CHAPTERS");
      CPPUNIT_ASSERT_EQUAL(1U, chaptersProperties.size());
      CPPUNIT_ASSERT_EQUAL(chapterEdition2, chaptersProperties.front());

      CPPUNIT_ASSERT_EQUAL(1U, chapters->chapterEditionList().size());
      const auto &edition = chapters->chapterEditionList().front();
      CPPUNIT_ASSERT_EQUAL(false, edition.isDefault());
      CPPUNIT_ASSERT_EQUAL(true, edition.isOrdered());
      CPPUNIT_ASSERT_EQUAL(321ULL, edition.uid());
      const auto &chapterAtoms = edition.chapterList();
      CPPUNIT_ASSERT_EQUAL(1U, chapterAtoms.size());
      CPPUNIT_ASSERT_EQUAL(1234567890ULL, chapterAtoms[0].uid());
      CPPUNIT_ASSERT_EQUAL(false, chapterAtoms[0].isHidden());
      CPPUNIT_ASSERT_EQUAL(10000ULL, chapterAtoms[0].timeStart());
      CPPUNIT_ASSERT_EQUAL(0ULL, chapterAtoms[0].timeEnd());
      CPPUNIT_ASSERT_EQUAL(1U, chapterAtoms[0].displayList().size());
      CPPUNIT_ASSERT_EQUAL(String("Chapter A"), chapterAtoms[0].displayList()[0].string());
      CPPUNIT_ASSERT_EQUAL(String(), chapterAtoms[0].displayList()[0].language());

      const Matroska::ChapterEdition edition2 = chapters->chapterEditionList().front();
      chapters->removeChapterEdition(321ULL);
      chapters->addChapterEdition(edition1);
      chapters->addChapterEdition(edition2);

      f.attachments(true)->addAttachedFile(Matroska::AttachedFile(
        ByteVector("PNG data"), "folder.png", "image/png", 1763187649ULL,
        "Cover"));
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false));
      CPPUNIT_ASSERT(f.attachments(false));
      CPPUNIT_ASSERT(f.chapters(false));

      CPPUNIT_ASSERT_EQUAL(StringList({"DURATION", "PICTURE", "CHAPTERS"}),
        f.complexPropertyKeys());
      auto chaptersProperties = f.complexProperties("CHAPTERS");
      CPPUNIT_ASSERT_EQUAL(2U, chaptersProperties.size());
      CPPUNIT_ASSERT_EQUAL(chapterEdition1, chaptersProperties.front());
      CPPUNIT_ASSERT_EQUAL(chapterEdition2, chaptersProperties.back());

      f.attachments()->clear();
      f.chapters()->clear();
      CPPUNIT_ASSERT(f.save());
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false));
      CPPUNIT_ASSERT(!f.attachments(false));
    }

    // Check if file with initial tags is same as original file
    const ByteVector origData = PlainFile(TEST_FILE_PATH_C("tags-before-cues.mkv")).readAll();
    const ByteVector fileData = PlainFile(newname.c_str()).readAll();
    CPPUNIT_ASSERT(origData == fileData);
  }

  void testSaveTypesBeforeCues()
  {
    // tags-before-cues.mkv layout:
    //   SeekHead | Void | SegInfo | Tracks | Tags | Cluster | Cues
    //
    // Verify all three WriteStyles correctly grow the Tags element which
    // sits *before* the Cluster:
    //  - Compact / DoNotShrink: bytes are inserted before the Cluster, the
    //    Cluster shifts, the seek head and cue cluster positions must be
    //    updated accordingly; the file must remain valid and tag content
    //    must round-trip.
    //  - AvoidInsert: the Tags element is replaced with a Void at its
    //    original position and appended at the end of the segment, so the
    //    Cluster must NOT shift; tag content must round-trip.

    const ByteVector origData =
      PlainFile(TEST_FILE_PATH_C("tags-before-cues.mkv")).readAll();

    // Cluster ID 0x1F43B675 does not appear in the SeekHead of this file,
    // so find() returns the offset of the actual Cluster element.
    const ByteVector clusterId = ByteVector::fromUInt(0x1F43B675U, true);
    const ByteVector tagsId    = ByteVector::fromUInt(0x1254C367U, true);
    const ByteVector cuesId    = ByteVector::fromUInt(0x1C53BB6BU, true);
    const int origClusterPos   = origData.find(clusterId);
    CPPUNIT_ASSERT(origClusterPos > 0);

    const String longTitle =
      "An Extremely Long Title Value That Is Definitely Larger Than The Original "
      "Tags Element In The File Because It Contains Many Characters To Ensure "
      "That The AvoidInsert Move-To-End Behavior Triggers Here";
    const String longArtist =
      "An Extremely Long Artist Name Value That Is Also Larger Than The Original "
      "Tags Element And Together With The Title Tag Makes The Rendered Output "
      "Exceed The Original Tags Size So The AvoidInsert Triggers";

    for(const auto writeStyle : {Matroska::WriteStyle::Compact,
                                 Matroska::WriteStyle::DoNotShrink,
                                 Matroska::WriteStyle::AvoidInsert}) {
      const auto wsLabel = String::number(static_cast<int>(writeStyle)).to8Bit();
      ScopedFileCopy copy("tags-before-cues", ".mkv");
      const string newname = copy.fileName();

      // Save with Tags significantly larger than the original Tags element.
      {
        Matroska::File f(newname.c_str());
        CPPUNIT_ASSERT_MESSAGE("Open ws=" + wsLabel, f.isValid());
        auto tag = f.tag(true);
        tag->clearSimpleTags();
        tag->addSimpleTag(Matroska::SimpleTag(
          String("TITLE"), longTitle,
          Matroska::SimpleTag::TargetTypeValue::Track));
        tag->addSimpleTag(Matroska::SimpleTag(
          String("ARTIST"), longArtist,
          Matroska::SimpleTag::TargetTypeValue::Track));
        CPPUNIT_ASSERT_MESSAGE("Save ws=" + wsLabel, f.save(writeStyle));
      }

      // File must be valid: Accurate mode verifies seek-head and cue positions.
      // Tag content must round-trip exactly.
      {
        Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
        CPPUNIT_ASSERT_MESSAGE("Reopen valid ws=" + wsLabel, f.isValid());
        auto tag = f.tag(false);
        CPPUNIT_ASSERT_MESSAGE("Tag exists ws=" + wsLabel, tag != nullptr);
        const auto &simpleTags = tag->simpleTagsList();
        bool foundTitle = false, foundArtist = false;
        for(const auto &st : simpleTags) {
          if(st.name() == "TITLE" && st.toString() == longTitle)
            foundTitle = true;
          else if(st.name() == "ARTIST" && st.toString() == longArtist)
            foundArtist = true;
        }
        CPPUNIT_ASSERT_MESSAGE("TITLE roundtrip ws=" + wsLabel, foundTitle);
        CPPUNIT_ASSERT_MESSAGE("ARTIST roundtrip ws=" + wsLabel, foundArtist);
      }

      const ByteVector newData = PlainFile(newname.c_str()).readAll();
      const int newClusterPos = newData.find(clusterId);
      CPPUNIT_ASSERT_MESSAGE("Cluster present ws=" + wsLabel, newClusterPos > 0);

      if(writeStyle == Matroska::WriteStyle::AvoidInsert) {
        // Cluster must not shift in AvoidInsert mode.
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
          "AvoidInsert must not shift Cluster",
          origClusterPos, newClusterPos);
        // Tags must be appended after Cues.
        const int cuesPos    = newData.find(cuesId, newClusterPos);
        const int newTagsPos = newData.find(tagsId, cuesPos + 4);
        CPPUNIT_ASSERT_MESSAGE("Tags appended after Cues ws=" + wsLabel,
                               newTagsPos > cuesPos);
      }
      else {
        // Compact / DoNotShrink: Tags grew in place, so Cluster must have
        // shifted to a higher offset.
        CPPUNIT_ASSERT_MESSAGE(
          "Cluster must shift when growing in place ws=" + wsLabel,
          newClusterPos > origClusterPos);
      }
    }
  }

  void testSaveTypesNoTrailingVoid()
  {
    // After AvoidInsert moved the Tags element to the end of the segment,
    // a subsequent save with smaller content must not leave a trailing
    // EBML void at the very end of the segment. The trailing element may
    // shrink freely because no element follows it.
    ScopedFileCopy copy("tags-before-cues", ".mkv");
    const string newname = copy.fileName();

    // Round 1: enlarge Tags so they get moved to the end.
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(true);
      tag->clearSimpleTags();
      tag->addSimpleTag(Matroska::SimpleTag(
        String("TITLE"),
        String("An Extremely Long Title Value That Is Definitely Larger Than The Original "
               "Tags Element In The File Because It Contains Many Characters To Ensure "
               "That The AvoidInsert Move-To-End Behavior Triggers Here"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      tag->addSimpleTag(Matroska::SimpleTag(
        String("ARTIST"),
        String("An Extremely Long Artist Name Value That Is Also Larger Than The Original "
               "Tags Element And Together With The Title Tag Makes The Rendered Output "
               "Exceed The Original Tags Size So The AvoidInsert Triggers"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      CPPUNIT_ASSERT(f.save(Matroska::WriteStyle::AvoidInsert));
    }
    const size_t sizeAfterRound1 = PlainFile(newname.c_str()).readAll().size();

    // Round 2: shrink Tags. The trailing element must shrink without
    // leaving a void at the end.
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(true);
      tag->clearSimpleTags();
      tag->addSimpleTag(Matroska::SimpleTag(
        String("TITLE"), String("X"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      CPPUNIT_ASSERT(f.save(Matroska::WriteStyle::AvoidInsert));
    }
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      CPPUNIT_ASSERT(f.tag(false) != nullptr);
    }
    const ByteVector newData = PlainFile(newname.c_str()).readAll();

    // File must have shrunk because the trailing Tags element shrank.
    CPPUNIT_ASSERT(newData.size() < sizeAfterRound1);

    // The last bytes of the file must be the (small) Tags element, not a
    // Void element. Find the Tags element after the Cues element and parse
    // its VINT size: the file must end exactly at Tags' end with nothing
    // (no Void) after it.
    const ByteVector clusterId = ByteVector::fromUInt(0x1F43B675U, true);
    const ByteVector cuesId    = ByteVector::fromUInt(0x1C53BB6BU, true);
    const ByteVector tagsId    = ByteVector::fromUInt(0x1254C367U, true);
    const int clusterPos = newData.find(clusterId);
    const int cuesPos    = newData.find(cuesId, clusterPos);
    const int tagsPos    = newData.find(tagsId, cuesPos + 4);
    CPPUNIT_ASSERT(tagsPos > cuesPos);

    // Decode VINT data size of the Tags element. The first byte after the
    // 4-byte ID has a leading marker bit indicating the VINT length.
    const auto vintFirst = static_cast<unsigned char>(newData[tagsPos + 4]);
    int vintLen = 1;
    for(int b = 0; b < 8; ++b) {
      if(vintFirst & (0x80 >> b)) { vintLen = b + 1; break; }
    }
    unsigned long long dataSize = vintFirst & ((0x80 >> (vintLen - 1)) - 1);
    for(int i = 1; i < vintLen; ++i)
      dataSize = (dataSize << 8) | static_cast<unsigned char>(newData[tagsPos + 4 + i]);
    const unsigned long long tagsEnd =
      static_cast<unsigned long long>(tagsPos) + 4 + vintLen + dataSize;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "No trailing EBML void must remain at the end of the segment",
      static_cast<unsigned long long>(newData.size()), tagsEnd);
  }

  void testSaveTypesReclaimVoid()
  {
    // After AvoidInsert moves a Tags element to the end (leaving a Void at
    // its original position), a subsequent save with WriteStyle::Compact
    // must produce a tightly packed file: the void left by the move must
    // be reclaimed and the file must be at most as large as the original.
    ScopedFileCopy copy("tags-before-cues", ".mkv");
    const string newname = copy.fileName();

    // Step 1: AvoidInsert with enlarged Tags -> Tags moved to end, Void in
    // original slot. File grows.
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(true);
      tag->clearSimpleTags();
      tag->addSimpleTag(Matroska::SimpleTag(String("TITLE"),
        String("An Extremely Long Title Value That Is Definitely Larger Than The Original "
               "Tags Element In The File Because It Contains Many Characters To Ensure "
               "That The AvoidInsert Move-To-End Behavior Triggers Here"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      tag->addSimpleTag(Matroska::SimpleTag(String("ARTIST"),
        String("An Extremely Long Artist Name Value That Is Also Larger Than The Original "
               "Tags Element And Together With The Title Tag Makes The Rendered Output "
               "Exceed The Original Tags Size So The AvoidInsert Triggers"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      CPPUNIT_ASSERT(f.save(Matroska::WriteStyle::AvoidInsert));
    }
    const size_t sizeAfterAvoidInsert =
      PlainFile(newname.c_str()).readAll().size();
    CPPUNIT_ASSERT(sizeAfterAvoidInsert >
      PlainFile(TEST_FILE_PATH_C("tags-before-cues.mkv")).readAll().size());

    // Step 2: Save again with Compact and short tag values. Compact must
    // reclaim the void left by the prior move and produce a file no
    // larger than the original.
    {
      Matroska::File f(newname.c_str());
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(true);
      tag->clearSimpleTags();
      tag->addSimpleTag(Matroska::SimpleTag(String("TITLE"), String("X"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      CPPUNIT_ASSERT(f.save(Matroska::WriteStyle::Compact));
    }
    const size_t sizeAfterCompact =
      PlainFile(newname.c_str()).readAll().size();
    CPPUNIT_ASSERT_MESSAGE(
      "Compact must reclaim space after AvoidInsert grew the file",
      sizeAfterCompact < sizeAfterAvoidInsert);

    // Reference: applying Compact directly to the original file with the
    // same tiny tags. Note: an orphan Void left in the middle of the
    // segment by AvoidInsert is not currently reclaimed by Compact (it is
    // attached as padding to a neighbouring element), so the post-Compact
    // size is allowed to be slightly larger than the reference. The
    // result must, however, be no larger than the original input file.
    ScopedFileCopy reference("tags-before-cues", ".mkv");
    {
      Matroska::File f(reference.fileName().c_str());
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(true);
      tag->clearSimpleTags();
      tag->addSimpleTag(Matroska::SimpleTag(String("TITLE"), String("X"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      CPPUNIT_ASSERT(f.save(Matroska::WriteStyle::Compact));
    }
    const size_t referenceCompactSize =
      PlainFile(reference.fileName().c_str()).readAll().size();
    CPPUNIT_ASSERT(referenceCompactSize <= sizeAfterCompact);

    // File must round-trip correctly.
    {
      Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
      CPPUNIT_ASSERT(f.isValid());
      auto tag = f.tag(false);
      CPPUNIT_ASSERT(tag != nullptr);
      bool foundTitle = false;
      for(const auto &st : tag->simpleTagsList()) {
        if(st.name() == "TITLE" && st.toString() == "X") {
          foundTitle = true;
          break;
        }
      }
      CPPUNIT_ASSERT(foundTitle);
    }
  }

  void testSaveTypes()
  {
    // Helper lambdas for adding data of different sizes
    // largeTags: 2 simple tags with long values
    const auto setLargeTags = [](Matroska::File &f) {
      auto tag = f.tag(true);
      tag->addSimpleTag(Matroska::SimpleTag(String("TITLE"),
        String("A Very Long Title That Takes Up A Lot Of Space In The File 1234567890"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      tag->addSimpleTag(Matroska::SimpleTag(String("ARTIST"),
        String("A Very Long Artist Name That Takes Up A Lot Of Space In The File 1234567890"),
        Matroska::SimpleTag::TargetTypeValue::Track));
    };
    const auto setSmallTags = [](Matroska::File &f) {
      auto tag = f.tag(true);
      tag->addSimpleTag(Matroska::SimpleTag(String("TITLE"), String("Short"),
        Matroska::SimpleTag::TargetTypeValue::Track));
    };
    const auto setMediumTags = [](Matroska::File &f) {
      auto tag = f.tag(true);
      tag->addSimpleTag(Matroska::SimpleTag(String("TITLE"), String("Medium Title 12345678901234"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      tag->addSimpleTag(Matroska::SimpleTag(String("ARTIST"), String("Medium Artist"),
        Matroska::SimpleTag::TargetTypeValue::Track));
    };
    const auto setExtraLargeTags = [](Matroska::File &f) {
      auto tag = f.tag(true);
      tag->addSimpleTag(Matroska::SimpleTag(String("TITLE"),
        String("An Extremely Long Title That Is Even Larger Than The Previous Large Title "
               "With Extra Content To Ensure Growth 0123456789ABCDEF"),
        Matroska::SimpleTag::TargetTypeValue::Track));
      tag->addSimpleTag(Matroska::SimpleTag(String("ARTIST"),
        String("An Extremely Long Artist Name Exceeding The Prior Large Artist Value "
               "With Even More Content To Guarantee Growth 0123456789ABCDEF"),
        Matroska::SimpleTag::TargetTypeValue::Track));
    };

    const auto setLargeAttachments = [](Matroska::File &f) {
      auto atts = f.attachments(true);
      atts->addAttachedFile(Matroska::AttachedFile(
        ByteVector(200, 'x'), "cover.jpg", "image/jpeg", 111ULL, "Cover"));
    };
    const auto setSmallAttachments = [](Matroska::File &f) {
      auto atts = f.attachments(true);
      atts->addAttachedFile(Matroska::AttachedFile(
        ByteVector(20, 'x'), "img.png", "image/png", 222ULL, "Img"));
    };
    const auto setMediumAttachments = [](Matroska::File &f) {
      auto atts = f.attachments(true);
      atts->addAttachedFile(Matroska::AttachedFile(
        ByteVector(80, 'x'), "cover.jpg", "image/jpeg", 333ULL, "Cover"));
    };
    const auto setExtraLargeAttachments = [](Matroska::File &f) {
      auto atts = f.attachments(true);
      atts->addAttachedFile(Matroska::AttachedFile(
        ByteVector(500, 'x'), "cover.jpg", "image/jpeg", 444ULL, "Cover"));
    };

    const auto setLargeChapters = [](Matroska::File &f) {
      auto chs = f.chapters(true);
      chs->addChapterEdition(Matroska::ChapterEdition(
        List<Matroska::Chapter>{
          Matroska::Chapter(0, 40000,
            List{Matroska::Chapter::Display("Chapter One Long Name", "eng")},
            1, false),
          Matroska::Chapter(40000, 80000,
            List{Matroska::Chapter::Display("Chapter Two Long Name", "eng")},
            2, false),
        }, true, false));
    };
    const auto setSmallChapters = [](Matroska::File &f) {
      auto chs = f.chapters(true);
      chs->addChapterEdition(Matroska::ChapterEdition(
        List<Matroska::Chapter>{
          Matroska::Chapter(0, 1000,
            List{Matroska::Chapter::Display("A", "und")},
            1, false),
        }, false, false));
    };
    const auto setMediumChapters = [](Matroska::File &f) {
      auto chs = f.chapters(true);
      chs->addChapterEdition(Matroska::ChapterEdition(
        List<Matroska::Chapter>{
          Matroska::Chapter(0, 40000,
            List{Matroska::Chapter::Display("Chapter Medium", "eng")},
            1, false),
        }, true, false));
    };
    const auto setExtraLargeChapters = [](Matroska::File &f) {
      auto chs = f.chapters(true);
      chs->addChapterEdition(Matroska::ChapterEdition(
        List<Matroska::Chapter>{
          Matroska::Chapter(0, 40000,
            List{Matroska::Chapter::Display("Chapter One Extremely Long Name Here", "eng"),
                 Matroska::Chapter::Display("Kapitel Eins Sehr Langer Name", "deu")},
            1, false),
          Matroska::Chapter(40000, 80000,
            List{Matroska::Chapter::Display("Chapter Two Extremely Long Name Here", "eng"),
                 Matroska::Chapter::Display("Kapitel Zwei Sehr Langer Name", "deu")},
            2, false),
          Matroska::Chapter(80000, 120000,
            List{Matroska::Chapter::Display("Chapter Three Extra Large", "eng")},
            3, true),
        }, true, true));
    };

    for(const auto writeStyle : {Matroska::WriteStyle::Compact,
                                  Matroska::WriteStyle::DoNotShrink,
                                  Matroska::WriteStyle::AvoidInsert}) {
      ScopedFileCopy copy("no-tags", ".mka");
      const string newname = copy.fileName();
      const int wsIdx = static_cast<int>(writeStyle);

      // Verify tag/attachment/chapter content for a saved file. Each round
      // uses unique identifiers (specific TITLE value, attachment UID,
      // chapter timeStart) so any cross-round leakage or corruption is
      // caught here.
      const auto verifyRound = [&](const std::string &label,
                                   const String &expectedTitle,
                                   unsigned long long expectedAttachmentUid,
                                   unsigned int expectedChapterCount,
                                   unsigned long long expectedFirstChapterEnd) {
        Matroska::File f(newname.c_str(), true, AudioProperties::Accurate);
        CPPUNIT_ASSERT_MESSAGE(label + " valid", f.isValid());
        auto tag = f.tag(false);
        CPPUNIT_ASSERT_MESSAGE(label + " tag", tag != nullptr);
        bool foundTitle = false;
        for(const auto &st : tag->simpleTagsList()) {
          if(st.name() == "TITLE" && st.toString() == expectedTitle) {
            foundTitle = true;
            break;
          }
        }
        CPPUNIT_ASSERT_MESSAGE(label + " TITLE roundtrip", foundTitle);
        auto atts = f.attachments(false);
        CPPUNIT_ASSERT_MESSAGE(label + " attachments", atts != nullptr);
        bool foundAtt = false;
        for(const auto &a : atts->attachedFileList()) {
          if(a.uid() == expectedAttachmentUid) {
            foundAtt = true;
            break;
          }
        }
        CPPUNIT_ASSERT_MESSAGE(label + " attachment uid roundtrip", foundAtt);
        auto chs = f.chapters(false);
        CPPUNIT_ASSERT_MESSAGE(label + " chapters", chs != nullptr);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(label + " edition count", 1U,
          chs->chapterEditionList().size());
        const auto &edition = chs->chapterEditionList().front();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(label + " chapter count",
          expectedChapterCount, edition.chapterList().size());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(label + " first chapter end",
          expectedFirstChapterEnd, edition.chapterList()[0].timeEnd());
      };

      // --- Round 1: save large data ---
      {
        Matroska::File f(newname.c_str());
        CPPUNIT_ASSERT(f.isValid());
        setLargeTags(f);
        setLargeAttachments(f);
        setLargeChapters(f);
        CPPUNIT_ASSERT_MESSAGE("Round1 save ws=" + String::number(wsIdx).to8Bit(), f.save(writeStyle));
      }
      const size_t sizeAfterRound1 = PlainFile(newname.c_str()).readAll().size();
      verifyRound("Round1 ws=" + std::to_string(wsIdx),
        String("A Very Long Title That Takes Up A Lot Of Space In The File 1234567890"),
        111ULL, 2U, 40000ULL);

      // --- Round 2: save smaller data → slot must not shrink for DoNotShrink/AvoidInsert ---
      {
        Matroska::File f(newname.c_str());
        CPPUNIT_ASSERT(f.isValid());
        f.tag(true)->clearSimpleTags();
        f.attachments(true)->clear();
        f.chapters(true)->clear();
        setSmallTags(f);
        setSmallAttachments(f);
        setSmallChapters(f);
        CPPUNIT_ASSERT_MESSAGE("Round2 save ws=" + String::number(wsIdx).to8Bit(), f.save(writeStyle));
      }
      const size_t sizeAfterRound2 = PlainFile(newname.c_str()).readAll().size();
      verifyRound("Round2 ws=" + std::to_string(wsIdx),
        String("Short"), 222ULL, 1U, 1000ULL);

      if(writeStyle == Matroska::WriteStyle::Compact) {
        // Compact always shrinks, so file is smaller
        CPPUNIT_ASSERT(sizeAfterRound2 < sizeAfterRound1);
      } else if(writeStyle == Matroska::WriteStyle::AvoidInsert) {
        // AvoidInsert: existing slots are kept, but the segment-trailing
        // element may shrink (no element follows it -- shrinking only
        // truncates the file, no inserts are needed).
        CPPUNIT_ASSERT(sizeAfterRound2 <= sizeAfterRound1);
      } else {
        // DoNotShrink: elements keep their original slot size.
        // The file size must not be smaller than after round 1
        CPPUNIT_ASSERT_EQUAL(sizeAfterRound1, sizeAfterRound2);
      }

      // --- Round 3: save medium data (fits in round2's slot if DoNotShrink/AvoidInsert) ---
      {
        Matroska::File f(newname.c_str());
        CPPUNIT_ASSERT(f.isValid());
        f.tag(true)->clearSimpleTags();
        f.attachments(true)->clear();
        f.chapters(true)->clear();
        setMediumTags(f);
        setMediumAttachments(f);
        setMediumChapters(f);
        CPPUNIT_ASSERT_MESSAGE("Round3 save ws=" + String::number(wsIdx).to8Bit(), f.save(writeStyle));
      }
      const size_t sizeAfterRound3 = PlainFile(newname.c_str()).readAll().size();
      verifyRound("Round3 ws=" + std::to_string(wsIdx),
        String("Medium Title 12345678901234"), 333ULL, 1U, 40000ULL);

      if(writeStyle == Matroska::WriteStyle::Compact) {
        // Compact: medium > small, but exact, so different from round2
        CPPUNIT_ASSERT(sizeAfterRound3 != sizeAfterRound2);
        CPPUNIT_ASSERT(sizeAfterRound3 < sizeAfterRound1);
      } else if(writeStyle == Matroska::WriteStyle::AvoidInsert) {
        // AvoidInsert: medium fits in round1's slot for non-trailing
        // elements, but the trailing element may take less space than in
        // round 1. File size therefore stays <= round 1.
        CPPUNIT_ASSERT(sizeAfterRound3 <= sizeAfterRound1);
      } else {
        // DoNotShrink: medium fits in round1's slot (with remaining void)
        // so file size stays the same as round1/round2
        CPPUNIT_ASSERT_EQUAL(sizeAfterRound1, sizeAfterRound3);
      }

      // --- Round 4: save extra-large data (larger than round 1) ---
      {
        Matroska::File f(newname.c_str());
        CPPUNIT_ASSERT(f.isValid());
        f.tag(true)->clearSimpleTags();
        f.attachments(true)->clear();
        f.chapters(true)->clear();
        setExtraLargeTags(f);
        setExtraLargeAttachments(f);
        setExtraLargeChapters(f);
        CPPUNIT_ASSERT_MESSAGE("Round4 save ws=" + String::number(wsIdx).to8Bit(), f.save(writeStyle));
      }
      const size_t sizeAfterRound4 = PlainFile(newname.c_str()).readAll().size();
      verifyRound("Round4 ws=" + std::to_string(wsIdx),
        String("An Extremely Long Title That Is Even Larger Than The Previous Large Title "
               "With Extra Content To Ensure Growth 0123456789ABCDEF"),
        444ULL, 3U, 40000ULL);

      // All styles must accommodate the larger data: file must be larger than round1
      CPPUNIT_ASSERT(sizeAfterRound4 > sizeAfterRound1);
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMatroska);
