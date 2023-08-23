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
#include "eventtimingcodesframe.h"
#include "fileref.h"
#include "flacfile.h"
#include "flacmetadatablock.h"
#include "flacpicture.h"
#include "flacproperties.h"
#include "flacunknownmetadatablock.h"
#include "generalencapsulatedobjectframe.h"
#include "id3v1genres.h"
#include "id3v1tag.h"
#include "id3v2.h"
#include "id3v2extendedheader.h"
#include "id3v2footer.h"
#include "id3v2frame.h"
#include "id3v2framefactory.h"
#include "id3v2header.h"
#include "id3v2synchdata.h"
#include "id3v2tag.h"
#include "infotag.h"
#include "itfile.h"
#include "itproperties.h"
#include "modfile.h"
#include "modfilebase.h"
#include "modproperties.h"
#include "modtag.h"
#include "mp4atom.h"
#include "mp4coverart.h"
#include "mp4file.h"
#include "mp4item.h"
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

#include <cstring>
#include <gtest/gtest.h>

using namespace std;
using namespace TagLib;

static constexpr size_t classSize(int baseClasses, bool isVirtual)
{
  return sizeof(void *) * (baseClasses + static_cast<int>(isVirtual) + 1);
}

TEST(Sizes, testSizes)
{
  // Class list was built by generating XML docs with Doxygen, and then running:
  // $ grep kind=\"class\" index.xml | sed -E -e 's/(.*<name>|<\/name>.*)//g'

  ASSERT_EQ(classSize(1, true), sizeof(TagLib::APE::File));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::APE::Footer));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::APE::Item));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::APE::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::APE::Tag));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ASF::Attribute));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ASF::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ASF::Picture));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ASF::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ASF::Tag));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::AudioProperties));
  ASSERT_EQ(classSize(0, false), sizeof(TagLib::ByteVector));
  ASSERT_EQ(classSize(2, false), sizeof(TagLib::ByteVectorList));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ByteVectorStream));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::DebugListener));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::FLAC::File));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::FLAC::MetadataBlock));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::FLAC::Picture));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::FLAC::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::FLAC::UnknownMetadataBlock));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::FileRef));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::FileRef::FileTypeResolver));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::FileRef::StreamTypeResolver));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::FileStream));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::ID3v1::StringHandler));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v1::Tag));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::AttachedPictureFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::AttachedPictureFrameV22));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::ChapterFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::CommentsFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::EventTimingCodesFrame));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::ID3v2::ExtendedHeader));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::ID3v2::Footer));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::ID3v2::Frame));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::ID3v2::FrameFactory));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::GeneralEncapsulatedObjectFrame));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::ID3v2::Header));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::ID3v2::Latin1StringHandler));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::OwnershipFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::PodcastFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::PopularimeterFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::PrivateFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::RelativeVolumeFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::SynchronizedLyricsFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::TableOfContentsFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::Tag));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::TextIdentificationFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::UniqueFileIdentifierFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::UnknownFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::UnsynchronizedLyricsFrame));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::ID3v2::UrlLinkFrame));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::ID3v2::UserTextIdentificationFrame));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::ID3v2::UserUrlLinkFrame));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::IOStream));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::IT::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::IT::Properties));
  ASSERT_EQ(classSize(1, false), sizeof(TagLib::List<int>));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MP4::CoverArt));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MP4::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MP4::Item));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MP4::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MP4::Tag));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MPC::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MPC::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MPEG::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MPEG::Header));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::MPEG::Properties));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::MPEG::XingHeader));
  ASSERT_EQ(classSize(1, false), sizeof(TagLib::Map<int, int>));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::Mod::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::Mod::FileBase));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::Mod::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::Mod::Tag));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::Ogg::FLAC::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::Ogg::File));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::Ogg::Opus::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::Ogg::Opus::Properties));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::Ogg::Page));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::Ogg::PageHeader));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::Ogg::Speex::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::Ogg::Speex::Properties));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::Ogg::Vorbis::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::Ogg::Vorbis::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::Ogg::XiphComment));
  ASSERT_EQ(classSize(2, false), sizeof(TagLib::PropertyMap));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::RIFF::AIFF::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::RIFF::AIFF::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::RIFF::File));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::RIFF::Info::StringHandler));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::RIFF::Info::Tag));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::RIFF::WAV::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::RIFF::WAV::Properties));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::S3M::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::S3M::Properties));
  ASSERT_EQ(classSize(1, false), sizeof(TagLib::String));
  ASSERT_EQ(classSize(2, false), sizeof(TagLib::StringList));
  ASSERT_EQ(classSize(0, true), sizeof(TagLib::Tag));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::TrueAudio::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::TrueAudio::Properties));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::WavPack::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::WavPack::Properties));
  ASSERT_EQ(classSize(2, true), sizeof(TagLib::XM::File));
  ASSERT_EQ(classSize(1, true), sizeof(TagLib::XM::Properties));
}
