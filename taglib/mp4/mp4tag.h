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

#ifndef TAGLIB_MP4TAG_H
#define TAGLIB_MP4TAG_H

#include "tbytevectorlist.h"
#include "tfile.h"
#include "tmap.h"
#include "tstringlist.h"
#include "taglib_export.h"
#include "tag.h"
#include "mp4atom.h"
#include "mp4item.h"

namespace TagLib {
  namespace MP4 {
    using ItemMap = TagLib::Map<String, Item>;

    class TAGLIB_EXPORT Tag: public TagLib::Tag
    {
    public:
        Tag();
        Tag(TagLib::File *file, Atoms *atoms);
        ~Tag() override;
        Tag(const Tag &) = delete;
        Tag &operator=(const Tag &) = delete;
        bool save();

        String title() const override;
        String artist() const override;
        String album() const override;
        String comment() const override;
        String genre() const override;
        unsigned int year() const override;
        unsigned int track() const override;

        void setTitle(const String &value) override;
        void setArtist(const String &value) override;
        void setAlbum(const String &value) override;
        void setComment(const String &value) override;
        void setGenre(const String &value) override;
        void setYear(unsigned int value) override;
        void setTrack(unsigned int value) override;

        bool isEmpty() const override;

        /*!
         * Returns a string-keyed map of the MP4::Items for this tag.
         */
        const ItemMap &itemMap() const;

        /*!
         * \return The item, if any, corresponding to \a key.
         */
        Item item(const String &key) const;

        /*!
         * Sets the value of \a key to \a value, overwriting any previous value.
         */
        void setItem(const String &key, const Item &value);

        /*!
         * Removes the entry with \a key from the tag, or does nothing if it does
         * not exist.
         */
        void removeItem(const String &key);

        /*!
         * \return True if the tag contains an entry for \a key.
         */
        bool contains(const String &key) const;

        /*!
         * Saves the associated file with the tag stripped.
         */
        bool strip();

        PropertyMap properties() const override;
        void removeUnsupportedProperties(const StringList &props) override;
        PropertyMap setProperties(const PropertyMap &props) override;

        StringList complexPropertyKeys() const override;
        List<VariantMap> complexProperties(const String &key) const override;
        bool setComplexProperties(const String &key, const List<VariantMap> &value) override;

      protected:
        /*!
         * Sets the value of \a key to \a value, overwriting any previous value.
         * If \a value is empty, the item is removed.
         */
        void setTextItem(const String &key, const String &value);

    private:
        AtomDataList parseData2(const Atom *atom, int expectedFlags = -1,
                                bool freeForm = false);
        ByteVectorList parseData(const Atom *atom, int expectedFlags = -1,
                                 bool freeForm = false);
        void parseText(const Atom *atom, int expectedFlags = 1);
        void parseFreeForm(const Atom *atom);
        void parseInt(const Atom *atom);
        void parseByte(const Atom *atom);
        void parseUInt(const Atom *atom);
        void parseLongLong(const Atom *atom);
        void parseGnre(const Atom *atom);
        void parseIntPair(const Atom *atom);
        void parseBool(const Atom *atom);
        void parseCovr(const Atom *atom);

        ByteVector padIlst(const ByteVector &data, int length = -1) const;
        ByteVector renderAtom(const ByteVector &name, const ByteVector &data) const;
        ByteVector renderData(const ByteVector &name, int flags,
                              const ByteVectorList &data) const;
        ByteVector renderText(const ByteVector &name, const Item &item,
                              int flags = TypeUTF8) const;
        ByteVector renderFreeForm(const String &name, const Item &item) const;
        ByteVector renderBool(const ByteVector &name, const Item &item) const;
        ByteVector renderInt(const ByteVector &name, const Item &item) const;
        ByteVector renderByte(const ByteVector &name, const Item &item) const;
        ByteVector renderUInt(const ByteVector &name, const Item &item) const;
        ByteVector renderLongLong(const ByteVector &name, const Item &item) const;
        ByteVector renderIntPair(const ByteVector &name, const Item &item) const;
        ByteVector renderIntPairNoTrailing(const ByteVector &name, const Item &item) const;
        ByteVector renderCovr(const ByteVector &name, const Item &item) const;

        void updateParents(const AtomList &path, offset_t delta, int ignore = 0);
        void updateOffsets(offset_t delta, offset_t offset);

        void saveNew(ByteVector data);
        void saveExisting(ByteVector data, const AtomList &path);

        void addItem(const String &name, const Item &value);

        class TagPrivate;
        std::unique_ptr<TagPrivate> d;
    };
  }  // namespace MP4
}  // namespace TagLib
#endif
