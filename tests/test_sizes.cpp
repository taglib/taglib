/***************************************************************************
    copyright           : (C) 2023 Scott Wheeler
    email               : wheeler@kde.org
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

#include <cstring>

#include "aifffile.h"
#include "aiffproperties.h"
#include "apefile.h"
#include "apefooter.h"
#include "apeitem.h"
#include "apeproperties.h"
#include "apetag.h"
#include "asfattribute.h"
#include "asffile.h"
#include "asfpicture.h"
#include "asfproperties.h"
#include "asftag.h"
#include "attachedpictureframe.h"
#include "audioproperties.h"
#include "chapterframe.h"
#include "commentsframe.h"
#include "dsffile.h"
#include "dsfproperties.h"
#include "dsdifffile.h"
#include "dsdiffproperties.h"
#include "eventtimingcodesframe.h"
#include "fileref.h"
#include "flacfile.h"
#include "flacmetadatablock.h"
#include "flacunknownmetadatablock.h"
#include "flacpicture.h"
#include "flacproperties.h"
#include "generalencapsulatedobjectframe.h"
#include "id3v1tag.h"
#include "id3v2extendedheader.h"
#include "id3v2footer.h"
#include "id3v2frame.h"
#include "id3v2framefactory.h"
#include "id3v2header.h"
#include "id3v2tag.h"
#include "infotag.h"
#include "itfile.h"
#include "itproperties.h"
#include "modfile.h"
#include "modfilebase.h"
#include "modproperties.h"
#include "modtag.h"
#include "mp4coverart.h"
#include "mp4file.h"
#include "mp4item.h"
#include "mp4itemfactory.h"
#include "mp4properties.h"
#include "mp4tag.h"
#include "mpcfile.h"
#include "mpcproperties.h"
#include "mpegfile.h"
#include "mpegheader.h"
#include "mpegproperties.h"
#include "oggfile.h"
#include "oggflacfile.h"
#include "oggpage.h"
#include "oggpageheader.h"
#include "opusfile.h"
#include "opusproperties.h"
#include "ownershipframe.h"
#include "podcastframe.h"
#include "popularimeterframe.h"
#include "privateframe.h"
#include "relativevolumeframe.h"
#include "rifffile.h"
#include "s3mfile.h"
#include "s3mproperties.h"
#include "shortenfile.h"
#include "shortenproperties.h"
#include "shortentag.h"
#include "speexfile.h"
#include "speexproperties.h"
#include "synchronizedlyricsframe.h"
#include "tableofcontentsframe.h"
#include "tag.h"
#include "tbytevector.h"
#include "tbytevectorlist.h"
#include "tbytevectorstream.h"
#include "tdebuglistener.h"
#include "textidentificationframe.h"
#include "tfile.h"
#include "tfilestream.h"
#include "tiostream.h"
#include "tlist.h"
#include "tmap.h"
#include "tpropertymap.h"
#include "trueaudiofile.h"
#include "trueaudioproperties.h"
#include "tstring.h"
#include "tstringlist.h"
#include "uniquefileidentifierframe.h"
#include "unknownframe.h"
#include "unsynchronizedlyricsframe.h"
#include "urllinkframe.h"
#include "vorbisfile.h"
#include "vorbisproperties.h"
#include "wavfile.h"
#include "wavpackfile.h"
#include "wavpackproperties.h"
#include "wavproperties.h"
#include "xingheader.h"
#include "xiphcomment.h"
#include "xmfile.h"
#include "xmproperties.h"

#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace TagLib;

class TestSizes : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestSizes);
    CPPUNIT_TEST(testSizes);
    CPPUNIT_TEST_SUITE_END();

public:
    void testSizes()
    {
        // Class list was built by generating XML docs with Doxygen, and then running:
        // $ grep kind=\"class\" index.xml | sed -E -e 's/(.*<name>|<\/name>.*)//g'

        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::APE::File));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::APE::Footer));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::APE::Item));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::APE::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::APE::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::ASF::Attribute));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ASF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::ASF::Picture));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ASF::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ASF::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::AudioProperties));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::ByteVector));
        CPPUNIT_ASSERT_EQUAL(classSize(2, false), sizeof(TagLib::ByteVectorList));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ByteVectorStream));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::DebugListener));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSDIFF::DIIN::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSDIFF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSDIFF::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FLAC::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSF::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::FLAC::MetadataBlock));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FLAC::Picture));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FLAC::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FLAC::UnknownMetadataBlock));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::FileRef));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::FileRef::FileTypeResolver));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FileRef::StreamTypeResolver));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FileStream));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::ID3v1::StringHandler));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v1::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::AttachedPictureFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::AttachedPictureFrameV22));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::ChapterFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::CommentsFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::EventTimingCodesFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::ID3v2::ExtendedHeader));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::ID3v2::Footer));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::ID3v2::Frame));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::ID3v2::FrameFactory));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::GeneralEncapsulatedObjectFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::ID3v2::Header));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::ID3v2::Latin1StringHandler));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::OwnershipFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::PodcastFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::PopularimeterFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::PrivateFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::RelativeVolumeFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::SynchronizedLyricsFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::TableOfContentsFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::TextIdentificationFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::UniqueFileIdentifierFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::UnknownFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::UnsynchronizedLyricsFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ID3v2::UrlLinkFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::ID3v2::UserTextIdentificationFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::ID3v2::UserUrlLinkFrame));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::IOStream));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::IT::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::IT::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::List<int>));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::MP4::CoverArt));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MP4::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::MP4::Item));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::MP4::ItemFactory));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MP4::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MP4::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MPC::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MPC::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MPEG::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::MPEG::Header));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MPEG::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::MPEG::XingHeader));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::Map<int, int>));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::Mod::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Mod::FileBase));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Mod::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Mod::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::Ogg::FLAC::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Ogg::File));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::Ogg::Opus::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Ogg::Opus::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::Ogg::Page));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::Ogg::PageHeader));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::Ogg::Speex::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Ogg::Speex::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::Ogg::Vorbis::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Ogg::Vorbis::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Ogg::XiphComment));
        CPPUNIT_ASSERT_EQUAL(classSize(2, false), sizeof(TagLib::PropertyMap));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::RIFF::AIFF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::RIFF::AIFF::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::RIFF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::RIFF::Info::StringHandler));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::RIFF::Info::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::RIFF::WAV::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::RIFF::WAV::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::S3M::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::S3M::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Shorten::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Shorten::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Shorten::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::String));
        CPPUNIT_ASSERT_EQUAL(classSize(2, false), sizeof(TagLib::StringList));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::TrueAudio::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::TrueAudio::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::WavPack::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::WavPack::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::XM::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::XM::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::Variant));
    }

private:
    constexpr size_t classSize(int baseClasses, bool isVirtual)
    {
        return sizeof(void *) * (baseClasses + static_cast<int>(isVirtual) + 1);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSizes);
