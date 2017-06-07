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
#include <s3mfile.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

namespace
{
  const String titleBefore("test song name");
  const String titleAfter("changed title");

  const String commentBefore(
    "This is an instrument name.\n"
    "Module file formats\n"
    "abuse instrument names\n"
    "as multiline comments.\n"
    " ");

  const String newComment(
    "This is an instrument name!\n"
    "Module file formats\n"
    "abuse instrument names\n"
    "as multiline comments.\n"
    "-----------------------------------\n"
    "This line will be dropped and the previous is truncated.");

  const String commentAfter(
    "This is an instrument name!\n"
    "Module file formats\n"
    "abuse instrument names\n"
    "as multiline comments.\n"
    "---------------------------");

  void testRead(FileName fileName, const String &title, const String &comment)
  {
    S3M::File file(fileName);

    REQUIRE(file.isValid());

    S3M::Properties *p = file.audioProperties();
    Mod::Tag *t = file.tag();

    REQUIRE(p);
    REQUIRE(t);

    REQUIRE(p->length() == 0);
    REQUIRE(p->bitrate() == 0);
    REQUIRE(p->sampleRate() == 0);
    REQUIRE(p->channels() == 16);
    REQUIRE(p->lengthInPatterns() == 0);
    REQUIRE_FALSE(p->stereo());
    REQUIRE(p->sampleCount() == 5);
    REQUIRE(p->patternCount() == 1);
    REQUIRE(p->flags() == 0);
    REQUIRE(p->trackerVersion() == 4896);
    REQUIRE(p->fileFormatVersion() == 2);
    REQUIRE(p->globalVolume() == 64);
    REQUIRE(p->masterVolume() == 48);
    REQUIRE(p->tempo() == 125);
    REQUIRE(p->bpmSpeed() == 6);
    REQUIRE(t->title() == title);
    REQUIRE(t->artist().isEmpty());
    REQUIRE(t->album().isEmpty());
    REQUIRE(t->comment() == comment);
    REQUIRE(t->genre().isEmpty());
    REQUIRE(t->year() == 0);
    REQUIRE(t->track() == 0);
    REQUIRE(t->trackerName() == "ScreamTracker III");
  }
}

TEST_CASE("S3M File")
{
  SECTION("Read tags")
  {
    testRead(TEST_FILE_PATH_C("test.s3m"), titleBefore, commentBefore);
  }
  SECTION("Write tags")
  {
    const ScopedFileCopy copy("test", ".s3m");
    {
      S3M::File file(copy.fileName().c_str());
      REQUIRE(file.tag() != 0);
      file.tag()->setTitle(titleAfter);
      file.tag()->setComment(newComment);
      file.tag()->setTrackerName("won't be saved");
      REQUIRE(file.save());
    }
    testRead(copy.fileName().c_str(), titleAfter, commentAfter);
    REQUIRE(fileEqual(copy.fileName(), TEST_FILE_PATH_C("changed.s3m")));
  }
}
