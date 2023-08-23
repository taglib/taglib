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

#include "attachedpictureframe.h"
#include "chapterframe.h"
#include "commentsframe.h"
#include "eventtimingcodesframe.h"
#include "generalencapsulatedobjectframe.h"
#include "id3v2frame.h"
#include "id3v2tag.h"
#include "mpegfile.h"
#include "ownershipframe.h"
#include "plainfile.h"
#include "podcastframe.h"
#include "popularimeterframe.h"
#include "privateframe.h"
#include "relativevolumeframe.h"
#include "synchronizedlyricsframe.h"
#include "tableofcontentsframe.h"
#include "tdebug.h"
#include "textidentificationframe.h"
#include "tpropertymap.h"
#include "tzlib.h"
#include "uniquefileidentifierframe.h"
#include "unknownframe.h"
#include "unsynchronizedlyricsframe.h"
#include "urllinkframe.h"
#include "utils.h"
#include <cstdio>
#include <string>
#include <utility>

#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

class PublicFrame : public ID3v2::Frame
{
public:
  PublicFrame() :
    ID3v2::Frame(ByteVector("XXXX\0\0\0\0\0\0", 10)) { }
  String readStringField(const ByteVector &data, String::Type encoding,
                         int *position = nullptr)
  {
    return ID3v2::Frame::readStringField(data, encoding, position);
  }
  String toString() const override { return String(); }
  void parseFields(const ByteVector &) override { }
  ByteVector renderFields() const override { return ByteVector(); }
};

TEST(ID3v2, testUnsynchDecode)
{
  MPEG::File f(TEST_FILE_PATH_C("unsynch.id3"), false);
  ASSERT_TRUE(f.tag());
  ASSERT_EQ(String("My babe just cares for me"), f.tag()->title());
}

TEST(ID3v2, testDowngradeUTF8ForID3v23_1)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  auto f
    = new ID3v2::TextIdentificationFrame(ByteVector("TPE1"), String::UTF8);
  StringList sl;
  sl.append("Foo");
  f->setText(sl);

  MPEG::File file(newname.c_str());
  file.ID3v2Tag(true)->addFrame(f);
  file.save(MPEG::File::ID3v2, File::StripOthers, ID3v2::v3);
  ASSERT_EQ(true, file.hasID3v2Tag());

  ByteVector data = f->render();
  ASSERT_EQ(static_cast<unsigned int>(4 + 4 + 2 + 1 + 6 + 2), data.size());

  ID3v2::TextIdentificationFrame f2(data);
  ASSERT_EQ(sl, f2.fieldList());
  ASSERT_EQ(String::UTF16, f2.textEncoding());
}

TEST(ID3v2, testDowngradeUTF8ForID3v23_2)
{
  ScopedFileCopy copy("xing", ".mp3");

  auto f = new ID3v2::UnsynchronizedLyricsFrame(String::UTF8);
  f->setText("Foo");

  MPEG::File file(copy.fileName().c_str());
  file.ID3v2Tag(true)->addFrame(f);
  file.save(MPEG::File::ID3v2, File::StripOthers, ID3v2::v3);
  ASSERT_TRUE(file.hasID3v2Tag());

  ByteVector data = f->render();
  ASSERT_EQ(static_cast<unsigned int>(4 + 4 + 2 + 1 + 3 + 2 + 2 + 6 + 2), data.size());

  ID3v2::UnsynchronizedLyricsFrame f2(data);
  ASSERT_EQ(String("Foo"), f2.text());
  ASSERT_EQ(String::UTF16, f2.textEncoding());
}

TEST(ID3v2, testUTF16BEDelimiter)
{
  ID3v2::TextIdentificationFrame f(ByteVector("TPE1"), String::UTF16BE);
  StringList sl;
  sl.append("Foo");
  sl.append("Bar");
  f.setText(sl);
  ByteVector data = f.render();
  ASSERT_EQ(static_cast<unsigned int>(4 + 4 + 2 + 1 + 6 + 2 + 6), data.size());
  ByteVector noBomBeData("TPE1\x00\x00\x00\x0f\x00\x00\x02"
                         "\0F\0o\0o\0\0"
                         "\0B\0a\0r",
                         25);
  ASSERT_EQ(noBomBeData, data);
  f.setData(data);
  ASSERT_EQ(String("Foo Bar"), f.toString());
}

TEST(ID3v2, testUTF16Delimiter)
{
  ID3v2::TextIdentificationFrame f(ByteVector("TPE1"), String::UTF16);
  StringList sl;
  sl.append("Foo");
  sl.append("Bar");
  f.setText(sl);
  ByteVector data = f.render();
  ASSERT_EQ(static_cast<unsigned int>(4 + 4 + 2 + 1 + 8 + 2 + 8), data.size());
  ByteVector multiBomLeData("TPE1\x00\x00\x00\x13\x00\x00\x01\xff\xfe"
                            "F\0o\0o\0\0\0"
                            "\xff\xfe"
                            "B\0a\0r\0",
                            29);
  ASSERT_EQ(multiBomLeData, data);
  f.setData(data);
  ASSERT_EQ(String("Foo Bar"), f.toString());

  ByteVector multiBomBeData("TPE1\x00\x00\x00\x13\x00\x00\x01\xfe\xff"
                            "\0F\0o\0o\0\0"
                            "\xfe\xff"
                            "\0B\0a\0r",
                            29);
  f.setData(multiBomBeData);
  ASSERT_EQ(String("Foo Bar"), f.toString());

  ByteVector singleBomLeData("TPE1\x00\x00\x00\x13\x00\x00\x01\xff\xfe"
                             "F\0o\0o\0\0\0"
                             "B\0a\0r\0",
                             27);
  f.setData(singleBomLeData);
  ASSERT_EQ(String("Foo Bar"), f.toString());

  ByteVector singleBomBeData("TPE1\x00\x00\x00\x13\x00\x00\x01\xfe\xff"
                             "\0F\0o\0o\0\0"
                             "\0B\0a\0r",
                             27);
  f.setData(singleBomBeData);
  ASSERT_EQ(String("Foo Bar"), f.toString());
}

TEST(ID3v2, testBrokenFrame1)
{
  MPEG::File f(TEST_FILE_PATH_C("broken-tenc.id3"), false);
  ASSERT_TRUE(f.tag());
  ASSERT_FALSE(f.ID3v2Tag()->frameListMap().contains("TENC"));
}

TEST(ID3v2, testReadStringField)
{
  PublicFrame f;
  ByteVector data("abc\0", 4);
  String str = f.readStringField(data, String::Latin1);
  ASSERT_EQ(String("abc"), str);
}

// http://bugs.kde.org/show_bug.cgi?id=151078
TEST(ID3v2, testParseAPIC)
{
  ID3v2::AttachedPictureFrame f(ByteVector("APIC"
                                           "\x00\x00\x00\x07"
                                           "\x00\x00"
                                           "\x00"
                                           "m\x00"
                                           "\x01"
                                           "d\x00"
                                           "\x00",
                                           17));
  ASSERT_EQ(String("m"), f.mimeType());
  ASSERT_EQ(ID3v2::AttachedPictureFrame::FileIcon, f.type());
  ASSERT_EQ(String("d"), f.description());
}

TEST(ID3v2, testParseAPIC_UTF16_BOM)
{
  ID3v2::AttachedPictureFrame f(ByteVector(
    "\x41\x50\x49\x43\x00\x02\x0c\x59\x00\x00\x01\x69\x6d\x61\x67\x65"
    "\x2f\x6a\x70\x65\x67\x00\x00\xfe\xff\x00\x63\x00\x6f\x00\x76\x00"
    "\x65\x00\x72\x00\x2e\x00\x6a\x00\x70\x00\x67\x00\x00\xff\xd8\xff",
    16 * 3));
  ASSERT_EQ(String("image/jpeg"), f.mimeType());
  ASSERT_EQ(ID3v2::AttachedPictureFrame::Other, f.type());
  ASSERT_EQ(String("cover.jpg"), f.description());
  ASSERT_EQ(ByteVector("\xff\xd8\xff", 3), f.picture());
}

TEST(ID3v2, testParseAPICv22)
{
  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ByteVector data              = ByteVector("PIC"
                                                         "\x00\x00\x08"
                                                         "\x00"
                                                         "JPG"
                                                         "\x01"
                                                         "d\x00"
                                                         "\x00",
                                            14);
  ID3v2::Header header;
  header.setMajorVersion(2);
  auto frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(factory->createFrame(data, &header));

  ASSERT_TRUE(frame);
  ASSERT_EQ(String("image/jpeg"), frame->mimeType());
  ASSERT_EQ(ID3v2::AttachedPictureFrame::FileIcon, frame->type());
  ASSERT_EQ(String("d"), frame->description());

  delete frame;
}

