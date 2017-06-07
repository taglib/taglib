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
#include <itfile.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

namespace
{
  const String titleBefore("test song name");
  const String titleAfter("changed title");

  const String commentBefore(
    "This is a sample name.\n"
    "In module file formats\n"
    "sample names are abused\n"
    "as multiline comments.\n"
    " ");

  const String newComment(
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

  const String commentAfter(
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

  void testRead(FileName fileName, const String &title, const String &comment)
  {
    IT::File file(fileName);

    REQUIRE(file.isValid());

    IT::Properties *p = file.audioProperties();
    Mod::Tag *t = file.tag();

    REQUIRE(p);
    REQUIRE(t);

    REQUIRE(p->length() == 0);
    REQUIRE(p->bitrate() == 0);
    REQUIRE(p->sampleRate() == 0);
    REQUIRE(p->channels() == 64);
    REQUIRE(p->lengthInPatterns() == 0);
    REQUIRE(p->stereo() == true);
    REQUIRE(p->instrumentCount() == 0);
    REQUIRE(p->sampleCount() == 5);
    REQUIRE(p->patternCount() == 1);
    REQUIRE(p->version() == 535);
    REQUIRE(p->compatibleVersion() == 532);
    REQUIRE(p->flags() == 9);
    REQUIRE(p->globalVolume() == 128);
    REQUIRE(p->mixVolume() == 48);
    REQUIRE(p->tempo() == 125);
    REQUIRE(p->bpmSpeed() == 6);
    REQUIRE(p->panningSeparation() == 128);
    REQUIRE(p->pitchWheelDepth() == 0);
    REQUIRE(t->title() == title);
    REQUIRE(t->artist().isEmpty());
    REQUIRE(t->album().isEmpty());
    REQUIRE(t->comment() == comment);
    REQUIRE(t->genre().isEmpty());
    REQUIRE(t->year() == 0);
    REQUIRE(t->track() == 0);
    REQUIRE(t->trackerName() == "Impulse Tracker");
  }
}

TEST_CASE("IT File")
{
  SECTION("Read tags")
  {
    testRead(TEST_FILE_PATH_C("test.it"), titleBefore, commentBefore);
  }
  SECTION("Write tags")
  {
    ScopedFileCopy copy("test", ".it");
    {
      IT::File file(copy.fileName().c_str());
      REQUIRE(file.tag() != 0);
      file.tag()->setTitle(titleAfter);
      file.tag()->setComment(newComment);
      file.tag()->setTrackerName("won't be saved");
      REQUIRE(file.save());
    }
    testRead(copy.fileName().c_str(), titleAfter, commentAfter);
  }
}
