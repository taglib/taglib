/**************************************************************************
    copyright            : (C) 2026 by Ryan Francesconi
 **************************************************************************/

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

#include "mp4qtchapterlist.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "tdebug.h"
#include "mp4file.h"
#include "mp4atom.h"

using namespace TagLib;

namespace
{

  // -- Atom building helpers ------------------------------------------------

  ByteVector renderAtom(const ByteVector &name, const ByteVector &data)
  {
    return ByteVector::fromUInt(static_cast<unsigned int>(data.size() + 8)) + name + data;
  }

  //! Build a full-box (version + flags) atom.
  ByteVector renderFullBox(const ByteVector &name, unsigned char version,
                           unsigned int flags, const ByteVector &data)
  {
    ByteVector vf;
    vf.append(static_cast<char>(version));
    vf.append(ByteVector::fromUInt(flags).mid(1, 3));  // 3 bytes of flags
    vf.append(data);
    return renderAtom(name, vf);
  }

  // -- Parent / offset fixup (mirrors mp4chapterlist.cpp) -------------------

  void updateParentSizes(TagLib::File *file, const MP4::AtomList &path,
                         offset_t delta, int ignore = 0)
  {
    if(static_cast<int>(path.size()) <= ignore)
      return;

    auto itEnd = path.end();
    std::advance(itEnd, 0 - ignore);

    for(auto it = path.begin(); it != itEnd; ++it) {
      file->seek((*it)->offset());
      long size = file->readBlock(4).toUInt();
      if(size == 1) {
        file->seek(4, TagLib::File::Current);
        long long longSize = file->readBlock(8).toLongLong();
        file->seek((*it)->offset() + 8);
        file->writeBlock(ByteVector::fromLongLong(longSize + delta));
      }
      else {
        file->seek((*it)->offset());
        file->writeBlock(ByteVector::fromUInt(static_cast<unsigned int>(size + delta)));
      }
    }
  }

  void updateChunkOffsets(TagLib::File *file, MP4::Atoms *atoms,
                          offset_t delta, offset_t offset)
  {
    if(MP4::Atom *moov = atoms->find("moov")) {
      const MP4::AtomList stco = moov->findall("stco", true);
      for(const auto &atom : stco) {
        if(atom->offset() > offset)
          atom->addToOffset(delta);
        file->seek(atom->offset() + 12);
        ByteVector data = file->readBlock(atom->length() - 12);
        unsigned int count = data.toUInt();
        file->seek(atom->offset() + 16);
        unsigned int pos = 4;
        while(count--) {
          auto o = static_cast<offset_t>(data.toUInt(pos));
          if(o > offset)
            o += delta;
          file->writeBlock(ByteVector::fromUInt(static_cast<unsigned int>(o)));
          pos += 4;
        }
      }

      const MP4::AtomList co64 = moov->findall("co64", true);
      for(const auto &atom : co64) {
        if(atom->offset() > offset)
          atom->addToOffset(delta);
        file->seek(atom->offset() + 12);
        ByteVector data = file->readBlock(atom->length() - 12);
        unsigned int count = data.toUInt();
        file->seek(atom->offset() + 16);
        unsigned int pos = 4;
        while(count--) {
          long long o = data.toLongLong(pos);
          if(o > offset)
            o += delta;
          file->writeBlock(ByteVector::fromLongLong(o));
          pos += 8;
        }
      }
    }

    if(MP4::Atom *moof = atoms->find("moof")) {
      const MP4::AtomList tfhd = moof->findall("tfhd", true);
      for(const auto &atom : tfhd) {
        if(atom->offset() > offset)
          atom->addToOffset(delta);
        file->seek(atom->offset() + 9);
        ByteVector data = file->readBlock(atom->length() - 9);
        if(const unsigned int flags = data.toUInt(0, 3, true);
           flags & 1) {
          long long o = data.toLongLong(7U);
          if(o > offset)
            o += delta;
          file->seek(atom->offset() + 16);
          file->writeBlock(ByteVector::fromLongLong(o));
        }
      }
    }
  }

  // -- Duration reading -----------------------------------------------------

  //! Movie-level header info from mvhd.
  struct MovieInfo {
    unsigned int timescale = 0;
    unsigned int duration = 0;   // in mvhd timescale units
    long long durationMs = 0;    // converted to milliseconds
  };

  //! Reads movie-level info from mvhd.
  MovieInfo readMovieInfo(TagLib::File *file, MP4::Atoms *atoms)
  {
    MovieInfo info;
    MP4::Atom *moov = atoms->find("moov");
    if(!moov)
      return info;

    MP4::Atom *mvhd = moov->find("mvhd");
    if(!mvhd)
      return info;

    file->seek(mvhd->offset());
    ByteVector data = file->readBlock(mvhd->length());
    if(data.size() < 8 + 4)
      return info;

    unsigned char version = static_cast<unsigned char>(data[8]);
    long long timescale, duration;
    if(version == 1 && data.size() >= 8 + 28) {
      timescale = data.toUInt(28U);
      duration  = data.toLongLong(32U);
    }
    else if(data.size() >= 8 + 16 + 4) {
      timescale = data.toUInt(20U);
      duration  = data.toUInt(24U);
    }
    else {
      return info;
    }

    if(timescale > 0 && duration > 0) {
      info.timescale = static_cast<unsigned int>(timescale);
      info.duration = static_cast<unsigned int>(duration);
      info.durationMs = static_cast<long long>(
        static_cast<double>(duration) * 1000.0 / static_cast<double>(timescale) + 0.5);
    }
    return info;
  }

  // -- Audio track helpers --------------------------------------------------

  struct TrackInfo {
    MP4::Atom *trak = nullptr;
    unsigned int trackId = 0;
  };

