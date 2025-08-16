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
#include "matroskaproperties.h"

namespace TagLib::Matroska {
  class Properties;
  class Tag;
  class Attachments;
  class TAGLIB_EXPORT File : public TagLib::File
  {
  public:
    explicit File(FileName file, bool readProperties = true,
                  Properties::ReadStyle readStyle = Properties::Average);
    explicit File(IOStream *stream, bool readProperties = true,
                  Properties::ReadStyle readStyle = Properties::Average);
    ~File() override;
    File(const File &) = delete;
    File &operator=(const File &) = delete;
    AudioProperties *audioProperties() const override;
    TagLib::Tag *tag() const override;
    Attachments *attachments(bool create = false) const;
    Tag *tag(bool create) const;
    bool save() override;
    //PropertyMap properties() const override { return PropertyMap(); }
    //void removeUnsupportedProperties(const StringList &properties) override { }

    /*!
     * Returns whether or not the given \a stream can be opened as a Matroska
     * file.
     *
     * \note This method is designed to do a quick check.  The result may
     * not necessarily be correct.
     */
    static bool isSupported(IOStream *stream);

  private:
    void read(bool readProperties, Properties::ReadStyle readStyle);
    class FilePrivate;
    friend class Properties;
    TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
    std::unique_ptr<FilePrivate> d;
  };
}

#endif
