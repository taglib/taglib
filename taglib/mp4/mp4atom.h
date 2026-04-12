/**************************************************************************
    copyright            : (C) 2007,2011 by Lukáš Lalinský
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

// This file is not part of the public API!

#ifndef TAGLIB_MP4ATOM_H
#define TAGLIB_MP4ATOM_H

#include "tfile.h"
#include "tlist.h"

namespace TagLib {
  namespace MP4 {

    enum AtomDataType {
      //! For use with tags for which no type needs to be indicated because only one type is allowed
      TypeImplicit  = 0,
      //! Without any count or null terminator
      TypeUTF8      = 1,
      //! Also known as UTF-16BE
      TypeUTF16     = 2,
      //! Deprecated unless it is needed for special Japanese characters
      TypeSJIS      = 3,
      //! The HTML file header specifies which HTML version
      TypeHTML      = 6,
      //! The XML header must identify the DTD or schemas
      TypeXML       = 7,
      //! Also known as GUID; stored as 16 bytes in binary (valid as an ID)
      TypeUUID      = 8,
      //! Stored as UTF-8 text (valid as an ID)
      TypeISRC      = 9,
      //! Stored as UTF-8 text (valid as an ID)
      TypeMI3P      = 10,
      //! (Deprecated) A GIF image
      TypeGIF       = 12,
      //! A JPEG image
      TypeJPEG      = 13,
      //! A PNG image
      TypePNG       = 14,
      //! Absolute, in UTF-8 characters
      TypeURL       = 15,
      //! In milliseconds, 32-bit integer
      TypeDuration  = 16,
      //! In UTC, counting seconds since midnight, January 1, 1904; 32 or 64-bits
      TypeDateTime  = 17,
      //! A list of enumerated values
      TypeGenred    = 18,
      //! A signed big-endian integer with length one of { 1,2,3,4,8 } bytes
      TypeInteger   = 21,
      //! RIAA parental advisory; { -1=no, 1=yes, 0=unspecified }, 8-bit integer
      TypeRIAAPA    = 24,
      //! Universal Product Code, in text UTF-8 format (valid as an ID)
      TypeUPC       = 25,
      //! Windows bitmap image
      TypeBMP       = 27,
      //! Undefined
      TypeUndefined = 255
    };

#ifndef DO_NOT_DOCUMENT
    struct AtomData {
      AtomData(AtomDataType ptype, const ByteVector &pdata) :
        type(ptype), data(pdata) { }
      AtomDataType type;
      int locale { 0 };
      ByteVector data;
    };

    class Atom;
    using AtomList = TagLib::List<Atom *>;
    using AtomDataList = TagLib::List<AtomData>;

    class TAGLIB_EXPORT Atom
    {
    public:
      Atom(File *file);
      ~Atom();
      Atom(const Atom &) = delete;
      Atom &operator=(const Atom &) = delete;
      Atom *find(const char *name1, const char *name2 = nullptr, const char *name3 = nullptr, const char *name4 = nullptr);
      bool path(AtomList &path, const char *name1, const char *name2 = nullptr, const char *name3 = nullptr);
      AtomList findall(const char *name, bool recursive = false) const;
      void addToOffset(offset_t delta);
      void prependChild(Atom *atom);
      bool removeChild(Atom *meta);
      offset_t offset() const;
      offset_t length() const;
      const ByteVector &name() const;
      const AtomList &children() const;

    protected:
      Atom(File *file, int depth);

    private:
      class AtomPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<AtomPrivate> d;
    };

    //! Root-level atoms
    class TAGLIB_EXPORT Atoms
    {
    public:
      Atoms(File *file);
      ~Atoms();
      Atoms(const Atoms &) = delete;
      Atoms &operator=(const Atoms &) = delete;
      Atom *find(const char *name1, const char *name2 = nullptr, const char *name3 = nullptr, const char *name4 = nullptr) const;
      AtomList path(const char *name1, const char *name2 = nullptr, const char *name3 = nullptr, const char *name4 = nullptr) const;
      bool checkRootLevelAtoms();
      const AtomList &atoms() const;

    private:
      class AtomsPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<AtomsPrivate> d;
    };
#endif  // DO_NOT_DOCUMENT
  } // namespace MP4
} // namespace TagLib

#endif