  //! Finds the first audio track (hdlr handler_type == "soun").
  TrackInfo findAudioTrack(TagLib::File *file, MP4::Atoms *atoms)
  {
    TrackInfo info;
    MP4::Atom *moov = atoms->find("moov");
    if(!moov)
      return info;

    const MP4::AtomList trakList = moov->findall("trak");
    for(const auto &trak : trakList) {
      const MP4::Atom *hdlr = trak->find("mdia", "hdlr");
      if(!hdlr)
        continue;
      file->seek(hdlr->offset());
      ByteVector data = file->readBlock(hdlr->length());
      // handler_type is at offset 16 from atom start (8 header + 4 version/flags + 4 pre_defined)
      if(data.containsAt("soun", 16)) {
        info.trak = trak;
        // Read track_id from tkhd
        if(MP4::Atom *tkhd = trak->find("tkhd")) {
          file->seek(tkhd->offset());
          ByteVector tkhdData = file->readBlock(tkhd->length());
          unsigned char version = static_cast<unsigned char>(tkhdData[8]);
          if(version == 1 && tkhdData.size() >= 8 + 20 + 4) {
            info.trackId = tkhdData.toUInt(28U);
          }
          else if(tkhdData.size() >= 8 + 12 + 4) {
            info.trackId = tkhdData.toUInt(20U);
          }
        }
        return info;
      }
    }
    return info;
  }

  //! Reads the next_track_ID from mvhd.
  unsigned int getNextTrackId(TagLib::File *file, MP4::Atoms *atoms)
  {
    MP4::Atom *moov = atoms->find("moov");
    if(!moov) return 0;

    MP4::Atom *mvhd = moov->find("mvhd");
    if(!mvhd) return 0;

    file->seek(mvhd->offset());
    ByteVector data = file->readBlock(mvhd->length());
    unsigned char version = static_cast<unsigned char>(data[8]);

    // next_track_ID is the last 4 bytes of mvhd
    // version 0: header(8) + version/flags(4) + creation(4) + modification(4)
    //            + timescale(4) + duration(4) + ... total fixed = 108 bytes
    // version 1: header(8) + version/flags(4) + creation(8) + modification(8)
    //            + timescale(4) + duration(8) + ... total fixed = 120 bytes
    unsigned int nextTrackIdOffset = (version == 1) ? 120 - 4 : 108 - 4;
    if(data.size() >= nextTrackIdOffset + 4)
      return data.toUInt(nextTrackIdOffset);

    return 0;
  }

  //! Writes next_track_ID in mvhd.
  void setNextTrackId(TagLib::File *file, MP4::Atoms *atoms, unsigned int newId)
  {
    MP4::Atom *moov = atoms->find("moov");
    if(!moov) return;

    MP4::Atom *mvhd = moov->find("mvhd");
    if(!mvhd) return;

    file->seek(mvhd->offset());
    ByteVector data = file->readBlock(mvhd->length());
    unsigned char version = static_cast<unsigned char>(data[8]);

    unsigned int nextTrackIdOffset = (version == 1) ? 120 - 4 : 108 - 4;
    if(data.size() >= nextTrackIdOffset + 4) {
      file->seek(mvhd->offset() + nextTrackIdOffset);
      file->writeBlock(ByteVector::fromUInt(newId));
    }
  }

  // -- Chapter track finder -------------------------------------------------

  //! Finds an existing chapter track by scanning for tref/chap in the audio track.
  //! tref is NOT in TagLib's container list, so we read it manually.
  MP4::Atom *findChapterTrak(TagLib::File *file, MP4::Atoms *atoms,
                             MP4::Atom *audioTrak)
  {
    if(!audioTrak)
      return nullptr;

    MP4::Atom *moov = atoms->find("moov");
    if(!moov)
      return nullptr;

    for(const auto &child : audioTrak->children()) {
      if(child->name() == "tref") {
        file->seek(child->offset() + 8);
        offset_t trefEnd = child->offset() + child->length();

        while(file->tell() + 8 <= trefEnd) {
          offset_t boxStart = file->tell();
          ByteVector header = file->readBlock(8);
          if(header.size() < 8)
            break;

          unsigned int boxSize = header.toUInt();
          if(boxSize < 8)
            break;

          ByteVector boxName = header.mid(4, 4);

          if(boxName == "chap" && boxSize >= 12) {
            ByteVector refData = file->readBlock(boxSize - 8);
            unsigned int refTrackId = refData.toUInt();

            const MP4::AtomList allTraks = moov->findall("trak");

            for(const auto &t : allTraks) {
              MP4::Atom *tkhd = t->find("tkhd");
              if(!tkhd)
                continue;

              file->seek(tkhd->offset());
              ByteVector tkhdData = file->readBlock(tkhd->length());
              if(tkhdData.size() < 24)
                continue;

              unsigned char version = static_cast<unsigned char>(tkhdData[8]);
              unsigned int tid;
              if(version == 1 && tkhdData.size() >= 32) {
                tid = tkhdData.toUInt(28U);
              }
              else {
                tid = tkhdData.toUInt(20U);
              }

              if(tid == refTrackId)
                return t;
            }
          }

          file->seek(boxStart + boxSize);
        }
      }
    }

    return nullptr;
  }

  // -- Text sample building -------------------------------------------------

  //! Size of the 'encd' (encoding) atom appended to each text sample.
  //! encd declares UTF-8 encoding: size(4) + "encd"(4) + 0x0100(4) = 12 bytes.
  constexpr unsigned int encdAtomSize = 12;

