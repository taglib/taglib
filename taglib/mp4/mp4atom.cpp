/**************************************************************************
    copyright            : (C) 2007 by Lukáš Lalinský
    email                : lalinsky@gmail.com
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

#include "mp4atom.h"

#include <array>
#include <climits>
#include <utility>

#include "tdebug.h"

using namespace TagLib;

namespace {
  constexpr std::array containers {
    "moov", "udta", "mdia", "meta", "ilst",
    "stbl", "minf", "moof", "traf", "trak",
    "stsd"
  };
} // namespace

class MP4::Atom::AtomPrivate
{
public:
  explicit AtomPrivate(offset_t ofs) : offset(ofs) {}
  offset_t offset;
  offset_t length { 0 };
  TagLib::ByteVector name;
  AtomList children;
};

MP4::Atom::Atom(File *file)
  : d(std::make_unique<AtomPrivate>(file->tell()))
{
  d->children.setAutoDelete(true);

  ByteVector header = file->readBlock(8);
  if(header.size() != 8) {
    // The atom header must be 8 bytes long, otherwise there is either
    // trailing garbage or the file is truncated
    debug("MP4: Couldn't read 8 bytes of data for atom header");
    d->length = 0;
    file->seek(0, File::End);
    return;
  }

  d->length = header.toUInt();

  if(d->length == 0) {
    // The last atom which extends to the end of the file.
    d->length = file->length() - d->offset;
  }
  else if(d->length == 1) {
    // The atom has a 64-bit length.
    if(const long long longLength = file->readBlock(8).toLongLong();
       longLength <= LONG_MAX) {
      // The actual length fits in long. That's always the case if long is 64-bit.
      d->length = static_cast<long>(longLength);
    }
    else {
      debug("MP4: 64-bit atoms are not supported");
      d->length = 0;
      file->seek(0, File::End);
      return;
    }
  }

  if(d->length < 8 || d->length > file->length() - d->offset) {
    debug("MP4: Invalid atom size");
    d->length = 0;
    file->seek(0, File::End);
    return;
  }

  d->name = header.mid(4, 4);

  for(auto c : containers) {
    if(d->name == c) {
      if(d->name == "meta") {
        offset_t posAfterMeta = file->tell();
        static constexpr std::array metaChildrenNames {
          "hdlr", "ilst", "mhdr", "ctry", "lang"
        };
        // meta is not a full atom (i.e. not followed by version, flags). It
        // is followed by the size and type of the first child atom.
        auto metaIsFullAtom = std::none_of(metaChildrenNames.begin(), metaChildrenNames.end(),
          [nextSize = file->readBlock(8).mid(4, 4)](const auto &child) { return nextSize == child; });
        // Only skip next four bytes, which contain version and flags, if meta
        // is a full atom.
        file->seek(posAfterMeta + (metaIsFullAtom ? 4 : 0));
      }
      else if(d->name == "stsd") {
        file->seek(8, File::Current);
      }
      while(file->tell() < d->offset + d->length) {
        auto child = new MP4::Atom(file);
        d->children.append(child);
        if(child->d->length == 0)
          return;
      }
      return;
    }
  }

  file->seek(d->offset + d->length);
}

MP4::Atom::~Atom() = default;

MP4::Atom *
MP4::Atom::find(const char *name1, const char *name2, const char *name3, const char *name4)
{
  if(name1 == nullptr) {
    return this;
  }
  auto it = std::find_if(d->children.cbegin(), d->children.cend(),
      [&name1](const Atom *child) { return child->d->name == name1; });
  return it != d->children.cend() ? (*it)->find(name2, name3, name4) : nullptr;
}

MP4::AtomList
MP4::Atom::findall(const char *name, bool recursive) const
{
  MP4::AtomList result;
  for(const auto &child : std::as_const(d->children)) {
    if(child->d->name == name) {
      result.append(child);
    }
    if(recursive) {
      result.append(child->findall(name, recursive));
    }
  }
  return result;
}

bool
MP4::Atom::path(MP4::AtomList &path, const char *name1, const char *name2, const char *name3)
{
  path.append(this);
  if(name1 == nullptr) {
    return true;
  }
  auto it = std::find_if(d->children.cbegin(), d->children.cend(),
      [&name1](const Atom *child) { return child->d->name == name1; });
  return it != d->children.cend() ? (*it)->path(path, name2, name3) : false;
}

void MP4::Atom::addToOffset(offset_t delta)
{
  d->offset += delta;
}

void MP4::Atom::prependChild(Atom *atom)
{
  d->children.prepend(atom);
}

bool MP4::Atom::removeChild(Atom *meta)
{
  auto it = d->children.find(meta);
  if(it != d->children.end()) {
    d->children.erase(it);
    return true;
  }
  return false;
}

offset_t MP4::Atom::offset() const
{
  return d->offset;
}

offset_t MP4::Atom::length() const
{
  return d->length;
}

const ByteVector &MP4::Atom::name() const
{
  return d->name;
}

const MP4::AtomList &MP4::Atom::children() const
{
  return d->children;
}


class MP4::Atoms::AtomsPrivate
{
public:
  AtomList atoms;
};

MP4::Atoms::Atoms(File *file) :
  d(std::make_unique<AtomsPrivate>())
{
  d->atoms.setAutoDelete(true);

  file->seek(0, File::End);
  offset_t end = file->tell();
  file->seek(0);
  while(file->tell() + 8 <= end) {
    auto atom = new MP4::Atom(file);
    d->atoms.append(atom);
    if (atom->length() == 0)
      break;
  }
}

MP4::Atoms::~Atoms() = default;

MP4::Atom *
MP4::Atoms::find(const char *name1, const char *name2, const char *name3, const char *name4) const
{
  auto it = std::find_if(d->atoms.cbegin(), d->atoms.cend(),
      [&name1](const Atom *atom) { return atom->name() == name1; });
  return it != d->atoms.cend() ? (*it)->find(name2, name3, name4) : nullptr;
}

MP4::AtomList
MP4::Atoms::path(const char *name1, const char *name2, const char *name3, const char *name4) const
{
  MP4::AtomList path;
  auto it = std::find_if(d->atoms.cbegin(), d->atoms.cend(),
      [&name1](const Atom *atom) { return atom->name() == name1; });
  if(it != d->atoms.cend()) {
    if(!(*it)->path(path, name2, name3, name4)) {
      path.clear();
    }
    return path;
  }
  return path;
}

namespace
{
  bool checkValid(const MP4::AtomList &list)
  {
    return std::none_of(list.begin(), list.end(),
      [](const auto &a) { return a->length() == 0 || !checkValid(a->children()); });
  }
}  // namespace

bool MP4::Atoms::checkRootLevelAtoms() {
  bool moovValid = false;
  for(auto it = d->atoms.begin(); it != d->atoms.end(); ++it) {
    bool invalid = (*it)->length() == 0 || !checkValid((*it)->children());
    if(!moovValid && !invalid && (*it)->name() == "moov") {
      moovValid = true;
    }
    if(invalid) {
      if(!moovValid || (*it)->name() == "moof")
        return false;

      // Only the root level atoms "moov" and (if present) "moof" are
      // modified.  If they are valid, ignore following invalid root level
      // atoms as trailing garbage.
      while(it != d->atoms.end()) {
        delete *it;
        it = d->atoms.erase(it);
      }
      return true;
    }
  }

  return true;
}

const MP4::AtomList &MP4::Atoms::atoms() const
{
  return d->atoms;
}
