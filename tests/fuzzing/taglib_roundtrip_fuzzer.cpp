// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Round-trip fuzzer: parse attacker data, write attacker-influenced tags back
// out through the format's save()/render path, then re-parse the result.
//
// Rationale: the generic fileref fuzzer only reads. The whole write direction
// (ID3v2 frame rendering, MP4 ilst/chapter rendering incl. the QuickTime and
// Nero chapter writers, ASF attribute rendering, APE/Xiph/RIFF-INFO rendering,
// Ogg page repagination, Matroska tag/chapter/attachment/cue writing,
// DSF/DSDIFF/Shorten/tracker saves) is never exercised, yet it renders
// attacker-derived strings, sizes and offsets with heavy integer arithmetic —
// historically exactly where TagLib's overflow bugs live.
//
// Input layout: byte 0 selects the format (same 22-entry table as
// taglib_formats_fuzzer), byte 1 selects options (bit 0 also call
// Tag::setProperties, bit 1 also run the format-specific write APIs), the rest
// is the file content; fuzz-derived tag strings are sliced from the payload.

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <ape/apetag.h>
#include <ape/apefile.h>
#include <asf/asfattribute.h>
#include <asf/asffile.h>
#include <audioproperties.h>
#include <dsdiff/dsdifffile.h>
#include <dsf/dsffile.h>
#include <flac/flacfile.h>
#include <flac/flacpicture.h>
#include <it/itfile.h>
#include <matroska/matroskaattachedfile.h>
#include <matroska/matroskaattachments.h>
#include <matroska/matroskachapter.h>
#include <matroska/matroskachapteredition.h>
#include <matroska/matroskachapters.h>
#include <matroska/matroskafile.h>
#include <mod/modfile.h>
#include <mp4/mp4chapter.h>
#include <mp4/mp4file.h>
#include <mp4/mp4item.h>
#include <mpc/mpcfile.h>
#include <mpeg/id3v1/id3v1tag.h>
#include <mpeg/id3v2/id3v2tag.h>
#include <mpeg/mpegfile.h>
#include <ogg/flac/oggflacfile.h>
#include <ogg/opus/opusfile.h>
#include <ogg/speex/speexfile.h>
#include <ogg/vorbis/vorbisfile.h>
#include <riff/aiff/aifffile.h>
#include <riff/wav/wavfile.h>
#include <s3m/s3mfile.h>
#include <shorten/shortenfile.h>
#include <tag.h>
#include <trueaudio/trueaudiofile.h>
#include <tbytevector.h>
#include <tbytevectorstream.h>
#include <tlist.h>
#include <tpropertymap.h>
#include <tstring.h>
#include <wavpack/wavpackfile.h>
#include <xm/xmfile.h>

