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

#include <catch/catch.hpp>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <attachedpictureframe.h>
#include <chapterframe.h>
#include <eventtimingcodesframe.h>
#include <generalencapsulatedobjectframe.h>
#include <ownershipframe.h>
#include <popularimeterframe.h>
#include <relativevolumeframe.h>
#include <synchronizedlyricsframe.h>
#include <tableofcontentsframe.h>
#include <textidentificationframe.h>
#include <uniquefileidentifierframe.h>
#include <unknownframe.h>
#include <unsynchronizedlyricsframe.h>
#include <urllinkframe.h>
#include <tpropertymap.h>
#include <tzlib.h>
#include "utils.h"

using namespace TagLib;

namespace
{
  class PublicFrame : public ID3v2::Frame
  {
    public:
      PublicFrame() : ID3v2::Frame(ByteVector("XXXX\0\0\0\0\0\0", 10)) {}
      String readStringField(const ByteVector &data, String::Type encoding,
                             int *positon = 0)
        { return ID3v2::Frame::readStringField(data, encoding, positon); }
      virtual String toString() const { return String(); }
      virtual void parseFields(const ByteVector &) {}
      virtual ByteVector renderFields() const { return ByteVector(); }
  };

  ID3v2::FrameFactory *factory()
  {
    return ID3v2::FrameFactory::instance();
  }
}