TEST(ID3v2, testRenderAPIC)
{
  ID3v2::AttachedPictureFrame f;
  f.setTextEncoding(String::UTF8);
  f.setMimeType("image/png");
  f.setType(ID3v2::AttachedPictureFrame::BackCover);
  f.setDescription("Description");
  f.setPicture("PNG data");
  ASSERT_EQ(
    ByteVector("APIC"
               "\x00\x00\x00\x20"
               "\x00\x00"
               "\x03"
               "image/png\x00"
               "\x04"
               "Description\x00"
               "PNG data",
               42),
    f.render());
}

TEST(ID3v2, testDontRender22)
{
  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ByteVector data              = ByteVector("FOO"
                                                         "\x00\x00\x08"
                                                         "\x00"
                                                         "JPG"
                                                         "\x01"
                                                         "d\x00"
                                                         "\x00",
                                            14);
  ID3v2::Header header;
  header.setMajorVersion(2);
  auto frame = dynamic_cast<TagLib::ID3v2::UnknownFrame *>(factory->createFrame(data, &header));

  ASSERT_TRUE(frame);

  ID3v2::Tag tag;
  tag.addFrame(frame);
  ASSERT_EQ(static_cast<unsigned int>(1034), tag.render().size());
}

// http://bugs.kde.org/show_bug.cgi?id=151078
TEST(ID3v2, testParseGEOB)
{
  ID3v2::GeneralEncapsulatedObjectFrame f(ByteVector("GEOB"
                                                     "\x00\x00\x00\x08"
                                                     "\x00\x00"
                                                     "\x00"
                                                     "m\x00"
                                                     "f\x00"
                                                     "d\x00"
                                                     "\x00",
                                                     18));
  ASSERT_EQ(String("m"), f.mimeType());
  ASSERT_EQ(String("f"), f.fileName());
  ASSERT_EQ(String("d"), f.description());
}

TEST(ID3v2, testRenderGEOB)
{
  ID3v2::GeneralEncapsulatedObjectFrame f;
  f.setTextEncoding(String::Latin1);
  f.setMimeType("application/octet-stream");
  f.setFileName("test.bin");
  f.setDescription("Description");
  f.setObject(ByteVector(3, '\x01'));
  ASSERT_EQ(
    ByteVector("GEOB"
               "\x00\x00\x00\x32"
               "\x00\x00"
               "\x00"
               "application/octet-stream\x00"
               "test.bin\x00"
               "Description\x00"
               "\x01\x01\x01",
               60),
    f.render());
}

TEST(ID3v2, testParsePOPM)
{
  ID3v2::PopularimeterFrame f(ByteVector("POPM"
                                         "\x00\x00\x00\x17"
                                         "\x00\x00"
                                         "email@example.com\x00"
                                         "\x02"
                                         "\x00\x00\x00\x03",
                                         33));
  ASSERT_EQ(String("email@example.com"), f.email());
  ASSERT_EQ(2, f.rating());
  ASSERT_EQ(static_cast<unsigned int>(3), f.counter());
}

TEST(ID3v2, testParsePOPMWithoutCounter)
{
  ID3v2::PopularimeterFrame f(ByteVector("POPM"
                                         "\x00\x00\x00\x13"
                                         "\x00\x00"
                                         "email@example.com\x00"
                                         "\x02",
                                         29));
  ASSERT_EQ(String("email@example.com"), f.email());
  ASSERT_EQ(2, f.rating());
  ASSERT_EQ(static_cast<unsigned int>(0), f.counter());
}

TEST(ID3v2, testRenderPOPM)
{
  ID3v2::PopularimeterFrame f;
  f.setEmail("email@example.com");
  f.setRating(2);
  f.setCounter(3);
  ASSERT_EQ(
    ByteVector("POPM"
               "\x00\x00\x00\x17"
               "\x00\x00"
               "email@example.com\x00"
               "\x02"
               "\x00\x00\x00\x03",
               33),
    f.render());
}

TEST(ID3v2, testPOPMtoString)
{
  ID3v2::PopularimeterFrame f;
  f.setEmail("email@example.com");
  f.setRating(2);
  f.setCounter(3);
  ASSERT_EQ(
    String("email@example.com rating=2 counter=3"), f.toString());
}

TEST(ID3v2, testPOPMFromFile)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  auto f         = new ID3v2::PopularimeterFrame();
  f->setEmail("email@example.com");
  f->setRating(200);
  f->setCounter(3);

  {
    MPEG::File foo(newname.c_str());
    foo.ID3v2Tag()->addFrame(f);
    foo.save();
  }
  {
    MPEG::File bar(newname.c_str());
    ASSERT_EQ(String("email@example.com"), dynamic_cast<ID3v2::PopularimeterFrame *>(bar.ID3v2Tag()->frameList("POPM").front())->email());
    ASSERT_EQ(200, dynamic_cast<ID3v2::PopularimeterFrame *>(bar.ID3v2Tag()->frameList("POPM").front())->rating());
  }
}

// http://bugs.kde.org/show_bug.cgi?id=150481
TEST(ID3v2, testParseRelativeVolumeFrame)
{
  ID3v2::RelativeVolumeFrame f(
    ByteVector("RVA2"             // Frame ID
               "\x00\x00\x00\x0B" // Frame size
               "\x00\x00"         // Frame flags
               "ident\x00"        // Identification
               "\x02"             // Type of channel
               "\x00\x0F"         // Volume adjustment
               "\x08"             // Bits representing peak
               "\x45",
               21));              // Peak volume
  ASSERT_EQ(String("ident"), f.identification());
  ASSERT_EQ(15.0f / 512.0f,
            f.volumeAdjustment(ID3v2::RelativeVolumeFrame::FrontRight));
  ASSERT_EQ(static_cast<short>(15),
            f.volumeAdjustmentIndex(ID3v2::RelativeVolumeFrame::FrontRight));
  ASSERT_EQ(static_cast<unsigned char>(8),
            f.peakVolume(ID3v2::RelativeVolumeFrame::FrontRight).bitsRepresentingPeak);
  ASSERT_EQ(ByteVector("\x45"),
            f.peakVolume(ID3v2::RelativeVolumeFrame::FrontRight).peakVolume);
  const List<ID3v2::RelativeVolumeFrame::ChannelType> channels = f.channels();
  ASSERT_EQ(1U, channels.size());
  ASSERT_EQ(ID3v2::RelativeVolumeFrame::FrontRight, channels[0]);
}

TEST(ID3v2, testRenderRelativeVolumeFrame)
{
  ID3v2::RelativeVolumeFrame f;
  f.setIdentification("ident");
  f.setVolumeAdjustment(15.0f / 512.0f, ID3v2::RelativeVolumeFrame::FrontRight);
  ID3v2::RelativeVolumeFrame::PeakVolume peakVolume;
  peakVolume.bitsRepresentingPeak = 8;
  peakVolume.peakVolume.setData("\x45");
  f.setPeakVolume(peakVolume, ID3v2::RelativeVolumeFrame::FrontRight);
  ASSERT_EQ(
    ByteVector("RVA2"
               "\x00\x00\x00\x0B"
               "\x00\x00"
               "ident\x00"
               "\x02"
               "\x00\x0F"
               "\x08"
               "\x45",
               21),
    f.render());
}

TEST(ID3v2, testParseUniqueFileIdentifierFrame)
{
  ID3v2::UniqueFileIdentifierFrame f(
    ByteVector("UFID"             // Frame ID
               "\x00\x00\x00\x09" // Frame size
               "\x00\x00"         // Frame flags
               "owner\x00"        // Owner identifier
               "\x00\x01\x02",
               19));              // Identifier
  ASSERT_EQ(String("owner"),
            f.owner());
  ASSERT_EQ(ByteVector("\x00\x01\x02", 3),
            f.identifier());
}

TEST(ID3v2, testParseEmptyUniqueFileIdentifierFrame)
{
  ID3v2::UniqueFileIdentifierFrame f(
    ByteVector("UFID"             // Frame ID
               "\x00\x00\x00\x01" // Frame size
               "\x00\x00"         // Frame flags
               "\x00"             // Owner identifier
               "",
               11));              // Identifier
  ASSERT_EQ(String(),
            f.owner());
  ASSERT_EQ(ByteVector(),
            f.identifier());
}

TEST(ID3v2, testRenderUniqueFileIdentifierFrame)
{
  ID3v2::UniqueFileIdentifierFrame f("owner", "\x01\x02\x03");
  ASSERT_EQ(
    ByteVector("UFID"
               "\x00\x00\x00\x09"
               "\x00\x00"
               "owner\x00"
               "\x01\x02\x03",
               19),
    f.render());
}

