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

#include "itfile.h"
#include "tstringlist.h"
#include "utils.h"
#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

static const String titleBefore("test song name");
static const String titleAfter("changed title");

static const String commentBefore(
  "This is a sample name.\n"
  "In module file formats\n"
  "sample names are abused\n"
  "as multiline comments.\n"
  " ");

static const String newComment(
  "This is a sample name!\n"
  "In module file formats\n"
  "sample names are abused\n"
  "as multiline comments.\n"
  "-----------------------------------\n"
  "The previous line is truncated but starting with this line\n"
  "the comment is not limeted in the line length but to 8000\n"
  "additional characters (bytes).\n"
  "\n"
  "This is because it is saved in the 'message' proportion of\n"
  "IT files.");

static const String commentAfter(
  "This is a sample name!\n"
  "In module file formats\n"
  "sample names are abused\n"
  "as multiline comments.\n"
  "-------------------------\n"
  "The previous line is truncated but starting with this line\n"
  "the comment is not limeted in the line length but to 8000\n"
  "additional characters (bytes).\n"
  "\n"
  "This is because it is saved in the 'message' proportion of\n"
  "IT files.");

static void testRead(FileName fileName, const String &title, const String &comment)
{
  IT::File file(fileName);

  ASSERT_TRUE(file.isValid());

  IT::Properties *p = file.audioProperties();
  Mod::Tag *t       = file.tag();

  ASSERT_NE(nullptr, p);
  ASSERT_NE(nullptr, t);

  ASSERT_EQ(0, p->lengthInSeconds());
  ASSERT_EQ(0, p->bitrate());
  ASSERT_EQ(0, p->sampleRate());
  ASSERT_EQ(64, p->channels());
  ASSERT_EQ(static_cast<unsigned short>(0), p->lengthInPatterns());
  ASSERT_TRUE(p->stereo());
  ASSERT_EQ(static_cast<unsigned short>(0), p->instrumentCount());
  ASSERT_EQ(static_cast<unsigned short>(5), p->sampleCount());
  ASSERT_EQ(static_cast<unsigned short>(1), p->patternCount());
  ASSERT_EQ(static_cast<unsigned short>(535), p->version());
  ASSERT_EQ(static_cast<unsigned short>(532), p->compatibleVersion());
  ASSERT_EQ(static_cast<unsigned short>(9), p->flags());
  ASSERT_EQ(static_cast<unsigned char>(128), p->globalVolume());
  ASSERT_EQ(static_cast<unsigned char>(48), p->mixVolume());
  ASSERT_EQ(static_cast<unsigned char>(125), p->tempo());
  ASSERT_EQ(static_cast<unsigned char>(6), p->bpmSpeed());
  ASSERT_EQ(static_cast<unsigned char>(128), p->panningSeparation());
  ASSERT_EQ(static_cast<unsigned char>(0), p->pitchWheelDepth());
  ASSERT_EQ(title, t->title());
  ASSERT_EQ(String(), t->artist());
  ASSERT_EQ(String(), t->album());
  ASSERT_EQ(comment, t->comment());
  ASSERT_EQ(String(), t->genre());
  ASSERT_EQ(0U, t->year());
  ASSERT_EQ(0U, t->track());
  ASSERT_EQ(String("Impulse Tracker"), t->trackerName());
}

TEST(IT, testReadTagS)
{
  testRead(TEST_FILE_PATH_C("test.it"), titleBefore, commentBefore);
}

TEST(IT, testWriteTags)
{
  ScopedFileCopy copy("test", ".it");
  {
    IT::File file(copy.fileName().c_str());
    ASSERT_NE(nullptr, file.tag());
    file.tag()->setTitle(titleAfter);
    file.tag()->setComment(newComment);
    file.tag()->setTrackerName("won't be saved");
    ASSERT_TRUE(file.save());
  }
  testRead(copy.fileName().c_str(), titleAfter, commentAfter);
}
