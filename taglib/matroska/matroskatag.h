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

#include "tag.h"
#include "tstring.h"
#include "tlist.h"
//#include "matroskasimpletag.h

namespace TagLib {
  namespace Matroska {
    class SimpleTag;
    using SimpleTagsList = List<SimpleTag*>;
    class TAGLIB_EXPORT Tag : public TagLib::Tag
    {
    public:
      enum TargetTypeValue {
        None = 0,
        Shot = 10,
        Subtrack = 20,
        Track = 30,
        Part = 40,
        Album = 50,
        Edition = 60,
        Collection = 70
      };
      Tag();
      ~Tag() override;
      void addSimpleTag(SimpleTag *tag);
      void removeSimpleTag(SimpleTag *tag);
      const SimpleTagsList& simpleTagsList() const;
      String title() const override { return ""; }
      String artist() const override { return ""; }
      String album() const override { return ""; }
      String comment() const override { return ""; }
      String genre() const override { return ""; }
      unsigned int year() const override { return 0; }
      unsigned int track() const override { return 0; }
      void setTitle(const String &s) override {}
      void setArtist(const String &s) override {}
      void setAlbum(const String &s) override {}
      void setComment(const String &s) override {}
      void setGenre(const String &s) override {}
      void setYear(unsigned int i) override {}
      void setTrack(unsigned int i) override {}
      bool isEmpty() const override { return false; }

    private:
      class TagPrivate;
      std::unique_ptr<TagPrivate> d;
    };
  }
}

#endif