  //! Builds a single text sample: 2-byte big-endian length + UTF-8 text + encd atom.
  ByteVector buildTextSample(const String &title)
  {
    ByteVector utf8 = title.data(String::UTF8);
    unsigned int textLen = static_cast<unsigned int>(utf8.size());

    ByteVector sample;
    sample.append(ByteVector::fromShort(static_cast<short>(textLen)));
    if(textLen > 0)
      sample.append(utf8);

    // Append encd atom (encoding declaration: UTF-8) immediately after text.
    // No padding -- AVFoundation expects encd to follow directly after the text.
    ByteVector encdData;
    encdData.append(ByteVector::fromShort(0));  // padding
    encdData.append(ByteVector::fromShort(0x0100)); // UTF-8 encoding
    sample.append(renderAtom("encd", encdData));

    return sample;
  }

  //! Calculate the actual size of each chapter text sample.
  //! Each sample is: 2-byte length prefix + UTF-8 text + 12-byte encd atom.
  //! Returns a vector of per-sample sizes (no padding, sizes may differ).
  std::vector<unsigned int> calculateSampleSizes(const MP4::ChapterList &chapters)
  {
    std::vector<unsigned int> sizes;
    for(const auto &ch : chapters) {
      unsigned int textLen = static_cast<unsigned int>(ch.title.data(String::UTF8).size());
      sizes.push_back(2 + textLen + encdAtomSize);
    }
    return sizes;
  }

  // -- stbl atom builders ---------------------------------------------------

  //! stsd: text sample description (required for QT text tracks)
  ByteVector buildStsd()
  {
    // QT text sample entry matching ffmpeg's chapter track output byte-for-byte.
    // Entry body (after 8-byte size+"text" header) is 51 bytes:
    //   reserved(6) + dref_index(2) + display_flags(4) + justification(4)
    //   + bgColor(6) + textBox(8) + fontID(2) + style_flags(1) + font_size(1)
    //   + text_color_RGBA(4) + ftab_atom(13)
    //
    // Hardcoded from ffmpeg's known-good output to avoid subtle field-size
    // mismatches with the under-documented QT text sample entry format.
    const unsigned char entryBody[] = {
      // reserved (6 bytes) + data reference index (2 bytes)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x01,
      // display flags (4) = 1
      0x00, 0x00, 0x00, 0x01,
      // text justification (4) = 0
      0x00, 0x00, 0x00, 0x00,
      // background color RGB (6 bytes, QT text format: 2 bytes per channel)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      // default text box: top(2), left(2), bottom(2), right(2) = all zero
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      // font ID (2) = 1
      0x00, 0x01,
      // font style flags(1) + font size(1) + text color RGBA(4)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      // ftab atom: size(4)=13 + "ftab"(4) + entry_count(2)=1 + fontID(2)=1 + name_len(1)=0
      0x00, 0x00, 0x00, 0x0d,  0x66, 0x74, 0x61, 0x62,
      0x00, 0x01,  0x00, 0x01,  0x00
    };

    ByteVector sampleEntry;
    unsigned int entrySize = 8 + sizeof(entryBody);
    sampleEntry.append(ByteVector::fromUInt(entrySize));
    sampleEntry.append(ByteVector("text", 4));
    sampleEntry.append(ByteVector(reinterpret_cast<const char *>(entryBody),
                                  sizeof(entryBody)));

    ByteVector stsdPayload;
    stsdPayload.append(ByteVector::fromUInt(1));  // entry count
    stsdPayload.append(sampleEntry);

    return renderFullBox("stsd", 0, 0, stsdPayload);
  }

  //! stts: time-to-sample table.
  //! For N chapters, entry i has duration = chapter[i+1].start - chapter[i].start.
  //! The last chapter runs to the end of the file.
  ByteVector buildStts(const MP4::ChapterList &chapters, unsigned int timescale,
                       long long durationMs)
  {
    unsigned int count = static_cast<unsigned int>(chapters.size());
    if(count == 0)
      return ByteVector();

    // Convert 100-ns units to timescale units
    auto toTimescale = [timescale](long long time100ns) -> unsigned int {
      return static_cast<unsigned int>(
        static_cast<double>(time100ns) * static_cast<double>(timescale) / 10000000.0 + 0.5);
    };

    unsigned int totalDuration = static_cast<unsigned int>(
      static_cast<double>(durationMs) * static_cast<double>(timescale) / 1000.0 + 0.5);

    // Build per-sample durations
    std::vector<unsigned int> durations;
    auto it = chapters.begin();
    for(unsigned int i = 0; i < count; ++i, ++it) {
      auto next = it;
      ++next;
      unsigned int startTs = toTimescale(it->startTime);
      unsigned int dur;
      if(next != chapters.end()) {
        unsigned int nextTs = toTimescale(next->startTime);
        dur = nextTs - startTs;
      }
      else {
        // Last chapter runs to end of file
        dur = totalDuration > startTs ? totalDuration - startTs : 0;
      }
      durations.push_back(dur);
    }

    // One stts entry per sample (sampleCount=1 each), matching ffmpeg's output.
    // AVFoundation requires this layout rather than run-length encoding.
    ByteVector payload;
    payload.append(ByteVector::fromUInt(count));
    for(auto d : durations) {
      payload.append(ByteVector::fromUInt(1));  // sample count
      payload.append(ByteVector::fromUInt(d));  // sample delta
    }

    return renderFullBox("stts", 0, 0, payload);
  }

  //! stsz: sample size table with per-sample entries (matching ffmpeg output).
  ByteVector buildStsz(const std::vector<unsigned int> &sampleSizes)
  {
    ByteVector payload;
    payload.append(ByteVector::fromUInt(0));  // default_sample_size = 0 (per-sample)
    payload.append(ByteVector::fromUInt(static_cast<unsigned int>(sampleSizes.size())));
    for(auto sz : sampleSizes)
      payload.append(ByteVector::fromUInt(sz));
    return renderFullBox("stsz", 0, 0, payload);
  }

