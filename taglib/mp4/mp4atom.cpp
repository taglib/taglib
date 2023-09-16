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

MP4::Atom::Atom(File *file)
  : offset(file->tell())
{
  children.setAutoDelete(true);

  ByteVector header = file->readBlock(8);
  if(header.size() != 8) {
    // The atom header must be 8 bytes long, otherwise there is either
    // trailing garbage or the file is truncated
    debug("MP4: Couldn't read 8 bytes of data for atom header");
    length = 0;
    file->seek(0, File::End);
    return;
  }

  length = header.toUInt();

  if(length == 0) {
    // The last atom which extends to the end of the file.
    length = file->length() - offset;
  }
  else if(length == 1) {
    // The atom has a 64-bit length.
    const long long longLength = file->readBlock(8).toLongLong();
    if(longLength <= LONG_MAX) {
      // The actual length fits in long. That's always the case if long is 64-bit.
      length = static_cast<long>(longLength);
    }
    else {
      debug("MP4: 64-bit atoms are not supported");
      length = 0;
      file->seek(0, File::End);
      return;
    }
  }

  if(length < 8 || length > file->length() - offset) {
    debug("MP4: Invalid atom size");
    length = 0;
    file->seek(0, File::End);
    return;
  }

  name = header.mid(4, 4);
  for(int i = 0; i < 4; ++i) {
    const char ch = name.at(i);
    if((ch < ' ' || ch > '~') && ch != '\251') {
      debug("MP4: Invalid atom type");
      length = 0;
      file->seek(0, File::End);
    }
  }

  for(auto c : containers) {
    if(name == c) {
      if(name == "meta") {
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
      else if(name == "stsd") {
        file->seek(8, File::Current);
      }
      while(file->tell() < offset + length) {
        auto child = new MP4::Atom(file);
        children.append(child);
        if(child->length == 0)
          return;
      }
      return;
    }
  }

  file->seek(offset + length);
}

MP4::Atom::~Atom() = default;

MP4::Atom *
MP4::Atom::find(const char *name1, const char *name2, const char *name3, const char *name4)
{
  if(name1 == nullptr) {
    return this;
  }
  for(const auto &child : std::as_const(children)) {
    if(child->name == name1) {
      return child->find(name2, name3, name4);
    }
  }
  return nullptr;
}

MP4::AtomList
MP4::Atom::findall(const char *name, bool recursive)
{
  MP4::AtomList result;
  for(const auto &child : std::as_const(children)) {
    if(child->name == name) {
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
  for(const auto &child : std::as_const(children)) {
    if(child->name == name1) {
      return child->path(path, name2, name3);
    }
  }
  return false;
}

MP4::Atoms::Atoms(File *file)
{
  atoms.setAutoDelete(true);

  file->seek(0, File::End);
  offset_t end = file->tell();
  file->seek(0);
  while(file->tell() + 8 <= end) {
    auto atom = new MP4::Atom(file);
    atoms.append(atom);
    if (atom->length == 0)
      break;
  }
}

MP4::Atoms::~Atoms() = default;

MP4::Atom *
MP4::Atoms::find(const char *name1, const char *name2, const char *name3, const char *name4)
{
  for(const auto &atom : std::as_const(atoms)) {
    if(atom->name == name1) {
      return atom->find(name2, name3, name4);
    }
  }
  return nullptr;
}

MP4::AtomList
MP4::Atoms::path(const char *name1, const char *name2, const char *name3, const char *name4)
{
  MP4::AtomList path;
  for(const auto &atom : std::as_const(atoms)) {
    if(atom->name == name1) {
      if(!atom->path(path, name2, name3, name4)) {
        path.clear();
      }
      return path;
    }
  }
  return path;
}
