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

#ifndef HAS_MATROSKAFILE_H
#define HAS_MATROSKAFILE_H

#include "taglib_export.h"
#include "tfile.h"
#include "tag.h"


namespace TagLib {
  namespace Matroska {
    class Properties;
    class Tag;
    class Attachments;
    class TAGLIB_EXPORT File : public TagLib::File
    {
    public:
      File(FileName file, bool readProperties = true);
      File(IOStream *stream, bool readProperties = true);
      ~File() override;
      File(const File &) = delete;
      File &operator=(const File &) = delete;
      AudioProperties *audioProperties() const override { return nullptr; }
      TagLib::Tag *tag() const override;
      Attachments* attachments(bool create = false) const;
      Matroska::Tag *tag(bool create) const;
      bool save() override;
      //PropertyMap properties() const override { return PropertyMap(); }
      //void removeUnsupportedProperties(const StringList &properties) override { }
    private:
      void read(bool readProperties);
      class FilePrivate;
      std::unique_ptr<FilePrivate> d;

    };
  }
}







 #endif