TEST_CASE("ID3v2 tag")
{
  SECTION("Downgrade UTF-8 strings when saving ID3v2.3 (1)")
  {
    const ScopedFileCopy copy("xing", ".mp3");

    ID3v2::TextIdentificationFrame *f
      = new ID3v2::TextIdentificationFrame(ByteVector("TPE1"), String::UTF8);
    StringList sl;
    sl.append("Foo");
    f->setText(sl);

    MPEG::File file(copy.fileName().c_str());
    file.ID3v2Tag(true)->addFrame(f);
    file.save(MPEG::File::ID3v2, true, 3);
    REQUIRE(file.hasID3v2Tag());

    const ByteVector data = f->render();
    REQUIRE(data.size() == 4 + 4 + 2 + 1 + 6 + 2);

    ID3v2::TextIdentificationFrame f2(data);
    REQUIRE(f2.fieldList() == sl);
    REQUIRE(f2.textEncoding() == String::UTF16);
  }
  SECTION("Downgrade UTF-8 strings when saving ID3v2.3 (2)")
  {
    const ScopedFileCopy copy("xing", ".mp3");

    ID3v2::UnsynchronizedLyricsFrame *f
      = new ID3v2::UnsynchronizedLyricsFrame(String::UTF8);
    f->setText("Foo");

    MPEG::File file(copy.fileName().c_str());
    file.ID3v2Tag(true)->addFrame(f);
    file.save(MPEG::File::ID3v2, true, 3);
    REQUIRE(file.hasID3v2Tag());

    const ByteVector data = f->render();
    REQUIRE(data.size() == 4 + 4 + 2 + 1 + 3 + 2 + 2 + 6 + 2);

    ID3v2::UnsynchronizedLyricsFrame f2(data);
    REQUIRE(f2.text() == "Foo");
    REQUIRE(f2.textEncoding() == String::UTF16);
  }
  SECTION("Deimiter in UTF-16BE")
  {
    ID3v2::TextIdentificationFrame f(ByteVector("TPE1"), String::UTF16BE);
    StringList sl;
    sl.append("Foo");
    sl.append("Bar");
    f.setText(sl);
    REQUIRE(f.render().size() == 4 + 4 + 2 + 1 + 6 + 2 + 6);
  }
  SECTION("Deimiter in UTF-16")
  {
    ID3v2::TextIdentificationFrame f(ByteVector("TPE1"), String::UTF16);
    StringList sl;
    sl.append("Foo");
    sl.append("Bar");
    f.setText(sl);
    REQUIRE(f.render().size() == 4 + 4 + 2 + 1 + 8 + 2 + 8);
  }
  SECTION("Decode unsynchronized frame")
  {
    MPEG::File f(TEST_FILE_PATH_C("unsynch.id3"), false);
    REQUIRE(f.tag());
    REQUIRE(f.tag()->title() == "My babe just cares for me");
  }
  SECTION("Safely ignore broken frame")
  {
    MPEG::File f(TEST_FILE_PATH_C("broken-tenc.id3"), false);
    REQUIRE(f.tag());
    REQUIRE_FALSE(f.ID3v2Tag()->frameListMap().contains("TENC"));
  }
  SECTION("Read string field")
  {
    PublicFrame f;
    ByteVector data("abc\0", 4);
    String str = f.readStringField(data, String::Latin1);
    REQUIRE(str == "abc");
  }
  SECTION("Don't render v2.2 frames")
  {
    const ByteVector data("FOO"
                          "\x00\x00\x08"
                          "\x00"
                          "JPG"
                          "\x01"
                          "d\x00"
                          "\x00", 14);
    ID3v2::UnknownFrame *frame =
        dynamic_cast<TagLib::ID3v2::UnknownFrame*>(factory()->createFrame(data, 2U));
    REQUIRE(frame);

    ID3v2::Tag tag;
    tag.addFrame(frame);
    REQUIRE(tag.render().size() == 1034);
  }
  SECTION("iTunes 2.4 frame size")
  {
    MPEG::File f(TEST_FILE_PATH_C("005411.id3"), false);
    REQUIRE(f.tag());
    REQUIRE(f.ID3v2Tag()->frameListMap().contains("TIT2"));
    REQUIRE(f.ID3v2Tag()->frameListMap()["TIT2"].front()->toString() == "Sunshine Superman");
  }
  SECTION("Save UTF-16 comment")
  {
    const String::Type defaultEncoding = factory()->defaultTextEncoding();
    const ScopedFileCopy copy("xing", ".mp3");

    factory()->setDefaultTextEncoding(String::UTF16);
    {
      MPEG::File foo(copy.fileName().c_str());
      foo.strip();
      foo.tag()->setComment("Test comment!");
      foo.save();
    }
    {
      MPEG::File bar(copy.fileName().c_str());
      REQUIRE(bar.tag()->comment() == "Test comment!");
    }
    factory()->setDefaultTextEncoding(defaultEncoding);
  }
  SECTION("Update genre in v2.3 (1)")
  {
    // "Refinement" is the same as the ID3v1 genre - duplicate
    const ByteVector data("TCON"                    // Frame ID
                          "\x00\x00\x00\x10"        // Frame size
                          "\x00\x00"                // Frame flags
                          "\x00"                    // Encoding
                          "(22)Death Metal", 26);   // Text
    ID3v2::TextIdentificationFrame *frame =
        dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(factory()->createFrame(data, 3U));
    REQUIRE(frame->fieldList().size() == 1);
    REQUIRE(frame->fieldList()[0] == "Death Metal");

    ID3v2::Tag tag;
    tag.addFrame(frame);
    REQUIRE(tag.genre() == "Death Metal");
  }
  SECTION("Update genre in v2.3 (2)")
  {
    // "Refinement" is different from the ID3v1 genre
    const ByteVector data("TCON"                    // Frame ID
                          "\x00\x00\x00\x13"        // Frame size
                          "\x00\x00"                // Frame flags
                          "\x00"                    // Encoding
                          "(4)Eurodisco", 23);      // Text
    ID3v2::TextIdentificationFrame *frame =
        dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(factory()->createFrame(data, 3U));
    REQUIRE(frame->fieldList().size() == 2);
    REQUIRE(frame->fieldList()[0] == "4");
    REQUIRE(frame->fieldList()[1] == "Eurodisco");

    ID3v2::Tag tag;
    tag.addFrame(frame);
    REQUIRE(tag.genre() == "Disco Eurodisco");
  }
  SECTION("Update genre in v2.4")
  {
    const ByteVector data("TCON"                    // Frame ID
                          "\x00\x00\x00\x0D"        // Frame size
                          "\x00\x00"                // Frame flags
                          "\0"                      // Encoding
                          "14\0Eurodisco", 23);     // Text
    ID3v2::TextIdentificationFrame *frame =
        dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(factory()->createFrame(data, 4U));
    REQUIRE(frame->fieldList().size() == 2);
    REQUIRE(frame->fieldList()[0] == "14");
    REQUIRE(frame->fieldList()[1] == "Eurodisco");

    ID3v2::Tag tag;
    tag.addFrame(frame);
    REQUIRE(tag.genre() == "R&B Eurodisco");
  }
  SECTION("Update date in v2.2")
  {
    MPEG::File f(TEST_FILE_PATH_C("id3v22-tda.mp3"), false);
    REQUIRE(f.tag());
    REQUIRE(f.tag()->year() == 2010);
  }
  // TODO: TYE + TDA should be upgraded to TDRC together
  //SECTION("Update full date in v2.2")
  //{
  //  MPEG::File f(TEST_FILE_PATH_C("id3v22-tda.mp3"), false);
  //  REQUIRE(f.tag());
  //  REQUIRE(f.ID3v2Tag()->frameListMap()["TDRC"].front()->toString() == String("2010-04-03"));
  //}
  SECTION("Downgrade frames to v2.3")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    ID3v2::TextIdentificationFrame *tf;
    {
      MPEG::File foo(copy.fileName().c_str());
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
      foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TDRL", String::Latin1));
      foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TDTG", String::Latin1));
      foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TMOO", String::Latin1));
      foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TPRO", String::Latin1));
      foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TSOA", String::Latin1));
      foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TSOT", String::Latin1));
      foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TSST", String::Latin1));
      foo.ID3v2Tag()->addFrame(new ID3v2::TextIdentificationFrame("TSOP", String::Latin1));
      foo.save(MPEG::File::AllTags, true, 3);
    }
    {
      MPEG::File bar(copy.fileName().c_str());
      tf = dynamic_cast<ID3v2::TextIdentificationFrame *>(bar.ID3v2Tag()->frameList("TDOR").front());
      REQUIRE(tf);
      REQUIRE(tf->fieldList().size() == 1);
      REQUIRE(tf->fieldList().front() == "2011");
      tf = dynamic_cast<ID3v2::TextIdentificationFrame *>(bar.ID3v2Tag()->frameList("TDRC").front());
      REQUIRE(tf);
      REQUIRE(tf->fieldList().size() == 1);
      REQUIRE(tf->fieldList().front() == "2012-04-17T12:01");
      tf = dynamic_cast<ID3v2::TextIdentificationFrame *>(bar.ID3v2Tag()->frameList("TIPL").front());
      REQUIRE(tf);
      REQUIRE(tf->fieldList().size() == 8);
      REQUIRE(tf->fieldList()[0] == "Guitar");
      REQUIRE(tf->fieldList()[1] == "Artist 1");
      REQUIRE(tf->fieldList()[2] == "Drums");
      REQUIRE(tf->fieldList()[3] == "Artist 2");
      REQUIRE(tf->fieldList()[4] == "Producer");
      REQUIRE(tf->fieldList()[5] == "Artist 3");
      REQUIRE(tf->fieldList()[6] == "Mastering");
      REQUIRE(tf->fieldList()[7] == "Artist 4");
      REQUIRE_FALSE(bar.ID3v2Tag()->frameListMap().contains("TDRL"));
      REQUIRE_FALSE(bar.ID3v2Tag()->frameListMap().contains("TDTG"));
      REQUIRE_FALSE(bar.ID3v2Tag()->frameListMap().contains("TMOO"));
      REQUIRE_FALSE(bar.ID3v2Tag()->frameListMap().contains("TPRO"));
#ifdef NO_ITUNES_HACKS
      REQUIRE_FALSE(bar.ID3v2Tag()->frameListMap().contains("TSOA"));
      REQUIRE_FALSE(bar.ID3v2Tag()->frameListMap().contains("TSOT"));
      REQUIRE_FALSE(bar.ID3v2Tag()->frameListMap().contains("TSOP"));
#endif
      REQUIRE_FALSE(bar.ID3v2Tag()->frameListMap().contains("TSST"));
    }
  }
  SECTION("Parse compressed frame with wrong length")
  {
    MPEG::File f(TEST_FILE_PATH_C("compressed_id3_frame.mp3"), false);
    REQUIRE(f.ID3v2Tag()->frameListMap().contains("APIC"));

    if(zlib::isAvailable()) {
      ID3v2::AttachedPictureFrame *frame
        = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(f.ID3v2Tag()->frameListMap()["APIC"].front());
      REQUIRE(frame);
      REQUIRE(frame->mimeType() == "image/bmp");
      REQUIRE(frame->type() == ID3v2::AttachedPictureFrame::Other);
      REQUIRE(frame->description().isEmpty());
      REQUIRE(frame->picture().size() == 86414);
    }
    else {
      // Skip the test if ZLIB is not installed.
      // The message "Compressed frames are currently not supported." will be displayed.

      ID3v2::UnknownFrame *frame
        = dynamic_cast<TagLib::ID3v2::UnknownFrame*>(f.ID3v2Tag()->frameListMap()["APIC"].front());
      REQUIRE(frame);
    }
  }
  SECTION("Read W000 frame from file")
  {
    MPEG::File f(TEST_FILE_PATH_C("w000.mp3"), false);
    REQUIRE(f.ID3v2Tag()->frameListMap().contains("W000"));
    ID3v2::UrlLinkFrame *frame =
      dynamic_cast<TagLib::ID3v2::UrlLinkFrame*>(f.ID3v2Tag()->frameListMap()["W000"].front());
    REQUIRE(frame);
    REQUIRE(frame->url() == "lukas.lalinsky@example.com____");
  }
  SECTION("Read property map (1)")
  {
    const ScopedFileCopy copy("rare_frames", ".mp3");

    MPEG::File f(copy.fileName().c_str());
    const PropertyMap dict = f.ID3v2Tag(false)->properties();
    REQUIRE(dict.size() == 6);

    REQUIRE(dict.contains("USERTEXTDESCRIPTION1"));
    REQUIRE(dict.contains("QuodLibet::USERTEXTDESCRIPTION2"));
    REQUIRE(dict["USERTEXTDESCRIPTION1"].size() == 2);
    REQUIRE(dict["QuodLibet::USERTEXTDESCRIPTION2"].size() == 2);
    REQUIRE(dict["USERTEXTDESCRIPTION1"][0] == "userTextData1");
    REQUIRE(dict["USERTEXTDESCRIPTION1"][1] == "userTextData2");
    REQUIRE(dict["QuodLibet::USERTEXTDESCRIPTION2"][0] == "userTextData1");
    REQUIRE(dict["QuodLibet::USERTEXTDESCRIPTION2"][1] == "userTextData2");

    REQUIRE(dict["GENRE"].front() == "Pop");

    REQUIRE(dict["URL:USERURL"].front() == "http://a.user.url");

    REQUIRE(dict["URL"].front() == "http://a.user.url/with/empty/description");
    REQUIRE(dict["COMMENT"].front() == "A COMMENT");

    REQUIRE(dict.unsupportedData().size() == 1);
    REQUIRE(dict.unsupportedData().front() == "UFID/supermihi@web.de");
  }
  SECTION("Read property map (2)")
  {
    ID3v2::Tag tag;
    ID3v2::UnsynchronizedLyricsFrame *frame1 = new ID3v2::UnsynchronizedLyricsFrame();
    frame1->setDescription("test");
    frame1->setText("la-la-la test");
    tag.addFrame(frame1);

    ID3v2::UnsynchronizedLyricsFrame *frame2 = new ID3v2::UnsynchronizedLyricsFrame();
    frame2->setDescription("");
    frame2->setText("la-la-la nodescription");
    tag.addFrame(frame2);

    ID3v2::AttachedPictureFrame *frame3 = new ID3v2::AttachedPictureFrame();
    frame3->setDescription("test picture");
    tag.addFrame(frame3);

    ID3v2::TextIdentificationFrame *frame4 = new ID3v2::TextIdentificationFrame("TIPL");
    frame4->setText("single value is invalid for TIPL");
    tag.addFrame(frame4);

    ID3v2::TextIdentificationFrame *frame5 = new ID3v2::TextIdentificationFrame("TMCL");
    StringList tmclData;
    tmclData.append("VIOLIN");
    tmclData.append("a violinist");
    tmclData.append("PIANO");
    tmclData.append("a pianist");
    frame5->setText(tmclData);
    tag.addFrame(frame5);

    ID3v2::UniqueFileIdentifierFrame *frame6 = new ID3v2::UniqueFileIdentifierFrame("http://musicbrainz.org", "152454b9-19ba-49f3-9fc9-8fc26545cf41");
    tag.addFrame(frame6);

    ID3v2::UniqueFileIdentifierFrame *frame7 = new ID3v2::UniqueFileIdentifierFrame("http://example.com", "123");
    tag.addFrame(frame7);

    ID3v2::UserTextIdentificationFrame *frame8 = new ID3v2::UserTextIdentificationFrame();
    frame8->setDescription("MusicBrainz Album Id");
    frame8->setText("95c454a5-d7e0-4d8f-9900-db04aca98ab3");
    tag.addFrame(frame8);

    const PropertyMap properties = tag.properties();

    REQUIRE(properties.unsupportedData().size() == 3);
    REQUIRE(properties.unsupportedData().contains("TIPL"));
    REQUIRE(properties.unsupportedData().contains("APIC"));
    REQUIRE(properties.unsupportedData().contains("UFID/http://example.com"));

    REQUIRE(properties.contains("PERFORMER:VIOLIN"));
    REQUIRE(properties.contains("PERFORMER:PIANO"));
    REQUIRE(properties["PERFORMER:VIOLIN"].front() == "a violinist");
    REQUIRE(properties["PERFORMER:PIANO"].front() == "a pianist");

    REQUIRE(properties.contains("LYRICS"));
    REQUIRE(properties.contains("LYRICS:TEST"));

    REQUIRE(properties.contains("MUSICBRAINZ_TRACKID"));
    REQUIRE(properties["MUSICBRAINZ_TRACKID"].front() == "152454b9-19ba-49f3-9fc9-8fc26545cf41");

    REQUIRE(properties.contains("MUSICBRAINZ_ALBUMID"));
    REQUIRE(properties["MUSICBRAINZ_ALBUMID"].front() == "95c454a5-d7e0-4d8f-9900-db04aca98ab3");

    tag.removeUnsupportedProperties(properties.unsupportedData());
    REQUIRE(tag.frameList("APIC").isEmpty());
    REQUIRE(tag.frameList("TIPL").isEmpty());
    REQUIRE(ID3v2::UniqueFileIdentifierFrame::findByOwner(&tag, "http://example.com") == (ID3v2::UniqueFileIdentifierFrame *)0);
    REQUIRE(ID3v2::UniqueFileIdentifierFrame::findByOwner(&tag, "http://musicbrainz.org") == frame6);
  }
  SECTION("Read property map (3)")
  {
    ID3v2::Tag tag;
    ID3v2::TextIdentificationFrame *frameMvnm = new ID3v2::TextIdentificationFrame("MVNM");
    frameMvnm->setText("Movement Name");
    tag.addFrame(frameMvnm);

    ID3v2::TextIdentificationFrame *frameMvin = new ID3v2::TextIdentificationFrame("MVIN");
    frameMvin->setText("2/3");
    tag.addFrame(frameMvin);

    const PropertyMap properties = tag.properties();
    REQUIRE(properties.contains("MOVEMENTNAME"));
    REQUIRE(properties.contains("MOVEMENTNUMBER"));
    REQUIRE(properties["MOVEMENTNAME"].front() == "Movement Name");
    REQUIRE(properties["MOVEMENTNUMBER"].front() == "2/3");

    const ByteVector frameDataMvnm("MVNM"
                                   "\x00\x00\x00\x0e"
                                   "\x00\x00"
                                   "\x00"
                                   "Movement Name", 24);
    REQUIRE(frameMvnm->render() == frameDataMvnm);
    const ByteVector frameDataMvin("MVIN"
                                   "\x00\x00\x00\x04"
                                   "\x00\x00"
                                   "\x00"
                                   "2/3", 14);
    REQUIRE(frameMvin->render() == frameDataMvin);

    ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
    ID3v2::TextIdentificationFrame *parsedFrameMvnm =
        dynamic_cast<ID3v2::TextIdentificationFrame *>(factory->createFrame(frameDataMvnm));
    ID3v2::TextIdentificationFrame *parsedFrameMvin =
        dynamic_cast<ID3v2::TextIdentificationFrame *>(factory->createFrame(frameDataMvin));
    REQUIRE(parsedFrameMvnm);
    REQUIRE(parsedFrameMvin);
    REQUIRE(parsedFrameMvnm->toString() == "Movement Name");
    REQUIRE(parsedFrameMvin->toString() == "2/3");

    tag.addFrame(parsedFrameMvnm);
    tag.addFrame(parsedFrameMvin);
  }
  SECTION("Remove frame")
  {
    const ScopedFileCopy copy("rare_frames", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      ID3v2::Tag *t = f.ID3v2Tag();
      ID3v2::Frame *frame = t->frameList("TCON")[0];
      REQUIRE(t->frameList("TCON").size() == 1);
      t->removeFrame(frame, true);
      f.save(MPEG::File::ID3v2);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.ID3v2Tag()->frameList("TCON").isEmpty());
    }
  }
  SECTION("Don't add removed frame to ID3v2 when stripping ID3v1")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File foo(copy.fileName().c_str());
      foo.tag()->setArtist("Artist");
      foo.save(MPEG::File::ID3v1 | MPEG::File::ID3v2);
    }
    {
      MPEG::File bar(copy.fileName().c_str());
      bar.ID3v2Tag()->removeFrames("TPE1");
      // Should strip ID3v1 here and not add old values to ID3v2 again
      bar.save(MPEG::File::ID3v2, true);
    }
    MPEG::File f(copy.fileName().c_str());
    REQUIRE_FALSE(f.ID3v2Tag()->frameListMap().contains("TPE1"));
  }
  SECTION("Shrink too large padding")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());
      f.ID3v2Tag()->setTitle(longText(64 * 1024));
      f.save(MPEG::File::ID3v2, true);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.length() == 74789);
      f.ID3v2Tag()->setTitle("ABCDEFGHIJ");
      f.save(MPEG::File::ID3v2, true);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.length() == 9263);
    }
  }
  SECTION("Don't save empty frame")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File f(copy.fileName().c_str());

      ID3v2::UrlLinkFrame *frame1 = new ID3v2::UrlLinkFrame(
        ByteVector("WOAF\x00\x00\x00\x01\x00\x00\x00", 11));
      f.ID3v2Tag(true)->addFrame(frame1);

      ID3v2::TextIdentificationFrame *frame2 = new ID3v2::TextIdentificationFrame("TIT2");
      frame2->setText("Title");
      f.ID3v2Tag(true)->addFrame(frame2);

      f.save();
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());

      ID3v2::Tag *tag = f.ID3v2Tag();
      REQUIRE(tag->title() == "Title");
      REQUIRE(tag->frameListMap()["WOAF"].isEmpty());
    }
  }
  SECTION("Safely skip and remove duplicate tags")
  {
    const ScopedFileCopy copy("duplicate_id3v2", ".mp3");

    ByteVector audioStream;
    {
      MPEG::File f(copy.fileName().c_str());
      f.seek(f.ID3v2Tag()->header()->completeTagSize());
      audioStream = f.readBlock(2089);

      // duplicate_id3v2.mp3 has duplicate ID3v2 tags.
      // Sample rate will be 32000 if we can't skip the second tag.

      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.ID3v2Tag()->header()->completeTagSize() == 8049);
      REQUIRE(f.audioProperties()->sampleRate() == 44100);

      f.ID3v2Tag()->setArtist("Artist A");
      f.save(MPEG::File::ID3v2, true);
    }
    {
      MPEG::File f(copy.fileName().c_str());
      REQUIRE(f.hasID3v2Tag());
      REQUIRE(f.length() == 3594);
      REQUIRE(f.ID3v2Tag()->header()->completeTagSize() == 1505);
      REQUIRE(f.ID3v2Tag()->artist() == "Artist A");
      REQUIRE(f.audioProperties()->sampleRate() == 44100);

      f.seek(f.ID3v2Tag()->header()->completeTagSize());
      REQUIRE(audioStream == f.readBlock(2089));

    }
  }
  SECTION("Parse attached picture frame")
  {
    // http://bugs.kde.org/show_bug.cgi?id=151078
    const ID3v2::AttachedPictureFrame f(ByteVector("APIC"
                                                   "\x00\x00\x00\x07"
                                                   "\x00\x00"
                                                   "\x00"
                                                   "m\x00"
                                                   "\x01"
                                                   "d\x00"
                                                   "\x00", 17));
    REQUIRE(f.mimeType() == "m");
    REQUIRE(f.type() == ID3v2::AttachedPictureFrame::FileIcon);
    REQUIRE(f.description() == "d");
  }
  SECTION("Parse attached picture frame with UTF-16 text")
  {
    ID3v2::AttachedPictureFrame f(ByteVector(
      "\x41\x50\x49\x43\x00\x02\x0c\x59\x00\x00\x01\x69\x6d\x61\x67\x65"
      "\x2f\x6a\x70\x65\x67\x00\x00\xfe\xff\x00\x63\x00\x6f\x00\x76\x00"
      "\x65\x00\x72\x00\x2e\x00\x6a\x00\x70\x00\x67\x00\x00\xff\xd8\xff",
      16 * 3));
    REQUIRE(f.mimeType() == "image/jpeg");
    REQUIRE(f.type() == ID3v2::AttachedPictureFrame::Other);
    REQUIRE(f.description() == "cover.jpg");
    REQUIRE(f.picture() == "\xff\xd8\xff");
  }
  SECTION("Parse attached picture frame v2.2")
  {
    const ByteVector data("PIC"
                          "\x00\x00\x08"
                          "\x00"
                          "JPG"
                          "\x01"
                          "d\x00"
                          "\x00", 14);
    ID3v2::AttachedPictureFrame *frame =
        dynamic_cast<ID3v2::AttachedPictureFrame*>(factory()->createFrame(data, 2U));

    REQUIRE(frame);
    REQUIRE(frame->mimeType() == "image/jpeg");
    REQUIRE(frame->type() == ID3v2::AttachedPictureFrame::FileIcon);
    REQUIRE(frame->description() == "d");

    delete frame;
  }
  SECTION("Parge general encapsulated object frame")
  {
    // http://bugs.kde.org/show_bug.cgi?id=151078
    const ID3v2::GeneralEncapsulatedObjectFrame f(ByteVector("GEOB"
                                                             "\x00\x00\x00\x08"
                                                             "\x00\x00"
                                                             "\x00"
                                                             "m\x00"
                                                             "f\x00"
                                                             "d\x00"
                                                             "\x00", 18));
    REQUIRE(f.mimeType() == "m");
    REQUIRE(f.fileName() == "f");
    REQUIRE(f.description() == "d");
  }
  SECTION("Parse popularimeter frame")
  {
    const ID3v2::PopularimeterFrame f(ByteVector("POPM"
                                                 "\x00\x00\x00\x17"
                                                 "\x00\x00"
                                                 "email@example.com\x00"
                                                 "\x02"
                                                 "\x00\x00\x00\x03", 33));
    REQUIRE(f.email() == "email@example.com");
    REQUIRE(f.rating() == 2);
    REQUIRE(f.counter() == 3);
  }
  SECTION("Parse popularimeter frame without counter")
  {
    const ID3v2::PopularimeterFrame f(ByteVector("POPM"
      "\x00\x00\x00\x13"
      "\x00\x00"
      "email@example.com\x00"
      "\x02", 29));
    REQUIRE(f.email() == "email@example.com");
    REQUIRE(f.rating() == 2);
    REQUIRE(f.counter() == 0);
  }
  SECTION("Render popularimeter frame")
  {
    ID3v2::PopularimeterFrame f;
    f.setEmail("email@example.com");
    f.setRating(2);
    f.setCounter(3);
    REQUIRE(f.render() == ByteVector("POPM"
                                     "\x00\x00\x00\x17"
                                     "\x00\x00"
                                     "email@example.com\x00"
                                     "\x02"
                                     "\x00\x00\x00\x03", 33));
  }
  SECTION("Convert popularimeter frame to string")
  {
    ID3v2::PopularimeterFrame f;
    f.setEmail("email@example.com");
    f.setRating(2);
    f.setCounter(3);
    REQUIRE(f.toString() == "email@example.com rating=2 counter=3");
  }
  SECTION("Read poularimeter frame from file")
  {
    const ScopedFileCopy copy("xing", ".mp3");
    {
      MPEG::File foo(copy.fileName().c_str());

      ID3v2::PopularimeterFrame *f = new ID3v2::PopularimeterFrame();
      f->setEmail("email@example.com");
      f->setRating(200);
      f->setCounter(3);

      foo.ID3v2Tag()->addFrame(f);
      foo.save();
    }
    {
      MPEG::File bar(copy.fileName().c_str());
      ID3v2::PopularimeterFrame *frame
        = dynamic_cast<ID3v2::PopularimeterFrame *>(bar.ID3v2Tag()->frameList("POPM").front());
      REQUIRE(frame->email() == "email@example.com");
      REQUIRE(frame->rating() == 200);
    }
  }
  SECTION("Parse relative volume frame")
  {
    // http://bugs.kde.org/show_bug.cgi?id=150481
    const ID3v2::RelativeVolumeFrame f(ByteVector("RVA2"              // Frame ID
                                                  "\x00\x00\x00\x0B"  // Frame size
                                                  "\x00\x00"          // Frame flags
                                                  "ident\x00"         // Identification
                                                  "\x02"              // Type of channel
                                                  "\x00\x0F"          // Volume adjustment
                                                  "\x08"              // Bits representing peak
                                                  "\x45", 21));       // Peak volume
    REQUIRE(f.identification() == String("ident"));
    REQUIRE(f.volumeAdjustment(ID3v2::RelativeVolumeFrame::FrontRight) == 15.0f / 512.0f);
    REQUIRE(f.peakVolume(ID3v2::RelativeVolumeFrame::FrontRight).bitsRepresentingPeak == 8);
    REQUIRE(f.peakVolume(ID3v2::RelativeVolumeFrame::FrontRight).peakVolume == "\x45");
  }
  SECTION("Parse unique file identifier frame")
  {
    const ID3v2::UniqueFileIdentifierFrame f(ByteVector("UFID"                  // Frame ID
                                                        "\x00\x00\x00\x09"      // Frame size
                                                        "\x00\x00"              // Frame flags
                                                        "owner\x00"             // Owner identifier
                                                        "\x00\x01\x02", 19));   // Identifier
    REQUIRE(f.owner() == "owner");
    REQUIRE(f.identifier() == ByteVector("\x00\x01\x02", 3));
  }
  SECTION("Parse empty unique file identifier frame")
  {
    const ID3v2::UniqueFileIdentifierFrame f(ByteVector("UFID"                  // Frame ID
                                                        "\x00\x00\x00\x01"      // Frame size
                                                        "\x00\x00"              // Frame flags
                                                        "\x00"                  // Owner identifier
                                                        "", 11));               // Identifier
    REQUIRE(f.owner().isEmpty());
    REQUIRE(f.identifier().isEmpty());
  }
  SECTION("Parse URL link frame")
  {
    const ID3v2::UrlLinkFrame f(ByteVector("WOAF"                      // Frame ID
                                           "\x00\x00\x00\x12"          // Frame size
                                           "\x00\x00"                  // Frame flags
                                           "http://example.com", 28)); // URL
    REQUIRE(f.url() == "http://example.com");
  }
  SECTION("Render URL link frame")
  {
    ID3v2::UrlLinkFrame f("WOAF");
    f.setUrl("http://example.com");
    REQUIRE(f.render() == ByteVector("WOAF"                       // Frame ID
                                     "\x00\x00\x00\x12"           // Frame size
                                     "\x00\x00"                   // Frame flags
                                     "http://example.com", 28));  // URL
  }
  SECTION("Parse user URL link frame")
  {
    const ID3v2::UserUrlLinkFrame f(ByteVector("WXXX"                       // Frame ID
                                               "\x00\x00\x00\x17"           // Frame size
                                               "\x00\x00"                   // Frame flags
                                               "\x00"                       // Text encoding
                                               "foo\x00"                    // Description
                                               "http://example.com", 33));  // URL
    REQUIRE(f.description() == "foo");
    REQUIRE(f.url() == "http://example.com");
  }
  SECTION("Render user URL link frame")
  {
    ID3v2::UserUrlLinkFrame f;
    f.setDescription("foo");
    f.setUrl("http://example.com");
    REQUIRE(f.render() == ByteVector("WXXX"                       // Frame ID
                                     "\x00\x00\x00\x17"           // Frame size
                                     "\x00\x00"                   // Frame flags
                                     "\x00"                       // Text encoding
                                     "foo\x00"                    // Description
                                     "http://example.com", 33));  // URL
  }
  SECTION("Parse ownership frame")
  {
    const ID3v2::OwnershipFrame f(ByteVector("OWNE"               // Frame ID
                                             "\x00\x00\x00\x19"   // Frame size
                                             "\x00\x00"           // Frame flags
                                             "\x00"               // Text encoding
                                             "GBP1.99\x00"        // Price paid
                                             "20120905"           // Date of purchase
                                             "Beatport", 35));    // Seller
    REQUIRE(f.pricePaid() == "GBP1.99");
    REQUIRE(f.datePurchased() == "20120905");
    REQUIRE(f.seller() == "Beatport");
  }
  SECTION("Render ownership frame")
  {
    ID3v2::OwnershipFrame f;
    f.setPricePaid("GBP1.99");
    f.setDatePurchased("20120905");
    f.setSeller("Beatport");
    REQUIRE(f.render() == ByteVector("OWNE"               // Frame ID
                                     "\x00\x00\x00\x19"   // Frame size
                                     "\x00\x00"           // Frame flags
                                     "\x00"               // Text encoding
                                     "GBP1.99\x00"        // Price paid
                                     "20120905"           // Date of purchase
                                     "Beatport", 35));    // URL
  }
  SECTION("Parse synchronized lyrics frame")
  {
    const ID3v2::SynchronizedLyricsFrame f(ByteVector("SYLT"                    // Frame ID
                                                      "\x00\x00\x00\x21"        // Frame size
                                                      "\x00\x00"                // Frame flags
                                                      "\x00"                    // Text encoding
                                                      "eng"                     // Language
                                                      "\x02"                    // Time stamp format
                                                      "\x01"                    // Content type
                                                      "foo\x00"                 // Content descriptor
                                                      "Example\x00"             // 1st text
                                                      "\x00\x00\x04\xd2"        // 1st time stamp
                                                      "Lyrics\x00"              // 2nd text
                                                      "\x00\x00\x11\xd7", 43)); // 2nd time stamp
    REQUIRE(f.textEncoding() == String::Latin1);
    REQUIRE(f.language() == "eng");
    REQUIRE(f.timestampFormat() == ID3v2::SynchronizedLyricsFrame::AbsoluteMilliseconds);
    REQUIRE(f.type() == ID3v2::SynchronizedLyricsFrame::Lyrics);
    REQUIRE(f.description() == "foo");
    const ID3v2::SynchronizedLyricsFrame::SynchedTextList stl = f.synchedText();
    REQUIRE(stl.size() == 2);
    REQUIRE(stl[0].text == "Example");
    REQUIRE(stl[0].time == 1234);
    REQUIRE(stl[1].text == "Lyrics");
    REQUIRE(stl[1].time == 4567);
  }
  SECTION("Parse synchronized lyrics frame with empty description")
  {
    const ID3v2::SynchronizedLyricsFrame f(ByteVector("SYLT"                    // Frame ID
                                                      "\x00\x00\x00\x21"        // Frame size
                                                      "\x00\x00"                // Frame flags
                                                      "\x00"                    // Text encoding
                                                      "eng"                     // Language
                                                      "\x02"                    // Time stamp format
                                                      "\x01"                    // Content type
                                                      "\x00"                    // Content descriptor
                                                      "Example\x00"             // 1st text
                                                      "\x00\x00\x04\xd2"        // 1st time stamp
                                                      "Lyrics\x00"              // 2nd text
                                                      "\x00\x00\x11\xd7", 40)); // 2nd time stamp
    REQUIRE(f.textEncoding() == String::Latin1);
    REQUIRE(f.language() == "eng");
    REQUIRE(f.timestampFormat() == ID3v2::SynchronizedLyricsFrame::AbsoluteMilliseconds);
    REQUIRE(f.type() == ID3v2::SynchronizedLyricsFrame::Lyrics);
    REQUIRE(f.description().isEmpty());
    const ID3v2::SynchronizedLyricsFrame::SynchedTextList stl = f.synchedText();
    REQUIRE(stl.size() == 2);
    REQUIRE(stl[0].text == "Example");
    REQUIRE(stl[0].time == 1234);
    REQUIRE(stl[1].text == "Lyrics");
    REQUIRE(stl[1].time == 4567);
  }
  SECTION("Render synchronized lyrics frame")
  {
    ID3v2::SynchronizedLyricsFrame f;
    f.setTextEncoding(String::Latin1);
    f.setLanguage("eng");
    f.setTimestampFormat(ID3v2::SynchronizedLyricsFrame::AbsoluteMilliseconds);
    f.setType(ID3v2::SynchronizedLyricsFrame::Lyrics);
    f.setDescription("foo");
    ID3v2::SynchronizedLyricsFrame::SynchedTextList stl;
    stl.append(ID3v2::SynchronizedLyricsFrame::SynchedText(1234, "Example"));
    stl.append(ID3v2::SynchronizedLyricsFrame::SynchedText(4567, "Lyrics"));
    f.setSynchedText(stl);
    REQUIRE(f.render() == ByteVector("SYLT"                     // Frame ID
                                     "\x00\x00\x00\x21"         // Frame size
                                     "\x00\x00"                 // Frame flags
                                     "\x00"                     // Text encoding
                                     "eng"                      // Language
                                     "\x02"                     // Time stamp format
                                     "\x01"                     // Content type
                                     "foo\x00"                  // Content descriptor
                                     "Example\x00"              // 1st text
                                     "\x00\x00\x04\xd2"         // 1st time stamp
                                     "Lyrics\x00"               // 2nd text
                                     "\x00\x00\x11\xd7", 43));  // 2nd time stamp
  }
  SECTION("Parse synchronized lyrics frame")
  {
    const ID3v2::EventTimingCodesFrame f(ByteVector("ETCO"                      // Frame ID
                                                    "\x00\x00\x00\x0b"          // Frame size
                                                    "\x00\x00"                  // Frame flags
                                                    "\x02"                      // Time stamp format
                                                    "\x02"                      // 1st event
                                                    "\x00\x00\xf3\x5c"          // 1st time stamp
                                                    "\xfe"                      // 2nd event
                                                    "\x00\x36\xee\x80", 21));   // 2nd time stamp
    REQUIRE(f.timestampFormat() == ID3v2::EventTimingCodesFrame::AbsoluteMilliseconds);
    const ID3v2::EventTimingCodesFrame::SynchedEventList sel = f.synchedEvents();
    REQUIRE(sel.size() == 2);
    REQUIRE(sel[0].type == ID3v2::EventTimingCodesFrame::IntroStart);
    REQUIRE(sel[0].time == 62300);
    REQUIRE(sel[1].type == ID3v2::EventTimingCodesFrame::AudioFileEnds);
    REQUIRE(sel[1].time == 3600000);
  }
  SECTION("Parse synchronized lyrics frame")
  {
    ID3v2::EventTimingCodesFrame f;
    f.setTimestampFormat(ID3v2::EventTimingCodesFrame::AbsoluteMilliseconds);
    ID3v2::EventTimingCodesFrame::SynchedEventList sel;
    sel.append(ID3v2::EventTimingCodesFrame::SynchedEvent(62300, ID3v2::EventTimingCodesFrame::IntroStart));
    sel.append(ID3v2::EventTimingCodesFrame::SynchedEvent(3600000, ID3v2::EventTimingCodesFrame::AudioFileEnds));
    f.setSynchedEvents(sel);
    REQUIRE(f.render() == ByteVector("ETCO"                     // Frame ID
                                     "\x00\x00\x00\x0b"         // Frame size
                                     "\x00\x00"                 // Frame flags
                                     "\x02"                     // Time stamp format
                                     "\x02"                     // 1st event
                                     "\x00\x00\xf3\x5c"         // 1st time stamp
                                     "\xfe"                     // 2nd event
                                     "\x00\x36\xee\x80", 21));  // 2nd time stamp
  }
  SECTION("Parse chapter frame")
  {
    const ID3v2::Header header;

    const ByteVector chapterData("CHAP"                     // Frame ID
                                 "\x00\x00\x00\x20"         // Frame size
                                 "\x00\x00"                 // Frame flags
                                 "\x43\x00"                 // Element ID ("C")
                                 "\x00\x00\x00\x03"         // Start time
                                 "\x00\x00\x00\x05"         // End time
                                 "\x00\x00\x00\x02"         // Start offset
                                 "\x00\x00\x00\x03", 28);   // End offset
    const ByteVector embeddedFrameData("TIT2"               // Embedded frame ID
                                       "\x00\x00\x00\x04"   // Embedded frame size
                                       "\x00\x00"           // Embedded frame flags
                                       "\x00"               // TIT2 frame text encoding
                                       "CH1", 14);          // Chapter title

    const ID3v2::ChapterFrame f1(&header, chapterData);

    REQUIRE(f1.elementID() == "C");
    REQUIRE(f1.startTime() == 3);
    REQUIRE(f1.endTime() == 5);
    REQUIRE(f1.startOffset() == 2);
    REQUIRE(f1.endOffset() == 3);
    REQUIRE(f1.embeddedFrameList().isEmpty());

    const ID3v2::ChapterFrame f2(&header, chapterData + embeddedFrameData);

    REQUIRE(f2.elementID() == "C");
    REQUIRE(f2.startTime() == 3);
    REQUIRE(f2.endTime() == 5);
    REQUIRE(f2.startOffset() == 2);
    REQUIRE(f2.endOffset() == 3);
    REQUIRE(f2.embeddedFrameList().size() == 1);
    REQUIRE(f2.embeddedFrameList("TIT2").size() == 1);
    REQUIRE(f2.embeddedFrameList("TIT2")[0]->toString() == "CH1");
  }
  SECTION("Render chapter frame")
  {
    const ID3v2::Header header;
    ID3v2::ChapterFrame f1(&header, "CHAP");
    f1.setElementID(ByteVector("\x43\x00", 2));
    f1.setStartTime(3);
    f1.setEndTime(5);
    f1.setStartOffset(2);
    f1.setEndOffset(3);
    ID3v2::TextIdentificationFrame *eF = new ID3v2::TextIdentificationFrame("TIT2");
    eF->setText("CH1");
    f1.addEmbeddedFrame(eF);

    const ByteVector expected("CHAP"                // Frame ID
                              "\x00\x00\x00\x20"    // Frame size
                              "\x00\x00"            // Frame flags
                              "\x43\x00"            // Element ID
                              "\x00\x00\x00\x03"    // Start time
                              "\x00\x00\x00\x05"    // End time
                              "\x00\x00\x00\x02"    // Start offset
                              "\x00\x00\x00\x03"    // End offset
                              "TIT2"                // Embedded frame ID
                              "\x00\x00\x00\x04"    // Embedded frame size
                              "\x00\x00"            // Embedded frame flags
                              "\x00"                // TIT2 frame text encoding
                              "CH1", 42);           // Chapter title

    REQUIRE(f1.render() == expected);

    f1.setElementID("C");

    REQUIRE(f1.render() == expected);

    ID3v2::FrameList frames;
    eF = new ID3v2::TextIdentificationFrame("TIT2");
    eF->setText("CH1");
    frames.append(eF);

    ID3v2::ChapterFrame f2(ByteVector("\x43\x00", 2), 3, 5, 2, 3, frames);
    REQUIRE(f2.render() == expected);

    frames.clear();
    eF = new ID3v2::TextIdentificationFrame("TIT2");
    eF->setText("CH1");
    frames.append(eF);

    ID3v2::ChapterFrame f3(ByteVector("C\x00", 2), 3, 5, 2, 3, frames);
    REQUIRE(f3.render() == expected);

    frames.clear();
    eF = new ID3v2::TextIdentificationFrame("TIT2");
    eF->setText("CH1");
    frames.append(eF);

    ID3v2::ChapterFrame f4("C", 3, 5, 2, 3, frames);
    REQUIRE(f4.render() == expected);

    REQUIRE_FALSE(f4.toString().isEmpty());

    ID3v2::ChapterFrame f5("C", 3, 5, 2, 3);
    eF = new ID3v2::TextIdentificationFrame("TIT2");
    eF->setText("CH1");
    f5.addEmbeddedFrame(eF);
    REQUIRE(f5.render() == expected);
  }
  SECTION("Parse table of contents frame")
  {
    const ID3v2::Header header;
    const ID3v2::TableOfContentsFrame f(
      &header,
      ByteVector("CTOC"               // Frame ID
                 "\x00\x00\x00\x16"   // Frame size
                 "\x00\x00"           // Frame flags
                 "\x54\x00"           // Element ID ("T")
                 "\x01"               // CTOC flags
                 "\x02"               // Entry count
                 "\x43\x00"           // First entry ("C")
                 "\x44\x00"           // Second entry ("D")
                 "TIT2"               // Embedded frame ID
                 "\x00\x00\x00\x04"   // Embedded frame size
                 "\x00\x00"           // Embedded frame flags
                 "\x00"               // TIT2 frame text encoding
                 "TC1", 32));         // Table of contents title
    REQUIRE(f.elementID() == "T");
    REQUIRE_FALSE(f.isTopLevel());
    REQUIRE(f.isOrdered());
    REQUIRE(f.entryCount() == 2);
    REQUIRE(f.childElements()[0] == "C");
    REQUIRE(f.childElements()[1] == "D");
    REQUIRE(f.embeddedFrameList().size() == 1);
    REQUIRE(f.embeddedFrameList("TIT2").size() == 1);
    REQUIRE(f.embeddedFrameList("TIT2")[0]->toString() == "TC1");
  }
  SECTION("Parse table of contents frame with many children")
  {
    MPEG::File f(TEST_FILE_PATH_C("toc_many_children.mp3"));
    REQUIRE(f.isValid());
    REQUIRE(f.ID3v2Tag()->frameListMap().contains("CTOC"));
    ID3v2::TableOfContentsFrame *frame
      = dynamic_cast<ID3v2::TableOfContentsFrame *>(f.ID3v2Tag()->frameListMap()["CTOC"].front());
    REQUIRE(frame->entryCount() == 129);
  }
  SECTION("Render table of contents frame")
  {
    ID3v2::Header header;
    ID3v2::TableOfContentsFrame f(&header, "CTOC");
    f.setElementID(ByteVector("\x54\x00", 2));
    f.setIsTopLevel(false);
    f.setIsOrdered(true);
    f.addChildElement(ByteVector("\x43\x00", 2));
    f.addChildElement(ByteVector("\x44\x00", 2));
    ID3v2::TextIdentificationFrame *eF = new ID3v2::TextIdentificationFrame("TIT2");
    eF->setText("TC1");
    f.addEmbeddedFrame(eF);
    REQUIRE(f.render() == ByteVector("CTOC"               // Frame ID
                                     "\x00\x00\x00\x16"   // Frame size
                                     "\x00\x00"           // Frame flags
                                     "\x54\x00"           // Element ID
                                     "\x01"               // CTOC flags
                                     "\x02"               // Entry count
                                     "\x43\x00"           // First entry
                                     "\x44\x00"           // Second entry
                                     "TIT2"               // Embedded frame ID
                                     "\x00\x00\x00\x04"   // Embedded frame size
                                     "\x00\x00"           // Embedded frame flags
                                     "\x00"               // TIT2 frame text encoding
                                     "TC1", 32));         // Table of contents title
  }
}
