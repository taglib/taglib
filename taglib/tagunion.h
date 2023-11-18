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

#include "tag.h"

#ifndef DO_NOT_DOCUMENT

namespace TagLib {

  /*!
   * \internal
   */

  class TagUnion : public Tag
  {
  public:

    enum AccessType { Read, Write };

    /*!
     * Creates a TagLib::Tag that is the union of \a first, \a second, and
     * \a third.  The TagUnion takes ownership of these tags and will handle
     * their deletion.
     */
    TagUnion(Tag *first = nullptr, Tag *second = nullptr, Tag *third = nullptr);

    ~TagUnion() override;

    TagUnion(const TagUnion &) = delete;
    TagUnion &operator=(const TagUnion &) = delete;

    Tag *operator[](int index) const;
    Tag *tag(int index) const;

    void set(int index, Tag *tag);

    PropertyMap properties() const override;
    void removeUnsupportedProperties(const StringList &unsupported) override;

    StringList complexPropertyKeys() const override;
    List<VariantMap> complexProperties(const String &key) const override;
    bool setComplexProperties(const String &key, const List<VariantMap> &value) override;

    String title() const override;
    String artist() const override;
    String album() const override;
    String comment() const override;
    String genre() const override;
    unsigned int year() const override;
    unsigned int track() const override;

    void setTitle(const String &s) override;
    void setArtist(const String &s) override;
    void setAlbum(const String &s) override;
    void setComment(const String &s) override;
    void setGenre(const String &s) override;
    void setYear(unsigned int i) override;
    void setTrack(unsigned int i) override;
    bool isEmpty() const override;

    template <class T> T *access(int index, bool create)
    {
      if(!create || tag(index))
        return static_cast<T *>(tag(index));

      set(index, new T);
      return static_cast<T *>(tag(index));
    }

    template <class T, class F> T *access(int index, bool create, const F *factory)
    {
      if(!create || tag(index))
        return static_cast<T *>(tag(index));

      set(index, new T(nullptr, 0, factory));
      return static_cast<T *>(tag(index));
    }

  private:
    TagUnion(const Tag &);
    TagUnion &operator=(const Tag &);

    class TagUnionPrivate;
    std::unique_ptr<TagUnionPrivate> d;
  };
}  // namespace TagLib

#endif
#endif
