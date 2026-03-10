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

#include <string>

#include "fileref.h"
#include "mpegfile.h"
#include "taglib_config.h"
#include "tbytevectorstream.h"
#include "tfilestream.h"

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
#include "utils.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
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

class TestFileRefDetectByContent : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestFileRefDetectByContent);

  // MPEG (always available)
  CPPUNIT_TEST(test_ape_id3v1_mp3);
  CPPUNIT_TEST(test_ape_id3v2_mp3);
  CPPUNIT_TEST(test_ape_mp3);
  CPPUNIT_TEST(test_bladeenc_mp3);
  CPPUNIT_TEST(test_empty1s_aac);
  CPPUNIT_TEST(test_id3v22_tda_mp3);
  CPPUNIT_TEST(test_invalid_frames1_mp3);
  CPPUNIT_TEST(test_invalid_frames2_mp3);
  CPPUNIT_TEST(test_invalid_frames3_mp3);
  CPPUNIT_TEST(test_itunes10_mp3);
  CPPUNIT_TEST(test_lame_cbr_mp3);
  CPPUNIT_TEST(test_lame_vbr_mp3);
  CPPUNIT_TEST(test_mpeg2_mp3);
  CPPUNIT_TEST(test_rare_frames_mp3);
  CPPUNIT_TEST(test_toc_many_children_mp3);
  CPPUNIT_TEST(test_xing_mp3);

#ifdef TAGLIB_WITH_VORBIS
  // Ogg::Vorbis::File
  CPPUNIT_TEST(test_empty_ogg);
  CPPUNIT_TEST(test_empty_vorbis_oga);
  CPPUNIT_TEST(test_lowercase_fields_ogg);
  CPPUNIT_TEST(test_test_ogg);
  // Ogg::FLAC::File
  CPPUNIT_TEST(test_empty_flac_oga);
  // FLAC::File
  CPPUNIT_TEST(test_empty_seektable_flac);
  CPPUNIT_TEST(test_multiple_vc_flac);
  CPPUNIT_TEST(test_no_tags_flac);
  CPPUNIT_TEST(test_silence_44_s_flac);
  CPPUNIT_TEST(test_sinewave_flac);
  CPPUNIT_TEST(test_zero_sized_padding_flac);
  // Ogg::Speex::File
  CPPUNIT_TEST(test_empty_spx);
  // Ogg::Opus::File
  CPPUNIT_TEST(test_correctness_gain_silent_output_opus);
  // Corrupt files: isSupported() returns true but isValid() returns false
  CPPUNIT_TEST(testNull_segfault_oga);
#endif

#ifdef TAGLIB_WITH_APE
  // MPC::File
  CPPUNIT_TEST(test_click_mpc);
  CPPUNIT_TEST(test_infloop_mpc);
  CPPUNIT_TEST(test_segfault_mpc);
  CPPUNIT_TEST(test_segfault2_mpc);
  CPPUNIT_TEST(test_sv8_header_mpc);
  CPPUNIT_TEST(test_zerodiv_mpc);
  // WavPack::File
  CPPUNIT_TEST(test_click_wv);
  CPPUNIT_TEST(test_dsd_stereo_wv);
  CPPUNIT_TEST(test_four_channels_wv);
  CPPUNIT_TEST(test_infloop_wv);
  CPPUNIT_TEST(test_no_length_wv);
  CPPUNIT_TEST(test_non_standard_rate_wv);
  CPPUNIT_TEST(test_tagged_wv);
  // APE::File
  CPPUNIT_TEST(test_longloop_ape);
  CPPUNIT_TEST(test_mac_390_hdr_ape);
  CPPUNIT_TEST(test_mac_396_ape);
  CPPUNIT_TEST(test_mac_399_id3v2_ape);
  CPPUNIT_TEST(test_mac_399_tagged_ape);
  CPPUNIT_TEST(test_mac_399_ape);
  CPPUNIT_TEST(test_zerodiv_ape);
#endif

#ifdef TAGLIB_WITH_TRUEAUDIO
  CPPUNIT_TEST(test_empty_tta);
  CPPUNIT_TEST(test_tagged_tta);
#endif

#ifdef TAGLIB_WITH_MP4
  CPPUNIT_TEST(test_blank_video_m4v);
  CPPUNIT_TEST(test_covr_junk_m4a);
  CPPUNIT_TEST(test_empty_alac_m4a);
  CPPUNIT_TEST(test_gnre_m4a);
  CPPUNIT_TEST(test_has_tags_m4a);
  CPPUNIT_TEST(test_ilst_is_last_m4a);
  CPPUNIT_TEST(test_infloop_m4a);
  CPPUNIT_TEST(test_no_tags_3g2);
  CPPUNIT_TEST(test_no_tags_m4a);
  CPPUNIT_TEST(test_non_full_meta_m4a);
  CPPUNIT_TEST(test_nonprintable_atom_type_m4a);
  CPPUNIT_TEST(test_zero_length_mdat_m4a);
