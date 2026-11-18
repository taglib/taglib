#!/usr/bin/env python3
# Copyright 2026 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Structured seed generator for taglib's seed-limited parsers.

Mutation alone cannot cross these mutation-resistant gates, so the seeds are
built structurally valid from scratch (see tests/fuzzing/README.md):

1. ID3v2 rare frames (CHAP/CTOC/GEOB/SYLT/OWNE/POPM/PRIV/UFID/USLT/RVA2/ETCO/
   TXXX/WXXX/MCDI): a frame is only instantiated when the 4-byte frame ID
   matches exactly, the size field is well-formed (syncsafe in v2.4) and the
   body satisfies the frame's own structure (nested frames for CHAP/CTOC).
   The existing corpus contains almost none of these.
2. Matroska/EBML deep elements (SeekHead, Void, Cues, Chapters, Attachments,
   Info): requires valid EBML variable-length integers and correct element
   nesting; none of the existing seeds carry these elements.
3. Multi-page Ogg Vorbis streams: exercises the page/lacing walker
   (Ogg::File::nextPage, Ogg::Page::parse) over multiple pages. The Ogg CRC is
   not validated on read (only computed on render), so pages are emitted with
   a zero checksum.

Usage: generate_frame_seeds.py <out-dir>. Output is deterministic.
"""

import os
import struct
import sys


# ---------------------------------------------------------------- helpers --

def syncsafe(n):
    """ID3v2.4 syncsafe 32-bit integer (4 x 7 bits)."""
    return bytes([(n >> 21) & 0x7F, (n >> 14) & 0x7F, (n >> 7) & 0x7F, n & 0x7F])


def be32(n):
    return struct.pack(">I", n)


def le32(n):
    return struct.pack("<I", n)


def frame(fid, body, version=4):
    """ID3v2 frame with header. v2.4 uses syncsafe sizes, v2.3 plain sizes."""
    if version == 4:
        return lat1(fid) + syncsafe(len(body)) + b"\x00\x00" + body
    return lat1(fid) + be32(len(body)) + b"\x00\x00" + body


def lat1(text):
    return text.encode("latin-1")


def id3v2_tag(frames_body, version=4, padding=16):
    if version == 4:
        size_field = syncsafe(len(frames_body) + padding)
    else:
        size_field = struct.pack(">I", len(frames_body) + padding)
    return (b"ID3" + bytes([version, 0, 0]) + size_field + frames_body
            + b"\x00" * padding)


def mpeg_audio_frame():
    """Two consecutive, full-length MPEG-1 layer 3 frames (128 kbps @ 44.1 kHz).

    MPEG::File::isSupported() validates the first frame in checkLength mode:
    it seeks to offset + frameLength and requires the next frame's header to
    be readable and consistent (mpegheader.cpp). A single truncated frame is
    therefore rejected as an unsupported file. The frame length for these
    parameters is 144 * 128000 / 44100 = 417 bytes.
    """
    frame = b"\xff\xfb\x90\x64" + b"\x00" * 413
    assert len(frame) == 417
    return frame * 2


def mp3_with_frames(frame_list, version=4):
    return id3v2_tag(b"".join(frame_list), version) + mpeg_audio_frame()


def ebml_vint(n):
    """EBML data-size VINT for n payload bytes."""
    if n < 0x7F:
        return bytes([0x80 | n])
    if n < 0x3FFF:
        return struct.pack(">H", 0x4000 | n)
    if n < 0x1FFFFF:
        return struct.pack(">I", 0x200000 | n)[1:]
    return struct.pack(">I", 0x10000000 | n)


def ebml_element(eid, payload):
    return eid + ebml_vint(len(payload)) + payload


# ------------------------------------------------------- 1. ID3v2 frames --

def gen_id3v2_seeds(out_dir):
    seeds = {}

    # Standard text frame with multiple values (v2.4 unit separators).
    seeds["id3_tit2_multi.mp3"] = mp3_with_frames(
        [frame("TIT2", b"\x00" + b"A\x00B\x00C")])

    # TXXX: user-defined text.
    seeds["id3_txxx.mp3"] = mp3_with_frames(
        [frame("TXXX", b"\x00fuzzdesc\x00fuzzvalue")])

    # COMM: language + short text + description.
    seeds["id3_comm.mp3"] = mp3_with_frames(
        [frame("COMM", b"\x00engshort\x00comment text")])

    # USLT: unsynchronized lyrics.
    seeds["id3_uslt.mp3"] = mp3_with_frames(
        [frame("USLT", b"\x00engdesc\x00lyrics line")])

    # UFID: owner + binary identifier.
    seeds["id3_ufid.mp3"] = mp3_with_frames(
        [frame("UFID", b"http://example.org/owner\x00\x01\x02\x03")])

    # PRIV: private owner + binary.
    seeds["id3_priv.mp3"] = mp3_with_frames(
        [frame("PRIV", b"com.example\x00\xde\xad\xbe\xef")])

    # POPM: e-mail + rating + counter.
    seeds["id3_popm.mp3"] = mp3_with_frames(
        [frame("POPM", b"user@example.org\x00\xc8" + be32(42))])

    # GEOB: general encapsulated object.
    seeds["id3_geob.mp3"] = mp3_with_frames(
        [frame("GEOB", b"\x00application/octet-stream\x00name.bin\x00"
                       b"blob desc\x00" + b"\x11\x22\x33\x44")])

    # OWNE: ownership.
    seeds["id3_owne.mp3"] = mp3_with_frames(
        [frame("OWNE", b"\x00USD1.99" + b"\x00" + b"20261118" + b"seller")])

    # SYLT: synchronized lyrics, two entries, latin-1.
    sylt_body = (b"\x00" + b"eng" + b"\x02" + b"\x01"
                 + b"SYLT frame\x00"
                 + b"first line\n" + be32(1000)
                 + b"second line\n" + be32(2000))
    seeds["id3_sylt.mp3"] = mp3_with_frames([frame("SYLT", sylt_body)])

    # ETCO: event timing codes, two events.
    etco_body = b"\x02" + b"\x01" + be32(0) + b"\x02" + be32(5000)
    seeds["id3_etco.mp3"] = mp3_with_frames([frame("ETCO", etco_body)])

    # RVA2: relative volume adjustment, one channel with 8-bit peak.
    rva2_body = (b"identify\x00" + b"\x01" + struct.pack(">h", -128)
                 + b"\x08" + b"\xa5")
    seeds["id3_rva2.mp3"] = mp3_with_frames([frame("RVA2", rva2_body)])

    # MCDI: binary music CD identifier.
    seeds["id3_mcdi.mp3"] = mp3_with_frames([frame("MCDI", b"\x01\x02\x03\x04")])

    # URL frames.
    seeds["id3_wxxx.mp3"] = mp3_with_frames(
        [frame("WXXX", b"\x00fuzzsite\x00https://example.org")])
    seeds["id3_wcom.mp3"] = mp3_with_frames(
        [frame("WCOM", b"https://example.org/commercial")])

    # CHAP: chapter with one embedded TIT2 frame.
    embedded = frame("TIT2", b"\x00Chapter One")
    chap_body = (b"chp1\x00" + be32(0) + be32(30000) + be32(0xFFFFFFFF)
                 + be32(0xFFFFFFFF) + embedded)
    seeds["id3_chap.mp3"] = mp3_with_frames([frame("CHAP", chap_body)])

    # CTOC: table of contents, ordered top-level with two children and one
    # embedded frame.
    ctoc_body = (b"toc\x00" + b"\x03" + b"\x02" + b"chp1\x00" + b"chp2\x00"
                 + frame("TIT2", b"\x00Album TOC"))
    seeds["id3_ctoc.mp3"] = mp3_with_frames([frame("CTOC", ctoc_body)])

    # Everything in one file (frame-factory dispatch sweep in one input).
    all_frames = [
        frame("TIT2", b"\x00title"),
        frame("TXXX", b"\x00d\x00v"),
        frame("COMM", b"\x00engdesc\x00text"),
        frame("USLT", b"\x00engd\x00lyr"),
        frame("UFID", b"owner\x00\x01"),
        frame("PRIV", b"priv\x00\x01"),
        frame("POPM", b"p@example.org\x00\x99" + be32(7)),
        frame("GEOB", b"\x00mime\x00fn\x00de\x00b"),
        frame("OWNE", b"\x00USD0\x00" + b"20261118" + b"s"),
        frame("SYLT", sylt_body),
        frame("ETCO", etco_body),
        frame("RVA2", rva2_body),
        frame("MCDI", b"\x0a\x0b\x0c\x0d"),
        frame("WXXX", b"\x00d\x00https://example.org"),
        frame("WCOM", b"https://example.org"),
        frame("CHAP", chap_body),
        frame("CTOC", ctoc_body),
    ]
    seeds["id3_all_frames.mp3"] = mp3_with_frames(all_frames)

    # ID3v2.3 with RVAD (plain sizes; the v2.3 translation path).
    rvad_body = (b"v3id\x00" + b"\x01" + struct.pack(">h", 64)
                 + b"\x10" + b"\x01\x02")
    seeds["id3_v23_rvad.mp3"] = mp3_with_frames(
        [frame("RVAD", rvad_body, version=3)], version=3)

    for name, blob in seeds.items():
        with open(os.path.join(out_dir, name), "wb") as fh:
            fh.write(blob)
    return len(seeds)


# --------------------------------------------- 2. Matroska/EBML elements --

EBML_HEADER = b"\x1a\x45\xdf\xa3"
SEGMENT = b"\x18\x53\x80\x67"
SEEKHEAD = b"\x11\x4d\x9b\x74"
SEEK_ENTRY = b"\x4d\xbb"
SEEK_ID = b"\x53\xab"
SEEK_POSITION = b"\x53\xac"
VOID = b"\xec"
INFO = b"\x15\x49\xa9\x66"
TIMECODE_SCALE = b"\x2a\xd7\xb1"
TITLE = b"\x7b\xa9"
TAGS = b"\x12\x54\xc3\x67"
TAG = b"\x73\x73"
TARGETS = b"\x63\xc0"
TARGET_TYPE = b"\x63\xca"
SIMPLE_TAG = b"\x67\xc8"
TAG_NAME = b"\x45\xa3"
TAG_LANG = b"\x44\x7b"
TAG_STRING = b"\x44\x87"
ATTACHMENTS = b"\x19\x41\xa4\x69"
ATTACHED_FILE = b"\x61\xa7"
FILE_NAME = b"\x46\x6e"
FILE_MIME = b"\x46\x60"
FILE_DATA = b"\x46\x5c"
CHAPTERS = b"\x10\x43\xa7\x70"
EDITION_ENTRY = b"\x45\xb9"
CHAPTER_ATOM = b"\xb6"
CHAPTER_TIME_START = b"\x91"
CHAPTER_DISPLAY = b"\x80"
CHAP_STRING = b"\x85"
CHAP_LANG = b"\x43\x7c"
CUES = b"\x1c\x53\xbb\x6b"
CUE_POINT = b"\xbb"
CUE_TIME = b"\xb3"
CUE_TRACK_POSITIONS = b"\xb7"
CUE_TRACK = b"\xf7"
CUE_CLUSTER_POSITION = b"\xf1"


def ebml_id_u32(n):
    return struct.pack(">I", n)


def gen_matroska_seeds(out_dir):
    # Info segment info.
    info = ebml_element(INFO,
                        ebml_element(TIMECODE_SCALE, be32(1000000))
                        + ebml_element(TITLE, b"Fuzz Title"))

    # SeekHead pointing at the Info element (segment-relative offsets are
    # guesses; the parser must handle arbitrary, even bogus, positions).
    seek_entry = (ebml_element(SEEK_ID, ebml_id_u32(0x1549a966))
                  + ebml_element(SEEK_POSITION, be32(60)))
    seekhead = ebml_element(SEEKHEAD, ebml_element(SEEK_ENTRY, seek_entry))

    # Void element (pure filler with a distinct parse path).
    void = ebml_element(VOID, b"\x00" * 16)

    # Tags with targets and a simple tag.
    tag = ebml_element(TAG,
                       ebml_element(TARGETS,
                                    ebml_element(TARGET_TYPE, b"album"))
                       + ebml_element(SIMPLE_TAG,
                                      ebml_element(TAG_NAME, b"TITLE")
                                      + ebml_element(TAG_LANG, b"und")
                                      + ebml_element(TAG_STRING, b"EBML Seed")))
    tags = ebml_element(TAGS, tag)

    # Attachments with file data.
    attached = ebml_element(ATTACHED_FILE,
                            ebml_element(FILE_NAME, b"cover.png")
                            + ebml_element(FILE_MIME, b"image/png")
                            + ebml_element(FILE_DATA, b"\x89PNG\r\n\x1a\nfuzz"))
    attachments = ebml_element(ATTACHMENTS, attached)

    # Chapters: one edition, one atom, one display.
    chapter_atom = ebml_element(CHAPTER_ATOM,
                                ebml_element(CHAPTER_TIME_START, be32(0))
                                + ebml_element(CHAPTER_DISPLAY,
                                               ebml_element(CHAP_STRING, b"Intro")
                                               + ebml_element(CHAP_LANG, b"eng")))
    chapters = ebml_element(CHAPTERS, ebml_element(EDITION_ENTRY, chapter_atom))

    # Cues: one cue point.
    cue = ebml_element(CUE_POINT,
                       ebml_element(CUE_TIME, be32(1000))
                       + ebml_element(CUE_TRACK_POSITIONS,
                                      ebml_element(CUE_TRACK, be32(1))
                                      + ebml_element(CUE_CLUSTER_POSITION, be32(128))))
    cues = ebml_element(CUES, cue)

    # EBML header with a matroska DocType.
    ebml_header = ebml_element(
        EBML_HEADER,
        ebml_element(b"\x42\x82", b"matroska")      # DocType
        + ebml_element(b"\x42\x86", be32(1))        # EBMLVersion
        + ebml_element(b"\x42\xf7", be32(1)))       # EBMLReadVersion

    segment = ebml_element(
        SEGMENT,
        info + seekhead + void + tags + attachments + chapters + cues)

    blob = ebml_header + segment
    with open(os.path.join(out_dir, "matroska_all_elements.mkv"), "wb") as fh:
        fh.write(blob)

    # A smaller variant: only SeekHead + Void inside the segment.
    tiny_segment = ebml_element(SEGMENT, seekhead + void)
    tiny = ebml_header + tiny_segment
    with open(os.path.join(out_dir, "matroska_seekhead_void.mkv"), "wb") as fh:
        fh.write(tiny)

    # WebM-flavoured variant (DocType webm picks the other detection branch).
    webm_header = ebml_element(
        EBML_HEADER,
        ebml_element(b"\x42\x82", b"webm")
        + ebml_element(b"\x42\x86", be32(1)))
    webm = webm_header + ebml_element(SEGMENT, tags + cues)
    with open(os.path.join(out_dir, "webm_tags_cues.webm"), "wb") as fh:
        fh.write(webm)

    return 3


# ------------------------------------------------------ 4. Multi-page Ogg --

def ogg_page(header_type, seq, payload_segments):
    seg_count = len(payload_segments)
    header = (b"OggS" + b"\x00" + bytes([header_type]) + b"\x00" * 8
              + le32(0xF0000F00) + le32(seq) + le32(0) + bytes([seg_count]))
    body = b"".join(payload_segments)
    return header + bytes([len(s) for s in payload_segments]) + body


def gen_ogg_seeds(out_dir):
    # Vorbis identification header (30 bytes, as produced by libvorbis).
    ident = (b"\x01vorbis" + le32(0) + b"\x02" + struct.pack("<I", 44100)
             + struct.pack("<i", 192000) + struct.pack("<i", 128000)
             + struct.pack("<i", 64000) + b"\x0b" + b"\x01")
    assert len(ident) == 30

    # Comment header (vendor string + no user comments).
    vendor = b"fuzzgen"
    comment = (b"\x03vorbis" + le32(len(vendor)) + vendor + le32(0) + b"\x01")

    # Setup header placeholder carried across two lacing entries.
    setup = b"\x05vorbis" + b"\x00" * 260

    page1 = ogg_page(0x02, 0, [ident])
    page2 = ogg_page(0x00, 1, [comment])
    page3 = ogg_page(0x00, 2, [setup[:255], setup[255:]])

    with open(os.path.join(out_dir, "ogg_vorbis_3pages.ogg"), "wb") as fh:
        fh.write(page1 + page2 + page3)

    # Opus variant: identification + comment over two pages.
    opus_ident = (b"OpusHead" + struct.pack("<H", 2) + struct.pack("<H", 0)
                  + struct.pack("<I", 48000) + struct.pack("<H", 0)
                  + struct.pack("<H", 2))
    opus_comment = (b"OpusTags" + le32(len(vendor)) + vendor + le32(0))
    with open(os.path.join(out_dir, "ogg_opus_2pages.opus"), "wb") as fh:
        fh.write(ogg_page(0x02, 0, [opus_ident])
                 + ogg_page(0x00, 1, [opus_comment]))

    return 2


def main():
    if len(sys.argv) < 2:
        print("usage: generate_frame_seeds.py <out-dir>", file=sys.stderr)
        return 1
    out_dir = sys.argv[1]
    os.makedirs(out_dir, exist_ok=True)
    total = (gen_id3v2_seeds(out_dir)
             + gen_matroska_seeds(out_dir)
             + gen_ogg_seeds(out_dir))
    print(f"wrote {total} structured seeds to {out_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
