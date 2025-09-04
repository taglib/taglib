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
 *           name=PART_NUMBER,targetTypeValue=20,value=2 file.mka
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
        "Test Name 1", String("Test Value 1"),
        Matroska::SimpleTag::TargetTypeValue::Track, "en"));
      tag->addSimpleTag(Matroska::SimpleTag(
        "Test Name 2", String("Test Value 2"),
        Matroska::SimpleTag::TargetTypeValue::Album));
      tag->setTitle("Test title");
      tag->setArtist("Test artist");
      tag->setYear(1969);
      auto attachments = f.attachments(true);
      Matroska::AttachedFile attachedFile;
      attachedFile.setFileName("cover.jpg");
      attachedFile.setMediaType("image/jpeg");
      attachedFile.setDescription("Cover");
      attachedFile.setData(ByteVector("JPEG data"));
      attachedFile.setUID(5081000385627515072ULL);
      attachments->addAttachedFile(attachedFile);
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
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[0].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[1].language());
      CPPUNIT_ASSERT_EQUAL(String("TITLE"), simpleTags[1].name());
      CPPUNIT_ASSERT_EQUAL(String("Test title"), simpleTags[1].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[1].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[1].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[1].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[1].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[2].language());
      CPPUNIT_ASSERT_EQUAL(String("ARTIST"), simpleTags[2].name());
      CPPUNIT_ASSERT_EQUAL(String("Test artist"), simpleTags[2].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[2].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[2].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[2].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[2].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[3].language());
      CPPUNIT_ASSERT_EQUAL(String("Test Name 2"), simpleTags[3].name());
      CPPUNIT_ASSERT_EQUAL(String("Test Value 2"), simpleTags[3].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[3].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[3].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Album, simpleTags[3].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[3].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[4].language());
      CPPUNIT_ASSERT_EQUAL(String("DATE_RELEASED"), simpleTags[4].name());
      CPPUNIT_ASSERT_EQUAL(String("1969"), simpleTags[4].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[4].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[4].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Album, simpleTags[4].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[4].trackUid());
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
      expectedProps.addUnsupportedData("Test Name 2");
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
      f.attachments(true)->addAttachedFile(Matroska::AttachedFile());
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
    initialProps["DATE"] = StringList("2023");
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
      CPPUNIT_ASSERT_EQUAL(2023U, tag->year());
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
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[0].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[1].language());
      CPPUNIT_ASSERT_EQUAL(String("ARTIST"), simpleTags[1].name());
      CPPUNIT_ASSERT_EQUAL(String("Actors"), simpleTags[1].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[1].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[1].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[1].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[1].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[1].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[2].language());
      CPPUNIT_ASSERT_EQUAL(String("DESCRIPTION"), simpleTags[2].name());
      CPPUNIT_ASSERT_EQUAL(String("Description"), simpleTags[2].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[2].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[2].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[2].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[2].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[2].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[3].language());
      CPPUNIT_ASSERT_EQUAL(String("DIRECTOR"), simpleTags[3].name());
      CPPUNIT_ASSERT_EQUAL(String("Director"), simpleTags[3].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[3].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[3].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[3].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[3].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[3].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[4].language());
      CPPUNIT_ASSERT_EQUAL(String("ENCODER"), simpleTags[4].name());
      CPPUNIT_ASSERT_EQUAL(String("Lavf59.27.100"), simpleTags[4].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[4].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[4].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[4].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[4].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[4].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[5].language());
      CPPUNIT_ASSERT_EQUAL(String("GENRE"), simpleTags[5].name());
      CPPUNIT_ASSERT_EQUAL(String("Genre"), simpleTags[5].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[5].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[5].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[5].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[5].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[5].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[6].language());
      CPPUNIT_ASSERT_EQUAL(String("SUMMARY"), simpleTags[6].name());
      CPPUNIT_ASSERT_EQUAL(String("Comment"), simpleTags[6].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[6].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[6].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[6].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[6].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[6].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[7].language());
      CPPUNIT_ASSERT_EQUAL(String("SYNOPSIS"), simpleTags[7].name());
      CPPUNIT_ASSERT_EQUAL(String("Plot"), simpleTags[7].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[7].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[7].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Track, simpleTags[7].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[7].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[7].type());

      CPPUNIT_ASSERT_EQUAL(String("und"), simpleTags[8].language());
      CPPUNIT_ASSERT_EQUAL(String("DATE_RELEASED"), simpleTags[8].name());
      CPPUNIT_ASSERT_EQUAL(String("2023"), simpleTags[8].toString());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[8].toByteVector());
      CPPUNIT_ASSERT_EQUAL(true, simpleTags[8].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Album, simpleTags[8].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(0ULL, simpleTags[8].trackUid());
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
        "DATE_RELEASED", "PART_NUMBER", "TOTAL_PARTS",
        "MUSICBRAINZ_ALBUMARTISTID", "MUSICBRAINZ_ALBUMID",
        "MUSICBRAINZ_RELEASEGROUPID", "ARTIST", "ARTISTS", "ARTISTSORT",
        "ASIN", "BARCODE", "CATALOG_NUMBER", "CATALOG_NUMBER", "COMMENT",
        "MIXED_BY", "ENCODER", "ENCODER_SETTINGS", "DATE_ENCODED", "GENRE",
        "INITIAL_KEY", "ISRC", "LABEL_CODE", "LABEL_CODE",
        "ORIGINAL_MEDIA_TYPE", "MUSICBRAINZ_ARTISTID",
        "MUSICBRAINZ_RELEASETRACKID", "MUSICBRAINZ_TRACKID", "ORIGINALDATE",
        "PURCHASE_OWNER", "RELEASECOUNTRY", "DATE_RELEASED", "RELEASESTATUS",
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
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::None, simpleTags[0].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[0].toByteVector());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[0].type());

      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("value").value<String>(), simpleTags[1].toString());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("language").value<String>(), simpleTags[1].language());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("name").value<String>(), simpleTags[1].name());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("defaultLanguage").value<bool>(), simpleTags[1].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(targetTypeTag.value("trackUid").value<unsigned long long>(), simpleTags[1].trackUid());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::TargetTypeValue::Subtrack, simpleTags[1].targetTypeValue());
      CPPUNIT_ASSERT_EQUAL(ByteVector(), simpleTags[1].toByteVector());
      CPPUNIT_ASSERT_EQUAL(Matroska::SimpleTag::ValueType::StringType, simpleTags[1].type());

      CPPUNIT_ASSERT_EQUAL(binaryTag.value("data").value<ByteVector>(), simpleTags[2].toByteVector());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("language").value<String>(), simpleTags[2].language());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("name").value<String>(), simpleTags[2].name());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("defaultLanguage").value<bool>(), simpleTags[2].defaultLanguageFlag());
      CPPUNIT_ASSERT_EQUAL(binaryTag.value("trackUid").value<unsigned long long>(), simpleTags[2].trackUid());
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
      Matroska::AttachedFile attachedFile;
      attachedFile.setFileName("cover.jpg");
      attachedFile.setMediaType("image/jpeg");
      attachedFile.setDescription("Cover");
      // Large enough for emitSizeChanged() from Matroska::Segment::render()
      attachedFile.setData(ByteVector(20000, 'x'));
      attachedFile.setUID(5081000385627515072ULL);
      attachments->addAttachedFile(attachedFile);
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
        {"DATE", {"2023"}},
        {"DESCRIPTION", {"Description"}},
        {"DIRECTOR", {"Director"}},
        {"ENCODEDBY", {"Lavf59.27.100"}},
        {"GENRE", {"Genre"}},
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMatroska);