#endif

#ifdef TAGLIB_WITH_ASF
  CPPUNIT_TEST(test_lossless_wma);
  CPPUNIT_TEST(test_silence_1_wma);
#endif

#ifdef TAGLIB_WITH_RIFF
  // RIFF::AIFF::File
  CPPUNIT_TEST(test_alaw_aifc);
  CPPUNIT_TEST(test_duplicate_id3v2_aiff);
  CPPUNIT_TEST(test_empty_aiff);
  CPPUNIT_TEST(test_noise_aif);
  CPPUNIT_TEST(test_noise_odd_aif);
  CPPUNIT_TEST(test_segfault_aif);
  // RIFF::WAV::File
  CPPUNIT_TEST(test_alaw_wav);
  CPPUNIT_TEST(test_duplicate_tags_wav);
  CPPUNIT_TEST(test_empty_wav);
  CPPUNIT_TEST(test_float64_wav);
  CPPUNIT_TEST(test_infloop_wav);
  CPPUNIT_TEST(test_invalid_chunk_wav);
  CPPUNIT_TEST(test_pcm_with_fact_chunk_wav);
  CPPUNIT_TEST(test_segfault_wav);
  CPPUNIT_TEST(test_uint8we_wav);
  CPPUNIT_TEST(test_zero_size_chunk_wav);
#endif

#ifdef TAGLIB_WITH_DSF
  CPPUNIT_TEST(test_empty10ms_dsf);
  CPPUNIT_TEST(test_empty10ms_dff);
#endif

#ifdef TAGLIB_WITH_SHORTEN
  CPPUNIT_TEST(test_2sec_silence_shn);
#endif

#ifdef TAGLIB_WITH_MATROSKA
  CPPUNIT_TEST(test_no_tags_mka);
  CPPUNIT_TEST(test_no_tags_webm);
  CPPUNIT_TEST(test_optimized_mkv);
  CPPUNIT_TEST(test_tags_before_cues_mkv);
#endif

  CPPUNIT_TEST_SUITE_END();

public:
  template <typename T> void detectByContent(const char *testFile) {
    FileStream fs(TEST_FILE_PATH_C(testFile));
    CPPUNIT_ASSERT(fs.isOpen());
    ByteVector data = fs.readBlock(fs.length());
    ByteVectorStream bvs(data);
    FileRef f(&bvs);
    CPPUNIT_ASSERT(!f.isNull());
    CPPUNIT_ASSERT(dynamic_cast<T *>(f.file()) != nullptr);
  }

  void detectNullByContent(const char *testFile) {
    FileStream fs(TEST_FILE_PATH_C(testFile));
    CPPUNIT_ASSERT(fs.isOpen());
    ByteVector data = fs.readBlock(fs.length());
    ByteVectorStream bvs(data);
    FileRef f(&bvs);
    CPPUNIT_ASSERT(f.isNull());
  }

  // -- MPEG::File (always available) --

  void test_ape_id3v1_mp3() { detectByContent<MPEG::File>("ape-id3v1.mp3"); }
  void test_ape_id3v2_mp3() { detectByContent<MPEG::File>("ape-id3v2.mp3"); }
  void test_ape_mp3() { detectByContent<MPEG::File>("ape.mp3"); }
  void test_bladeenc_mp3() { detectByContent<MPEG::File>("bladeenc.mp3"); }
  void test_empty1s_aac() { detectByContent<MPEG::File>("empty1s.aac"); }
  void test_id3v22_tda_mp3() { detectByContent<MPEG::File>("id3v22-tda.mp3"); }
  void test_invalid_frames1_mp3() {
    detectByContent<MPEG::File>("invalid-frames1.mp3");
  }
  void test_invalid_frames2_mp3() {
    detectByContent<MPEG::File>("invalid-frames2.mp3");
  }
  void test_invalid_frames3_mp3() {
    detectByContent<MPEG::File>("invalid-frames3.mp3");
  }
  void test_itunes10_mp3() { detectByContent<MPEG::File>("itunes10.mp3"); }
  void test_lame_cbr_mp3() { detectByContent<MPEG::File>("lame_cbr.mp3"); }
  void test_lame_vbr_mp3() { detectByContent<MPEG::File>("lame_vbr.mp3"); }
  void test_mpeg2_mp3() { detectByContent<MPEG::File>("mpeg2.mp3"); }
  void test_rare_frames_mp3() {
    detectByContent<MPEG::File>("rare_frames.mp3");
  }
  void test_toc_many_children_mp3() {
    detectByContent<MPEG::File>("toc_many_children.mp3");
  }
  void test_xing_mp3() { detectByContent<MPEG::File>("xing.mp3"); }

