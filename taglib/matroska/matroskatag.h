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

#ifndef TAGLIB_MATROSKATAG_H
#define TAGLIB_MATROSKATAG_H

#include <memory>

#include "tag.h"
#include "tstring.h"
#include "tlist.h"
#include "matroskafile.h"
#include "matroskaelement.h"
#include "matroskasimpletag.h"

namespace TagLib {
  class File;
  namespace EBML {
    class MkTags;
  }

  namespace Matroska {
    using SimpleTagsList = List<SimpleTag>;
    class TAGLIB_EXPORT Tag : public TagLib::Tag
#ifndef DO_NOT_DOCUMENT
    , private Element
#endif
    {
    public:
      Tag();
      ~Tag() override;
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
      PropertyMap properties() const override;
      PropertyMap setProperties(const PropertyMap &propertyMap) override;
      void removeUnsupportedProperties(const StringList& properties) override;

      /*!
       * Returns the names of the binary simple tags.
       */
      StringList complexPropertyKeys() const override;

      /*!
       * Get the binary simple tags as maps with keys "data", "name",
       * "targetTypeValue", "language", "defaultLanguage".
       * The attached files such as pictures with key "PICTURE" are available
       * with Matroska::File::complexProperties().
       */
      List<VariantMap> complexProperties(const String &key) const override;

      /*!
       * Set the binary simple tags as maps with keys "data", "name",
       * "targetTypeValue", "language", "defaultLanguage".
       * The attached files such as pictures with key "PICTURE" can be set
       * with Matroska::File::setComplexProperties().
       *
       * Returns \c true if \c key can be stored as binary simple tags.
       */
      bool setComplexProperties(const String& key, const List<VariantMap>& value) override;

      void addSimpleTag(const SimpleTag &tag);
      void removeSimpleTag(const String &name, SimpleTag::TargetTypeValue targetTypeValue);
      void clearSimpleTags();
      const SimpleTagsList &simpleTagsList() const;

    private:
      friend class File;
      friend class EBML::MkTags;
      class TagPrivate;

      // private Element implementation
      bool render() override;

      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<TagPrivate> d;
    };
  }
}

#endif
