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
#include <modfile.h>
#include <tpropertymap.h>
#include "utils.h"

using namespace TagLib;

namespace
{
  const String titleBefore("title of song");
  const String titleAfter("changed title");
  
  const String commentBefore(
    "Instrument names\n"
    "are abused as\n"
    "comments in\n"
    "module file formats.\n"
    "-+-+-+-+-+-+-+-+-+-+-+\n"
    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
  
  const String newComment(
    "This line will be truncated because it is too long for a mod instrument name.\n"
    "This line is ok.");
  
  const String commentAfter(
    "This line will be trun\n"
    "This line is ok.\n"
    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

  void testRead(FileName fileName, const String &title, const String &comment)
  {
    Mod::File file(fileName);
  
    REQUIRE(file.isValid());
  
    Mod::Properties *p = file.audioProperties();
    Mod::Tag *t = file.tag();
  
    REQUIRE(p);
    REQUIRE(t);
  
    REQUIRE(p->length() == 0);
    REQUIRE(p->bitrate() == 0);
    REQUIRE(p->sampleRate() == 0);
    REQUIRE(p->channels() == 8);
    REQUIRE(p->instrumentCount() == 31);
    REQUIRE(p->lengthInPatterns() == 1);
    REQUIRE(t->title() == title);
    REQUIRE(t->artist().isEmpty());
    REQUIRE(t->album().isEmpty());
    REQUIRE(t->comment() == comment);
    REQUIRE(t->genre().isEmpty());
    REQUIRE(t->year() == 0);
    REQUIRE(t->track() == 0);
    REQUIRE(t->trackerName() == "StarTrekker");
  }
}

TEST_CASE("MOD File")
{
  SECTION("Read tags")
  {
    testRead(TEST_FILE_PATH_C("test.mod"), titleBefore, commentBefore);
  }
  SECTION("Write tags")
  {
    const ScopedFileCopy copy("test", ".mod");
    {
      Mod::File file(copy.fileName().c_str());
      REQUIRE(file.tag() != 0);
      file.tag()->setTitle(titleAfter);
      file.tag()->setComment(newComment);
      REQUIRE(file.save());
    }
    testRead(copy.fileName().c_str(), titleAfter, commentAfter);
    REQUIRE(fileEqual(copy.fileName(), TEST_FILE_PATH_C("changed.mod")));
  }
  SECTION("Read and write property map")
  {
    Mod::Tag t;
    PropertyMap properties;
    properties["BLA"] = String("bla");
    properties["ARTIST"] = String("artist1");
    properties["ARTIST"].append("artist2");
    properties["TITLE"] = String("title");
    
    PropertyMap unsupported = t.setProperties(properties);
    REQUIRE(unsupported.contains("BLA"));
    REQUIRE(unsupported.contains("ARTIST"));
    REQUIRE(unsupported["ARTIST"] == properties["ARTIST"]);
    REQUIRE_FALSE(unsupported.contains("TITLE"));
  }
}