  //! stsc: sample-to-chunk table.  All samples in one chunk.
  ByteVector buildStsc(unsigned int sampleCount)
  {
    // All samples in a single chunk: one entry saying
    // "starting at chunk 1, N samples per chunk, description index 1"
    ByteVector payload;
    payload.append(ByteVector::fromUInt(1));           // entry count
    payload.append(ByteVector::fromUInt(1));           // first chunk
    payload.append(ByteVector::fromUInt(sampleCount)); // samples per chunk
    payload.append(ByteVector::fromUInt(1));           // sample description index

    return renderFullBox("stsc", 0, 0, payload);
  }

  //! stco: chunk offset table.  Single chunk offset pointing to the
  //! start of the contiguous text sample data.
  ByteVector buildStco(unsigned int offset)
  {
    ByteVector payload;
    payload.append(ByteVector::fromUInt(1));      // entry count = 1
    payload.append(ByteVector::fromUInt(offset)); // chunk offset
    return renderFullBox("stco", 0, 0, payload);
  }

  // -- Full trak builder ----------------------------------------------------

  //! Builds a complete chapter text trak atom.
  //! \a textDataOffset is where the text samples will start in the file.
  //! \a sampleSizes contains per-sample sizes for the stsz table.
  //! \a movieDuration is the movie-level duration in mvhd timescale units (for edts/elst).
  ByteVector buildChapterTrak(unsigned int trackId, unsigned int timescale,
                              long long durationMs,
                              const MP4::ChapterList &chapters,
                              const std::vector<unsigned int> &sampleSizes,
                              offset_t textDataOffset,
                              unsigned int movieDuration)
  {
    unsigned int count = static_cast<unsigned int>(chapters.size());
    unsigned int totalDuration = static_cast<unsigned int>(
      static_cast<double>(durationMs) * static_cast<double>(timescale) / 1000.0 + 0.5);

    // Single chunk offset -- all samples are contiguous starting at textDataOffset
    unsigned int chunkOffset = static_cast<unsigned int>(textDataOffset);

    // -- tkhd (track header) --
    // version 0: 8 header + 4 ver/flags + 4 creation + 4 modification
    //          + 4 track_id + 4 reserved + 4 duration + 8 reserved
    //          + 2 layer + 2 alternate_group + 2 volume + 2 reserved
    //          + 36 matrix + 4 width + 4 height = 92 bytes total
    ByteVector tkhdData;
    tkhdData.append(ByteVector(4, '\0'));     // creation time
    tkhdData.append(ByteVector(4, '\0'));     // modification time
    tkhdData.append(ByteVector::fromUInt(trackId));
    tkhdData.append(ByteVector(4, '\0'));     // reserved
    // Duration in mvhd timescale.
    tkhdData.append(ByteVector::fromUInt(totalDuration));
    tkhdData.append(ByteVector(8, '\0'));     // reserved
    tkhdData.append(ByteVector::fromShort(0)); // layer
    tkhdData.append(ByteVector::fromShort(0)); // alternate_group
    tkhdData.append(ByteVector::fromShort(0)); // volume (0 for text)
    tkhdData.append(ByteVector::fromShort(0)); // reserved
    // Identity matrix (3x3 fixed point)
    tkhdData.append(ByteVector::fromUInt(0x00010000)); // a = 1.0
    tkhdData.append(ByteVector(4, '\0'));               // b
    tkhdData.append(ByteVector(4, '\0'));               // u
    tkhdData.append(ByteVector(4, '\0'));               // c
    tkhdData.append(ByteVector::fromUInt(0x00010000));  // d = 1.0
    tkhdData.append(ByteVector(4, '\0'));               // v
    tkhdData.append(ByteVector(4, '\0'));               // x
    tkhdData.append(ByteVector(4, '\0'));               // y
    tkhdData.append(ByteVector::fromUInt(0x40000000));  // w = 1.0
    tkhdData.append(ByteVector::fromUInt(0)); // width
    tkhdData.append(ByteVector::fromUInt(0)); // height

    // flags = 0x02: track_in_movie only (matches ffmpeg's chapter track output).
    // Chapter tracks are NOT track_enabled(1) -- they are disabled but present in movie.
    ByteVector tkhd = renderFullBox("tkhd", 0, 0x02, tkhdData);

    // -- mdhd (media header) --
    ByteVector mdhdData;
    mdhdData.append(ByteVector(4, '\0'));     // creation time
    mdhdData.append(ByteVector(4, '\0'));     // modification time
    mdhdData.append(ByteVector::fromUInt(timescale));
    mdhdData.append(ByteVector::fromUInt(totalDuration));
    // language: 0x0000 (matches ffmpeg chapter track output)
    mdhdData.append(ByteVector::fromShort(0));
    mdhdData.append(ByteVector::fromShort(0)); // pre_defined

    ByteVector mdhd = renderFullBox("mdhd", 0, 0, mdhdData);

    // -- hdlr (handler reference) --
    ByteVector hdlrData;
    hdlrData.append(ByteVector(4, '\0'));              // pre_defined
    hdlrData.append(ByteVector("text", 4));            // handler_type
    hdlrData.append(ByteVector(12, '\0'));              // reserved
    // name: null-terminated "Chapter" string
    hdlrData.append(ByteVector("Chapter", 7));
    hdlrData.append(static_cast<char>(0));

    ByteVector hdlr = renderFullBox("hdlr", 0, 0, hdlrData);

    // -- gmhd (base media information header) --
    // QT text/chapter tracks use gmhd with gmin + text children.

    // gmin: graphicsMode(2) + opcolor(6) + balance(2) + reserved(2)
    ByteVector gminData;
    gminData.append(ByteVector::fromShort(0x0040)); // graphicsMode = ditherCopy (0x40)
    gminData.append(ByteVector("\x80\x00\x80\x00\x80\x00", 6)); // opcolor (gray)
    gminData.append(ByteVector::fromShort(0));      // balance
    gminData.append(ByteVector::fromShort(0));      // reserved
    ByteVector gmin = renderFullBox("gmin", 0, 0, gminData);

    // text media information atom: matrix(36) + ... = 36 bytes of data
    ByteVector textInfoData;
    textInfoData.append(ByteVector::fromShort(1));   // 0x0001
    textInfoData.append(ByteVector(14, '\0'));        // reserved
    textInfoData.append(ByteVector::fromShort(1));   // 0x0001
    textInfoData.append(ByteVector(14, '\0'));        // reserved
    textInfoData.append(ByteVector::fromUInt(0x40000000)); // 1.0 fixed point
    ByteVector textInfo = renderAtom("text", textInfoData);

    ByteVector gmhdContent;
    gmhdContent.append(gmin);
    gmhdContent.append(textInfo);
    ByteVector gmhd = renderAtom("gmhd", gmhdContent);

    // -- dinf / dref (data reference) --
    ByteVector drefEntry;
    // "url " self-reference entry (flag 1 = data is in this file)
    drefEntry = renderFullBox("url ", 0, 1, ByteVector());

    ByteVector drefData;
    drefData.append(ByteVector::fromUInt(1));  // entry count
    drefData.append(drefEntry);
    ByteVector dref = renderFullBox("dref", 0, 0, drefData);
    ByteVector dinf = renderAtom("dinf", dref);

    // -- stbl (sample table) --
    ByteVector stsd = buildStsd();
    ByteVector stts = buildStts(chapters, timescale, durationMs);
    ByteVector stsz = buildStsz(sampleSizes);
    ByteVector stsc = buildStsc(count);
    ByteVector stco = buildStco(chunkOffset);

    ByteVector stblContent;
    stblContent.append(stsd);
    stblContent.append(stts);
    stblContent.append(stsz);
    stblContent.append(stsc);
    stblContent.append(stco);
    ByteVector stbl = renderAtom("stbl", stblContent);

    // -- minf (media information) --
    ByteVector minfContent;
    minfContent.append(gmhd);
    minfContent.append(dinf);
    minfContent.append(stbl);
    ByteVector minf = renderAtom("minf", minfContent);

    // -- mdia (media) --
    ByteVector mdiaContent;
    mdiaContent.append(mdhd);
    mdiaContent.append(hdlr);
    mdiaContent.append(minf);
    ByteVector mdia = renderAtom("mdia", mdiaContent);

    // -- edts / elst (edit list) --
    // AVFoundation requires an edit list for the chapter track.
    // Single entry: play the whole media from time 0, at normal rate.
    ByteVector elstData;
    elstData.append(ByteVector::fromUInt(1));            // entry count
    elstData.append(ByteVector::fromUInt(movieDuration)); // segment duration (mvhd timescale)
    elstData.append(ByteVector::fromUInt(0));             // media time = 0
    elstData.append(ByteVector::fromUInt(0x00010000));    // media rate = 1.0 (fixed point)
    ByteVector elst = renderFullBox("elst", 0, 0, elstData);
    ByteVector edts = renderAtom("edts", elst);

    // -- trak --
    ByteVector trakContent;
    trakContent.append(tkhd);
    trakContent.append(edts);
    trakContent.append(mdia);
    ByteVector trak = renderAtom("trak", trakContent);

    return trak;
  }

