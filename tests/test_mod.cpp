#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <modfile.h>
#include <stdlib.h>
#include "utils.h"

using namespace std;
using namespace TagLib;

class TestMod : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestMod);
	CPPUNIT_TEST(testRead);
	CPPUNIT_TEST(testChangeTitle);
	CPPUNIT_TEST_SUITE_END();

public:
	void testRead()
	{
		testRead(TEST_FILE_PATH_C("test.mod"), "title of song");
	}

	void testChangeTitle()
	{
		ScopedFileCopy copy("test", ".mod");
		{
			Mod::File file(copy.fileName().c_str());
			CPPUNIT_ASSERT(file.tag() != 0);
			file.tag()->setTitle("changed title");
			CPPUNIT_ASSERT(file.save());
		}
		{
			testRead(copy.fileName().c_str(), "changed title");
		}
		{
			assertFileEqual(
				copy.fileName().c_str(),
				TEST_FILE_PATH_C("changed_title.mod"));
		}
	}

private:
	class Closer
	{
	public:
		Closer(FILE *stream) : m_stream(stream)
		{
		}

		~Closer()
		{
			if (m_stream)
			{
				fclose(m_stream);
			}
		}
	private:
		FILE *m_stream;
	};

	void assertFileEqual(const char *file1, const char *file2)
	{
		char buf1[BUFSIZ];
		char buf2[BUFSIZ];

		FILE *stream1 = fopen(file1, "rb");
		FILE *stream2 = fopen(file2, "rb");

		Closer closer1(stream1);
		Closer closer2(stream2);

		CPPUNIT_ASSERT(stream1 != 0);
		CPPUNIT_ASSERT(stream2 != 0);

		for (;;)
		{
			size_t n1 = fread(buf1, 1, BUFSIZ, stream1);
			size_t n2 = fread(buf2, 1, BUFSIZ, stream2);

			CPPUNIT_ASSERT_EQUAL(n1, n2);

			if (n1 == 0) break;

			CPPUNIT_ASSERT(memcmp(buf1, buf2, n1) == 0);
		}
	}

	void testRead(FileName fileName, const String &title)
	{
		Mod::File file(fileName);

		CPPUNIT_ASSERT(file.isValid());

		Mod::Properties *p = file.audioProperties();
		Mod::Tag *t = file.tag();
		
		CPPUNIT_ASSERT(0 != p);
		CPPUNIT_ASSERT(0 != t);

		CPPUNIT_ASSERT_EQUAL(0, p->length());
		CPPUNIT_ASSERT_EQUAL(0, p->bitrate());
		CPPUNIT_ASSERT_EQUAL(0, p->sampleRate());
		CPPUNIT_ASSERT_EQUAL(8, p->channels());
		CPPUNIT_ASSERT_EQUAL(31U, p->instrumentCount());
		CPPUNIT_ASSERT_EQUAL(1U, p->patternCount());
		CPPUNIT_ASSERT_EQUAL(title, t->title());
		CPPUNIT_ASSERT_EQUAL(String::null, t->artist());
		CPPUNIT_ASSERT_EQUAL(String::null, t->album());
		CPPUNIT_ASSERT_EQUAL(String(
			"Instrument names\n"
			"are abused as\n"
			"comments in\n"
			"module file formats.\n"
			"-+-+-+-+-+-+-+-+-+-+-+\n"
			"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		), t->comment());
		CPPUNIT_ASSERT_EQUAL(String::null, t->genre());
		CPPUNIT_ASSERT_EQUAL(0U, t->year());
		CPPUNIT_ASSERT_EQUAL(0U, t->track());
		CPPUNIT_ASSERT_EQUAL(String("StarTrekker"), t->trackerName());
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMod);
