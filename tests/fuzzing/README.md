# Fuzzing taglib

This directory contains a libFuzzer target for taglib's generic parsing entry
point. It is intentionally decoupled from the normal CMake build: nothing in
the main source tree references it, and it is compiled on demand with the
commands below.

## Target

`taglib_fileref_fuzzer.cpp` parses arbitrary in-memory input through
`TagLib::FileRef` with a `ByteVectorStream`, exercising file type detection
plus tag, audio property and complex property (picture) parsing for all
supported formats: MP3/ID3v1/ID3v2, FLAC, Ogg Vorbis/Opus/Speex, MP4/M4A,
WAV/AIFF, APE/MPC/WavPack, DSF, DSDIFF, Matroska/WebM, Shorten, TrueAudio
and the tracker module formats.

`taglib.dict` is a libFuzzer dictionary of format magic numbers for the
supported file types.

## Round-trip target

`taglib_roundtrip_fuzzer.cpp` fuzzes the write direction: byte 0 selects the
format, byte 1 selects options (bit 0 also calls `Tag::setProperties`, bit 1
also runs the format-specific write APIs), and the rest is the file content
plus the source of fuzz-derived tag strings. The harness parses the content,
applies generic tag mutations (title, artist, album, comment, genre, year,
track) plus the format's dedicated writers — MPEG
`ID3v2Tag(true)/APETag(true)/ID3v1Tag(true)`, FLAC `addPicture`, MP4
`setItem`/`setQtChapters`/`setNeroChapters`, ASF `setAttribute`, WAV
`InfoTag()`/`setiXMLData`/`setBEXTData`, Matroska
`chapters(true)`/`attachments(true)` — then calls `save()` and re-parses the
rendered bytes through the same format's readers (including the MP4/Matroska
chapter and attachment readers).

This is the only exercise of the render/save half of every format: ID3v2 frame
and tag rendering, MP4 ilst and chapter-atom writing, ASF attribute rendering,
APE/Xiph/RIFF-INFO rendering, Ogg page repagination, Matroska tag/chapter/
attachment/cue writing and the tracker/DSF/DSDIFF/Shorten saves — paths that
process attacker-influenced strings, sizes and offsets and were previously
dark in coverage.

`taglib_roundtrip.dict` keeps mutations inside valid container structures so
the round trip keeps running on parseable bases.

### Rare-frame seed generator

`generate_frame_seeds.py` (deterministic, run by the OSS-Fuzz build) emits
structured seeds that unlock round-trip paths `tests/data` cannot reach,
because a frame is only parsed — and then rendered on `save()` — when an input
actually carries it:

1. **ID3v2 rare frames** — one MP3 per rare frame ID (TXXX, COMM, USLT, UFID,
   PRIV, POPM, GEOB, OWNE, SYLT, ETCO, RVA2, MCDI, WXXX, WCOM) plus CHAP and
   CTOC frames with valid nested embedded frames, a combined all-frames file,
   and an ID3v2.3 file. Frame IDs are exact 4-byte matches with version- and
   frame-specific body structures, so mutation alone cannot synthesize them.
   Round-tripping these seeds exercises each frame's `renderFields` and the
   tag-level re-render — paths no read-only input can reach.
2. **Matroska/EBML deep elements** — SeekHead, Void, Info, Tags, Attachments,
   Chapters and Cues with correct variable-length integer encodings.
3. **Multi-page Ogg streams** — a 3-page Vorbis stream and a 2-page Opus
   stream for the page/lacing walker.

## Building and running

Build taglib first, for example:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF
cmake --build build -j$(nproc)
```

Then compile the fuzz target against it with the libFuzzer and
AddressSanitizer flags:

```
clang++ -std=c++17 -g -fsanitize=fuzzer,address -fno-omit-frame-pointer \
  -I taglib -I taglib/toolkit -I build \
  tests/fuzzing/taglib_fileref_fuzzer.cpp \
  build/taglib/libtag.a -lz \
  -o taglib_fileref_fuzzer
```

Run it over taglib's own test data as a seed corpus (plus a dictionary):

```
./taglib_fileref_fuzzer -dict=tests/fuzzing/taglib.dict tests/data/
```

## OSS-Fuzz

This target is also built continuously by Google's OSS-Fuzz service for the
taglib project, which runs it under AddressSanitizer and
UndefinedBehaviorSanitizer with the test data as seed corpus.
