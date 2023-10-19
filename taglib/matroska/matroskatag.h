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

#ifndef HAS_MATROSKATAG_H
#define HAS_MATROSKATAG_H

#include <memory>
#include <algorithm>
#include <utility>

#include "tag.h"
#include "tstring.h"
#include "tlist.h"
#include "matroskafile.h"
#include "matroskaelement.h"
#include "matroskasimpletag.h"

namespace TagLib {
  namespace EBML {
    class MkTags;
  }

  namespace Matroska {
    using SimpleTagsList = List<SimpleTag*>;
    class TAGLIB_EXPORT Tag : public TagLib::Tag, private Element
    {
    public:
      Tag();
      ~Tag() override;
      void addSimpleTag(SimpleTag *tag);
      void removeSimpleTag(SimpleTag *tag);
      void clearSimpleTags();
      const SimpleTagsList& simpleTagsList() const;
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
      ByteVector render() override;
      PropertyMap properties() const override;
      PropertyMap setProperties(const PropertyMap &propertyMap) override;
      template <typename T>
      int removeSimpleTags(T&& p)
      {
        auto &list = simpleTagsListPrivate();
        int numRemoved = 0;
        for(auto it = list.begin(); it != list.end();) {
          it = std::find_if(it, list.end(), std::forward<T>(p));
          if(it != list.end()) {
            delete *it;
            *it = nullptr;
            it = list.erase(it);
            numRemoved++;
          }
        }
        return numRemoved;
      }

      template<typename T>
      SimpleTagsList findSimpleTags(T&& p)
      {
        auto &list = simpleTagsListPrivate();
        for(auto it = list.begin(); it != list.end();) {
          it = std::find_if(it, list.end(), std::forward<T>(p));
          if(it != list.end()) {
            list.append(*it);
            ++it;
          }
        }
        return list;
      }

      template<typename T>
      const Matroska::SimpleTag* findSimpleTag(T&& p) const
      {
        auto &list = simpleTagsListPrivate();
        auto it = std::find_if(list.begin(), list.end(), std::forward<T>(p));
        return it != list.end() ? *it : nullptr;
      }

      template <typename T>
      Matroska::SimpleTag* findSimpleTag(T&&p)
      {
        return const_cast<Matroska::SimpleTag*>(
          const_cast<const Matroska::Tag*>(this)->findSimpleTag(std::forward<T>(p))
        );
      }

    private:
      friend class Matroska::File;
      friend class EBML::MkTags;
      SimpleTagsList& simpleTagsListPrivate();
      const SimpleTagsList& simpleTagsListPrivate() const;
      bool setTag(const String &key, const String &value);
      const String* getTag(const String &key) const;
      class TagPrivate;
      std::unique_ptr<TagPrivate> d;
    };
  }
}

#endif