TEST(ID3v2, testParseUrlLinkFrame)
{
  ID3v2::UrlLinkFrame f(
    ByteVector("WOAF"             // Frame ID
               "\x00\x00\x00\x12" // Frame size
               "\x00\x00"         // Frame flags
               "http://example.com",
               28));              // URL
  ASSERT_EQ(String("http://example.com"), f.url());
}

TEST(ID3v2, testRenderUrlLinkFrame)
{
  ID3v2::UrlLinkFrame f("WOAF");
  f.setUrl("http://example.com");
  ASSERT_EQ(
    ByteVector("WOAF"             // Frame ID
               "\x00\x00\x00\x12" // Frame size
               "\x00\x00"         // Frame flags
               "http://example.com",
               28),               // URL
    f.render());
}

TEST(ID3v2, testParseUserUrlLinkFrame)
{
  ID3v2::UserUrlLinkFrame f(
    ByteVector("WXXX"             // Frame ID
               "\x00\x00\x00\x17" // Frame size
               "\x00\x00"         // Frame flags
               "\x00"             // Text encoding
               "foo\x00"          // Description
               "http://example.com",
               33));              // URL
  ASSERT_EQ(String("foo"), f.description());
  ASSERT_EQ(String("http://example.com"), f.url());
}

TEST(ID3v2, testRenderUserUrlLinkFrame)
{
  ID3v2::UserUrlLinkFrame f;
  f.setDescription("foo");
  f.setUrl("http://example.com");
  ASSERT_EQ(
    ByteVector("WXXX"             // Frame ID
               "\x00\x00\x00\x17" // Frame size
               "\x00\x00"         // Frame flags
               "\x00"             // Text encoding
               "foo\x00"          // Description
               "http://example.com",
               33),               // URL
    f.render());
}

TEST(ID3v2, testParseOwnershipFrame)
{
  ID3v2::OwnershipFrame f(
    ByteVector("OWNE"             // Frame ID
               "\x00\x00\x00\x19" // Frame size
               "\x00\x00"         // Frame flags
               "\x00"             // Text encoding
               "GBP1.99\x00"      // Price paid
               "20120905"         // Date of purchase
               "Beatport",
               35));              // Seller
  ASSERT_EQ(String("GBP1.99"), f.pricePaid());
  ASSERT_EQ(String("20120905"), f.datePurchased());
  ASSERT_EQ(String("Beatport"), f.seller());
}

TEST(ID3v2, testRenderOwnershipFrame)
{
  ID3v2::OwnershipFrame f;
  f.setPricePaid("GBP1.99");
  f.setDatePurchased("20120905");
  f.setSeller("Beatport");
  ASSERT_EQ(
    ByteVector("OWNE"             // Frame ID
               "\x00\x00\x00\x19" // Frame size
               "\x00\x00"         // Frame flags
               "\x00"             // Text encoding
               "GBP1.99\x00"      // Price paid
               "20120905"         // Date of purchase
               "Beatport",
               35),               // URL
    f.render());
}

TEST(ID3v2, testParseSynchronizedLyricsFrame)
{
  ID3v2::SynchronizedLyricsFrame f(
    ByteVector("SYLT"             // Frame ID
               "\x00\x00\x00\x21" // Frame size
               "\x00\x00"         // Frame flags
               "\x00"             // Text encoding
               "eng"              // Language
               "\x02"             // Time stamp format
               "\x01"             // Content type
               "foo\x00"          // Content descriptor
               "Example\x00"      // 1st text
               "\x00\x00\x04\xd2" // 1st time stamp
               "Lyrics\x00"       // 2nd text
               "\x00\x00\x11\xd7",
               43));              // 2nd time stamp
  ASSERT_EQ(String::Latin1, f.textEncoding());
  ASSERT_EQ(ByteVector("eng", 3), f.language());
  ASSERT_EQ(ID3v2::SynchronizedLyricsFrame::AbsoluteMilliseconds,
            f.timestampFormat());
  ASSERT_EQ(ID3v2::SynchronizedLyricsFrame::Lyrics, f.type());
  ASSERT_EQ(String("foo"), f.description());
  ID3v2::SynchronizedLyricsFrame::SynchedTextList stl = f.synchedText();
  ASSERT_EQ(static_cast<unsigned int>(2), stl.size());
  ASSERT_EQ(String("Example"), stl[0].text);
  ASSERT_EQ(static_cast<unsigned int>(1234), stl[0].time);
  ASSERT_EQ(String("Lyrics"), stl[1].text);
  ASSERT_EQ(static_cast<unsigned int>(4567), stl[1].time);
}

TEST(ID3v2, testParseSynchronizedLyricsFrameWithEmptyDescritpion)
{
  ID3v2::SynchronizedLyricsFrame f(
    ByteVector("SYLT"             // Frame ID
               "\x00\x00\x00\x21" // Frame size
               "\x00\x00"         // Frame flags
               "\x00"             // Text encoding
               "eng"              // Language
               "\x02"             // Time stamp format
               "\x01"             // Content type
               "\x00"             // Content descriptor
               "Example\x00"      // 1st text
               "\x00\x00\x04\xd2" // 1st time stamp
               "Lyrics\x00"       // 2nd text
               "\x00\x00\x11\xd7",
               40));              // 2nd time stamp
  ASSERT_EQ(String::Latin1, f.textEncoding());
  ASSERT_EQ(ByteVector("eng", 3), f.language());
  ASSERT_EQ(ID3v2::SynchronizedLyricsFrame::AbsoluteMilliseconds,
            f.timestampFormat());
  ASSERT_EQ(ID3v2::SynchronizedLyricsFrame::Lyrics, f.type());
  ASSERT_TRUE(f.description().isEmpty());
  ID3v2::SynchronizedLyricsFrame::SynchedTextList stl = f.synchedText();
  ASSERT_EQ(static_cast<unsigned int>(2), stl.size());
  ASSERT_EQ(String("Example"), stl[0].text);
  ASSERT_EQ(static_cast<unsigned int>(1234), stl[0].time);
  ASSERT_EQ(String("Lyrics"), stl[1].text);
  ASSERT_EQ(static_cast<unsigned int>(4567), stl[1].time);
}

TEST(ID3v2, testRenderSynchronizedLyricsFrame)
{
  ID3v2::SynchronizedLyricsFrame f;
  f.setTextEncoding(String::Latin1);
  f.setLanguage(ByteVector("eng", 3));
  f.setTimestampFormat(ID3v2::SynchronizedLyricsFrame::AbsoluteMilliseconds);
  f.setType(ID3v2::SynchronizedLyricsFrame::Lyrics);
  f.setDescription("foo");
  ID3v2::SynchronizedLyricsFrame::SynchedTextList stl;
  stl.append(ID3v2::SynchronizedLyricsFrame::SynchedText(1234, "Example"));
  stl.append(ID3v2::SynchronizedLyricsFrame::SynchedText(4567, "Lyrics"));
  f.setSynchedText(stl);
  ASSERT_EQ(
    ByteVector("SYLT"             // Frame ID
               "\x00\x00\x00\x21" // Frame size
               "\x00\x00"         // Frame flags
               "\x00"             // Text encoding
               "eng"              // Language
               "\x02"             // Time stamp format
               "\x01"             // Content type
               "foo\x00"          // Content descriptor
               "Example\x00"      // 1st text
               "\x00\x00\x04\xd2" // 1st time stamp
               "Lyrics\x00"       // 2nd text
               "\x00\x00\x11\xd7",
               43),               // 2nd time stamp
    f.render());
}

TEST(ID3v2, testParseEventTimingCodesFrame)
{
  ID3v2::EventTimingCodesFrame f(
    ByteVector("ETCO"             // Frame ID
               "\x00\x00\x00\x0b" // Frame size
               "\x00\x00"         // Frame flags
               "\x02"             // Time stamp format
               "\x02"             // 1st event
               "\x00\x00\xf3\x5c" // 1st time stamp
               "\xfe"             // 2nd event
               "\x00\x36\xee\x80",
               21));              // 2nd time stamp
  ASSERT_EQ(ID3v2::EventTimingCodesFrame::AbsoluteMilliseconds,
            f.timestampFormat());
  ID3v2::EventTimingCodesFrame::SynchedEventList sel = f.synchedEvents();
  ASSERT_EQ(static_cast<unsigned int>(2), sel.size());
  ASSERT_EQ(ID3v2::EventTimingCodesFrame::IntroStart, sel[0].type);
  ASSERT_EQ(static_cast<unsigned int>(62300), sel[0].time);
  ASSERT_EQ(ID3v2::EventTimingCodesFrame::AudioFileEnds, sel[1].type);
  ASSERT_EQ(static_cast<unsigned int>(3600000), sel[1].time);
}

