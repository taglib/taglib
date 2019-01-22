/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#ifndef TAGLIB_TAGUNION_H
#define TAGLIB_TAGUNION_H

// This file is not a part of TagLib public interface. This is not installed.

#include "tag.h"

#ifndef DO_NOT_DOCUMENT

namespace TagLib {

  /*!
   * \internal
   */

  template <size_t COUNT>
  class TagUnion : public Tag
  {
  public:

    enum AccessType { Read, Write };

    /*!
     * Creates a TagLib::Tag that is the union of \a count tags.
     */
    TagUnion();

    virtual ~TagUnion();

    Tag *operator[](size_t index) const;
    Tag *tag(size_t index) const;

    void set(size_t index, Tag *tag);

    virtual PropertyMap properties() const;
    virtual void removeUnsupportedProperties(const StringList& properties);
    virtual PropertyMap setProperties(const PropertyMap& properties);

    virtual String title() const;
    virtual String artist() const;
    virtual String album() const;
    virtual String comment() const;
    virtual String genre() const;
    virtual unsigned int year() const;
    virtual unsigned int track() const;
    virtual PictureMap pictures() const;

    virtual void setTitle(const String &s);
    virtual void setArtist(const String &s);
    virtual void setAlbum(const String &s);
    virtual void setComment(const String &s);
    virtual void setGenre(const String &s);
    virtual void setYear(unsigned int i);
    virtual void setTrack(unsigned int i);
    virtual void setPictures( const PictureMap& l );

    virtual bool isEmpty() const;

    template <class T> T *access(size_t index, bool create)
    {
      if(!create || tag(index))
        return static_cast<T *>(tag(index));

      set(index, new T);
      return static_cast<T *>(tag(index));
    }

  private:
    class TagUnionPrivate;
    TagUnionPrivate *d;
  };

  // If you add a new typedef here, add a corresponding explicit instantiation
  // at the end of tagunion.cpp as well.

  typedef TagUnion<2> DoubleTagUnion;
  typedef TagUnion<3> TripleTagUnion;
}

#endif
#endif
