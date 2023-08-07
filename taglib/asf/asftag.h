/**************************************************************************
    copyright            : (C) 2005-2007 by Lukáš Lalinský
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

#ifndef TAGLIB_ASFTAG_H
#define TAGLIB_ASFTAG_H

#include "tag.h"
#include "tlist.h"
#include "tmap.h"
#include "taglib_export.h"
#include "asfattribute.h"

namespace TagLib {

  namespace ASF {

    typedef List<Attribute> AttributeList;
    typedef Map<String, AttributeList> AttributeListMap;

    class TAGLIB_EXPORT Tag : public TagLib::Tag {

      friend class File;

    public:

      Tag();

      ~Tag() override;

      Tag(const Tag &) = delete;
      Tag &operator=(const Tag &) = delete;

      /*!
       * Returns the track name.
       */
      String title() const override;

      /*!
       * Returns the artist name.
       */
      String artist() const override;

      /*!
       * Returns the album name; if no album name is present in the tag
       * String::null will be returned.
       */
      String album() const override;

      /*!
       * Returns the track comment.
       */
      String comment() const override;

      /*!
       * Returns the genre name; if no genre is present in the tag String::null
       * will be returned.
       */
      String genre() const override;

      /*!
       * Returns the rating.
       */
      virtual String rating() const;

      /*!
       * Returns the genre name; if no genre is present in the tag String::null
       * will be returned.
       */
      virtual String copyright() const;

      /*!
       * Returns the year; if there is no year set, this will return 0.
       */
      unsigned int year() const override;

      /*!
       * Returns the track number; if there is no track number set, this will
       * return 0.
       */
      unsigned int track() const override;

      /*!
       * Sets the title to \a s.
       */
      void setTitle(const String &s) override;

      /*!
       * Sets the artist to \a s.
       */
      void setArtist(const String &s) override;

      /*!
       * Sets the album to \a s.  If \a s is String::null then this value will be
       * cleared.
       */
      void setAlbum(const String &s) override;

      /*!
       * Sets the comment to \a s.
       */
      void setComment(const String &s) override;

      /*!
       * Sets the rating to \a s.
       */
      virtual void setRating(const String &s);

      /*!
       * Sets the copyright to \a s.
       */
      virtual void setCopyright(const String &s);

      /*!
       * Sets the genre to \a s.
       */
      void setGenre(const String &s) override;

      /*!
       * Sets the year to \a i.  If \a s is 0 then this value will be cleared.
       */
      void setYear(unsigned int i) override;

      /*!
       * Sets the track to \a i.  If \a s is 0 then this value will be cleared.
       */
      void setTrack(unsigned int i) override;

      /*!
       * Returns true if the tag does not contain any data.  This should be
       * reimplemented in subclasses that provide more than the basic tagging
       * abilities in this class.
       */
      bool isEmpty() const override;

      /*!
       * \warning You should not modify this data structure directly, instead
       * use attributeListMap() const, contains(), removeItem(),
       * attribute(), setAttribute(), addAttribute().
       */
      AttributeListMap &attributeListMap();

      /*!
       * Returns a reference to the item list map.  This is an AttributeListMap of
       * all of the items in the tag.
       */
      const AttributeListMap &attributeListMap() const;

      /*!
       * \return True if a value for \a attribute is currently set.
       */
      bool contains(const String &name) const;

      /*!
       * Removes the \a key attribute from the tag
       */
      void removeItem(const String &name);

      /*!
       * \return The list of values for the key \a name, or an empty list if no
       * values have been set.
       */
      AttributeList attribute(const String &name) const;

      /*!
       * Sets the \a key attribute to the value of \a attribute. If an attribute
       * with the \a key is already present, it will be replaced.
       */
      void setAttribute(const String &name, const Attribute &attribute);

      /*!
       * Sets multiple \a values to the key \a name.
       */
      void setAttribute(const String &name, const AttributeList &values);

      /*!
       * Sets the \a key attribute to the value of \a attribute. If an attribute
       * with the \a key is already present, it will be added to the list.
       */
      void addAttribute(const String &name, const Attribute &attribute);

      PropertyMap properties() const override;
      void removeUnsupportedProperties(const StringList& properties) override;
      PropertyMap setProperties(const PropertyMap &properties) override;

    private:

      class TagPrivate;
      std::unique_ptr<TagPrivate> d;
    };
  }  // namespace ASF
}  // namespace TagLib
#endif