TEST(ID3v2, testRenderEventTimingCodesFrame)
{
  ID3v2::EventTimingCodesFrame f;
  f.setTimestampFormat(ID3v2::EventTimingCodesFrame::AbsoluteMilliseconds);
  ID3v2::EventTimingCodesFrame::SynchedEventList sel;
  sel.append(ID3v2::EventTimingCodesFrame::SynchedEvent(62300, ID3v2::EventTimingCodesFrame::IntroStart));
  sel.append(ID3v2::EventTimingCodesFrame::SynchedEvent(3600000, ID3v2::EventTimingCodesFrame::AudioFileEnds));
  f.setSynchedEvents(sel);
  ASSERT_EQ(
    ByteVector("ETCO"             // Frame ID
               "\x00\x00\x00\x0b" // Frame size
               "\x00\x00"         // Frame flags
               "\x02"             // Time stamp format
               "\x02"             // 1st event
               "\x00\x00\xf3\x5c" // 1st time stamp
               "\xfe"             // 2nd event
               "\x00\x36\xee\x80",
               21),               // 2nd time stamp
    f.render());
}

TEST(ID3v2, testParseCommentsFrame)
{
  ID3v2::CommentsFrame f(
    ByteVector("COMM"
               "\x00\x00\x00\x14"
               "\x00\x00"
               "\x03"
               "deu"
               "Description\x00"
               "Text",
               30));
  ASSERT_EQ(String::UTF8, f.textEncoding());
  ASSERT_EQ(ByteVector("deu"), f.language());
  ASSERT_EQ(String("Description"), f.description());
  ASSERT_EQ(String("Text"), f.text());
}

TEST(ID3v2, testRenderCommentsFrame)
{
  ID3v2::CommentsFrame f;
  f.setTextEncoding(String::UTF16);
  f.setLanguage("eng");
  f.setDescription("Description");
  f.setText("Text");
  ASSERT_EQ(
    ByteVector("COMM"
               "\x00\x00\x00\x28"
               "\x00\x00"
               "\x01"
               "eng"
               "\xff\xfe"
               "D\0e\0s\0c\0r\0i\0p\0t\0i\0o\0n\0"
               "\x00\x00"
               "\xff\xfe"
               "T\0e\0x\0t\0",
               50),
    f.render());
}

TEST(ID3v2, testParsePodcastFrame)
{
  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ByteVector data              = ByteVector("PCST"
                                                         "\x00\x00\x00\x04"
                                                         "\x00\x00"
                                                         "\x00\x00\x00\x00",
                                            14);
  const ID3v2::Header header;
  ASSERT_TRUE(dynamic_cast<ID3v2::PodcastFrame *>(
    factory->createFrame(data, &header)));
}

TEST(ID3v2, testRenderPodcastFrame)
{
  ID3v2::PodcastFrame f;
  ASSERT_EQ(
    ByteVector("PCST"
               "\x00\x00\x00\x04"
               "\x00\x00"
               "\x00\x00\x00\x00",
               14),
    f.render());
}

TEST(ID3v2, testParsePrivateFrame)
{
  ID3v2::PrivateFrame f(
    ByteVector("PRIV"
               "\x00\x00\x00\x0e"
               "\x00\x00"
               "WM/Provider\x00"
               "TL",
               24));
  ASSERT_EQ(String("WM/Provider"), f.owner());
  ASSERT_EQ(ByteVector("TL"), f.data());
}

TEST(ID3v2, testRenderPrivateFrame)
{
  ID3v2::PrivateFrame f;
  f.setOwner("WM/Provider");
  f.setData("TL");
  ASSERT_EQ(
    ByteVector("PRIV"
               "\x00\x00\x00\x0e"
               "\x00\x00"
               "WM/Provider\x00"
               "TL",
               24),
    f.render());
}

TEST(ID3v2, testParseUserTextIdentificationFrame)
{
  ID3v2::UserTextIdentificationFrame frameWithoutDescription(
    ByteVector("TXXX"
               "\x00\x00\x00\x06"
               "\x00\x00\x00"
               "\x00"
               "Text",
               16));
  ASSERT_EQ(String(""), frameWithoutDescription.description());
  ASSERT_EQ(String("Text"), frameWithoutDescription.fieldList()[1]);

  ID3v2::UserTextIdentificationFrame frameWithDescription(
    ByteVector("TXXX"
               "\x00\x00\x00\x11"
               "\x00\x00\x00"
               "Description\x00"
               "Text",
               27));
  ASSERT_EQ(String("Description"), frameWithDescription.description());
  ASSERT_EQ(String("Text"), frameWithDescription.fieldList()[1]);
}

TEST(ID3v2, testRenderUserTextIdentificationFrame)
{
  ID3v2::UserTextIdentificationFrame f;
  f.setDescription("");
  f.setText("Text");
  ASSERT_EQ(
    ByteVector("TXXX"
               "\x00\x00\x00\x06"
               "\x00\x00\x00"
               "\x00"
               "Text",
               16),
    f.render());

  f.setDescription("Description");
  f.setText("Text");
  ASSERT_EQ(
    ByteVector("TXXX"
               "\x00\x00\x00\x11"
               "\x00\x00\x00"
               "Description\x00"
               "Text",
               27),
    f.render());
}

TEST(ID3v2, testItunes24FrameSize)
{
  MPEG::File f(TEST_FILE_PATH_C("005411.id3"), false);
  ASSERT_TRUE(f.tag());
  ASSERT_TRUE(f.ID3v2Tag()->frameListMap().contains("TIT2"));
  ASSERT_EQ(String("Sunshine Superman"), f.ID3v2Tag()->frameListMap()["TIT2"].front()->toString());
}

TEST(ID3v2, testSaveUTF16Comment)
{
  String::Type defaultEncoding = ID3v2::FrameFactory::instance()->defaultTextEncoding();
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();
  ID3v2::FrameFactory::instance()->setDefaultTextEncoding(String::UTF16);
  {
    MPEG::File foo(newname.c_str());
    foo.strip();
    foo.tag()->setComment("Test comment!");
    foo.save();
  }
  {
    MPEG::File bar(newname.c_str());
    ASSERT_EQ(String("Test comment!"), bar.tag()->comment());
    ID3v2::FrameFactory::instance()->setDefaultTextEncoding(defaultEncoding);
  }
}

TEST(ID3v2, testUpdateGenre23_1)
{
  // "Refinement" is the same as the ID3v1 genre - duplicate
  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ByteVector data              = ByteVector("TCON"             // Frame ID
                                            "\x00\x00\x00\x10" // Frame size
                                            "\x00\x00"         // Frame flags
                                            "\x00"             // Encoding
                                            "(22)Death Metal",
                                            26);               // Text
  ID3v2::Header header;
  header.setMajorVersion(3);
  auto frame = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame *>(factory->createFrame(data, &header));
  ASSERT_EQ(static_cast<unsigned int>(1), frame->fieldList().size());
  ASSERT_EQ(String("Death Metal"), frame->fieldList()[0]);

  ID3v2::Tag tag;
  tag.addFrame(frame);
  ASSERT_EQ(String("Death Metal"), tag.genre());
}

TEST(ID3v2, testUpdateGenre23_2)
{
  // "Refinement" is different from the ID3v1 genre
  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ByteVector data              = ByteVector("TCON"             // Frame ID
                                            "\x00\x00\x00\x0d" // Frame size
                                            "\x00\x00"         // Frame flags
                                            "\x00"             // Encoding
                                            "(4)Eurodisco",
                                            23);               // Text
  ID3v2::Header header;
  header.setMajorVersion(3);
  auto frame = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame *>(factory->createFrame(data, &header));
  ASSERT_EQ(static_cast<unsigned int>(2), frame->fieldList().size());
  ASSERT_EQ(String("4"), frame->fieldList()[0]);
  ASSERT_EQ(String("Eurodisco"), frame->fieldList()[1]);

  ID3v2::Tag tag;
  tag.addFrame(frame);
  ASSERT_EQ(String("Disco Eurodisco"), tag.genre());
}

TEST(ID3v2, testUpdateGenre23_3)
{
  // Multiple references and a refinement
  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ByteVector data              = ByteVector("TCON"             // Frame ID
                                            "\x00\x00\x00\x15" // Frame size
                                            "\x00\x00"         // Frame flags
                                            "\x00"             // Encoding
                                            "(9)(138)Viking Metal",
                                            31);               // Text
  ID3v2::Header header;
  header.setMajorVersion(3);
  auto frame = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame *>(factory->createFrame(data, &header));
  ASSERT_EQ(3U, frame->fieldList().size());
  ASSERT_EQ(String("9"), frame->fieldList()[0]);
  ASSERT_EQ(String("138"), frame->fieldList()[1]);
  ASSERT_EQ(String("Viking Metal"), frame->fieldList()[2]);

  ID3v2::Tag tag;
  tag.addFrame(frame);
  ASSERT_EQ(String("Metal Black Metal Viking Metal"), tag.genre());
}

