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

#include "mp4chapterlist.h"

#include <algorithm>

#include "tdebug.h"
#include "mp4file.h"
#include "mp4atom.h"

using namespace TagLib;

namespace
{
  // Nero chpl version 1 header: version(1) + flags(3) + reserved(4) + count(1) = 9 bytes
  constexpr int chplHeaderSize = 9;

  ByteVector renderAtom(const ByteVector &name, const ByteVector &data)
  {
    return ByteVector::fromUInt(static_cast<unsigned int>(data.size() + 8)) + name + data;
  }

  // Update parent atom sizes along a path when child size changes by delta.
  // Mirrors MP4::Tag::updateParents().
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
        // 64-bit size
        file->seek(4, TagLib::File::Current);
        long long longSize = file->readBlock(8).toLongLong();
        file->seek((*it)->offset() + 8);
        file->writeBlock(ByteVector::fromLongLong(longSize + delta));
      }
      else {
        // 32-bit size
        file->seek((*it)->offset());
        file->writeBlock(ByteVector::fromUInt(static_cast<unsigned int>(size + delta)));
      }
    }
  }

  // Update stco/co64/tfhd chunk offsets when file content shifts.
  // Mirrors MP4::Tag::updateOffsets().
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

  // Build the binary payload for a chpl atom (version 1).
  ByteVector renderChplData(const MP4::ChapterList &chapters)
  {
    unsigned int count = std::min(static_cast<unsigned int>(chapters.size()), 255U);

    ByteVector data;
    // Version (1 byte) + flags (3 bytes) + reserved (4 bytes)
    data.append(static_cast<char>(0x01));        // version 1
    data.append(ByteVector(3, '\0'));             // flags
    data.append(ByteVector(4, '\0'));             // reserved

    // Chapter count
    data.append(static_cast<char>(count & 0xFF));

    unsigned int i = 0;
    for(const auto &ch : chapters) {
      if(i++ >= count)
        break;

      // Start time: 8 bytes big-endian
      data.append(ByteVector::fromLongLong(ch.startTime));

      // Title: 1-byte length + UTF-8 bytes (max 255 bytes)
      ByteVector titleBytes = ch.title.data(String::UTF8);
      unsigned int titleLen = std::min(static_cast<unsigned int>(titleBytes.size()), 255U);
      data.append(static_cast<char>(titleLen & 0xFF));
      if(titleLen > 0)
        data.append(titleBytes.mid(0, titleLen));
    }

    return data;
  }

  // Parse the binary content of a chpl atom into a ChapterList.
  MP4::ChapterList parseChplData(const ByteVector &data)
  {
    MP4::ChapterList chapters;

    if(data.size() < static_cast<unsigned int>(chplHeaderSize))
      return chapters;

    unsigned int pos = 0;
    unsigned char version = static_cast<unsigned char>(data[pos++]);

    // Skip flags (3 bytes)
    pos += 3;

    // Version 1 has 4 reserved bytes
    if(version >= 1)
      pos += 4;

    if(pos >= data.size())
      return chapters;

    unsigned int count = static_cast<unsigned char>(data[pos++]);

    for(unsigned int i = 0; i < count && pos + 9 <= data.size(); ++i) {
      long long startTime = data.toLongLong(pos);
      pos += 8;

      unsigned int titleLen = static_cast<unsigned char>(data[pos++]);

      String title;
      if(titleLen > 0 && pos + titleLen <= data.size()) {
        title = String(data.mid(pos, titleLen), String::UTF8);
        pos += titleLen;
      }

      MP4::Chapter ch;
      ch.startTime = startTime;
      ch.title = title;
      chapters.append(ch);
    }

    return chapters;
  }

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MP4::ChapterList
MP4::MP4ChapterList::read(const char *path)
{
  MP4::File file(path, false);
  if(!file.isOpen() || !file.isValid()) {
    debug("MP4ChapterList::read() -- Could not open file");
    return ChapterList();
  }

  Atoms atoms(&file);

  Atom *chpl = atoms.find("moov", "udta", "chpl");
  if(!chpl)
    return ChapterList();

  // Read the atom content (skip 8-byte atom header)
  file.seek(chpl->offset() + 8);
  ByteVector data = file.readBlock(chpl->length() - 8);

  return parseChplData(data);
}

bool
MP4::MP4ChapterList::write(const char *path, const ChapterList &chapters)
{
  MP4::File file(path, false);
  if(!file.isOpen() || !file.isValid() || file.readOnly()) {
    debug("MP4ChapterList::write() -- Could not open file for writing");
    return false;
  }

  Atoms atoms(&file);

  if(!atoms.find("moov")) {
    debug("MP4ChapterList::write() -- No moov atom found");
    return false;
  }

  ByteVector chplPayload = renderChplData(chapters);
  ByteVector chplAtom = renderAtom("chpl", chplPayload);

  Atom *existingChpl = atoms.find("moov", "udta", "chpl");

  if(existingChpl) {
    // Replace existing chpl atom
    offset_t offset = existingChpl->offset();
    offset_t oldLength = existingChpl->length();
    offset_t delta = static_cast<offset_t>(chplAtom.size()) - oldLength;

    file.insert(chplAtom, offset, oldLength);

    if(delta != 0) {
      // Update parent sizes: moov and udta
      AtomList parentPath = atoms.path("moov", "udta", "chpl");
      updateParentSizes(&file, parentPath, delta, 1);  // ignore chpl itself
      updateChunkOffsets(&file, &atoms, delta, offset);
    }
  }
  else {
    // Need to insert a new chpl atom
    AtomList udtaPath = atoms.path("moov", "udta");

    if(udtaPath.size() == 2) {
      // udta exists -- insert chpl at the beginning of udta's content
      offset_t insertOffset = udtaPath.back()->offset() + 8;
      file.insert(chplAtom, insertOffset, 0);

      updateParentSizes(&file, udtaPath, chplAtom.size());
      updateChunkOffsets(&file, &atoms, chplAtom.size(), insertOffset);
    }
    else {
      // No udta -- insert udta + chpl at the beginning of moov's content
      ByteVector udtaAtom = renderAtom("udta", chplAtom);

      AtomList moovPath = atoms.path("moov");
      if(moovPath.isEmpty()) {
        debug("MP4ChapterList::write() -- No moov atom in path");
        return false;
      }

      offset_t insertOffset = moovPath.back()->offset() + 8;
      file.insert(udtaAtom, insertOffset, 0);

      updateParentSizes(&file, moovPath, udtaAtom.size());
      updateChunkOffsets(&file, &atoms, udtaAtom.size(), insertOffset);
    }
  }

  return true;
}

bool
MP4::MP4ChapterList::remove(const char *path)
{
  MP4::File file(path, false);
  if(!file.isOpen() || !file.isValid() || file.readOnly()) {
    debug("MP4ChapterList::remove() -- Could not open file for writing");
    return false;
  }

  Atoms atoms(&file);

  Atom *chpl = atoms.find("moov", "udta", "chpl");
  if(!chpl) {
    // No chpl atom -- nothing to remove
    return true;
  }

  offset_t offset = chpl->offset();
  offset_t length = chpl->length();

  file.removeBlock(offset, length);

  // Update parent sizes with negative delta
  AtomList parentPath = atoms.path("moov", "udta", "chpl");
  updateParentSizes(&file, parentPath, -length, 1);  // ignore chpl itself
  updateChunkOffsets(&file, &atoms, -length, offset);

  return true;
}
