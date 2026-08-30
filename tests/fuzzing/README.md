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