TEST(ID3v2, testUpdateGenre24)
{
  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ByteVector data              = ByteVector("TCON"             // Frame ID
                                            "\x00\x00\x00\x0D" // Frame size
                                            "\x00\x00"         // Frame flags
                                            "\0"               // Encoding
                                            "14\0Eurodisco",
                                            23);               // Text
  ID3v2::Header header;
  auto frame = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame *>(factory->createFrame(data, &header));
  ASSERT_EQ(static_cast<unsigned int>(2), frame->fieldList().size());
  ASSERT_EQ(String("14"), frame->fieldList()[0]);
  ASSERT_EQ(String("Eurodisco"), frame->fieldList()[1]);

  ID3v2::Tag tag;
  tag.addFrame(frame);
  ASSERT_EQ(String("R&B Eurodisco"), tag.genre());
}

TEST(ID3v2, testUpdateDate22)
{
  MPEG::File f(TEST_FILE_PATH_C("id3v22-tda.mp3"), false);
  ASSERT_TRUE(f.tag());
  ASSERT_EQ(static_cast<unsigned int>(2010), f.tag()->year());
}

/*
ST(ID3v2,  testUpdateFullDate22)
{
  MPEG::File f(TEST_FILE_PATH_C("id3v22-tda.mp3"), false);
  ASSERT_TRUE(f.tag());
  ASSERT_EQ(String("2010-04-03"), f.ID3v2Tag()->frameListMap()["TDRC"].front()->toString());
}
*/

TEST(ID3v2, testDowngradeTo23)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  ID3v2::TextIdentificationFrame *tf;
  {
    MPEG::File foo(newname.c_str());
    tf = new ID3v2::TextIdentificationFrame("TDOR", String::Latin1);
    tf->setText("2011-03-16");
    foo.ID3v2Tag()->addFrame(tf);
    tf = new ID3v2::TextIdentificationFrame("TDRC", String::Latin1);
    tf->setText("2012-04-17T12:01");
    foo.ID3v2Tag()->addFrame(tf);
    tf = new ID3v2::TextIdentificationFrame("TMCL", String::Latin1);
    tf->setText(StringList().append("Guitar").append("Artist 1").append("Drums").append("Artist 2"));
    foo.ID3v2Tag()->addFrame(tf);
    tf = new ID3v2::TextIdentificationFrame("TIPL", String::Latin1);
    tf->setText(StringList().append("Producer").append("Artist 3").append("Mastering").append("Artist 4"));
    foo.ID3v2Tag()->addFrame(tf);
    tf = new ID3v2::TextIdentificationFrame("TCON", String::Latin1);
    tf->setText(StringList().append("51").append("Noise").append("Power Noise"));
    foo.ID3v2Tag()->addFrame(tf);
    foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TDRL", String::Latin1));
    foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TDTG", String::Latin1));
    foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TMOO", String::Latin1));
    foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TPRO", String::Latin1));
    foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TSOA", String::Latin1));
    foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TSOT", String::Latin1));
    foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TSST", String::Latin1));
    foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TSOP", String::Latin1));
    foo.save(MPEG::File::AllTags, File::StripOthers, ID3v2::v3);
  }
  {
    MPEG::File bar(newname.c_str());
    tf = dynamic_cast<ID3v2::TextIdentificationFrame *>(bar.ID3v2Tag()->frameList("TDOR").front());
    ASSERT_TRUE(tf);
    ASSERT_EQ(static_cast<unsigned int>(1), tf->fieldList().size());
    ASSERT_EQ(String("2011"), tf->fieldList().front());
    tf = dynamic_cast<ID3v2::TextIdentificationFrame *>(bar.ID3v2Tag()->frameList("TDRC").front());
    ASSERT_TRUE(tf);
    ASSERT_EQ(static_cast<unsigned int>(1), tf->fieldList().size());
    ASSERT_EQ(String("2012-04-17T12:01"), tf->fieldList().front());
    tf = dynamic_cast<ID3v2::TextIdentificationFrame *>(bar.ID3v2Tag()->frameList("TIPL").front());
    ASSERT_TRUE(tf);
    ASSERT_EQ(static_cast<unsigned int>(8), tf->fieldList().size());
    ASSERT_EQ(String("Guitar"), tf->fieldList()[0]);
    ASSERT_EQ(String("Artist 1"), tf->fieldList()[1]);
    ASSERT_EQ(String("Drums"), tf->fieldList()[2]);
    ASSERT_EQ(String("Artist 2"), tf->fieldList()[3]);
    ASSERT_EQ(String("Producer"), tf->fieldList()[4]);
    ASSERT_EQ(String("Artist 3"), tf->fieldList()[5]);
    ASSERT_EQ(String("Mastering"), tf->fieldList()[6]);
    ASSERT_EQ(String("Artist 4"), tf->fieldList()[7]);
    tf = dynamic_cast<ID3v2::TextIdentificationFrame *>(bar.ID3v2Tag()->frameList("TCON").front());
    ASSERT_TRUE(tf);
    ASSERT_EQ(3U, tf->fieldList().size());
    ASSERT_EQ(String("51"), tf->fieldList()[0]);
    ASSERT_EQ(String("39"), tf->fieldList()[1]);
    ASSERT_EQ(String("Power Noise"), tf->fieldList()[2]);
    ASSERT_FALSE(bar.ID3v2Tag()->frameListMap().contains("TDRL"));
    ASSERT_FALSE(bar.ID3v2Tag()->frameListMap().contains("TDTG"));
    ASSERT_FALSE(bar.ID3v2Tag()->frameListMap().contains("TMOO"));
    ASSERT_FALSE(bar.ID3v2Tag()->frameListMap().contains("TPRO"));
#ifdef NO_ITUNES_HACKS
    ASSERT_FALSE(bar.ID3v2Tag()->frameListMap().contains("TSOA"));
    ASSERT_FALSE(bar.ID3v2Tag()->frameListMap().contains("TSOT"));
    ASSERT_FALSE(bar.ID3v2Tag()->frameListMap().contains("TSOP"));
#endif
    ASSERT_FALSE(bar.ID3v2Tag()->frameListMap().contains("TSST"));
  }
  {
    const ByteVector expectedId3v23Data(
      "ID3"
      "\x03\x00\x00\x00\x00\x09\x49"
      "TSOA"
      "\x00\x00\x00\x01\x00\x00\x00"
      "TSOT"
      "\x00\x00\x00\x01\x00\x00\x00"
      "TSOP"
      "\x00\x00\x00\x01\x00\x00\x00"
      "TORY"
      "\x00\x00\x00\x05\x00\x00\x00"
      "2011"
      "TYER"
      "\x00\x00\x00\x05\x00\x00\x00"
      "2012"
      "TDAT"
      "\x00\x00\x00\x05\x00\x00\x00"
      "1704"
      "TIME"
      "\x00\x00\x00\x05\x00\x00\x00"
      "1201"
      "IPLS"
      "\x00\x00\x00\x44\x00\x00\x00"
      "Guitar"
      "\x00"
      "Artist 1"
      "\x00"
      "Drums"
      "\x00"
      "Artist 2"
      "\x00"
      "Producer"
      "\x00"
      "Artist 3"
      "\x00"
      "Mastering"
      "\x00"
      "Artist 4"
      "TCON"
      "\x00\x00\x00\x14\x00\x00\x00"
      "(51)(39)Power Noise",
      211);
    const ByteVector actualId3v23Data = PlainFile(newname.c_str()).readBlock(expectedId3v23Data.size());
    ASSERT_EQ(expectedId3v23Data, actualId3v23Data);
  }

  ScopedFileCopy rareFramesCopy("rare_frames", ".mp3");

  {
    MPEG::File f(rareFramesCopy.fileName().c_str());
    f.save(MPEG::File::AllTags, File::StripOthers, ID3v2::v3);
    f.seek(f.find("TCON") + 11);
    ASSERT_EQ(ByteVector("(13)"), f.readBlock(4));
  }
}

TEST(ID3v2, testCompressedFrameWithBrokenLength)
{
  MPEG::File f(TEST_FILE_PATH_C("compressed_id3_frame.mp3"), false);
  ASSERT_TRUE(f.ID3v2Tag()->frameListMap().contains("APIC"));

  if(zlib::isAvailable()) {
    ID3v2::AttachedPictureFrame *frame
      = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(f.ID3v2Tag()->frameListMap()["APIC"].front());
    ASSERT_TRUE(frame);
    ASSERT_EQ(String("image/bmp"), frame->mimeType());
    ASSERT_EQ(ID3v2::AttachedPictureFrame::Other, frame->type());
    ASSERT_EQ(String(""), frame->description());
    ASSERT_EQ(static_cast<unsigned int>(86414), frame->picture().size());
  }
  else {
    // Skip the test if ZLIB is not installed.
    // The message "Compressed frames are currently not supported." will be displayed.

    ID3v2::UnknownFrame *frame
      = dynamic_cast<TagLib::ID3v2::UnknownFrame *>(f.ID3v2Tag()->frameListMap()["APIC"].front());
    ASSERT_TRUE(frame);
  }
}

