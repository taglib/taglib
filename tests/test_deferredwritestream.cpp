/***************************************************************************
    copyright           : (C) 2026 by Ryan Francesconi
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

#include <algorithm>
#include <random>
#include <string>

#include "tbytevectorstream.h"
#include "tdeferredwritestream.h"
#include "tfilestream.h"
#include "mp4file.h"
#include "mp4tag.h"
#include "mp4chapter.h"
#include "plainfile.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

namespace
{
  // Patterned rather than constant, so a moved range never reads back as unchanged.
  ByteVector pattern(unsigned int size)
  {
    ByteVector data(size);
    unsigned int state = 0x13579BDF;
    for(unsigned int i = 0; i < size; ++i) {
      state = state * 1664525 + 1013904223;
      data[i] = static_cast<char>(state >> 24);
    }
    return data;
  }

  ByteVector wholeContent(IOStream &stream)
  {
    stream.seek(0);
    return stream.readBlock(static_cast<size_t>(stream.length()));
  }

  class CountingStream : public ByteVectorStream
  {
  public:
    using ByteVectorStream::ByteVectorStream;
    long long written { 0 };

    void writeBlock(const ByteVector &data) override
    {
      written += data.size();
      ByteVectorStream::writeBlock(data);
    }
  };

  MP4::ChapterList chapters()
  {
    MP4::ChapterList list;
    for(int i = 0; i < 3; ++i)
      list.append(MP4::Chapter(String("Chapter " + to_string(i + 1)), i * 100LL));
    return list;
  }

  void editMP4(MP4::File &f)
  {
    f.tag()->setTitle("A title long enough to grow the moov atom");
    f.setNeroChapters(chapters());
    f.setQtChapters(chapters());
  }
} // namespace

class TestDeferredWriteStream : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestDeferredWriteStream);
  CPPUNIT_TEST(testRandomEditsMatchByteVectorStream);
  CPPUNIT_TEST(testCommitWritesOnlyWhatMoved);
  CPPUNIT_TEST(testGrowthMovesTheRestOnce);
  CPPUNIT_TEST(testBytesToMove);
  CPPUNIT_TEST(testReadOnlyCommitRefused);
  CPPUNIT_TEST(testMP4SaveMatchesFileStream);
  CPPUNIT_TEST_SUITE_END();

public:

  // Replays seeded random writes, insertions, removals and truncations against a
  // DeferredWriteStream and a ByteVectorStream, comparing their content after each
  // operation, then compares writeTo() and commit() with the expected bytes.
  void testRandomEditsMatchByteVectorStream()
  {
    for(unsigned int seed = 0; seed < 200; ++seed) {
      ByteVectorStream underlying(pattern(20000));
      ByteVectorStream reference(pattern(20000));
      DeferredWriteStream deferred(&underlying);
      mt19937 random(seed);

      auto below = [&random](long long bound) {
        return bound <= 0 ? 0LL : static_cast<long long>(random() % static_cast<unsigned long long>(bound));
      };
      auto bytes = [&random](long long count) {
        ByteVector data(static_cast<unsigned int>(count));
        for(unsigned int i = 0; i < data.size(); ++i)
          data[i] = static_cast<char>(random());
        return data;
      };

      for(int operation = 0; operation < 24; ++operation) {
        const long long length = reference.length();

        // Weighted so most of the original content survives to be moved by the commit.
        const unsigned int kind = random() % 20;
        if(kind < 6) {
          const long long at = below(length + 16);
          const ByteVector data = bytes(1 + below(300));
          reference.seek(at);
          reference.writeBlock(data);
          deferred.seek(at);
          deferred.writeBlock(data);
        }
        else if(kind < 12) {
          const long long at = below(length + 1);
          const auto replace = static_cast<size_t>(below(min(length - at, 64LL) + 1));
          const ByteVector data = bytes(below(2000));
          reference.insert(data, at, replace);
          deferred.insert(data, at, replace);
        }
        else if(kind < 17) {
          const long long at = below(length + 1);
          const auto count = static_cast<size_t>(below(min(length - at, 2000LL) + 1));
          reference.removeBlock(at, count);
          deferred.removeBlock(at, count);
        }
        else if(kind < 18) {
          const long long to = max(0LL, length - 64 + below(128));
          reference.truncate(to);
          deferred.truncate(to);
        }
        else {
          const long long at = below(length + 1);
          const auto count = static_cast<size_t>(below(500));
          reference.seek(at);
          deferred.seek(at);
          CPPUNIT_ASSERT_EQUAL(reference.readBlock(count), deferred.readBlock(count));
          CPPUNIT_ASSERT_EQUAL(reference.tell(), deferred.tell());
        }

        CPPUNIT_ASSERT_EQUAL(reference.length(), deferred.length());
        CPPUNIT_ASSERT_EQUAL(wholeContent(reference), wholeContent(deferred));
      }

      ByteVectorStream copy(pattern(1));
      CPPUNIT_ASSERT(deferred.writeTo(&copy));
      CPPUNIT_ASSERT_EQUAL(*reference.data(), *copy.data());

      CPPUNIT_ASSERT(deferred.commit());
      CPPUNIT_ASSERT_EQUAL(*reference.data(), *underlying.data());
      CPPUNIT_ASSERT_EQUAL(*reference.data(), wholeContent(deferred));
    }
  }

  // Growing a block near the start and shrinking one further on moves only the bytes
  // between them; the rest of the stream keeps its place and is not written.
  void testCommitWritesOnlyWhatMoved()
  {
    CountingStream underlying(pattern(1 << 20));
    DeferredWriteStream deferred(&underlying);
    deferred.insert(ByteVector(16, 'x'), 100, 0);
    deferred.removeBlock(4116, 16);

    CPPUNIT_ASSERT(deferred.commit());
    CPPUNIT_ASSERT_EQUAL(16LL + 4000LL, underlying.written);
  }

  void testGrowthMovesTheRestOnce()
  {
    CountingStream underlying(pattern(1 << 20));
    DeferredWriteStream deferred(&underlying);
    deferred.insert(ByteVector(16, 'x'), 100, 0);

    CPPUNIT_ASSERT(deferred.commit());
    CPPUNIT_ASSERT_EQUAL(16LL + (1LL << 20) - 100LL, underlying.written);
  }

  // Overwriting and appending leave every original byte where it was; inserting or
  // removing at 100 moves the bytes after the edit.
  void testBytesToMove()
  {
    const ByteVector data(16, 'x');
    {
      ByteVectorStream underlying(pattern(1000));
      DeferredWriteStream deferred(&underlying);
      CPPUNIT_ASSERT(!deferred.hasChanges());
      deferred.seek(100);
      deferred.writeBlock(data);
      CPPUNIT_ASSERT(deferred.hasChanges());
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0), deferred.bytesToMove());
    }
    {
      ByteVectorStream underlying(pattern(1000));
      DeferredWriteStream deferred(&underlying);
      deferred.seek(0, IOStream::End);
      deferred.writeBlock(data);
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(0), deferred.bytesToMove());
    }
    {
      ByteVectorStream underlying(pattern(1000));
      DeferredWriteStream deferred(&underlying);
      deferred.insert(data, 100, 0);
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(900), deferred.bytesToMove());
    }
    {
      ByteVectorStream underlying(pattern(1000));
      DeferredWriteStream deferred(&underlying);
      deferred.removeBlock(100, 16);
      CPPUNIT_ASSERT_EQUAL(static_cast<offset_t>(884), deferred.bytesToMove());
    }
  }

  void testReadOnlyCommitRefused()
  {
    const ByteVector before = PlainFile(TEST_FILE_PATH_C("blank_video.m4v")).readAll();

    FileStream file(TEST_FILE_PATH_C("blank_video.m4v"), true);
    DeferredWriteStream deferred(&file);
    deferred.insert(ByteVector("x", 1), 0, 0);

    CPPUNIT_ASSERT(!deferred.commit());
    CPPUNIT_ASSERT_EQUAL(before, PlainFile(TEST_FILE_PATH_C("blank_video.m4v")).readAll());
  }

  // A save of a title and both chapter lists on a file with moov ahead of mdat comes
  // out byte-identical through the deferring stream.
  void testMP4SaveMatchesFileStream()
  {
    const ByteVector original = PlainFile(TEST_FILE_PATH_C("blank_video.m4v")).readAll();

    ByteVectorStream plain(original);
    {
      MP4::File f(&plain);
      CPPUNIT_ASSERT(f.isValid());
      editMP4(f);
      CPPUNIT_ASSERT(f.save());
    }

    ScopedFileCopy copy("blank_video", ".m4v");
    offset_t moved = 0;
    {
      FileStream file(copy.fileName().c_str());
      DeferredWriteStream deferred(&file);
      MP4::File f(&deferred);
      CPPUNIT_ASSERT(f.isValid());
      editMP4(f);
      CPPUNIT_ASSERT(f.save());
      moved = deferred.bytesToMove();
      CPPUNIT_ASSERT(deferred.commit());
    }

    CPPUNIT_ASSERT_EQUAL(*plain.data(), PlainFile(copy.fileName().c_str()).readAll());

    // The grown moov displaces all of mdat (and the part of moov after the edit).
    const offset_t mdatSize = static_cast<offset_t>(original.size()) - (original.find("mdat") - 4);
    CPPUNIT_ASSERT(moved >= mdatSize);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDeferredWriteStream);