  // -- tref / chap builder --------------------------------------------------

  //! Builds a tref atom containing a chap reference to the given track ID.
  ByteVector buildTref(unsigned int chapterTrackId)
  {
    ByteVector chapData;
    chapData.append(ByteVector::fromUInt(chapterTrackId));
    ByteVector chap = renderAtom("chap", chapData);
    return renderAtom("tref", chap);
  }

  // -- Reading helpers ------------------------------------------------------

  //! Reads chapter track duration info from the chapter trak's mdhd.
  struct ChapterTrackInfo {
    unsigned int timescale = 0;
    unsigned int totalDuration = 0;
  };

  ChapterTrackInfo readChapterTrackInfo(TagLib::File *file, MP4::Atom *chapterTrak)
  {
    ChapterTrackInfo info;

    MP4::Atom *mdhd = chapterTrak->find("mdia", "mdhd");
    if(!mdhd)
      return info;

    file->seek(mdhd->offset());
    ByteVector data = file->readBlock(mdhd->length());
    if(data.size() < 8 + 4)
      return info;

    unsigned char version = static_cast<unsigned char>(data[8]);
    if(version == 1 && data.size() >= 40) {
      // v1 mdhd: header(8) + ver/flags(4) + creation(8) + modification(8)
      //          + timescale(4)@28 + duration(8)@32 + lang(2) + pre(2) = 44
      info.timescale = data.toUInt(28U);
      info.totalDuration = static_cast<unsigned int>(data.toLongLong(32U));
    }
    else if(version == 0 && data.size() >= 28) {
      // v0 mdhd: header(8) + ver/flags(4) + creation(4) + modification(4)
      //          + timescale(4)@20 + duration(4)@24 + lang(2) + pre(2) = 32
      info.timescale = data.toUInt(20U);
      info.totalDuration = data.toUInt(24U);
    }
    return info;
  }

  //! Reads stts entries from the chapter track.
  struct SttsEntry {
    unsigned int sampleCount;
    unsigned int sampleDelta;
  };