TEST(ID3v2, testW000)
{
  MPEG::File f(TEST_FILE_PATH_C("w000.mp3"), false);
  ASSERT_TRUE(f.ID3v2Tag()->frameListMap().contains("W000"));
  ID3v2::UrlLinkFrame *frame = dynamic_cast<TagLib::ID3v2::UrlLinkFrame *>(f.ID3v2Tag()->frameListMap()["W000"].front());
  ASSERT_TRUE(frame);
  ASSERT_EQ(String("lukas.lalinsky@example.com____"), frame->url());
}

TEST(ID3v2, testPropertyInterface)
{
  ScopedFileCopy copy("rare_frames", ".mp3");
  string newname = copy.fileName();
  MPEG::File f(newname.c_str());
  PropertyMap dict = f.ID3v2Tag(false)->properties();
  ASSERT_EQ(static_cast<unsigned int>(6), dict.size());

  ASSERT_TRUE(dict.contains("USERTEXTDESCRIPTION1"));
  ASSERT_TRUE(dict.contains("QuodLibet::USERTEXTDESCRIPTION2"));
  ASSERT_EQ(static_cast<unsigned int>(2), dict["USERTEXTDESCRIPTION1"].size());
  ASSERT_EQ(static_cast<unsigned int>(2), dict["QuodLibet::USERTEXTDESCRIPTION2"].size());
  ASSERT_EQ(String("userTextData1"), dict["USERTEXTDESCRIPTION1"][0]);
  ASSERT_EQ(String("userTextData2"), dict["USERTEXTDESCRIPTION1"][1]);
  ASSERT_EQ(String("userTextData1"), dict["QuodLibet::USERTEXTDESCRIPTION2"][0]);
  ASSERT_EQ(String("userTextData2"), dict["QuodLibet::USERTEXTDESCRIPTION2"][1]);

  ASSERT_EQ(String("Pop"), dict["GENRE"].front());

  ASSERT_EQ(String("http://a.user.url"), dict["URL:USERURL"].front());

  ASSERT_EQ(String("http://a.user.url/with/empty/description"), dict["URL"].front());
  ASSERT_EQ(String("A COMMENT"), dict["COMMENT"].front());

  ASSERT_EQ(1u, dict.unsupportedData().size());
  ASSERT_EQ(String("UFID/supermihi@web.de"), dict.unsupportedData().front());
}

TEST(ID3v2, testPropertyInterface2)
{
  ID3v2::Tag tag;
  auto frame1 = new ID3v2::UnsynchronizedLyricsFrame();
  frame1->setDescription("test");
  frame1->setText("la-la-la test");
  tag.addFrame(frame1);

  auto frame2 = new ID3v2::UnsynchronizedLyricsFrame();
  frame2->setDescription("");
  frame2->setText("la-la-la nodescription");
  tag.addFrame(frame2);

  auto frame3 = new ID3v2::AttachedPictureFrame();
  frame3->setDescription("test picture");
  tag.addFrame(frame3);

  auto frame4 = new ID3v2::TextIdentificationFrame("TIPL");
  frame4->setText("single value is invalid for TIPL");
  tag.addFrame(frame4);

  auto frame5 = new ID3v2::TextIdentificationFrame("TMCL");
  StringList tmclData;
  tmclData.append("VIOLIN");
  tmclData.append("a violinist");
  tmclData.append("PIANO");
  tmclData.append("a pianist");
  frame5->setText(tmclData);
  tag.addFrame(frame5);

  auto frame6 = new ID3v2::UniqueFileIdentifierFrame("http://musicbrainz.org", "152454b9-19ba-49f3-9fc9-8fc26545cf41");
  tag.addFrame(frame6);

  auto frame7 = new ID3v2::UniqueFileIdentifierFrame("http://example.com", "123");
  tag.addFrame(frame7);

  auto frame8 = new ID3v2::UserTextIdentificationFrame();
  frame8->setDescription("MusicBrainz Album Id");
  frame8->setText("95c454a5-d7e0-4d8f-9900-db04aca98ab3");
  tag.addFrame(frame8);

  auto frame9 = new ID3v2::UnknownFrame("WXYZ");
  tag.addFrame(frame9);

  auto frame10 = new ID3v2::CommentsFrame();
  frame10->setDescription("iTunNORM");
  frame10->setText("00002C3B");
  frame10->setLanguage("eng");
  tag.addFrame(frame10);

  PropertyMap properties = tag.properties();

  ASSERT_EQ(4u, properties.unsupportedData().size());
  ASSERT_TRUE(properties.unsupportedData().contains("TIPL"));
  ASSERT_TRUE(properties.unsupportedData().contains("APIC"));
  ASSERT_TRUE(properties.unsupportedData().contains("UFID/http://example.com"));
  ASSERT_TRUE(properties.unsupportedData().contains("UNKNOWN/WXYZ"));

  ASSERT_TRUE(properties.contains("PERFORMER:VIOLIN"));
  ASSERT_TRUE(properties.contains("PERFORMER:PIANO"));
  ASSERT_EQ(String("a violinist"), properties["PERFORMER:VIOLIN"].front());
  ASSERT_EQ(String("a pianist"), properties["PERFORMER:PIANO"].front());

  ASSERT_TRUE(properties.contains("LYRICS"));
  ASSERT_TRUE(properties.contains("LYRICS:TEST"));

  ASSERT_TRUE(properties.contains("MUSICBRAINZ_TRACKID"));
  ASSERT_EQ(String("152454b9-19ba-49f3-9fc9-8fc26545cf41"), properties["MUSICBRAINZ_TRACKID"].front());

  ASSERT_TRUE(properties.contains("MUSICBRAINZ_ALBUMID"));
  ASSERT_EQ(String("95c454a5-d7e0-4d8f-9900-db04aca98ab3"), properties["MUSICBRAINZ_ALBUMID"].front());

  ASSERT_TRUE(properties.contains("COMMENT:ITUNNORM"));
  ASSERT_EQ(String("00002C3B"), properties["COMMENT:ITUNNORM"].front());

  tag.removeUnsupportedProperties(properties.unsupportedData());
  ASSERT_TRUE(tag.frameList("APIC").isEmpty());
  ASSERT_TRUE(tag.frameList("TIPL").isEmpty());
  ASSERT_TRUE(tag.frameList("WXYZ").isEmpty());
  ASSERT_EQ(static_cast<ID3v2::UniqueFileIdentifierFrame *>(nullptr), ID3v2::UniqueFileIdentifierFrame::findByOwner(&tag, "http://example.com"));
  ASSERT_EQ(frame1, ID3v2::UnsynchronizedLyricsFrame::findByDescription(&tag, "test"));
  ASSERT_EQ(frame6, ID3v2::UniqueFileIdentifierFrame::findByOwner(&tag, "http://musicbrainz.org"));
  ASSERT_EQ(frame8, ID3v2::UserTextIdentificationFrame::find(&tag, "MusicBrainz Album Id"));
  ASSERT_EQ(static_cast<ID3v2::UserTextIdentificationFrame *>(nullptr), ID3v2::UserTextIdentificationFrame::find(&tag, "non existing"));
  ASSERT_EQ(frame10, ID3v2::CommentsFrame::findByDescription(&tag, "iTunNORM"));
  ASSERT_EQ(static_cast<ID3v2::CommentsFrame *>(nullptr), ID3v2::CommentsFrame::findByDescription(&tag, "non existing"));
}

TEST(ID3v2, testPropertiesMovement)
{
  ID3v2::Tag tag;
  auto frameMvnm = new ID3v2::TextIdentificationFrame("MVNM");
  frameMvnm->setText("Movement Name");
  tag.addFrame(frameMvnm);

  auto frameMvin = new ID3v2::TextIdentificationFrame("MVIN");
  frameMvin->setText("2/3");
  tag.addFrame(frameMvin);

  PropertyMap properties = tag.properties();
  ASSERT_TRUE(properties.contains("MOVEMENTNAME"));
  ASSERT_TRUE(properties.contains("MOVEMENTNUMBER"));
  ASSERT_EQ(String("Movement Name"), properties["MOVEMENTNAME"].front());
  ASSERT_EQ(String("2/3"), properties["MOVEMENTNUMBER"].front());

  ByteVector frameDataMvnm("MVNM"
                           "\x00\x00\x00\x0e"
                           "\x00\x00"
                           "\x00"
                           "Movement Name",
                           24);
  ASSERT_EQ(frameDataMvnm, frameMvnm->render());
  ByteVector frameDataMvin("MVIN"
                           "\x00\x00\x00\x04"
                           "\x00\x00"
                           "\x00"
                           "2/3",
                           14);
  ASSERT_EQ(frameDataMvin, frameMvin->render());

  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ID3v2::Header header;
  auto parsedFrameMvnm = dynamic_cast<ID3v2::TextIdentificationFrame *>(
    factory->createFrame(frameDataMvnm, &header));
  auto parsedFrameMvin = dynamic_cast<ID3v2::TextIdentificationFrame *>(
    factory->createFrame(frameDataMvin, &header));
  ASSERT_TRUE(parsedFrameMvnm);
  ASSERT_TRUE(parsedFrameMvin);
  ASSERT_EQ(String("Movement Name"), parsedFrameMvnm->toString());
  ASSERT_EQ(String("2/3"), parsedFrameMvin->toString());

  tag.addFrame(parsedFrameMvnm);
  tag.addFrame(parsedFrameMvin);
}