namespace {

constexpr int kNumFormats = 22;

const char *const kExtensions[kNumFormats] = {
    "mp3",   // 0  MPEG (ID3v1/ID3v2/APE)
    "flac",  // 1  FLAC
    "ogg",   // 2  Ogg Vorbis
    "opus",  // 3  Ogg Opus
    "spx",   // 4  Ogg Speex
    "oga",   // 5  Ogg FLAC
    "mpc",   // 6  MusePack
    "wv",    // 7  WavPack
    "ape",   // 8  Monkey's Audio
    "tta",   // 9  TrueAudio
    "m4a",   // 10 MP4
    "wma",   // 11 ASF/WMA
    "aiff",  // 12 AIFF
    "wav",   // 13 WAV
    "mod",   // 14 Protracker
    "s3m",   // 15 ScreamTracker 3
    "it",    // 16 Impulse Tracker
    "xm",    // 17 FastTracker II
    "dsf",   // 18 DSF
    "dff",   // 19 DSDIFF
    "shn",   // 20 Shorten
    "mka",   // 21 Matroska
};

struct FuzzStrings {
  TagLib::String a;
  TagLib::String b;
  TagLib::String c;
  unsigned int year = 0;
  unsigned int track = 0;
  TagLib::ByteVector blob;
};

FuzzStrings deriveStrings(const uint8_t *payload, unsigned int size)
{
  FuzzStrings fs;
  fs.a = TagLib::String(
      TagLib::ByteVector(reinterpret_cast<const char *>(payload),
                         std::min(size, 24u)),
      TagLib::String::Latin1);
  if(size > 24) {
    fs.b = TagLib::String(
        TagLib::ByteVector(reinterpret_cast<const char *>(payload + 24),
                           std::min(size - 24, 24u)),
        TagLib::String::Latin1);
  }
  if(size > 48) {
    fs.c = TagLib::String(
        TagLib::ByteVector(reinterpret_cast<const char *>(payload + 48),
                           std::min(size - 48, 64u)),
        TagLib::String::Latin1);
  }
  fs.year = 1900u + (size > 2 ? payload[2] : 0u);
  fs.track = size > 3 ? payload[3] : 0u;
  fs.blob = TagLib::ByteVector(reinterpret_cast<const char *>(payload),
                               std::min(size, 512u));
  return fs;
}

void probeTagAndProperties(TagLib::File *file)
{
  if(TagLib::Tag *tag = file->tag()) {
    (void)tag->title();
    (void)tag->artist();
    (void)tag->album();
    (void)tag->comment();
    (void)tag->genre();
    (void)tag->year();
    (void)tag->track();
    (void)tag->properties();
    (void)tag->complexProperties("PICTURE");
    (void)tag->complexProperties("CHAPTER");
  }
  if(const TagLib::AudioProperties *props = file->audioProperties()) {
    (void)props->lengthInSeconds();
    (void)props->lengthInMilliseconds();
    (void)props->bitrate();
    (void)props->sampleRate();
    (void)props->channels();
  }
  (void)file->properties();
}

// Re-parse probe: reads the rendered data back through the format readers,
// including the dedicated MP4/Matroska chapter and attachment readers.
void reprobe(TagLib::File *file)
{
  probeTagAndProperties(file);
  if(auto *mp4 = dynamic_cast<TagLib::MP4::File *>(file)) {
    for(const TagLib::MP4::Chapter &chapter : mp4->qtChapters()) {
      (void)chapter.title();
      (void)chapter.startTime();
    }
    for(const TagLib::MP4::Chapter &chapter : mp4->neroChapters()) {
      (void)chapter.title();
      (void)chapter.startTime();
    }
  }
  if(auto *mk = dynamic_cast<TagLib::Matroska::File *>(file)) {
    if(const TagLib::Matroska::Chapters *chapters = mk->chapters(false)) {
      for(const TagLib::Matroska::ChapterEdition &edition :
          chapters->chapterEditionList()) {
        for(const TagLib::Matroska::Chapter &chapter : edition.chapterList()) {
          for(const TagLib::Matroska::Chapter::Display &display :
              chapter.displayList()) {
            (void)display.string();
            (void)display.language();
          }
        }
      }
    }
    if(const TagLib::Matroska::Attachments *attachments = mk->attachments(false)) {
      for(const TagLib::Matroska::AttachedFile &attached :
          attachments->attachedFileList()) {
        (void)attached.fileName();
        (void)attached.data();
      }
    }
  }
}

void mutateGeneric(TagLib::File *file, const FuzzStrings &fs, bool setProps)
{
  if(TagLib::Tag *tag = file->tag()) {
    tag->setTitle(fs.a);
    tag->setArtist(fs.b);
    tag->setAlbum(fs.c);
    tag->setComment(fs.b);
    tag->setGenre(fs.a);
    tag->setYear(fs.year);
    tag->setTrack(fs.track);
    if(setProps) {
      TagLib::PropertyMap pm;
      pm.insert("TLRT", TagLib::StringList{fs.a, fs.c});
      tag->setProperties(pm);
    }
  }
}

// Round trip: mutate, save through the format renderer, then re-parse the
// rendered bytes through the same format's reader.
template <typename FileType, typename SpecialsFn>
void roundTrip(FileType &file, TagLib::ByteVectorStream &stream,
               SpecialsFn specials, bool runSpecials, const FuzzStrings &fs,
               bool setProps)
{
  if(!file.isValid())
    return;
  mutateGeneric(&file, fs, setProps);
  if(runSpecials)
    specials(file);
  if(!file.save())
    return;

  TagLib::ByteVector rendered = *stream.data();
  TagLib::ByteVectorStream reparseStream(rendered);
  FileType reparseFile(&reparseStream, true, TagLib::AudioProperties::Average);
  reprobe(&reparseFile);
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if(size < 2)
    return 0;

  const unsigned int fmt = data[0] % kNumFormats;
  const bool setProps = (data[1] & 0x01) != 0;
  const bool specials = (data[1] & 0x02) != 0;
  const FuzzStrings fs = deriveStrings(data + 2,
                                       static_cast<unsigned int>(size - 2));
  TagLib::ByteVector input(reinterpret_cast<const char *>(data + 2),
                           static_cast<unsigned int>(size - 2));
  TagLib::ByteVectorStream stream(input);
  switch(fmt) {
  case 0: {
    TagLib::MPEG::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [&](TagLib::MPEG::File &file) {
      if(TagLib::ID3v2::Tag *t = file.ID3v2Tag(true))
        t->setTitle(fs.a);
      if(TagLib::APE::Tag *t = file.APETag(true))
        t->setTitle(fs.b);
      if(TagLib::ID3v1::Tag *t = file.ID3v1Tag(true))
        t->setTitle(fs.c);
    }, specials, fs, setProps);
    break;
  }
  case 1: {
    TagLib::FLAC::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [&](TagLib::FLAC::File &file) {
      auto *pic = new TagLib::FLAC::Picture(fs.blob);
      pic->setMimeType(TagLib::String("image/png", TagLib::String::Latin1));
      pic->setDescription(fs.a);
      file.addPicture(pic);  // the file takes ownership
    }, specials, fs, setProps);
    break;
  }
  case 2: {
    TagLib::Ogg::Vorbis::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::Ogg::Vorbis::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 3: {
    TagLib::Ogg::Opus::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::Ogg::Opus::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 4: {
    TagLib::Ogg::Speex::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::Ogg::Speex::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 5: {
    TagLib::Ogg::FLAC::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::Ogg::FLAC::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 6: {
    TagLib::MPC::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::MPC::File &) {}, specials, fs, setProps);
    break;
  }
  case 7: {
    TagLib::WavPack::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::WavPack::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 8: {
    TagLib::APE::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::APE::File &) {}, specials, fs, setProps);
    break;
  }
  case 9: {
    TagLib::TrueAudio::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::TrueAudio::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 10: {
    TagLib::MP4::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [&](TagLib::MP4::File &file) {
      if(TagLib::MP4::Tag *t = file.tag()) {
        t->setItem(TagLib::String("fuzz", TagLib::String::Latin1),
                   TagLib::MP4::Item(TagLib::StringList{fs.a, fs.b}));
        t->setItem(TagLib::String("tmpo", TagLib::String::Latin1),
                   TagLib::MP4::Item(static_cast<int>(fs.track)));
      }
      file.setQtChapters(TagLib::MP4::ChapterList{
          TagLib::MP4::Chapter(fs.a, 0), TagLib::MP4::Chapter(fs.b, 15000)});
      file.setNeroChapters(TagLib::MP4::ChapterList{
          TagLib::MP4::Chapter(fs.c, 30000)});
    }, specials, fs, setProps);
    break;
  }
  case 11: {
    TagLib::ASF::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [&](TagLib::ASF::File &file) {
      if(TagLib::ASF::Tag *t = file.tag()) {
        t->setAttribute(TagLib::String("TLRF", TagLib::String::Latin1),
                        TagLib::ASF::Attribute(fs.a));
        t->addAttribute(TagLib::String("TLRB", TagLib::String::Latin1),
                        TagLib::ASF::Attribute(fs.blob));
      }
    }, specials, fs, setProps);
    break;
  }
  case 12: {
    TagLib::RIFF::AIFF::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::RIFF::AIFF::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 13: {
    TagLib::RIFF::WAV::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [&](TagLib::RIFF::WAV::File &file) {
      file.InfoTag()->setFieldText(TagLib::ByteVector("INAM"), fs.a);
      file.InfoTag()->setFieldText(TagLib::ByteVector("IART"), fs.b);
      file.InfoTag()->removeField(TagLib::ByteVector("ICMT"));
      file.setiXMLData(fs.c);
      file.setBEXTData(fs.blob);
    }, specials, fs, setProps);
    break;
  }
  case 14: {
    TagLib::Mod::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::Mod::File &) {}, specials, fs, setProps);
    break;
  }
  case 15: {
    TagLib::S3M::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::S3M::File &) {}, specials, fs, setProps);
    break;
  }
  case 16: {
    TagLib::IT::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::IT::File &) {}, specials, fs, setProps);
    break;
  }
  case 17: {
    TagLib::XM::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::XM::File &) {}, specials, fs, setProps);
    break;
  }
  case 18: {
    TagLib::DSF::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::DSF::File &) {}, specials, fs, setProps);
    break;
  }
  case 19: {
    TagLib::DSDIFF::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::DSDIFF::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 20: {
    TagLib::Shorten::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [](TagLib::Shorten::File &) {}, specials, fs,
              setProps);
    break;
  }
  case 21: {
    TagLib::Matroska::File f(&stream, true, TagLib::AudioProperties::Average);
    roundTrip(f, stream, [&](TagLib::Matroska::File &file) {
      using MChapter = TagLib::Matroska::Chapter;
      using MDisplay = MChapter::Display;
      if(TagLib::Matroska::Chapters *chs = file.chapters(true)) {
        TagLib::List<MChapter> chapters;
        chapters.append(MChapter(0ULL, 5000000ULL,
                                 TagLib::List<MDisplay>{MDisplay(fs.a, fs.b)},
                                 1ULL));
        chs->addChapterEdition(
            TagLib::Matroska::ChapterEdition(chapters, true, false, 1ULL));
      }
      if(TagLib::Matroska::Attachments *atts = file.attachments(true)) {
        atts->addAttachedFile(TagLib::Matroska::AttachedFile(
            fs.blob, fs.b, TagLib::String("text/plain", TagLib::String::Latin1),
            42ULL, fs.c));
      }
    }, specials, fs, setProps);
    break;
  }
  default:
    break;
  }
  return 0;
}
