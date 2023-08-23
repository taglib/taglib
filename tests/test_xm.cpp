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

#include "utils.h"
#include "xmfile.h"

#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

static const String titleBefore("title of song");
static const String titleAfter("changed title");

static const String trackerNameBefore("MilkyTracker        ");
static const String trackerNameAfter("TagLib");

static const String commentBefore(
  "Instrument names\n"
  "are abused as\n"
  "comments in\n"
  "module file formats.\n"
  "-+-+-+-+-+-+-+-+-+-+-+\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n"
  "Sample\n"
  "names\n"
  "are sometimes\n"
  "also abused as\n"
  "comments.");

static const String newCommentShort(
  "Instrument names\n"
  "are abused as\n"
  "comments in\n"
  "module file formats.\n"
  "======================\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n"
  "Sample names\n"
  "are sometimes\n"
  "also abused as\n"
  "comments.");

static const String newCommentLong(
  "Instrument names\n"
  "are abused as\n"
  "comments in\n"
  "module file formats.\n"
  "======================\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n"
  "Sample names\n"
  "are sometimes\n"
  "also abused as\n"
  "comments.\n"
  "\n\n\n\n\n\n\n"
  "TEST");

static const String commentAfter(
  "Instrument names\n"
  "are abused as\n"
  "comments in\n"
  "module file formats.\n"
  "======================\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  "\n\n\n"
  "Sample names\n"
  "are sometimes\n"
  "also abused as\n"
  "comments.\n");

void testRead(FileName fileName, const String &title,
              const String &comment, const String &trackerName)
{
  XM::File file(fileName);

  ASSERT_TRUE(file.isValid());

  XM::Properties *p = file.audioProperties();
  Mod::Tag *t       = file.tag();

  ASSERT_NE(nullptr, p);
  ASSERT_NE(nullptr, t);

  ASSERT_EQ(0, p->lengthInSeconds());
  ASSERT_EQ(0, p->bitrate());
  ASSERT_EQ(0, p->sampleRate());
  ASSERT_EQ(8, p->channels());
  ASSERT_EQ(static_cast<unsigned short>(1), p->lengthInPatterns());
  ASSERT_EQ(static_cast<unsigned short>(260), p->version());
  ASSERT_EQ(static_cast<unsigned short>(0), p->restartPosition());
  ASSERT_EQ(static_cast<unsigned short>(1), p->patternCount());
  ASSERT_EQ(static_cast<unsigned short>(128), p->instrumentCount());
  ASSERT_EQ(static_cast<unsigned short>(1), p->flags());
  ASSERT_EQ(static_cast<unsigned short>(6), p->tempo());
  ASSERT_EQ(static_cast<unsigned short>(125), p->bpmSpeed());
  ASSERT_EQ(title, t->title());
  ASSERT_EQ(String(), t->artist());
  ASSERT_EQ(String(), t->album());
  ASSERT_EQ(comment, t->comment());
  ASSERT_EQ(String(), t->genre());
  ASSERT_EQ(0U, t->year());
  ASSERT_EQ(0U, t->track());
  ASSERT_EQ(trackerName, t->trackerName());
}

void testWriteTags(const String &comment)
{
  ScopedFileCopy copy("test", ".xm");
  {
    XM::File file(copy.fileName().c_str());
    ASSERT_NE(nullptr, file.tag());
    file.tag()->setTitle(titleAfter);
    file.tag()->setComment(comment);
    file.tag()->setTrackerName(trackerNameAfter);
    ASSERT_TRUE(file.save());
  }
  testRead(copy.fileName().c_str(), titleAfter,
           commentAfter, trackerNameAfter);
  ASSERT_TRUE(fileEqual(
    copy.fileName(),
    TEST_FILE_PATH_C("changed.xm")));
}
TEST(XM, testReadTags)
{
  testRead(TEST_FILE_PATH_C("test.xm"), titleBefore,
           commentBefore, trackerNameBefore);
}

TEST(XM, testReadStrippedTags)
{
  XM::File file(TEST_FILE_PATH_C("stripped.xm"));
  ASSERT_TRUE(file.isValid());

  XM::Properties *p = file.audioProperties();
  Mod::Tag *t       = file.tag();

  ASSERT_NE(nullptr, p);
  ASSERT_NE(nullptr, t);

  ASSERT_EQ(0, p->lengthInSeconds());
  ASSERT_EQ(0, p->bitrate());
  ASSERT_EQ(0, p->sampleRate());
  ASSERT_EQ(8, p->channels());
  ASSERT_EQ(static_cast<unsigned short>(1), p->lengthInPatterns());
  ASSERT_EQ(static_cast<unsigned short>(0), p->version());
  ASSERT_EQ(static_cast<unsigned short>(0), p->restartPosition());
  ASSERT_EQ(static_cast<unsigned short>(1), p->patternCount());
  ASSERT_EQ(static_cast<unsigned short>(0), p->instrumentCount());
  ASSERT_EQ(static_cast<unsigned short>(1), p->flags());
  ASSERT_EQ(static_cast<unsigned short>(6), p->tempo());
  ASSERT_EQ(static_cast<unsigned short>(125), p->bpmSpeed());
  ASSERT_EQ(titleBefore, t->title());
  ASSERT_EQ(String(), t->artist());
  ASSERT_EQ(String(), t->album());
  ASSERT_EQ(String(), t->comment());
  ASSERT_EQ(String(), t->genre());
  ASSERT_EQ(0U, t->year());
  ASSERT_EQ(0U, t->track());
  ASSERT_EQ(String(), t->trackerName());
}

TEST(XM, testWriteTagsShort)
{
  testWriteTags(newCommentShort);
}

TEST(XM, testWriteTagsLong)
{
  testWriteTags(newCommentLong);
}
