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

#include "s3mfile.h"
#include "utils.h"
#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

static const String titleBefore("test song name");
static const String titleAfter("changed title");

static const String commentBefore(
  "This is an instrument name.\n"
  "Module file formats\n"
  "abuse instrument names\n"
  "as multiline comments.\n"
  " ");

static const String newComment(
  "This is an instrument name!\n"
  "Module file formats\n"
  "abuse instrument names\n"
  "as multiline comments.\n"
  "-----------------------------------\n"
  "This line will be dropped and the previous is truncated.");

static const String commentAfter(
  "This is an instrument name!\n"
  "Module file formats\n"
  "abuse instrument names\n"
  "as multiline comments.\n"
  "---------------------------");

void testRead(FileName fileName, const String &title, const String &comment)
{
  S3M::File file(fileName);

  ASSERT_TRUE(file.isValid());

  S3M::Properties *p = file.audioProperties();
  Mod::Tag *t        = file.tag();

  ASSERT_NE(nullptr, p);
  ASSERT_NE(nullptr, t);

  ASSERT_EQ(0, p->lengthInSeconds());
  ASSERT_EQ(0, p->bitrate());
  ASSERT_EQ(0, p->sampleRate());
  ASSERT_EQ(16, p->channels());
  ASSERT_EQ(static_cast<unsigned short>(0), p->lengthInPatterns());
  ASSERT_FALSE(p->stereo());
  ASSERT_EQ(static_cast<unsigned short>(5), p->sampleCount());
  ASSERT_EQ(static_cast<unsigned short>(1), p->patternCount());
  ASSERT_EQ(static_cast<unsigned short>(0), p->flags());
  ASSERT_EQ(static_cast<unsigned short>(4896), p->trackerVersion());
  ASSERT_EQ(static_cast<unsigned short>(2), p->fileFormatVersion());
  ASSERT_EQ(static_cast<unsigned char>(64), p->globalVolume());
  ASSERT_EQ(static_cast<unsigned char>(48), p->masterVolume());
  ASSERT_EQ(static_cast<unsigned char>(125), p->tempo());
  ASSERT_EQ(static_cast<unsigned char>(6), p->bpmSpeed());
  ASSERT_EQ(title, t->title());
  ASSERT_EQ(String(), t->artist());
  ASSERT_EQ(String(), t->album());
  ASSERT_EQ(comment, t->comment());
  ASSERT_EQ(String(), t->genre());
  ASSERT_EQ(0U, t->year());
  ASSERT_EQ(0U, t->track());
  ASSERT_EQ(String("ScreamTracker III"), t->trackerName());
}

TEST(S3M, testReadTags)
{
  testRead(TEST_FILE_PATH_C("test.s3m"), titleBefore, commentBefore);
}

TEST(S3M, testWriteTags)
{
  ScopedFileCopy copy("test", ".s3m");
  {
    S3M::File file(copy.fileName().c_str());
    ASSERT_NE(nullptr, file.tag());
    file.tag()->setTitle(titleAfter);
    file.tag()->setComment(newComment);
    file.tag()->setTrackerName("won't be saved");
    ASSERT_TRUE(file.save());
  }
  testRead(copy.fileName().c_str(), titleAfter, commentAfter);
  ASSERT_TRUE(fileEqual(
    copy.fileName(),
    TEST_FILE_PATH_C("changed.s3m")));
}