TEST(ID3v2, testPropertyGrouping)
{
  ID3v2::Tag tag;
  auto frameGrp1 = new ID3v2::TextIdentificationFrame("GRP1");
  frameGrp1->setText("Grouping");
  tag.addFrame(frameGrp1);

  PropertyMap properties = tag.properties();
  ASSERT_TRUE(properties.contains("GROUPING"));
  ASSERT_EQ(String("Grouping"), properties["GROUPING"].front());

  ByteVector frameDataGrp1("GRP1"
                           "\x00\x00\x00\x09"
                           "\x00\x00"
                           "\x00"
                           "Grouping",
                           19);
  ASSERT_EQ(frameDataGrp1, frameGrp1->render());

  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  ID3v2::Header header;
  auto parsedFrameGrp1 = dynamic_cast<ID3v2::TextIdentificationFrame *>(
    factory->createFrame(frameDataGrp1, &header));
  ASSERT_TRUE(parsedFrameGrp1);
  ASSERT_EQ(String("Grouping"), parsedFrameGrp1->toString());

  tag.addFrame(parsedFrameGrp1);
}

TEST(ID3v2, testDeleteFrame)
{
  ScopedFileCopy copy("rare_frames", ".mp3");
  string newname = copy.fileName();

  {
    MPEG::File f(newname.c_str());
    ID3v2::Tag *t       = f.ID3v2Tag();
    ID3v2::Frame *frame = t->frameList("TCON")[0];
    ASSERT_EQ(1u, t->frameList("TCON").size());
    t->removeFrame(frame, true);
    f.save(MPEG::File::ID3v2);
  }
  {
    MPEG::File f2(newname.c_str());
    ID3v2::Tag *t = f2.ID3v2Tag();
    ASSERT_TRUE(t->frameList("TCON").isEmpty());
  }
}

TEST(ID3v2, testSaveAndStripID3v1ShouldNotAddFrameFromID3v1ToId3v2)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  {
    MPEG::File foo(newname.c_str());
    foo.tag()->setArtist("Artist");
    foo.save(MPEG::File::ID3v1 | MPEG::File::ID3v2);
  }

  {
    MPEG::File bar(newname.c_str());
    bar.ID3v2Tag()->removeFrames("TPE1");
    // Should strip ID3v1 here and not add old values to ID3v2 again
    bar.save(MPEG::File::ID3v2, File::StripOthers);
  }

  MPEG::File f(newname.c_str());
  ASSERT_FALSE(f.ID3v2Tag()->frameListMap().contains("TPE1"));
}

TEST(ID3v2, testParseChapterFrame)
{
  ID3v2::Header header;

  ByteVector chapterData       = ByteVector("CHAP"             // Frame ID
                                            "\x00\x00\x00\x20" // Frame size
                                            "\x00\x00"         // Frame flags
                                            "\x43\x00"         // Element ID ("C")
                                            "\x00\x00\x00\x03" // Start time
                                            "\x00\x00\x00\x05" // End time
                                            "\x00\x00\x00\x02" // Start offset
                                            "\x00\x00\x00\x03",
                                            28);               // End offset
  ByteVector embeddedFrameData = ByteVector("TIT2"             // Embedded frame ID
                                            "\x00\x00\x00\x04" // Embedded frame size
                                            "\x00\x00"         // Embedded frame flags
                                            "\x00"             // TIT2 frame text encoding
                                            "CH1",
                                            14);               // Chapter title

  ID3v2::ChapterFrame f1(&header, chapterData);

  ASSERT_EQ(ByteVector("C"), f1.elementID());
  ASSERT_EQ(static_cast<unsigned int>(0x03), f1.startTime());
  ASSERT_EQ(static_cast<unsigned int>(0x05), f1.endTime());
  ASSERT_EQ(static_cast<unsigned int>(0x02), f1.startOffset());
  ASSERT_EQ(static_cast<unsigned int>(0x03), f1.endOffset());
  ASSERT_EQ(static_cast<unsigned int>(0x00), f1.embeddedFrameList().size());

  ID3v2::ChapterFrame f2(&header, chapterData + embeddedFrameData);

  ASSERT_EQ(ByteVector("C"), f2.elementID());
  ASSERT_EQ(static_cast<unsigned int>(0x03), f2.startTime());
  ASSERT_EQ(static_cast<unsigned int>(0x05), f2.endTime());
  ASSERT_EQ(static_cast<unsigned int>(0x02), f2.startOffset());
  ASSERT_EQ(static_cast<unsigned int>(0x03), f2.endOffset());
  ASSERT_EQ(static_cast<unsigned int>(0x01), f2.embeddedFrameList().size());
  ASSERT_EQ(1u, f2.embeddedFrameList("TIT2").size());
  ASSERT_EQ(String("CH1"), f2.embeddedFrameList("TIT2")[0]->toString());
}

TEST(ID3v2, testRenderChapterFrame)
{
  ID3v2::Header header;
  ID3v2::ChapterFrame f1(&header, "CHAP");
  f1.setElementID(ByteVector("\x43\x00", 2));
  f1.setStartTime(3);
  f1.setEndTime(5);
  f1.setStartOffset(2);
  f1.setEndOffset(3);
  auto eF = new ID3v2::TextIdentificationFrame("TIT2");
  eF->setText("CH1");
  f1.addEmbeddedFrame(eF);

  ByteVector expected = ByteVector("CHAP"             // Frame ID
                                   "\x00\x00\x00\x20" // Frame size
                                   "\x00\x00"         // Frame flags
                                   "\x43\x00"         // Element ID
                                   "\x00\x00\x00\x03" // Start time
                                   "\x00\x00\x00\x05" // End time
                                   "\x00\x00\x00\x02" // Start offset
                                   "\x00\x00\x00\x03" // End offset
                                   "TIT2"             // Embedded frame ID
                                   "\x00\x00\x00\x04" // Embedded frame size
                                   "\x00\x00"         // Embedded frame flags
                                   "\x00"             // TIT2 frame text encoding
                                   "CH1",
                                   42);               // Chapter title

  ASSERT_EQ(expected, f1.render());

  f1.setElementID("C");

  ASSERT_EQ(expected, f1.render());

  ID3v2::FrameList frames;
  eF = new ID3v2::TextIdentificationFrame("TIT2");
  eF->setText("CH1");
  frames.append(eF);

  ID3v2::ChapterFrame f2(ByteVector("\x43\x00", 2), 3, 5, 2, 3, frames);
  ASSERT_EQ(expected, f2.render());

  frames.clear();
  eF = new ID3v2::TextIdentificationFrame("TIT2");
  eF->setText("CH1");
  frames.append(eF);

  ID3v2::ChapterFrame f3(ByteVector("C\x00", 2), 3, 5, 2, 3, frames);
  ASSERT_EQ(expected, f3.render());

  frames.clear();
  eF = new ID3v2::TextIdentificationFrame("TIT2");
  eF->setText("CH1");
  frames.append(eF);

  ID3v2::ChapterFrame f4("C", 3, 5, 2, 3, frames);
  ASSERT_EQ(expected, f4.render());

  ASSERT_FALSE(f4.toString().isEmpty());

  ID3v2::ChapterFrame f5("C", 3, 5, 2, 3);
  eF = new ID3v2::TextIdentificationFrame("TIT2");
  eF->setText("CH1");
  f5.addEmbeddedFrame(eF);
  ASSERT_EQ(expected, f5.render());
}