  std::vector<SttsEntry> readStts(TagLib::File *file, MP4::Atom *chapterTrak)
  {
    std::vector<SttsEntry> entries;
    MP4::Atom *stts = chapterTrak->find("mdia", "minf", "stbl", "stts");
    if(!stts)
      return entries;

    file->seek(stts->offset() + 12);  // skip header(8) + version/flags(4)
    ByteVector data = file->readBlock(stts->length() - 12);
    if(data.size() < 4)
      return entries;

    unsigned int count = data.toUInt();
    unsigned int pos = 4;
    for(unsigned int i = 0; i < count && pos + 8 <= data.size(); ++i) {
      SttsEntry e;
      e.sampleCount = data.toUInt(pos);
      e.sampleDelta = data.toUInt(pos + 4);
      entries.push_back(e);
      pos += 8;
    }
    return entries;
  }

  //! Reads chunk offsets from stco.
  std::vector<unsigned int> readStco(TagLib::File *file, MP4::Atom *chapterTrak)
  {
    std::vector<unsigned int> offsets;
    MP4::Atom *stco = chapterTrak->find("mdia", "minf", "stbl", "stco");
    if(!stco)
      return offsets;

    file->seek(stco->offset() + 12);
    ByteVector data = file->readBlock(stco->length() - 12);
    if(data.size() < 4)
      return offsets;

    unsigned int count = data.toUInt();
    unsigned int pos = 4;
    for(unsigned int i = 0; i < count && pos + 4 <= data.size(); ++i) {
      offsets.push_back(data.toUInt(pos));
      pos += 4;
    }
    return offsets;
  }

  //! Reads sample sizes.  Returns (defaultSize, perSampleSizes).
  //! If defaultSize > 0, perSampleSizes is empty.
  struct SampleSizeInfo {
    unsigned int defaultSize = 0;
    unsigned int sampleCount = 0;
    std::vector<unsigned int> perSampleSizes;
  };

  SampleSizeInfo readStsz(TagLib::File *file, MP4::Atom *chapterTrak)
  {
    SampleSizeInfo info;
    MP4::Atom *stsz = chapterTrak->find("mdia", "minf", "stbl", "stsz");
    if(!stsz)
      return info;

    file->seek(stsz->offset() + 12);
    ByteVector data = file->readBlock(stsz->length() - 12);
    if(data.size() < 8)
      return info;

    info.defaultSize = data.toUInt();
    info.sampleCount = data.toUInt(4U);

    if(info.defaultSize == 0) {
      unsigned int pos = 8;
      for(unsigned int i = 0; i < info.sampleCount && pos + 4 <= data.size(); ++i) {
        info.perSampleSizes.push_back(data.toUInt(pos));
        pos += 4;
      }
    }
    return info;
  }

  //! Resolves chunk-level offsets (stco) into per-sample file offsets
  //! using the stsc (sample-to-chunk) table and sample sizes from stsz.
  //! This handles both single-chunk and multi-chunk layouts.
  std::vector<unsigned int> resolveSampleOffsets(TagLib::File *file,
                                                  MP4::Atom *chapterTrak,
                                                  const SampleSizeInfo &sizeInfo)
  {
    std::vector<unsigned int> chunkOffsets = readStco(file, chapterTrak);
    if(chunkOffsets.empty())
      return {};

    // Read stsc entries
    struct StscEntry {
      unsigned int firstChunk;
      unsigned int samplesPerChunk;
      unsigned int descIndex;
    };
    std::vector<StscEntry> stscEntries;

    MP4::Atom *stsc = chapterTrak->find("mdia", "minf", "stbl", "stsc");
    if(stsc) {
      file->seek(stsc->offset() + 12);
      ByteVector data = file->readBlock(stsc->length() - 12);
      if(data.size() >= 4) {
        unsigned int entryCount = data.toUInt();
        unsigned int pos = 4;
        for(unsigned int i = 0; i < entryCount && pos + 12 <= data.size(); ++i) {
          StscEntry e;
          e.firstChunk = data.toUInt(pos);
          e.samplesPerChunk = data.toUInt(pos + 4);
          e.descIndex = data.toUInt(pos + 8);
          stscEntries.push_back(e);
          pos += 12;
        }
      }
    }

    // Default: 1 sample per chunk if no stsc
    if(stscEntries.empty()) {
      stscEntries.push_back({1, 1, 1});
    }

    // Resolve per-sample offsets by walking chunks
    std::vector<unsigned int> sampleOffsets;
    unsigned int totalChunks = static_cast<unsigned int>(chunkOffsets.size());
    unsigned int sampleIndex = 0;

    for(unsigned int chunkIdx = 0; chunkIdx < totalChunks; ++chunkIdx) {
      // Find which stsc entry applies to this chunk (1-based)
      unsigned int chunkNum = chunkIdx + 1;
      unsigned int samplesInChunk = stscEntries[0].samplesPerChunk;
      for(unsigned int e = 0; e < stscEntries.size(); ++e) {
        if(stscEntries[e].firstChunk <= chunkNum) {
          samplesInChunk = stscEntries[e].samplesPerChunk;
        }
        else {
          break;
        }
      }

      unsigned int offsetInChunk = 0;
      for(unsigned int s = 0; s < samplesInChunk; ++s) {
        sampleOffsets.push_back(chunkOffsets[chunkIdx] + offsetInChunk);

        // Advance by this sample's size
        unsigned int sz = sizeInfo.defaultSize;
        if(sz == 0 && sampleIndex < sizeInfo.perSampleSizes.size())
          sz = sizeInfo.perSampleSizes[sampleIndex];
        offsetInChunk += sz;
        sampleIndex++;
      }
    }

    return sampleOffsets;
  }

