/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <xmfile.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestXM : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestXM);
  CPPUNIT_TEST(testRead);
  CPPUNIT_TEST(testChangeTitle);
  CPPUNIT_TEST_SUITE_END();

public:
  void testRead()
  {
    testRead(TEST_FILE_PATH_C("test.xm"), "title of song");
  }

  void testChangeTitle()
  {
    ScopedFileCopy copy("test", ".xm");
    {
      XM::File file(copy.fileName().c_str());
      CPPUNIT_ASSERT(file.tag() != 0);
      file.tag()->setTitle("changed title");
      CPPUNIT_ASSERT(file.save());
    }
    testRead(copy.fileName().c_str(), "changed title");
    CPPUNIT_ASSERT(fileEqual(
      copy.fileName(),
      TEST_FILE_PATH_C("changed_title.xm")));
  }

private:
  void testRead(FileName fileName, const String &title)
  {
    XM::File file(fileName);

    CPPUNIT_ASSERT(file.isValid());

    XM::Properties *p = file.audioProperties();
    Mod::Tag *t = file.tag();
    
    CPPUNIT_ASSERT(0 != p);
    CPPUNIT_ASSERT(0 != t);

    CPPUNIT_ASSERT_EQUAL(0, p->length());
    CPPUNIT_ASSERT_EQUAL(0, p->bitrate());
    CPPUNIT_ASSERT_EQUAL(0, p->sampleRate());
    CPPUNIT_ASSERT_EQUAL(8, p->channels());
	CPPUNIT_ASSERT_EQUAL((TagLib::ushort)  1, p->tableLength());
	CPPUNIT_ASSERT_EQUAL((TagLib::ushort)260, p->version());
	CPPUNIT_ASSERT_EQUAL((TagLib::ushort)  0, p->restartPosition());
    CPPUNIT_ASSERT_EQUAL((TagLib::ushort)  1, p->patternCount());
    CPPUNIT_ASSERT_EQUAL((TagLib::ushort)128, p->instrumentCount());
    CPPUNIT_ASSERT_EQUAL((TagLib::ushort)  1, p->flags());
    CPPUNIT_ASSERT_EQUAL((TagLib::ushort)  6, p->tempo());
    CPPUNIT_ASSERT_EQUAL((TagLib::ushort)125, p->bpmSpeed());
    CPPUNIT_ASSERT_EQUAL(title, t->title());
    CPPUNIT_ASSERT_EQUAL(String::null, t->artist());
    CPPUNIT_ASSERT_EQUAL(String::null, t->album());
    CPPUNIT_ASSERT_EQUAL(String(
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
	  "comments."
    ), t->comment());
    CPPUNIT_ASSERT_EQUAL(String::null, t->genre());
    CPPUNIT_ASSERT_EQUAL(0U, t->year());
    CPPUNIT_ASSERT_EQUAL(0U, t->track());
    CPPUNIT_ASSERT_EQUAL(String("MilkyTracker        "), t->trackerName());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestXM);
