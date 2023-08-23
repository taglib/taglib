/***************************************************************************
    copyright           : (C) 2010 by Lukas Lalinsky
    email               : lukas@oxygene.sk
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

#include "apefile.h"
#include "apetag.h"
#include "id3v1tag.h"
#include "tbytevectorlist.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "utils.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

using namespace std;
using namespace TagLib;

TEST(Ape, testProperties399)
{
  APE::File f(TEST_FILE_PATH_C("mac-399.ape"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3550, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(192, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(156556U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(3990, f.audioProperties()->version());
}

TEST(Ape, testProperties399Tagged)
{
  APE::File f(TEST_FILE_PATH_C("mac-399-tagged.ape"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3550, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(192, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(156556U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(3990, f.audioProperties()->version());
}

TEST(Ape, testProperties399Id3v2)
{
  APE::File f(TEST_FILE_PATH_C("mac-399-id3v2.ape"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3550, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(192, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(156556U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(3990, f.audioProperties()->version());
}

TEST(Ape, testProperties396)
{
  APE::File f(TEST_FILE_PATH_C("mac-396.ape"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(3, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(3685, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(0, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(162496U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(3960, f.audioProperties()->version());
}

TEST(Ape, testProperties390)
{
  APE::File f(TEST_FILE_PATH_C("mac-390-hdr.ape"));
  ASSERT_TRUE(f.audioProperties());
  ASSERT_EQ(15, f.audioProperties()->lengthInSeconds());
  ASSERT_EQ(15630, f.audioProperties()->lengthInMilliseconds());
  ASSERT_EQ(0, f.audioProperties()->bitrate());
  ASSERT_EQ(2, f.audioProperties()->channels());
  ASSERT_EQ(44100, f.audioProperties()->sampleRate());
  ASSERT_EQ(16, f.audioProperties()->bitsPerSample());
  ASSERT_EQ(689262U, f.audioProperties()->sampleFrames());
  ASSERT_EQ(3900, f.audioProperties()->version());
}

TEST(Ape, testFuzzedFile1)
{
  APE::File f(TEST_FILE_PATH_C("longloop.ape"));
  ASSERT_TRUE(f.isValid());
}

TEST(Ape, testFuzzedFile2)
{
  APE::File f(TEST_FILE_PATH_C("zerodiv.ape"));
  ASSERT_TRUE(f.isValid());
}

TEST(Ape, testStripAndProperties)
{
  ScopedFileCopy copy("mac-399", ".ape");

  {
    APE::File f(copy.fileName().c_str());
    f.APETag(true)->setTitle("APE");
    f.ID3v1Tag(true)->setTitle("ID3v1");
    f.save();
  }
  {
    APE::File f(copy.fileName().c_str());
    ASSERT_EQ(String("APE"), f.properties()["TITLE"].front());
    f.strip(APE::File::APE);
    ASSERT_EQ(String("ID3v1"), f.properties()["TITLE"].front());
    f.strip(APE::File::ID3v1);
    ASSERT_TRUE(f.properties().isEmpty());
  }
}

TEST(Ape, testProperties)
{
  PropertyMap tags;
  tags["ALBUM"]                      = StringList("Album");
  tags["ALBUMARTIST"]                = StringList("Album Artist");
  tags["ALBUMARTISTSORT"]            = StringList("Album Artist Sort");
  tags["ALBUMSORT"]                  = StringList("Album Sort");
  tags["ARTIST"]                     = StringList("Artist");
  tags["ARTISTS"]                    = StringList("Artists");
  tags["ARTISTSORT"]                 = StringList("Artist Sort");
  tags["ASIN"]                       = StringList("ASIN");
  tags["BARCODE"]                    = StringList("Barcode");
  tags["CATALOGNUMBER"]              = StringList("Catalog Number 1").append("Catalog Number 2");
  tags["COMMENT"]                    = StringList("Comment");
  tags["DATE"]                       = StringList("2021-01-10");
  tags["DISCNUMBER"]                 = StringList("3/5");
  tags["GENRE"]                      = StringList("Genre");
  tags["ISRC"]                       = StringList("UKAAA0500001");
  tags["LABEL"]                      = StringList("Label 1").append("Label 2");
  tags["MEDIA"]                      = StringList("Media");
  tags["MUSICBRAINZ_ALBUMARTISTID"]  = StringList("MusicBrainz_AlbumartistID");
  tags["MUSICBRAINZ_ALBUMID"]        = StringList("MusicBrainz_AlbumID");
  tags["MUSICBRAINZ_ARTISTID"]       = StringList("MusicBrainz_ArtistID");
  tags["MUSICBRAINZ_RELEASEGROUPID"] = StringList("MusicBrainz_ReleasegroupID");
  tags["MUSICBRAINZ_RELEASETRACKID"] = StringList("MusicBrainz_ReleasetrackID");
  tags["MUSICBRAINZ_TRACKID"]        = StringList("MusicBrainz_TrackID");
  tags["ORIGINALDATE"]               = StringList("2021-01-09");
  tags["RELEASECOUNTRY"]             = StringList("Release Country");
  tags["RELEASESTATUS"]              = StringList("Release Status");
  tags["RELEASETYPE"]                = StringList("Release Type");
  tags["SCRIPT"]                     = StringList("Script");
  tags["TITLE"]                      = StringList("Title");
  tags["TRACKNUMBER"]                = StringList("2/3");

  ScopedFileCopy copy("mac-399", ".ape");
  {
    APE::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    ASSERT_TRUE(properties.isEmpty());
    f.setProperties(tags);
    f.save();
  }
  {
    const APE::File f(copy.fileName().c_str());
    PropertyMap properties = f.properties();
    if(tags != properties) {
      ASSERT_EQ(tags.toString(), properties.toString());
    }
    ASSERT_EQ(tags, properties);
  }
}

TEST(Ape, testRepeatedSave)
{
  ScopedFileCopy copy("mac-399", ".ape");

  {
    APE::File f(copy.fileName().c_str());
    ASSERT_FALSE(f.hasAPETag());
    ASSERT_FALSE(f.hasID3v1Tag());

    f.APETag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.save();

    f.APETag()->setTitle("0");
    f.save();

    f.ID3v1Tag(true)->setTitle("01234 56789 ABCDE FGHIJ");
    f.APETag()->setTitle("01234 56789 ABCDE FGHIJ 01234 56789 ABCDE FGHIJ 01234 56789");
    f.save();
  }
  {
    APE::File f(copy.fileName().c_str());
    ASSERT_TRUE(f.hasAPETag());
    ASSERT_TRUE(f.hasID3v1Tag());
  }
}