  //! Read a text sample at a given file offset.
  String readTextSample(TagLib::File *file, unsigned int offset, unsigned int maxSize)
  {
    file->seek(offset);
    ByteVector data = file->readBlock(maxSize);
    if(data.size() < 2)
      return String();

    unsigned int textLen = data.toUShort();
    if(textLen == 0 || textLen + 2 > data.size())
      return String();

    return String(data.mid(2, textLen), String::UTF8);
  }

  // -- Remove helpers -------------------------------------------------------

  //! Removes the tref atom from the audio track.
  //! Updates trak size, parent sizes, and chunk offsets.
  //! audioTrak's in-memory children list is NOT modified (caller re-parses if needed).
  void removeAudioTref(TagLib::File *file, MP4::Atoms *atoms, MP4::Atom *audioTrak)
  {
    for(const auto &child : audioTrak->children()) {
      if(child->name() != "tref")
        continue;

      offset_t trefOff = child->offset();
      offset_t trefLen = child->length();

      file->removeBlock(trefOff, trefLen);

      // Fix audio trak size on disk
      file->seek(audioTrak->offset());
      unsigned int trakSize = file->readBlock(4).toUInt();
      file->seek(audioTrak->offset());
      file->writeBlock(ByteVector::fromUInt(
        static_cast<unsigned int>(trakSize - trefLen)));

      MP4::AtomList moovPath = atoms->path("moov");
      updateParentSizes(file, moovPath, -trefLen);
      updateChunkOffsets(file, atoms, -trefLen, trefOff);
      return;
    }
  }

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MP4::ChapterList
MP4::MP4QTChapterList::read(const char *path)
{
  MP4::File file(path, false);
  if(!file.isOpen() || !file.isValid())
    return ChapterList();

  Atoms atoms(&file);

  TrackInfo audio = findAudioTrack(&file, &atoms);
  if(!audio.trak)
    return ChapterList();

  Atom *chapterTrak = findChapterTrak(&file, &atoms, audio.trak);
  if(!chapterTrak)
    return ChapterList();

  ChapterTrackInfo trackInfo = readChapterTrackInfo(&file, chapterTrak);
  if(trackInfo.timescale == 0)
    return ChapterList();

  std::vector<SttsEntry> sttsEntries = readStts(&file, chapterTrak);
  SampleSizeInfo sizeInfo = readStsz(&file, chapterTrak);
  std::vector<unsigned int> offsets = resolveSampleOffsets(&file, chapterTrak, sizeInfo);

  if(offsets.empty())
    return ChapterList();

  ChapterList chapters;
  unsigned int sampleIndex = 0;
  long long currentTime = 0;

  for(const auto &entry : sttsEntries) {
    for(unsigned int s = 0; s < entry.sampleCount; ++s) {
      if(sampleIndex >= offsets.size())
        break;

      unsigned int sampleSize = sizeInfo.defaultSize;
      if(sampleSize == 0 && sampleIndex < sizeInfo.perSampleSizes.size())
        sampleSize = sizeInfo.perSampleSizes[sampleIndex];

      String title = readTextSample(&file, offsets[sampleIndex], sampleSize);

      long long startTime100ns = static_cast<long long>(
        static_cast<double>(currentTime) * 10000000.0 /
        static_cast<double>(trackInfo.timescale) + 0.5);

      Chapter ch;
      ch.startTime = startTime100ns;
      ch.title = title;
      chapters.append(ch);

      currentTime += entry.sampleDelta;
      sampleIndex++;
    }
  }

  // Strip a leading dummy chapter (empty title at time 0) that was inserted
  // during write to preserve non-zero first-chapter start times.
  if(chapters.size() > 1) {
    const Chapter &first = chapters.front();
    if(first.startTime == 0 && first.title.isEmpty()) {
      chapters.erase(chapters.begin());
    }
  }

  return chapters;
}

bool
MP4::MP4QTChapterList::write(const char *path, const ChapterList &chapters)
{
  MP4::File file(path, false);
  if(!file.isOpen() || !file.isValid() || file.readOnly()) {
    debug("MP4QTChapterList::write() -- Could not open file for writing");
    return false;
  }

  // ---- Phase 1: Parse and gather info ----

  Atoms atoms(&file);

  Atom *moov = atoms.find("moov");
  if(!moov) {
    debug("MP4QTChapterList::write() -- No moov atom found");
    return false;
  }

  MovieInfo movieInfo = readMovieInfo(&file, &atoms);
  if(movieInfo.durationMs <= 0) {
    debug("MP4QTChapterList::write() -- Could not determine file duration");
    return false;
  }
  long long durationMs = movieInfo.durationMs;

  TrackInfo audio = findAudioTrack(&file, &atoms);
  if(!audio.trak) {
    debug("MP4QTChapterList::write() -- No audio track found");
    return false;
  }

  // ---- Phase 2: Remove existing chapter data (if any) ----

  // Pointer to the Atoms object we'll use for the insert phase.
  // Points to `atoms` for fresh writes, or `cleanAtoms` after cleanup.
  Atoms *activeAtoms = &atoms;
  // Optional second parse -- only constructed when replacing existing chapters.
  std::unique_ptr<Atoms> cleanAtoms;

  Atom *existingChapter = findChapterTrak(&file, &atoms, audio.trak);
  if(existingChapter) {
    // Remove chapter trak FIRST (higher offset in file).
    offset_t chapterOff = existingChapter->offset();
    offset_t chapterLen = existingChapter->length();

    // Remove from in-memory tree so updateChunkOffsets skips its stco.
    moov->removeChild(existingChapter);
    delete existingChapter;

    file.removeBlock(chapterOff, chapterLen);

    AtomList moovPath = atoms.path("moov");
    updateParentSizes(&file, moovPath, -chapterLen);
    updateChunkOffsets(&file, &atoms, -chapterLen, chapterOff);

    // Remove tref from audio trak (lower offset, still valid).
    removeAudioTref(&file, &atoms, audio.trak);

    // Re-parse to get clean state after removals.
    cleanAtoms = std::make_unique<Atoms>(&file);
    activeAtoms = cleanAtoms.get();

    moov = activeAtoms->find("moov");
    if(!moov) {
      debug("MP4QTChapterList::write() -- moov disappeared after cleanup");
      return false;
    }
    audio = findAudioTrack(&file, activeAtoms);
    if(!audio.trak) {
      debug("MP4QTChapterList::write() -- No audio track after cleanup");
      return false;
    }
  }

  // ---- Phase 3: Build and insert new chapter data ----

  // QT chapter tracks always start at media time 0.  If the first chapter has a
  // non-zero start time, prepend a dummy chapter at time 0 with an empty title
  // so the absolute positions are preserved as stts durations.
  ChapterList workingChapters(chapters);
  if(!workingChapters.isEmpty() && workingChapters.front().startTime > 0) {
    Chapter dummy;
    dummy.startTime = 0;
    dummy.title = String();
    workingChapters.prepend(dummy);
  }

  unsigned int nextId = getNextTrackId(&file, activeAtoms);
  unsigned int chapterTrackId = nextId > 0 ? nextId : audio.trackId + 1;
  constexpr unsigned int timescale = 1000;
  std::vector<unsigned int> sampleSizes = calculateSampleSizes(workingChapters);

  // Build tref/chap atom for audio track
  ByteVector trefAtom = buildTref(chapterTrackId);

  // Two-pass build for chapter trak: first to measure size, then with correct stco offsets.
  ByteVector trakMeasure = buildChapterTrak(
    chapterTrackId, timescale, durationMs, workingChapters, sampleSizes, 0,
    movieInfo.duration);
  offset_t totalInsert = static_cast<offset_t>(trefAtom.size() + trakMeasure.size());
  // Text samples go inside an mdat atom at EOF.  stco offsets point past the 8-byte mdat header.
  offset_t textDataOffset = file.length() + totalInsert + 8;

  // Build final trak with correct stco offsets pointing to where text data will land.
  ByteVector trakAtom = buildChapterTrak(
    chapterTrackId, timescale, durationMs, workingChapters, sampleSizes, textDataOffset,
    movieInfo.duration);

  // Combined payload: tref (goes inside audio trak) + chapter trak (moov sibling)
  ByteVector combinedPayload = trefAtom;
  combinedPayload.append(trakAtom);

  // Insert at the end of the audio trak boundary.
  // tref is logically inside audio trak; chapter trak is logically after it.
  offset_t insertOffset = audio.trak->offset() + audio.trak->length();

  file.insert(combinedPayload, insertOffset, 0);

  // Fix audio trak size on disk -- only tref goes inside
  file.seek(audio.trak->offset());
  unsigned int audioTrakSize = file.readBlock(4).toUInt();
  unsigned int newAudioTrakSize = static_cast<unsigned int>(audioTrakSize + trefAtom.size());
  file.seek(audio.trak->offset());
  file.writeBlock(ByteVector::fromUInt(newAudioTrakSize));

  // Fix moov size -- both tref and chapter trak are inside moov
  AtomList moovPath = activeAtoms->path("moov");
  updateParentSizes(&file, moovPath, combinedPayload.size());

  // Fix existing chunk offsets -- only the ORIGINAL atom tree is iterated,
  // so the new chapter trak's stco (which already has correct offsets) is untouched.
  updateChunkOffsets(&file, activeAtoms, combinedPayload.size(), insertOffset);

  // ---- Phase 4: Append text samples in mdat at EOF ----

  ByteVector textSamples;
  for(const auto &ch : workingChapters) {
    textSamples.append(buildTextSample(ch.title));
  }
  // Wrap text samples in an mdat atom so players can find them.
  ByteVector mdatAtom = renderAtom("mdat", textSamples);

  file.seek(0, TagLib::File::End);
  file.writeBlock(mdatAtom);

  // ---- Phase 5: Update mvhd next_track_ID ----
  // mvhd is before insertOffset, so its offset is unchanged.

  unsigned int currentNextId = getNextTrackId(&file, activeAtoms);
  if(chapterTrackId >= currentNextId) {
    setNextTrackId(&file, activeAtoms, chapterTrackId + 1);
  }

  return true;
}

bool
MP4::MP4QTChapterList::remove(const char *path)
{
  MP4::File file(path, false);
  if(!file.isOpen() || !file.isValid() || file.readOnly()) {
    debug("MP4QTChapterList::remove() -- Could not open file for writing");
    return false;
  }

  Atoms atoms(&file);

  TrackInfo audio = findAudioTrack(&file, &atoms);
  if(!audio.trak)
    return true;  // No audio track -- nothing to do

  Atom *chapterTrak = findChapterTrak(&file, &atoms, audio.trak);
  if(!chapterTrak)
    return true;  // No chapter track -- nothing to do

  Atom *moov = atoms.find("moov");
  if(!moov)
    return false;

  // Remove chapter trak FIRST (higher offset in file).
  offset_t chapterOff = chapterTrak->offset();
  offset_t chapterLen = chapterTrak->length();

  // Remove from in-memory tree so updateChunkOffsets skips its stco.
  moov->removeChild(chapterTrak);
  delete chapterTrak;

  file.removeBlock(chapterOff, chapterLen);

  AtomList moovPath = atoms.path("moov");
  updateParentSizes(&file, moovPath, -chapterLen);
  updateChunkOffsets(&file, &atoms, -chapterLen, chapterOff);

  // Remove tref from audio trak (lower offset, still valid after chapter trak removal).
  removeAudioTref(&file, &atoms, audio.trak);

  return true;
}