TEST(ID3v2, testParseTableOfContentsFrame)
{
  ID3v2::Header header;
  ID3v2::TableOfContentsFrame f(
    &header,
    ByteVector("CTOC"             // Frame ID
               "\x00\x00\x00\x16" // Frame size
               "\x00\x00"         // Frame flags
               "\x54\x00"         // Element ID ("T")
               "\x01"             // CTOC flags
               "\x02"             // Entry count
               "\x43\x00"         // First entry ("C")
               "\x44\x00"         // Second entry ("D")
               "TIT2"             // Embedded frame ID
               "\x00\x00\x00\x04" // Embedded frame size
               "\x00\x00"         // Embedded frame flags
               "\x00"             // TIT2 frame text encoding
               "TC1",
               32));              // Table of contents title
  ASSERT_EQ(ByteVector("T"), f.elementID());
  ASSERT_FALSE(f.isTopLevel());
  ASSERT_TRUE(f.isOrdered());
  ASSERT_TRUE(static_cast<unsigned int>(0x02) == f.entryCount());
  ASSERT_EQ(ByteVector("C"), f.childElements()[0]);
  ASSERT_EQ(ByteVector("D"), f.childElements()[1]);
  ASSERT_TRUE(static_cast<unsigned int>(0x01) == f.embeddedFrameList().size());
  ASSERT_TRUE(f.embeddedFrameList("TIT2").size() == 1);
  ASSERT_TRUE(f.embeddedFrameList("TIT2")[0]->toString() == "TC1");

  f.removeChildElement("E"); // not existing
  ASSERT_EQ(2U, f.entryCount());
  f.removeChildElement("C");
  ASSERT_EQ(1U, f.entryCount());
  ASSERT_EQ(ByteVector("D"), f.childElements()[0]);

  ID3v2::Frame *frame = f.embeddedFrameList("TIT2")[0];
  f.removeEmbeddedFrame(frame);
  ASSERT_TRUE(f.embeddedFrameList("TIT2").isEmpty());
}

TEST(ID3v2, testRenderTableOfContentsFrame)
{
  ID3v2::Header header;
  ID3v2::TableOfContentsFrame f(&header, "CTOC");
  f.setElementID(ByteVector("\x54\x00", 2));
  f.setIsTopLevel(false);
  f.setIsOrdered(true);
  f.addChildElement(ByteVector("\x43\x00", 2));
  f.addChildElement(ByteVector("\x44\x00", 2));
  auto eF = new ID3v2::TextIdentificationFrame("TIT2");
  eF->setText("TC1");
  f.addEmbeddedFrame(eF);
  ASSERT_EQ(
    ByteVector("CTOC"             // Frame ID
               "\x00\x00\x00\x16" // Frame size
               "\x00\x00"         // Frame flags
               "\x54\x00"         // Element ID
               "\x01"             // CTOC flags
               "\x02"             // Entry count
               "\x43\x00"         // First entry
               "\x44\x00"         // Second entry
               "TIT2"             // Embedded frame ID
               "\x00\x00\x00\x04" // Embedded frame size
               "\x00\x00"         // Embedded frame flags
               "\x00"             // TIT2 frame text encoding
               "TC1",
               32),               // Table of contents title
    f.render());
}

TEST(ID3v2, testShrinkPadding)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  {
    MPEG::File f(newname.c_str());
    f.ID3v2Tag()->setTitle(longText(64 * 1024));
    f.save(MPEG::File::ID3v2, File::StripOthers);
  }
  {
    MPEG::File f(newname.c_str());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(static_cast<offset_t>(74789), f.length());
    f.ID3v2Tag()->setTitle("ABCDEFGHIJ");
    f.save(MPEG::File::ID3v2, File::StripOthers);
  }
  {
    MPEG::File f(newname.c_str());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(static_cast<offset_t>(9263), f.length());
  }
}

TEST(ID3v2, testEmptyFrame)
{
  ScopedFileCopy copy("xing", ".mp3");
  string newname = copy.fileName();

  {
    MPEG::File f(newname.c_str());
    ID3v2::Tag *tag = f.ID3v2Tag(true);

    auto frame1     = new ID3v2::UrlLinkFrame(
      ByteVector("WOAF\x00\x00\x00\x01\x00\x00\x00", 11));
    tag->addFrame(frame1);

    auto frame2 = new ID3v2::TextIdentificationFrame("TIT2");
    frame2->setText("Title");
    tag->addFrame(frame2);

    f.save();
  }

  {
    MPEG::File f(newname.c_str());
    ASSERT_EQ(true, f.hasID3v2Tag());

    ID3v2::Tag *tag = f.ID3v2Tag();
    ASSERT_EQ(String("Title"), tag->title());
    ASSERT_EQ(true, tag->frameListMap()["WOAF"].isEmpty());
  }
}

TEST(ID3v2, testDuplicateTags)
{
  ScopedFileCopy copy("duplicate_id3v2", ".mp3");

  ByteVector audioStream;
  {
    MPEG::File f(copy.fileName().c_str());
    f.seek(f.ID3v2Tag()->header()->completeTagSize());
    audioStream = f.readBlock(2089);

    // duplicate_id3v2.mp3 has duplicate ID3v2 tags.
    // Sample rate will be 32000 if we can't skip the second tag.

    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(static_cast<unsigned int>(8049), f.ID3v2Tag()->header()->completeTagSize());

    ASSERT_EQ(44100, f.audioProperties()->sampleRate());

    f.ID3v2Tag()->setArtist("Artist A");
    f.save(MPEG::File::ID3v2, File::StripOthers);
  }
  {
    MPEG::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasID3v2Tag());
    ASSERT_EQ(static_cast<offset_t>(3594), f.length());
    ASSERT_EQ(static_cast<unsigned int>(1505), f.ID3v2Tag()->header()->completeTagSize());
    ASSERT_EQ(String("Artist A"), f.ID3v2Tag()->artist());
    ASSERT_EQ(44100, f.audioProperties()->sampleRate());

    f.seek(f.ID3v2Tag()->header()->completeTagSize());
    ASSERT_EQ(f.readBlock(2089), audioStream);
  }
}

TEST(ID3v2, testParseTOCFrameWithManyChildren)
{
  MPEG::File f(TEST_FILE_PATH_C("toc_many_children.mp3"));
  ASSERT_TRUE(f.isValid());

  ID3v2::Tag *tag = f.ID3v2Tag();
  ASSERT_EQ(130U, tag->frameList().size());
  int i = 0;
  for(const auto &frame : std::as_const(tag->frameList())) {
    if(i > 0) {
      ASSERT_EQ(ByteVector("CHAP"), frame->frameID());
      auto chapFrame = dynamic_cast<const ID3v2::ChapterFrame *>(frame);
      ASSERT_EQ(ByteVector("chapter") + ByteVector(String::number(i - 1).toCString()),
                chapFrame->elementID());
      ASSERT_EQ(static_cast<unsigned int>(100 * i),
                chapFrame->startTime());
      ASSERT_EQ(static_cast<unsigned int>(100 * i),
                chapFrame->endTime());
      const ID3v2::FrameList &embeddedFrames = chapFrame->embeddedFrameList();
      ASSERT_EQ(1U, embeddedFrames.size());
      auto tit2Frame = dynamic_cast<const ID3v2::TextIdentificationFrame *>(
        embeddedFrames.front());
      ASSERT_TRUE(tit2Frame);
      ASSERT_EQ(String("Marker ") + String::number(i),
                tit2Frame->fieldList().front());
    }
    else {
      ASSERT_EQ(ByteVector("CTOC"), frame->frameID());
      auto ctocFrame = dynamic_cast<const ID3v2::TableOfContentsFrame *>(frame);
      ASSERT_EQ(ByteVector("toc"), ctocFrame->elementID());
      ASSERT_FALSE(ctocFrame->isTopLevel());
      ASSERT_FALSE(ctocFrame->isOrdered());
      ASSERT_EQ(129U, ctocFrame->entryCount());
      const ID3v2::FrameList &embeddedFrames = ctocFrame->embeddedFrameList();
      ASSERT_EQ(1U, embeddedFrames.size());
      auto tit2Frame = dynamic_cast<const ID3v2::TextIdentificationFrame *>(
        embeddedFrames.front());
      ASSERT_TRUE(tit2Frame);
      ASSERT_EQ(StringList("toplevel toc"), tit2Frame->fieldList());
    }
    ++i;
  }

  ASSERT_FALSE(ID3v2::ChapterFrame::findByElementID(tag, "chap2"));
  ASSERT_TRUE(ID3v2::ChapterFrame::findByElementID(tag, "chapter2"));

  ASSERT_FALSE(ID3v2::TableOfContentsFrame::findTopLevel(tag));
  ASSERT_FALSE(ID3v2::TableOfContentsFrame::findByElementID(tag, "ctoc"));
  ASSERT_TRUE(ID3v2::TableOfContentsFrame::findByElementID(tag, "toc"));

  auto tocFrame = ID3v2::TableOfContentsFrame::findByElementID(tag, "toc");
  ASSERT_EQ(1U, tocFrame->embeddedFrameList().size());
  tocFrame->removeEmbeddedFrames("TIT2");
  ASSERT_TRUE(tocFrame->embeddedFrameList().isEmpty());
}
