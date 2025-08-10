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

#ifndef HAS_MATROSKAATTACHMENTS_H
#define HAS_MATROSKAATTACHMENTS_H

#include <memory>
#include "taglib_export.h"
#include "tlist.h"
#include "matroskaelement.h"


namespace TagLib {
  class File;
  namespace EBML {
    class MkAttachments;
  }
  namespace Matroska {
    class AttachedFile;
    class File;
    class TAGLIB_EXPORT Attachments
#ifndef DO_NOT_DOCUMENT
    : private Element
#endif
    {
    public:
      using AttachedFileList = List<AttachedFile*>;
      Attachments();
      virtual ~Attachments();

    void addAttachedFile(AttachedFile *file);
    void removeAttachedFile(AttachedFile *file);
    void clear();
    const AttachedFileList& attachedFileList() const;
    bool render() override;

    private:
      friend class EBML::MkAttachments;
      friend class Matroska::File;
      class AttachmentsPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<AttachmentsPrivate> d;
    };
  }
}

#endif
