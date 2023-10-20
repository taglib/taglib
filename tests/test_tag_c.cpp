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

#include <string>
#include <unordered_map>
#include <list>

#include "tag_c.h"
#include "tbytevector.h"
#include "tstring.h"
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace TagLib;

class TestTagC : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestTagC);
  CPPUNIT_TEST(testMp3);
  CPPUNIT_TEST_SUITE_END();

public:
  void testMp3()
  {
    ScopedFileCopy copy("xing", ".mp3");

    {
      TagLib_File *file = taglib_file_new(copy.fileName().c_str());
      CPPUNIT_ASSERT(taglib_file_is_valid(file));
      const TagLib_AudioProperties *audioProperties = taglib_file_audioproperties(file);
      CPPUNIT_ASSERT_EQUAL(32, taglib_audioproperties_bitrate(audioProperties));
      CPPUNIT_ASSERT_EQUAL(2, taglib_audioproperties_channels(audioProperties));
      CPPUNIT_ASSERT_EQUAL(2, taglib_audioproperties_length(audioProperties));
      CPPUNIT_ASSERT_EQUAL(44100, taglib_audioproperties_samplerate(audioProperties));

      TagLib_Tag *tag = taglib_file_tag(file);
      CPPUNIT_ASSERT_EQUAL(""s, std::string(taglib_tag_album(tag)));
      taglib_tag_set_album(tag, "Album");
      taglib_tag_set_artist(tag, "Artist");
      taglib_tag_set_comment(tag, "Comment");
      taglib_tag_set_genre(tag, "Genre");
      taglib_tag_set_title(tag, "Title");
      taglib_tag_set_track(tag, 2);
      taglib_tag_set_year(tag, 2023);

      taglib_property_set(file, "COMPOSER", "Composer 1");
      taglib_property_set_append(file, "COMPOSER", "Composer 2");
      taglib_property_set(file, "ALBUMARTIST", "Album Artist");

      TAGLIB_COMPLEX_PROPERTY_PICTURE(props, "JPEG Data", 9, "Written by TagLib",
                                      "image/jpeg", "Front Cover");
      taglib_complex_property_set(file, "PICTURE", props);

      taglib_file_save(file);
      taglib_file_free(file);
    }
    {
      TagLib_File *file = taglib_file_new(copy.fileName().c_str());
      CPPUNIT_ASSERT(taglib_file_is_valid(file));

      TagLib_Tag *tag = taglib_file_tag(file);
      CPPUNIT_ASSERT_EQUAL("Album"s, std::string(taglib_tag_album(tag)));
      CPPUNIT_ASSERT_EQUAL("Artist"s, std::string(taglib_tag_artist(tag)));
      CPPUNIT_ASSERT_EQUAL("Comment"s, std::string(taglib_tag_comment(tag)));
      CPPUNIT_ASSERT_EQUAL("Genre"s, std::string(taglib_tag_genre(tag)));
      CPPUNIT_ASSERT_EQUAL("Title"s, std::string(taglib_tag_title(tag)));
      CPPUNIT_ASSERT_EQUAL(2U, taglib_tag_track(tag));
      CPPUNIT_ASSERT_EQUAL(2023U, taglib_tag_year(tag));

      char **keys = taglib_property_keys(file);
      CPPUNIT_ASSERT(keys);
      std::unordered_map<std::string, std::list<std::string>> propertyMap;
      char **keyPtr = keys;
      while(*keyPtr) {
        char **values = taglib_property_get(file, *keyPtr);
        char **valuePtr = values;
        std::list<std::string> valueList;
        while(*valuePtr) {
          valueList.push_back(*valuePtr++);
        }
        taglib_property_free(values);
        propertyMap[*keyPtr++] = valueList;
      }
      taglib_property_free(keys);
      const std::unordered_map<std::string, std::list<std::string>> expected {
        {"TRACKNUMBER"s, {"2"s}},
        {"TITLE"s, {"Title"s}},
        {"GENRE"s, {"Genre"s}},
        {"DATE"s, {"2023"s}},
        {"COMPOSER"s, {"Composer 1"s, "Composer 2"s}},
        {"COMMENT"s, {"Comment"s}},
        {"ARTIST"s, {"Artist"s}},
        {"ALBUMARTIST"s, {"Album Artist"s}},
        {"ALBUM"s, {"Album"s}}
      };
      CPPUNIT_ASSERT(expected == propertyMap);

      char **complexKeys = taglib_complex_property_keys(file);
      CPPUNIT_ASSERT(complexKeys);
      std::list<std::string> keyList;
      char **complexKeyPtr = complexKeys;
      while(*complexKeyPtr) {
        keyList.push_back(*complexKeyPtr++);
      }
      taglib_complex_property_free_keys(complexKeys);
      CPPUNIT_ASSERT(std::list{"PICTURE"s} == keyList);

      TagLib_Complex_Property_Attribute*** properties =
        taglib_complex_property_get(file, "PICTURE");
      TagLib_Complex_Property_Picture_Data picture;
      taglib_picture_from_complex_property(properties, &picture);
      CPPUNIT_ASSERT_EQUAL("image/jpeg"s, std::string(picture.mimeType));
      CPPUNIT_ASSERT_EQUAL("Written by TagLib"s, std::string(picture.description));
      CPPUNIT_ASSERT_EQUAL("Front Cover"s, std::string(picture.pictureType));
      CPPUNIT_ASSERT_EQUAL(ByteVector("JPEG Data"), ByteVector(picture.data, 9));
      CPPUNIT_ASSERT_EQUAL(9U, picture.size);
      taglib_complex_property_free(properties);

      taglib_file_free(file);
    }

    taglib_tag_free_strings();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTagC);
