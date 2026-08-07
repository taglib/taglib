/***************************************************************************
    copyright           : (C) 2026 by Frederik Seiffert
    email               : frederik@algoriddim.com
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

#include <atomic>
#include <functional>
#include <stdexcept>
#include <thread>
#include <vector>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "taglib_config.h"

#include "id3v2framefactory.h"
#include "id3v2synchdata.h"
#include "id3v2tag.h"
#include "mpegfile.h"
#include "tbytevector.h"
#include "tbytevectorstream.h"
#include "tstringlist.h"
#include "textidentificationframe.h"
#include "tpropertymap.h"

#ifdef TAGLIB_WITH_ASF
#include "asftag.h"
#endif
#ifdef TAGLIB_WITH_MP4
#include "mp4file.h"
#include "mp4tag.h"
#endif
#ifdef TAGLIB_WITH_RIFF
#include "infotag.h"
#endif

#include "plainfile.h"
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestThreadSafety : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestThreadSafety);
  CPPUNIT_TEST(testConcurrentLazyInitialization);
  CPPUNIT_TEST_SUITE_END();

public:

  static constexpr int threadCount = 8;

  static int runConcurrently(const std::function<void()> &work)
  {
    constexpr int iterations = 25;

    std::atomic<int> ready { 0 };
    std::atomic<bool> go { false };
    std::atomic<int> succeeded { 0 };

    std::vector<std::thread> threads;
    threads.reserve(threadCount);
    for(int i = 0; i < threadCount; ++i) {
      threads.emplace_back([&] {
        ++ready;
        while(!go.load(std::memory_order_acquire)) {
          std::this_thread::yield();
        }
        try {
          for(int j = 0; j < iterations; ++j) {
            work();
          }
          ++succeeded;
        }
        catch(const std::exception &) {
          // Counted as a failure by the caller.
        }
      });
    }

    while(ready.load() < threadCount) {
      std::this_thread::yield();
    }
    go.store(true, std::memory_order_release);

    for(auto &thread : threads) {
      thread.join();
    }

    return succeeded.load();
  }

  static ByteVector id3v23TagWithIpls()
  {
    // Involvement/involvee pairs. "Producer" and "Engineer" are supported by
    // the TIPL property map, "Guitar" and "Drums" are not, so
    // rebuildAggregateFrames() moves the latter pairs into a new TMCL frame.
    // Assembled rather than written as one escaped literal: in "\x00Artist",
    // the escape would swallow the 'A' as a third hex digit.
    const StringList involvements {
      "Guitar", "Artist 1",
      "Drums", "Artist 2",
      "Producer", "Artist 3",
      "Engineer", "Artist 4"
    };

    ByteVector fields(1, '\0');   // Latin-1 text encoding
    for(unsigned int i = 0; i < involvements.size(); ++i) {
      if(i > 0) {
        fields.append('\0');      // NUL separated, no trailing NUL
      }
      fields.append(involvements[i].data(String::Latin1));
    }

    const ByteVector frame = ByteVector("IPLS")
      + ByteVector::fromUInt(fields.size())
      + ByteVector(2, '\0')       // frame flags
      + fields;

    return ByteVector("ID3") + ByteVector(1, '\x03') + ByteVector(2, '\0')
      + ID3v2::SynchData::fromUInt(frame.size())
      + frame;
  }

  void testConcurrentLazyInitialization()
  {
    const ByteVector id3v23Data = id3v23TagWithIpls();

    CPPUNIT_ASSERT_EQUAL(threadCount, runConcurrently([] {
      if(ID3v2::TextIdentificationFrame::involvedPeopleMap().size() != 5) {
        throw std::runtime_error("unexpected involvedPeopleMap() contents");
      }
    }));

    CPPUNIT_ASSERT_EQUAL(threadCount, runConcurrently([&id3v23Data] {
      ByteVectorStream stream(id3v23Data);
      MPEG::File file(&stream, false);
      if(!file.hasID3v2Tag() || file.ID3v2Tag()->frameList("TMCL").size() != 1) {
        throw std::runtime_error("IPLS was not migrated to TMCL");
      }
    }));

#ifdef TAGLIB_WITH_MP4
    const ByteVector mp4Data = PlainFile(TEST_FILE_PATH_C("has-tags.m4a")).readAll();
    CPPUNIT_ASSERT(!mp4Data.isEmpty());

    CPPUNIT_ASSERT_EQUAL(threadCount, runConcurrently([&mp4Data] {
      ByteVectorStream stream(mp4Data);
      MP4::File file(&stream);
      if(!file.isValid() || file.tag()->properties().isEmpty()) {
        throw std::runtime_error("MP4 properties were not read");
      }
    }));
#endif

#ifdef TAGLIB_WITH_RIFF
    CPPUNIT_ASSERT_EQUAL(threadCount, runConcurrently([] {
      PropertyMap properties;
      properties["TITLE"] = StringList("Title");
      properties["ARTIST"] = StringList("Artist");
      RIFF::Info::Tag tag;
      if(!tag.setProperties(properties).isEmpty()) {
        throw std::runtime_error("unexpected unsupported RIFF Info properties");
      }
    }));
#endif

#ifdef TAGLIB_WITH_ASF
    CPPUNIT_ASSERT_EQUAL(threadCount, runConcurrently([] {
      PropertyMap properties;
      properties["TITLE"] = StringList("Title");
      properties["ARTIST"] = StringList("Artist");
      ASF::Tag tag;
      if(!tag.setProperties(properties).isEmpty()) {
        throw std::runtime_error("unexpected unsupported ASF properties");
      }
    }));
#endif

    ByteVectorStream stream(id3v23Data);
    MPEG::File file(&stream, false);
    CPPUNIT_ASSERT(file.hasID3v2Tag());
    CPPUNIT_ASSERT_EQUAL(1u, file.ID3v2Tag()->frameList("TMCL").size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestThreadSafety);
