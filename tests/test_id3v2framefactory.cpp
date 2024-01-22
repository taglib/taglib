/***************************************************************************
    copyright            : (C) 2023 by Urs Fleisch
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

#include <functional>
#include <memory>

#include "flacproperties.h"
#include "mpegproperties.h"
#include "tbytevector.h"
#include "tpropertymap.h"
#include "mpegfile.h"
#include "flacfile.h"
#include "trueaudiofile.h"
#include "trueaudioproperties.h"
#include "wavfile.h"
#include "aifffile.h"
#include "dsffile.h"
#include "dsdifffile.h"
#include "id3v2tag.h"
#include "id3v2frame.h"
#include "id3v2framefactory.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

namespace
{

  class CustomFrameFactory;

  // Just a silly example of a custom frame holding a number.
  class CustomFrame : public ID3v2::Frame
  {
    friend class CustomFrameFactory;
  public:
    explicit CustomFrame(unsigned int value = 0)
      : Frame("CUST"), m_value(value) {}
    CustomFrame(const CustomFrame &) = delete;
    CustomFrame &operator=(const CustomFrame &) = delete;
    ~CustomFrame() override = default;
    String toString() const override { return String::number(m_value); }
    PropertyMap asProperties() const override {
      return SimplePropertyMap{{"CUSTOM", StringList(String::number(m_value))}};
    }
    unsigned int value() const { return m_value; }

  protected:
    void parseFields(const ByteVector &data) override {
      m_value = data.toUInt();
    }
    ByteVector renderFields() const override {
      return ByteVector::fromUInt(m_value);
    }

  private:
    CustomFrame(const ByteVector &data, Header *h)
      : Frame(h), m_value(fieldData(data).toUInt()) {}
    unsigned int m_value;
  };

  // Example for frame factory with support for CustomFrame.
  class CustomFrameFactory : public ID3v2::FrameFactory {
  public:
    CustomFrameFactory(const CustomFrameFactory &) = delete;
    CustomFrameFactory &operator=(const CustomFrameFactory &) = delete;
    static CustomFrameFactory *instance() { return &factory; }
    ID3v2::Frame *createFrameForProperty(
        const String &key, const StringList &values) const override {
      if(key == "CUSTOM") {
        return new CustomFrame(!values.isEmpty() ? values.front().toInt() : 0);
      }
      return ID3v2::FrameFactory::createFrameForProperty(key, values);
    }

  protected:
    CustomFrameFactory() = default;
    ~CustomFrameFactory() = default;
    ID3v2::Frame *createFrame(const ByteVector &data, ID3v2::Frame::Header *header,
                        const ID3v2::Header *tagHeader) const override {
      if(header->frameID() == "CUST") {
        return new CustomFrame(data, header);
      }
      return ID3v2::FrameFactory::createFrame(data, header, tagHeader);
    }

  private:
    static CustomFrameFactory factory;
  };

  CustomFrameFactory CustomFrameFactory::factory;
}  // namespace

class TestId3v2FrameFactory : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestId3v2FrameFactory);
  CPPUNIT_TEST(testMPEG);
  CPPUNIT_TEST(testFLAC);
  CPPUNIT_TEST(testTrueAudio);
  CPPUNIT_TEST(testWAV);
  CPPUNIT_TEST(testAIFF);
  CPPUNIT_TEST(testDSF);
  CPPUNIT_TEST(testDSDIFF);
  CPPUNIT_TEST_SUITE_END();

public:

  void testGenericFrameFactory(
    const char *fileName,
    function<File *(const char *)> createFileWithDefaultFactory,
    function<File *(const char *, ID3v2::FrameFactory *factory)> createFileWithFactory,
    function<bool(const File &)> hasID3v2Tag,
    function<ID3v2::Tag *(File &)> getID3v2Tag,
    function<bool(File &)> stripAllTags)
  {
    {
      auto f = std::unique_ptr<File>(createFileWithDefaultFactory(fileName));
      CPPUNIT_ASSERT(f->isValid());
      ID3v2::Tag *tag = getID3v2Tag(*f);
      ID3v2::FrameList frames = tag->frameList();
      for(auto it = frames.begin(); it != frames.end(); it = frames.erase(it)) {
        tag->removeFrame(*it);
      }
      tag->setArtist("An artist");
      tag->setTitle("A title");
      f->save();
    }
    {
      auto f = std::unique_ptr<File>(createFileWithDefaultFactory(fileName));
      CPPUNIT_ASSERT(f->isValid());
      CPPUNIT_ASSERT(hasID3v2Tag(*f));
      ID3v2::Tag *tag = getID3v2Tag(*f);
      tag->addFrame(new CustomFrame(1234567890));
      f->save();
    }
    {
      auto f = std::unique_ptr<File>(createFileWithDefaultFactory(fileName));
      CPPUNIT_ASSERT(f->isValid());
      CPPUNIT_ASSERT(hasID3v2Tag(*f));
      ID3v2::Tag *tag = getID3v2Tag(*f);
      const auto &frames = tag->frameList("CUST");
      CPPUNIT_ASSERT(!frames.isEmpty());
      // Without a specialized FrameFactory, you can add custom frames,
      // but your cannot parse them.
      CPPUNIT_ASSERT(!dynamic_cast<CustomFrame *>(frames.front()));
    }
    {
      auto f = std::unique_ptr<File>(createFileWithFactory(fileName, CustomFrameFactory::instance()));
      CPPUNIT_ASSERT(f->isValid());
      CPPUNIT_ASSERT(hasID3v2Tag(*f));
      ID3v2::Tag *tag = getID3v2Tag(*f);
      const auto &frames = tag->frameList("CUST");
      CPPUNIT_ASSERT(!frames.isEmpty());
      auto frame = dynamic_cast<CustomFrame *>(frames.front());
      CPPUNIT_ASSERT(frame);
      CPPUNIT_ASSERT_EQUAL(1234567890U, frame->value());
      PropertyMap properties = tag->properties();
      CPPUNIT_ASSERT_EQUAL(StringList("1234567890"),
        properties.value("CUSTOM"));
      CPPUNIT_ASSERT_EQUAL(StringList("An artist"),
        properties.value("ARTIST"));
      CPPUNIT_ASSERT_EQUAL(StringList("A title"),
        properties.value("TITLE"));
      stripAllTags(*f);
    }
    {
      auto f = std::unique_ptr<File>(createFileWithFactory(fileName, CustomFrameFactory::instance()));
      CPPUNIT_ASSERT(f->isValid());
      CPPUNIT_ASSERT(!hasID3v2Tag(*f));
      ID3v2::Tag *tag = getID3v2Tag(*f);
      PropertyMap properties = tag->properties();
      CPPUNIT_ASSERT(properties.isEmpty());
      properties.insert("CUSTOM", StringList("305419896"));
      tag->setProperties(properties);
      f->save();
    }
    {
      auto f = std::unique_ptr<File>(createFileWithFactory(fileName, CustomFrameFactory::instance()));
      CPPUNIT_ASSERT(f->isValid());
      CPPUNIT_ASSERT(hasID3v2Tag(*f));
      ID3v2::Tag *tag = getID3v2Tag(*f);
      PropertyMap properties = tag->properties();
      CPPUNIT_ASSERT_EQUAL(StringList("305419896"), properties.value("CUSTOM"));
      const auto &frames = tag->frameList("CUST");
      CPPUNIT_ASSERT(!frames.isEmpty());
      auto frame = dynamic_cast<CustomFrame *>(frames.front());
      CPPUNIT_ASSERT(frame);
      CPPUNIT_ASSERT_EQUAL(0x12345678U, frame->value());
    }
  }

  void testMPEG()
  {
    ScopedFileCopy copy("lame_cbr", ".mp3");
    testGenericFrameFactory(
      copy.fileName().c_str(),
      [](const char *fileName) {
        return new MPEG::File(fileName);
      },
      [](const char *fileName, ID3v2::FrameFactory *factory) {
        return new MPEG::File(fileName, true, MPEG::Properties::Average,
          factory);
      },
      [](const File &f) {
        return dynamic_cast<const MPEG::File &>(f).hasID3v2Tag();
      },
      [](File &f) {
        return dynamic_cast<MPEG::File &>(f).ID3v2Tag(true);
      },
      [](File &f) {
        return dynamic_cast<MPEG::File &>(f).strip();
      }
    );
  }

  void testFLAC()
  {
    ScopedFileCopy copy("no-tags", ".flac");
    testGenericFrameFactory(
      copy.fileName().c_str(),
      [](const char *fileName) {
        return new FLAC::File(fileName);
      },
      [](const char *fileName, ID3v2::FrameFactory *factory) {
        return new FLAC::File(fileName, true, FLAC::Properties::Average,
          factory);
      },
      [](const File &f) {
        return dynamic_cast<const FLAC::File &>(f).hasID3v2Tag();
      },
      [](File &f) {
        return dynamic_cast<FLAC::File &>(f).ID3v2Tag(true);
      },
      [](File &f) {
        dynamic_cast<FLAC::File &>(f).strip();
        return f.save();
      }
    );
  }

  void testTrueAudio()
  {
    ScopedFileCopy copy("empty", ".tta");
    testGenericFrameFactory(
      copy.fileName().c_str(),
      [](const char *fileName) {
        return new TrueAudio::File(fileName);
      },
      [](const char *fileName, ID3v2::FrameFactory *factory) {
        return new TrueAudio::File(fileName, true,
          TrueAudio::Properties::Average, factory);
      },
      [](const File &f) {
        return dynamic_cast<const TrueAudio::File &>(f).hasID3v2Tag();
      },
      [](File &f) {
        return dynamic_cast<TrueAudio::File &>(f).ID3v2Tag(true);
      },
      [](File &f) {
        dynamic_cast<TrueAudio::File &>(f).strip();
        return f.save();
      }
    );
  }

  void testWAV()
  {
    ScopedFileCopy copy("empty", ".wav");
    testGenericFrameFactory(
      copy.fileName().c_str(),
      [](const char *fileName) {
          return new RIFF::WAV::File(fileName);
      },
      [](const char *fileName, ID3v2::FrameFactory *factory) {
          return new RIFF::WAV::File(
              fileName, true, RIFF::WAV::Properties::Average, factory);
      },
      [](const File &f) {
          return dynamic_cast<const RIFF::WAV::File &>(f).hasID3v2Tag();
      },
      [](File &f) {
          return dynamic_cast<RIFF::WAV::File &>(f).ID3v2Tag();
      },
      [](File &f) {
        dynamic_cast<RIFF::WAV::File &>(f).strip();
        return true;
      }
    );
  }

  void testAIFF()
  {
    ScopedFileCopy copy("empty", ".aiff");
    testGenericFrameFactory(
      copy.fileName().c_str(),
      [](const char *fileName) {
        return new RIFF::AIFF::File(fileName);
      },
      [](const char *fileName, ID3v2::FrameFactory *factory) {
        return new RIFF::AIFF::File(
              fileName, true, RIFF::AIFF::Properties::Average, factory);
      },
      [](const File &f) {
        return dynamic_cast<const RIFF::AIFF::File &>(f).hasID3v2Tag();
      },
      [](File &f) {
        return dynamic_cast<RIFF::AIFF::File &>(f).tag();
      },
      [](File &f) {
          f.setProperties({});
          return f.save();
      }
    );
  }

  void testDSF()
  {
    ScopedFileCopy copy("empty10ms", ".dsf");
    testGenericFrameFactory(
      copy.fileName().c_str(),
      [](const char *fileName) {
        return new DSF::File(fileName);
      },
      [](const char *fileName, ID3v2::FrameFactory *factory) {
        return new DSF::File(
              fileName, true, DSF::Properties::Average, factory);
      },
      [](const File &f) {
        return !f.tag()->isEmpty();
      },
      [](File &f) {
        return dynamic_cast<DSF::File &>(f).tag();
      },
      [](File &f) {
        f.setProperties({});
        return f.save();
      }
    );
  }

  void testDSDIFF()
  {
    ScopedFileCopy copy("empty10ms", ".dff");
    testGenericFrameFactory(
      copy.fileName().c_str(),
      [](const char *fileName) {
        return new DSDIFF::File(fileName);
      },
      [](const char *fileName, ID3v2::FrameFactory *factory) {
        return new DSDIFF::File(
              fileName, true, DSDIFF::Properties::Average, factory);
      },
      [](const File &f) {
        return dynamic_cast<const DSDIFF::File &>(f).hasID3v2Tag();
      },
      [](File &f) {
        return dynamic_cast<DSDIFF::File &>(f).ID3v2Tag(true);
      },
      [](File &f) {
        dynamic_cast<DSDIFF::File &>(f).strip();
        return true;
      }
    );
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestId3v2FrameFactory);
