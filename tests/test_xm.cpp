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

#include <catch/catch.hpp>
#include <xmfile.h>
#include "utils.h"

using namespace TagLib;

namespace
{
  const String titleBefore("title of song");
  const String titleAfter("changed title");

  const String trackerNameBefore("MilkyTracker        ");
  const String trackerNameAfter("TagLib");

  const String commentBefore(
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

  const String newCommentShort(
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

  const String newCommentLong(
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

  const String commentAfter(
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

    REQUIRE(file.isValid());

    XM::Properties *p = file.audioProperties();
    Mod::Tag *t = file.tag();

    REQUIRE(p);
    REQUIRE(t);

    REQUIRE(p->length() == 0);
    REQUIRE(p->bitrate() == 0);
    REQUIRE(p->sampleRate() == 0);
    REQUIRE(p->channels() == 8);
    REQUIRE(p->lengthInPatterns() == 1);
    REQUIRE(p->version() == 260);
    REQUIRE(p->restartPosition() == 0);
    REQUIRE(p->patternCount() == 1);
    REQUIRE(p->instrumentCount() == 128);
    REQUIRE(p->flags() == 1);
    REQUIRE(p->tempo() == 6);
    REQUIRE(p->bpmSpeed() == 125);
    REQUIRE(t->title() == title);
    REQUIRE(t->artist().isEmpty());
    REQUIRE(t->album().isEmpty());
    REQUIRE(t->comment() == comment);
    REQUIRE(t->genre().isEmpty());
    REQUIRE(t->year() == 0);
    REQUIRE(t->track() == 0);
    REQUIRE(t->trackerName() == trackerName);
  }

  void testWriteTags(const String &comment)
  {
    const ScopedFileCopy copy("test", ".xm");
    {
      XM::File file(copy.fileName().c_str());
      REQUIRE(file.tag() != 0);
      file.tag()->setTitle(titleAfter);
      file.tag()->setComment(comment);
      file.tag()->setTrackerName(trackerNameAfter);
      REQUIRE(file.save());
    }
    testRead(copy.fileName().c_str(), titleAfter,
      commentAfter, trackerNameAfter);
    REQUIRE(fileEqual(copy.fileName(), TEST_FILE_PATH_C("changed.xm")));
  }
}

TEST_CASE("XM File")
{
  SECTION("Read tags")
  {
    testRead(TEST_FILE_PATH_C("test.xm"), titleBefore,
      commentBefore, trackerNameBefore);
  }
  SECTION("Read stripped tags")
  {
    XM::File file(TEST_FILE_PATH_C("stripped.xm"));
    REQUIRE(file.isValid());

    XM::Properties *p = file.audioProperties();
    Mod::Tag *t = file.tag();

    REQUIRE(p);
    REQUIRE(t);

    REQUIRE(p->length() == 0);
    REQUIRE(p->bitrate() == 0);
    REQUIRE(p->sampleRate() == 0);
    REQUIRE(p->channels() == 8);
    REQUIRE(p->lengthInPatterns() == 1);
    REQUIRE(p->version() == 0);
    REQUIRE(p->restartPosition() == 0);
    REQUIRE(p->patternCount() == 1);
    REQUIRE(p->instrumentCount() == 0);
    REQUIRE(p->flags() == 1);
    REQUIRE(p->tempo() == 6);
    REQUIRE(p->bpmSpeed() == 125);
    REQUIRE(t->title() == titleBefore);
    REQUIRE(t->artist().isEmpty());
    REQUIRE(t->album().isEmpty());
    REQUIRE(t->comment().isEmpty());
    REQUIRE(t->genre().isEmpty());
    REQUIRE(t->year() == 0);
    REQUIRE(t->track() == 0);
    REQUIRE(t->trackerName().isEmpty());
  }
  SECTION("Write short comment")
  {
    testWriteTags(newCommentShort);
  }
  SECTION("Write long comment")
  {
    testWriteTags(newCommentLong);
  }
}