#ifdef TAGLIB_WITH_VORBIS
  // -- Ogg::Vorbis::File --
  void test_empty_ogg() { detectByContent<Ogg::Vorbis::File>("empty.ogg"); }
  void test_empty_vorbis_oga() {
    detectByContent<Ogg::Vorbis::File>("empty_vorbis.oga");
  }
  void test_lowercase_fields_ogg() {
    detectByContent<Ogg::Vorbis::File>("lowercase-fields.ogg");
  }
  void test_test_ogg() { detectByContent<Ogg::Vorbis::File>("test.ogg"); }

  // -- Ogg::FLAC::File --
  void test_empty_flac_oga() {
    detectByContent<Ogg::FLAC::File>("empty_flac.oga");
  }

  // -- FLAC::File --
  void test_empty_seektable_flac() {
    detectByContent<FLAC::File>("empty-seektable.flac");
  }
  void test_multiple_vc_flac() {
    detectByContent<FLAC::File>("multiple-vc.flac");
  }
  void test_no_tags_flac() { detectByContent<FLAC::File>("no-tags.flac"); }
  void test_silence_44_s_flac() {
    detectByContent<FLAC::File>("silence-44-s.flac");
  }
  void test_sinewave_flac() { detectByContent<FLAC::File>("sinewave.flac"); }
  void test_zero_sized_padding_flac() {
    detectByContent<FLAC::File>("zero-sized-padding.flac");
  }

  // -- Ogg::Speex::File --
  void test_empty_spx() { detectByContent<Ogg::Speex::File>("empty.spx"); }

  // -- Ogg::Opus::File --
  void test_correctness_gain_silent_output_opus() {
    detectByContent<Ogg::Opus::File>("correctness_gain_silent_output.opus");
  }

  // segfault.oga: Ogg::FLAC::File::isSupported() returns true (valid Ogg
  // container with a fLaC marker), but the FLAC metadata header inside is
  // corrupt so Ogg::FLAC::File::isValid() returns false.
  void testNull_segfault_oga() { detectNullByContent("segfault.oga"); }
#endif

#ifdef TAGLIB_WITH_APE
  // -- MPC::File --
  void test_click_mpc() { detectByContent<MPC::File>("click.mpc"); }
  void test_infloop_mpc() { detectByContent<MPC::File>("infloop.mpc"); }
  void test_segfault_mpc() { detectByContent<MPC::File>("segfault.mpc"); }
  void test_segfault2_mpc() { detectByContent<MPC::File>("segfault2.mpc"); }
  void test_sv8_header_mpc() { detectByContent<MPC::File>("sv8_header.mpc"); }
  void test_zerodiv_mpc() { detectByContent<MPC::File>("zerodiv.mpc"); }

  // -- WavPack::File --
  void test_click_wv() { detectByContent<WavPack::File>("click.wv"); }
  void test_dsd_stereo_wv() { detectByContent<WavPack::File>("dsd_stereo.wv"); }
  void test_four_channels_wv() {
    detectByContent<WavPack::File>("four_channels.wv");
  }
  void test_infloop_wv() { detectByContent<WavPack::File>("infloop.wv"); }
  void test_no_length_wv() { detectByContent<WavPack::File>("no_length.wv"); }
  void test_non_standard_rate_wv() {
    detectByContent<WavPack::File>("non_standard_rate.wv");
  }
  void test_tagged_wv() { detectByContent<WavPack::File>("tagged.wv"); }

  // -- APE::File --
  void test_longloop_ape() { detectByContent<APE::File>("longloop.ape"); }
  void test_mac_390_hdr_ape() { detectByContent<APE::File>("mac-390-hdr.ape"); }
  void test_mac_396_ape() { detectByContent<APE::File>("mac-396.ape"); }
  void test_mac_399_id3v2_ape() {
    detectByContent<APE::File>("mac-399-id3v2.ape");
  }
  void test_mac_399_tagged_ape() {
    detectByContent<APE::File>("mac-399-tagged.ape");
  }
  void test_mac_399_ape() { detectByContent<APE::File>("mac-399.ape"); }
  void test_zerodiv_ape() { detectByContent<APE::File>("zerodiv.ape"); }
#endif

#ifdef TAGLIB_WITH_TRUEAUDIO
  // -- TrueAudio::File --
  void test_empty_tta() { detectByContent<TrueAudio::File>("empty.tta"); }
  void test_tagged_tta() { detectByContent<TrueAudio::File>("tagged.tta"); }
