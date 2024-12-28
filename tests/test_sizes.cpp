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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstring>

#ifdef WITH_RIFF
#include "aifffile.h"
#include "aiffproperties.h"
#endif
#ifdef WITH_APE
#include "apefile.h"
#include "apefooter.h"
#include "apeitem.h"
#include "apeproperties.h"
#include "apetag.h"
#endif
#ifdef WITH_ASF
#include "asfattribute.h"
#include "asffile.h"
#include "asfpicture.h"
#include "asfproperties.h"
#include "asftag.h"
#endif
#include "attachedpictureframe.h"
#include "audioproperties.h"
#include "chapterframe.h"
#include "commentsframe.h"
#ifdef WITH_DSF
#include "dsffile.h"
#include "dsfproperties.h"
#include "dsdifffile.h"
#include "dsdiffproperties.h"
#endif
#include "eventtimingcodesframe.h"
#include "fileref.h"
#ifdef WITH_VORBIS
#include "flacfile.h"
#include "flacmetadatablock.h"
#include "flacunknownmetadatablock.h"
#include "flacpicture.h"
#include "flacproperties.h"
#endif
#include "generalencapsulatedobjectframe.h"
#include "id3v1tag.h"
#include "id3v2extendedheader.h"
#include "id3v2footer.h"
#include "id3v2frame.h"
#include "id3v2framefactory.h"
#include "id3v2header.h"
#include "id3v2tag.h"
#ifdef WITH_RIFF
#include "infotag.h"
#endif
#ifdef WITH_MOD
#include "itfile.h"
#include "itproperties.h"
#include "modfile.h"
#include "modfilebase.h"
#include "modproperties.h"
#include "modtag.h"
#endif
#ifdef WITH_MP4
#include "mp4coverart.h"
#include "mp4file.h"
#include "mp4item.h"
#include "mp4itemfactory.h"
#include "mp4properties.h"
#include "mp4tag.h"
#endif
#ifdef WITH_APE
#include "mpcfile.h"
#include "mpcproperties.h"
#endif
#include "mpegfile.h"
#include "mpegheader.h"
#include "mpegproperties.h"
#ifdef WITH_VORBIS
#include "oggfile.h"
#include "oggflacfile.h"
#include "oggpage.h"
#include "oggpageheader.h"
#include "opusfile.h"
#include "opusproperties.h"
#endif
#include "ownershipframe.h"
#include "podcastframe.h"
#include "popularimeterframe.h"
#include "privateframe.h"
#include "relativevolumeframe.h"
#ifdef WITH_RIFF
#include "rifffile.h"
#endif
#ifdef WITH_MOD
#include "s3mfile.h"
#include "s3mproperties.h"
#endif
#ifdef WITH_SHORTEN
#include "shortenfile.h"
#include "shortenproperties.h"
#include "shortentag.h"
#endif
#ifdef WITH_VORBIS
#include "speexfile.h"
#include "speexproperties.h"
#endif
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
#ifdef WITH_TRUEAUDIO
#include "trueaudiofile.h"
#include "trueaudioproperties.h"
#endif
#include "tstring.h"
#include "tstringlist.h"
#include "uniquefileidentifierframe.h"
#include "unknownframe.h"
#include "unsynchronizedlyricsframe.h"
#include "urllinkframe.h"
#ifdef WITH_VORBIS
#include "vorbisfile.h"
#include "vorbisproperties.h"
#endif
#ifdef WITH_RIFF
#include "wavfile.h"
#endif
#ifdef WITH_APE
#include "wavpackfile.h"
#include "wavpackproperties.h"
#endif
#ifdef WITH_RIFF
#include "wavproperties.h"
#endif
#include "xingheader.h"
#ifdef WITH_VORBIS
#include "xiphcomment.h"
#endif
#ifdef WITH_MOD
#include "xmfile.h"
#include "xmproperties.h"
#endif

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

#ifdef WITH_APE
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::APE::File));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::APE::Footer));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::APE::Item));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::APE::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::APE::Tag));
#endif
#ifdef WITH_ASF
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::ASF::Attribute));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ASF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::ASF::Picture));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ASF::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ASF::Tag));
#endif
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::AudioProperties));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::ByteVector));
        CPPUNIT_ASSERT_EQUAL(classSize(2, false), sizeof(TagLib::ByteVectorList));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::ByteVectorStream));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::DebugListener));
#ifdef WITH_DSF
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSDIFF::DIIN::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSDIFF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSDIFF::Properties));
#endif
#ifdef WITH_VORBIS
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FLAC::File));
#endif
#ifdef WITH_DSF
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::DSF::Properties));
#endif
#ifdef WITH_VORBIS
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::FLAC::MetadataBlock));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FLAC::Picture));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FLAC::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::FLAC::UnknownMetadataBlock));
#endif
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
#ifdef WITH_MOD
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::IT::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::IT::Properties));
#endif
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::List<int>));
#ifdef WITH_MP4
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::MP4::CoverArt));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MP4::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::MP4::Item));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::MP4::ItemFactory));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MP4::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MP4::Tag));
#endif
#ifdef WITH_APE
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MPC::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MPC::Properties));
#endif
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MPEG::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::MPEG::Header));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::MPEG::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(0, false), sizeof(TagLib::MPEG::XingHeader));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::Map<int, int>));
#ifdef WITH_MOD
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::Mod::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Mod::FileBase));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Mod::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Mod::Tag));
#endif
#ifdef WITH_VORBIS
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
#endif
        CPPUNIT_ASSERT_EQUAL(classSize(2, false), sizeof(TagLib::PropertyMap));
#ifdef WITH_RIFF
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::RIFF::AIFF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::RIFF::AIFF::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::RIFF::File));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::RIFF::Info::StringHandler));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::RIFF::Info::Tag));
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::RIFF::WAV::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::RIFF::WAV::Properties));
#endif
#ifdef WITH_MOD
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::S3M::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::S3M::Properties));
#endif
#ifdef WITH_SHORTEN
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Shorten::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Shorten::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::Shorten::Tag));
#endif
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::String));
        CPPUNIT_ASSERT_EQUAL(classSize(2, false), sizeof(TagLib::StringList));
        CPPUNIT_ASSERT_EQUAL(classSize(0, true), sizeof(TagLib::Tag));
#ifdef WITH_TRUEAUDIO
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::TrueAudio::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::TrueAudio::Properties));
#endif
#ifdef WITH_APE
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::WavPack::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::WavPack::Properties));
#endif
#ifdef WITH_MOD
        CPPUNIT_ASSERT_EQUAL(classSize(2, true), sizeof(TagLib::XM::File));
        CPPUNIT_ASSERT_EQUAL(classSize(1, true), sizeof(TagLib::XM::Properties));
        CPPUNIT_ASSERT_EQUAL(classSize(1, false), sizeof(TagLib::Variant));
#endif
    }

private:
    constexpr size_t classSize(int baseClasses, bool isVirtual)
    {
        return sizeof(void *) * (baseClasses + static_cast<int>(isVirtual) + 1);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSizes);
