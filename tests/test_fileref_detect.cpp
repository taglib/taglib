/***************************************************************************
    copyright           : (C) 2026 by TagLib developers
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

#include "taglib_config.h"
#include "tfilestream.h"
#include "tbytevectorstream.h"
#include "fileref.h"
#include "mpegfile.h"
#ifdef TAGLIB_WITH_VORBIS
#include "flacfile.h"
#include "oggflacfile.h"
#include "opusfile.h"
#include "speexfile.h"
#include "vorbisfile.h"
#endif
#ifdef TAGLIB_WITH_APE
#include "apefile.h"
#include "mpcfile.h"
#include "wavpackfile.h"
#endif
#ifdef TAGLIB_WITH_ASF
#include "asffile.h"
#endif
#ifdef TAGLIB_WITH_TRUEAUDIO
#include "trueaudiofile.h"
#endif
#ifdef TAGLIB_WITH_MP4
#include "mp4file.h"
#endif
#ifdef TAGLIB_WITH_RIFF
#include "aifffile.h"
#include "wavfile.h"
#endif
#ifdef TAGLIB_WITH_DSF
#include "dsdifffile.h"
#include "dsffile.h"
#endif
#ifdef TAGLIB_WITH_SHORTEN
#include "shortenfile.h"
#endif
#ifdef TAGLIB_WITH_MATROSKA
#include "matroskafile.h"
#endif
#include <cppunit/extensions/HelperMacros.h>
#include "utils.h"

using namespace TagLib;

// Files not covered by detection tests and the reason why:
// (All of these return null because no format's isSupported() matches them)
//
// MOD/S3M/IT/XM formats have no isSupported() implementation at all,
// so content-based detection is impossible for these files:
//   changed.mod, test.mod, changed.s3m, test.s3m, test.it,
//   changed.xm, test.xm, stripped.xm
//
// bare ID3 tag data without any surrounding audio stream:
//   005411.id3, broken-tenc.id3, unsynch.id3
//
// null bytes / truly unsupported binary format:
//   no-extension, unsupported-extension.xx
//
// .mp3-named files where MPEG::File::isSupported() returns false because the
// MPEG frame scanner cannot find any valid frames in the content:
//   garbage.mp3 (random binary data with no MPEG sync bytes),
//   compressed_id3_frame.mp3 (zlib-compressed ID3 frame inflates to garbage
//                              that the frame scanner cannot parse past),
//   duplicate_id3v2.mp3 (two consecutive ID3v2 headers confuse the size
//                         calculation, shifting the scan past any real frames),
//   excessive_alloc.mp3 (APIC frame carries a crafted huge size field that
//                         the ID3v2 skip overshoots the actual frames),
//   extended-header.mp3 (ID3v2.4 extended header flag causes incorrect size
//                         skip so the scanner starts inside the header),
//   w000.mp3 (malformed file with no discoverable MPEG sync bytes)
//
// MPC SV4/SV5: MPC::File::isSupported() only recognises "MPCK" (SV8) and
// "MP+" (SV7); older SV4/SV5 streams have no standardised magic bytes:
//   sv4_header.mpc, sv5_header.mpc
//
// MP4 with 64-bit atom sizes: first box is "moov" rather than the required
// "ftyp", so MP4::File::isSupported() returns false:
//   64bit.mp4
//
// corrupt AIFF: the FORM header is present but the sub-type bytes at offset 8
// are garbled (0x80 0x46 instead of 'AIFF'/'AIFC'), so
// RIFF::AIFF::File::isSupported() returns false:
//   excessive_alloc.aif

namespace {

  template <typename T> void detectByContent(const char *testFile) {
    FileStream fs(TEST_FILE_PATH_C(testFile));
    CPPUNIT_ASSERT(fs.isOpen());
    const ByteVector data = fs.readBlock(fs.length());
    ByteVectorStream bvs(data);
    const FileRef f(&bvs);
    CPPUNIT_ASSERT(!f.isNull());
    CPPUNIT_ASSERT(dynamic_cast<T *>(f.file()) != nullptr);
  }

  void detectNullByContent(const char *testFile) {
    FileStream fs(TEST_FILE_PATH_C(testFile));
    CPPUNIT_ASSERT(fs.isOpen());
    const ByteVector data = fs.readBlock(fs.length());
    ByteVectorStream bvs(data);
    const FileRef f(&bvs);
    CPPUNIT_ASSERT(f.isNull());
  }

} // namespace

class TestFileRefDetectByContent : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFileRefDetectByContent);

  // MPEG (always available)
  CPPUNIT_TEST(testApeId3v1Mp3);
  CPPUNIT_TEST(testApeId3v2Mp3);
  CPPUNIT_TEST(testApeMp3);
  CPPUNIT_TEST(testBladeencMp3);
  CPPUNIT_TEST(testEmpty1sAac);
  CPPUNIT_TEST(testId3v22TdaMp3);
  CPPUNIT_TEST(testInvalidFrames1Mp3);
  CPPUNIT_TEST(testInvalidFrames2Mp3);
  CPPUNIT_TEST(testInvalidFrames3Mp3);
  CPPUNIT_TEST(testItunes10Mp3);
  CPPUNIT_TEST(testLameCbrMp3);
  CPPUNIT_TEST(testLameVbrMp3);
  CPPUNIT_TEST(testMpeg2Mp3);
  CPPUNIT_TEST(testRareFramesMp3);
  CPPUNIT_TEST(testTocManyChildrenMp3);
  CPPUNIT_TEST(testXingMp3);

#ifdef TAGLIB_WITH_VORBIS
  // Ogg::Vorbis::File
  CPPUNIT_TEST(testEmptyOgg);
  CPPUNIT_TEST(testEmptyVorbisOga);
  CPPUNIT_TEST(testLowercaseFieldsOgg);
  CPPUNIT_TEST(testTestOgg);
  // Ogg::FLAC::File
  CPPUNIT_TEST(testEmptyFlacOga);
  // FLAC::File
  CPPUNIT_TEST(testEmptySeektableFlac);
  CPPUNIT_TEST(testMultipleVcFlac);
  CPPUNIT_TEST(testNoTagsFlac);
  CPPUNIT_TEST(testSilence44SFlac);
  CPPUNIT_TEST(testSinewaveFlac);
  CPPUNIT_TEST(testZeroSizedPaddingFlac);
  CPPUNIT_TEST(testFLACWithMPEGSyncBytes);
  // Ogg::Speex::File
  CPPUNIT_TEST(testEmptySpx);
  // Ogg::Opus::File
  CPPUNIT_TEST(testCorrectnessGainSilentOutputOpus);
  // Corrupt files: isSupported() returns true but isValid() returns false
  CPPUNIT_TEST(testNullSegfaultOga);
#endif

#ifdef TAGLIB_WITH_APE
  // MPC::File
  CPPUNIT_TEST(testClickMpc);
  CPPUNIT_TEST(testInfloopMpc);
  CPPUNIT_TEST(testSegfaultMpc);
  CPPUNIT_TEST(testSegfault2Mpc);
  CPPUNIT_TEST(testSv8HeaderMpc);
  CPPUNIT_TEST(testZerodivMpc);
  // WavPack::File
  CPPUNIT_TEST(testClickWv);
  CPPUNIT_TEST(testDsdStereoWv);
  CPPUNIT_TEST(testFourChannelsWv);
  CPPUNIT_TEST(testInfloopWv);
  CPPUNIT_TEST(testNoLengthWv);
  CPPUNIT_TEST(testNonStandardRateWv);
  CPPUNIT_TEST(testTaggedWv);
  // APE::File
  CPPUNIT_TEST(testLongloopApe);
  CPPUNIT_TEST(testMac390HdrApe);
  CPPUNIT_TEST(testMac396Ape);
  CPPUNIT_TEST(testMac399Id3v2Ape);
  CPPUNIT_TEST(testMac399TaggedApe);
  CPPUNIT_TEST(testMac399Ape);
  CPPUNIT_TEST(testZerodivApe);
#endif

#ifdef TAGLIB_WITH_TRUEAUDIO
  CPPUNIT_TEST(testEmptyTta);
  CPPUNIT_TEST(testTaggedTta);
#endif

#ifdef TAGLIB_WITH_MP4
  CPPUNIT_TEST(testBlankVideoM4v);
  CPPUNIT_TEST(testCovrJunkM4a);
  CPPUNIT_TEST(testEmptyAlacM4a);
  CPPUNIT_TEST(testGnreM4a);
  CPPUNIT_TEST(testHasTagsM4a);
  CPPUNIT_TEST(testIlstIsLastM4a);
  CPPUNIT_TEST(testInfloopM4a);
  CPPUNIT_TEST(testNoTags3g2);
  CPPUNIT_TEST(testNoTagsM4a);
  CPPUNIT_TEST(testNonFullMetaM4a);
  CPPUNIT_TEST(testNonprintableAtomTypeM4a);
  CPPUNIT_TEST(testZeroLengthMdatM4a);
#endif

#ifdef TAGLIB_WITH_ASF
  CPPUNIT_TEST(testLosslessWma);
  CPPUNIT_TEST(testSilence1Wma);
#endif

#ifdef TAGLIB_WITH_RIFF
  // RIFF::AIFF::File
  CPPUNIT_TEST(testAlawAifc);
  CPPUNIT_TEST(testDuplicateId3v2Aiff);
  CPPUNIT_TEST(testEmptyAiff);
  CPPUNIT_TEST(testNoiseAif);
  CPPUNIT_TEST(testNoiseOddAif);
  CPPUNIT_TEST(testSegfaultAif);
  // RIFF::WAV::File
  CPPUNIT_TEST(testAlawWav);
  CPPUNIT_TEST(testDuplicateTagsWav);
  CPPUNIT_TEST(testEmptyWav);
  CPPUNIT_TEST(testFloat64Wav);
  CPPUNIT_TEST(testInfloopWav);
  CPPUNIT_TEST(testInvalidChunkWav);
  CPPUNIT_TEST(testPcmWithFactChunkWav);
  CPPUNIT_TEST(testSegfaultWav);
  CPPUNIT_TEST(testUint8weWav);
  CPPUNIT_TEST(testZeroSizeChunkWav);
#endif

#ifdef TAGLIB_WITH_DSF
  CPPUNIT_TEST(testEmpty10msDsf);
  CPPUNIT_TEST(testEmpty10msDff);
#endif

#ifdef TAGLIB_WITH_SHORTEN
  CPPUNIT_TEST(test2SecSilenceShn);
#endif

#ifdef TAGLIB_WITH_MATROSKA
  CPPUNIT_TEST(testNoTagsMka);
  CPPUNIT_TEST(testNoTagsWebm);
  CPPUNIT_TEST(testOptimizedMkv);
  CPPUNIT_TEST(testTagsBeforeCuesMkv);
#endif

  CPPUNIT_TEST_SUITE_END();

public:
  // -- MPEG::File (always available) --

  void testApeId3v1Mp3() { detectByContent<MPEG::File>("ape-id3v1.mp3"); }
  void testApeId3v2Mp3() { detectByContent<MPEG::File>("ape-id3v2.mp3"); }
  void testApeMp3() { detectByContent<MPEG::File>("ape.mp3"); }
  void testBladeencMp3() { detectByContent<MPEG::File>("bladeenc.mp3"); }
  void testEmpty1sAac() { detectByContent<MPEG::File>("empty1s.aac"); }
  void testId3v22TdaMp3() { detectByContent<MPEG::File>("id3v22-tda.mp3"); }
  void testInvalidFrames1Mp3() {
    detectByContent<MPEG::File>("invalid-frames1.mp3");
  }
  void testInvalidFrames2Mp3() {
    detectByContent<MPEG::File>("invalid-frames2.mp3");
  }
  void testInvalidFrames3Mp3() {
    detectByContent<MPEG::File>("invalid-frames3.mp3");
  }
  void testItunes10Mp3() { detectByContent<MPEG::File>("itunes10.mp3"); }
  void testLameCbrMp3() { detectByContent<MPEG::File>("lame_cbr.mp3"); }
  void testLameVbrMp3() { detectByContent<MPEG::File>("lame_vbr.mp3"); }
  void testMpeg2Mp3() { detectByContent<MPEG::File>("mpeg2.mp3"); }
  void testRareFramesMp3() {
    detectByContent<MPEG::File>("rare_frames.mp3");
  }
  void testTocManyChildrenMp3() {
    detectByContent<MPEG::File>("toc_many_children.mp3");
  }
  void testXingMp3() { detectByContent<MPEG::File>("xing.mp3"); }

#ifdef TAGLIB_WITH_VORBIS
  // -- Ogg::Vorbis::File --
  void testEmptyOgg() { detectByContent<Ogg::Vorbis::File>("empty.ogg"); }
  void testEmptyVorbisOga() {
    detectByContent<Ogg::Vorbis::File>("empty_vorbis.oga");
  }
  void testLowercaseFieldsOgg() {
    detectByContent<Ogg::Vorbis::File>("lowercase-fields.ogg");
  }
  void testTestOgg() { detectByContent<Ogg::Vorbis::File>("test.ogg"); }

  // -- Ogg::FLAC::File --
  void testEmptyFlacOga() {
    detectByContent<Ogg::FLAC::File>("empty_flac.oga");
  }

  // -- FLAC::File --
  void testEmptySeektableFlac() {
    detectByContent<FLAC::File>("empty-seektable.flac");
  }
  void testMultipleVcFlac() {
    detectByContent<FLAC::File>("multiple-vc.flac");
  }
  void testNoTagsFlac() { detectByContent<FLAC::File>("no-tags.flac"); }
  void testSilence44SFlac() {
    detectByContent<FLAC::File>("silence-44-s.flac");
  }
  void testSinewaveFlac() { detectByContent<FLAC::File>("sinewave.flac"); }
  void testZeroSizedPaddingFlac() {
    detectByContent<FLAC::File>("zero-sized-padding.flac");
  }
  void testFLACWithMPEGSyncBytes() {
    detectByContent<FLAC::File>("mpeg-sync-flac.flac");
  }

  // -- Ogg::Speex::File --
  void testEmptySpx() { detectByContent<Ogg::Speex::File>("empty.spx"); }

  // -- Ogg::Opus::File --
  void testCorrectnessGainSilentOutputOpus() {
    detectByContent<Ogg::Opus::File>("correctness_gain_silent_output.opus");
  }

  // segfault.oga: Ogg::FLAC::File::isSupported() returns true (valid Ogg
  // container with a fLaC marker), but the FLAC metadata header inside is
  // corrupt so Ogg::FLAC::File::isValid() returns false.
  void testNullSegfaultOga() { detectNullByContent("segfault.oga"); }
#endif

#ifdef TAGLIB_WITH_APE
  // -- MPC::File --
  void testClickMpc() { detectByContent<MPC::File>("click.mpc"); }
  void testInfloopMpc() { detectByContent<MPC::File>("infloop.mpc"); }
  void testSegfaultMpc() { detectByContent<MPC::File>("segfault.mpc"); }
  void testSegfault2Mpc() { detectByContent<MPC::File>("segfault2.mpc"); }
  void testSv8HeaderMpc() { detectByContent<MPC::File>("sv8_header.mpc"); }
  void testZerodivMpc() { detectByContent<MPC::File>("zerodiv.mpc"); }

  // -- WavPack::File --
  void testClickWv() { detectByContent<WavPack::File>("click.wv"); }
  void testDsdStereoWv() { detectByContent<WavPack::File>("dsd_stereo.wv"); }
  void testFourChannelsWv() {
    detectByContent<WavPack::File>("four_channels.wv");
  }
  void testInfloopWv() { detectByContent<WavPack::File>("infloop.wv"); }
  void testNoLengthWv() { detectByContent<WavPack::File>("no_length.wv"); }
  void testNonStandardRateWv() {
    detectByContent<WavPack::File>("non_standard_rate.wv");
  }
  void testTaggedWv() { detectByContent<WavPack::File>("tagged.wv"); }

  // -- APE::File --
  void testLongloopApe() { detectByContent<APE::File>("longloop.ape"); }
  void testMac390HdrApe() { detectByContent<APE::File>("mac-390-hdr.ape"); }
  void testMac396Ape() { detectByContent<APE::File>("mac-396.ape"); }
  void testMac399Id3v2Ape() {
    detectByContent<APE::File>("mac-399-id3v2.ape");
  }
  void testMac399TaggedApe() {
    detectByContent<APE::File>("mac-399-tagged.ape");
  }
  void testMac399Ape() { detectByContent<APE::File>("mac-399.ape"); }
  void testZerodivApe() { detectByContent<APE::File>("zerodiv.ape"); }
#endif

#ifdef TAGLIB_WITH_TRUEAUDIO
  // -- TrueAudio::File --
  void testEmptyTta() { detectByContent<TrueAudio::File>("empty.tta"); }
  void testTaggedTta() { detectByContent<TrueAudio::File>("tagged.tta"); }
#endif

#ifdef TAGLIB_WITH_MP4
  // -- MP4::File --
  void testBlankVideoM4v() { detectByContent<MP4::File>("blank_video.m4v"); }
  void testCovrJunkM4a() { detectByContent<MP4::File>("covr-junk.m4a"); }
  void testEmptyAlacM4a() { detectByContent<MP4::File>("empty_alac.m4a"); }
  void testGnreM4a() { detectByContent<MP4::File>("gnre.m4a"); }
  void testHasTagsM4a() { detectByContent<MP4::File>("has-tags.m4a"); }
  void testIlstIsLastM4a() {
    detectByContent<MP4::File>("ilst-is-last.m4a");
  }
  void testInfloopM4a() { detectByContent<MP4::File>("infloop.m4a"); }
  void testNoTags3g2() { detectByContent<MP4::File>("no-tags.3g2"); }
  void testNoTagsM4a() { detectByContent<MP4::File>("no-tags.m4a"); }
  void testNonFullMetaM4a() {
    detectByContent<MP4::File>("non-full-meta.m4a");
  }
  void testNonprintableAtomTypeM4a() {
    detectByContent<MP4::File>("nonprintable-atom-type.m4a");
  }
  void testZeroLengthMdatM4a() {
    detectByContent<MP4::File>("zero-length-mdat.m4a");
  }
#endif

#ifdef TAGLIB_WITH_ASF
  // -- ASF::File --
  void testLosslessWma() { detectByContent<ASF::File>("lossless.wma"); }
  void testSilence1Wma() { detectByContent<ASF::File>("silence-1.wma"); }
#endif

#ifdef TAGLIB_WITH_RIFF
  // -- RIFF::AIFF::File --
  void testAlawAifc() { detectByContent<RIFF::AIFF::File>("alaw.aifc"); }
  void testDuplicateId3v2Aiff() {
    detectByContent<RIFF::AIFF::File>("duplicate_id3v2.aiff");
  }
  void testEmptyAiff() { detectByContent<RIFF::AIFF::File>("empty.aiff"); }
  void testNoiseAif() { detectByContent<RIFF::AIFF::File>("noise.aif"); }
  void testNoiseOddAif() {
    detectByContent<RIFF::AIFF::File>("noise_odd.aif");
  }
  void testSegfaultAif() {
    detectByContent<RIFF::AIFF::File>("segfault.aif");
  }

  // -- RIFF::WAV::File --
  void testAlawWav() { detectByContent<RIFF::WAV::File>("alaw.wav"); }
  void testDuplicateTagsWav() {
    detectByContent<RIFF::WAV::File>("duplicate_tags.wav");
  }
  void testEmptyWav() { detectByContent<RIFF::WAV::File>("empty.wav"); }
  void testFloat64Wav() { detectByContent<RIFF::WAV::File>("float64.wav"); }
  void testInfloopWav() { detectByContent<RIFF::WAV::File>("infloop.wav"); }
  void testInvalidChunkWav() {
    detectByContent<RIFF::WAV::File>("invalid-chunk.wav");
  }
  void testPcmWithFactChunkWav() {
    detectByContent<RIFF::WAV::File>("pcm_with_fact_chunk.wav");
  }
  void testSegfaultWav() { detectByContent<RIFF::WAV::File>("segfault.wav"); }
  void testUint8weWav() { detectByContent<RIFF::WAV::File>("uint8we.wav"); }
  void testZeroSizeChunkWav() {
    detectByContent<RIFF::WAV::File>("zero-size-chunk.wav");
  }
#endif

#ifdef TAGLIB_WITH_DSF
  // -- DSF::File --
  void testEmpty10msDsf() { detectByContent<DSF::File>("empty10ms.dsf"); }

  // -- DSDIFF::File --
  void testEmpty10msDff() { detectByContent<DSDIFF::File>("empty10ms.dff"); }
#endif

#ifdef TAGLIB_WITH_SHORTEN
  // -- Shorten::File --
  void test2SecSilenceShn() {
    detectByContent<Shorten::File>("2sec-silence.shn");
  }
#endif

#ifdef TAGLIB_WITH_MATROSKA
  // -- Matroska::File --
  void testNoTagsMka() { detectByContent<Matroska::File>("no-tags.mka"); }
  void testNoTagsWebm() { detectByContent<Matroska::File>("no-tags.webm"); }
  void testOptimizedMkv() {
    detectByContent<Matroska::File>("optimized.mkv");
  }
  void testTagsBeforeCuesMkv() {
    detectByContent<Matroska::File>("tags-before-cues.mkv");
  }
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFileRefDetectByContent);