#endif

#ifdef TAGLIB_WITH_MP4
  // -- MP4::File --
  void test_blank_video_m4v() { detectByContent<MP4::File>("blank_video.m4v"); }
  void test_covr_junk_m4a() { detectByContent<MP4::File>("covr-junk.m4a"); }
  void test_empty_alac_m4a() { detectByContent<MP4::File>("empty_alac.m4a"); }
  void test_gnre_m4a() { detectByContent<MP4::File>("gnre.m4a"); }
  void test_has_tags_m4a() { detectByContent<MP4::File>("has-tags.m4a"); }
  void test_ilst_is_last_m4a() {
    detectByContent<MP4::File>("ilst-is-last.m4a");
  }
  void test_infloop_m4a() { detectByContent<MP4::File>("infloop.m4a"); }
  void test_no_tags_3g2() { detectByContent<MP4::File>("no-tags.3g2"); }
  void test_no_tags_m4a() { detectByContent<MP4::File>("no-tags.m4a"); }
  void test_non_full_meta_m4a() {
    detectByContent<MP4::File>("non-full-meta.m4a");
  }
  void test_nonprintable_atom_type_m4a() {
    detectByContent<MP4::File>("nonprintable-atom-type.m4a");
  }
  void test_zero_length_mdat_m4a() {
    detectByContent<MP4::File>("zero-length-mdat.m4a");
  }
#endif

#ifdef TAGLIB_WITH_ASF
  // -- ASF::File --
  void test_lossless_wma() { detectByContent<ASF::File>("lossless.wma"); }
  void test_silence_1_wma() { detectByContent<ASF::File>("silence-1.wma"); }
#endif

#ifdef TAGLIB_WITH_RIFF
  // -- RIFF::AIFF::File --
  void test_alaw_aifc() { detectByContent<RIFF::AIFF::File>("alaw.aifc"); }
  void test_duplicate_id3v2_aiff() {
    detectByContent<RIFF::AIFF::File>("duplicate_id3v2.aiff");
  }
  void test_empty_aiff() { detectByContent<RIFF::AIFF::File>("empty.aiff"); }
  void test_noise_aif() { detectByContent<RIFF::AIFF::File>("noise.aif"); }
  void test_noise_odd_aif() {
    detectByContent<RIFF::AIFF::File>("noise_odd.aif");
  }
  void test_segfault_aif() {
    detectByContent<RIFF::AIFF::File>("segfault.aif");
  }

  // -- RIFF::WAV::File --
  void test_alaw_wav() { detectByContent<RIFF::WAV::File>("alaw.wav"); }
  void test_duplicate_tags_wav() {
    detectByContent<RIFF::WAV::File>("duplicate_tags.wav");
  }
  void test_empty_wav() { detectByContent<RIFF::WAV::File>("empty.wav"); }
  void test_float64_wav() { detectByContent<RIFF::WAV::File>("float64.wav"); }
  void test_infloop_wav() { detectByContent<RIFF::WAV::File>("infloop.wav"); }
  void test_invalid_chunk_wav() {
    detectByContent<RIFF::WAV::File>("invalid-chunk.wav");
  }
  void test_pcm_with_fact_chunk_wav() {
    detectByContent<RIFF::WAV::File>("pcm_with_fact_chunk.wav");
  }
  void test_segfault_wav() { detectByContent<RIFF::WAV::File>("segfault.wav"); }
  void test_uint8we_wav() { detectByContent<RIFF::WAV::File>("uint8we.wav"); }
  void test_zero_size_chunk_wav() {
    detectByContent<RIFF::WAV::File>("zero-size-chunk.wav");
  }
#endif

#ifdef TAGLIB_WITH_DSF
  // -- DSF::File --
  void test_empty10ms_dsf() { detectByContent<DSF::File>("empty10ms.dsf"); }

  // -- DSDIFF::File --
  void test_empty10ms_dff() { detectByContent<DSDIFF::File>("empty10ms.dff"); }
#endif

#ifdef TAGLIB_WITH_SHORTEN
  // -- Shorten::File --
  void test_2sec_silence_shn() {
    detectByContent<Shorten::File>("2sec-silence.shn");
  }
#endif

#ifdef TAGLIB_WITH_MATROSKA
  // -- Matroska::File --
  void test_no_tags_mka() { detectByContent<Matroska::File>("no-tags.mka"); }
  void test_no_tags_webm() { detectByContent<Matroska::File>("no-tags.webm"); }
  void test_optimized_mkv() {
    detectByContent<Matroska::File>("optimized.mkv");
  }
  void test_tags_before_cues_mkv() {
    detectByContent<Matroska::File>("tags-before-cues.mkv");
  }
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFileRefDetectByContent);
