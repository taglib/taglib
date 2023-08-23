/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
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

#include "modfile.h"
#include "tpropertymap.h"
#include "utils.h"
#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

static const String titleBefore("title of song");
static const String titleAfter("changed title");

static const String commentBefore(
  "Instrument names\n"
  "are abused as\n"
  "comments in\n"
  "module file formats.\n"
  "-+-+-+-+-+-+-+-+-+-+-+\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

static const String newComment(
  "This line will be truncated because it is too long for a mod instrument name.\n"
  "This line is ok.");

static const String commentAfter(
  "This line will be trun\n"
  "This line is ok.\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

static void testRead(FileName fileName, const String &title, const String &comment)
{
  Mod::File file(fileName);

  ASSERT_TRUE(file.isValid());

  Mod::Properties *p = file.audioProperties();
  Mod::Tag *t        = file.tag();

  ASSERT_NE(nullptr, p);
  ASSERT_NE(nullptr, t);

  ASSERT_EQ(0, p->lengthInSeconds());
  ASSERT_EQ(0, p->bitrate());
  ASSERT_EQ(0, p->sampleRate());
  ASSERT_EQ(8, p->channels());
  ASSERT_EQ(31U, p->instrumentCount());
  ASSERT_EQ(static_cast<unsigned char>(1), p->lengthInPatterns());
  ASSERT_EQ(title, t->title());
  ASSERT_EQ(String(), t->artist());
  ASSERT_EQ(String(), t->album());
  ASSERT_EQ(comment, t->comment());
  ASSERT_EQ(String(), t->genre());
  ASSERT_EQ(0U, t->year());
  ASSERT_EQ(0U, t->track());
  ASSERT_EQ(String("StarTrekker"), t->trackerName());
}

TEST(MOD, testReadTags)
{
  testRead(TEST_FILE_PATH_C("test.mod"), titleBefore, commentBefore);
}

TEST(MOD, testWriteTags)
{
  ScopedFileCopy copy("test", ".mod");
  {
    Mod::File file(copy.fileName().c_str());
    ASSERT_NE(nullptr, file.tag());
    file.tag()->setTitle(titleAfter);
    file.tag()->setComment(newComment);
    ASSERT_TRUE(file.save());
  }
  testRead(copy.fileName().c_str(), titleAfter, commentAfter);
  ASSERT_TRUE(fileEqual(
    copy.fileName(),
    TEST_FILE_PATH_C("changed.mod")));
}

TEST(MOD, testPropertyInterface)
{
  Mod::Tag t;
  PropertyMap properties;
  properties["BLA"]    = String("bla");
  properties["ARTIST"] = String("artist1");
  properties["ARTIST"].append("artist2");
  properties["TITLE"]     = String("title");

  PropertyMap unsupported = t.setProperties(properties);
  ASSERT_TRUE(unsupported.contains("BLA"));
  ASSERT_TRUE(unsupported.contains("ARTIST"));
  ASSERT_EQ(properties["ARTIST"], unsupported["ARTIST"]);
  ASSERT_FALSE(unsupported.contains("TITLE"));

  properties = t.properties();
  ASSERT_EQ(StringList("title"), properties["TITLE"]);
}
