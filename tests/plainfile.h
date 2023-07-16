/***************************************************************************
    copyright           : (C) 2015 by Tsuda Kageyu
    email               : tsuda.kageyu@gmail.com
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

#ifndef TAGLIB_PLAINFILE_H
#define TAGLIB_PLAINFILE_H

#include "tfile.h"

using namespace TagLib;

//! File subclass that gives tests access to filesystem operations
class PlainFile : public File {
public:
  explicit PlainFile(FileName name) : File(name) { }
  Tag *tag() const override { return nullptr; }
  AudioProperties *audioProperties() const override { return nullptr; }
  bool save() override { return false; }
  void truncate(long length) { File::truncate(length); }

  ByteVector readAll() {
    seek(0, End);
    offset_t end = tell();
    seek(0);
    return readBlock(end);
  }
};

#endif
