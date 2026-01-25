/***************************************************************************
    copyright            : (C) 2026 by Antoine Colombier
    email                : antoine@mixxx.org
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

#include "tag.h"
#include "mp4file.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

namespace {
const String STEM_KEY("STEM");
}

class TestMP4Stem : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMP4Stem);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testUpdate);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST_SUITE_END();

protected:
  void createTestFile(ScopedFileCopy& copy){
    MP4::File f(copy.fileName().c_str());
    CPPUNIT_ASSERT(!f.hasMP4Tag());
    auto &tag = *f.tag();
    CPPUNIT_ASSERT_EQUAL(0U, tag.complexProperties(STEM_KEY).size());
    CPPUNIT_ASSERT(tag.setComplexProperties(STEM_KEY, List<VariantMap>({{{"manifest", ByteVector("{some text data}")}}})));
    f.save();
  }
public:

  void testCreate()
  {
    ScopedFileCopy copy("no-tags", ".m4a");
    createTestFile(copy);

    // Assert whether the newly created stem content is as expected
    {
      MP4::File f(copy.fileName().c_str());
      auto &tag = *f.tag();
      CPPUNIT_ASSERT(f.hasMP4Tag());
      auto stems = tag.complexProperties(STEM_KEY);
      CPPUNIT_ASSERT_EQUAL(1U, stems.size());
      CPPUNIT_ASSERT(stems.front().contains("manifest"));
      CPPUNIT_ASSERT_EQUAL(Variant(ByteVector("{some text data}")), stems.front().find("manifest")->second);
    }
  }

  void testUpdate()
  {
    ScopedFileCopy copy("no-tags", ".m4a");
    createTestFile(copy);

    // Prepare so large test data, to ensure that free padding is correctly used
    char *buffer = new char[1025]{'X'};
    buffer[1024] = '\0';
    String artist(buffer);
    std::memset(buffer, 'Y', 1024);
    String title(buffer);
    std::memset(buffer, 'Z', 1024);
    ByteVector newStem(buffer);
    delete [] buffer;

    // Update tags and stems to force atom offset recalculation
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.setComplexProperties("PICTURE", List<VariantMap>({
        {
          {"data", ByteVector("DummyData")},
          {"pictureType", "Front Cover"},
          {"mimeType", String("image/png")},
          {"description", String("Test")}
        }
      })));
      CPPUNIT_ASSERT(f.tag()->setComplexProperties(STEM_KEY, List<VariantMap>({{{"manifest", newStem}}})));
      f.tag()->setArtist(artist);
      f.tag()->setTitle(title);
      f.save();
    }
    // Reload the file to assert its tags
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.hasMP4Tag());

      auto stems = f.tag()->complexProperties(STEM_KEY);
      CPPUNIT_ASSERT_EQUAL(1U, stems.size());
      CPPUNIT_ASSERT(stems.front().contains("manifest"));
      CPPUNIT_ASSERT_EQUAL(Variant(newStem), stems.front().find("manifest")->second);

      auto pictures = f.tag()->complexProperties("PICTURE");
      CPPUNIT_ASSERT_EQUAL(1U, pictures.size());
      CPPUNIT_ASSERT_EQUAL(VariantMap({
        {
          {"data", ByteVector("DummyData")},
          {"mimeType", String("image/png")},
        }
      }), pictures.front());

      CPPUNIT_ASSERT_EQUAL(title, f.tag()->title());
      CPPUNIT_ASSERT_EQUAL(artist, f.tag()->artist());
    }
  }

  void testRemove()
  {
    ScopedFileCopy copy("no-tags", ".m4a");
    createTestFile(copy);

    // Remove the stem
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(f.hasMP4Tag());
      CPPUNIT_ASSERT(f.tag()->setComplexProperties(STEM_KEY, List<VariantMap>()));
      f.save();
    }
    {
      MP4::File f(copy.fileName().c_str());
      CPPUNIT_ASSERT(!f.hasMP4Tag());
      CPPUNIT_ASSERT(!f.tag()->complexPropertyKeys().contains(STEM_KEY));
    }
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMP4Stem);
