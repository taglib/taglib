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

#ifndef TAGLIB_EBMLMKSEGMENT_H
#define TAGLIB_EBMLMKSEGMENT_H
#ifndef DO_NOT_DOCUMENT

#include "ebmlmasterelement.h"
#include "ebmlmktags.h"
#include "ebmlmkattachments.h"
#include "ebmlmkchapters.h"
#include "ebmlmkseekhead.h"
#include "ebmlmkcues.h"
#include "ebmlmkinfo.h"
#include "ebmlmktracks.h"
#include "taglib.h"

namespace TagLib {
  namespace Matroska {
    class Tag;
    class Attachments;
    class Chapters;
    class SeekHead;
    class Segment;
  }

  namespace EBML {
    class MkSegment : public MasterElement
    {
    public:
      MkSegment(int sizeLength, offset_t dataSize, offset_t offset);
      MkSegment(Id, int sizeLength, offset_t dataSize, offset_t offset);
      ~MkSegment() override;

      offset_t segmentDataOffset() const;
      bool read(File &file) override;
      std::unique_ptr<Matroska::Tag> parseTag() const;
      std::unique_ptr<Matroska::Attachments> parseAttachments() const;
      std::unique_ptr<Matroska::Chapters> parseChapters() const;
      std::unique_ptr<Matroska::SeekHead> parseSeekHead() const;
      std::unique_ptr<Matroska::Cues> parseCues() const;
      std::unique_ptr<Matroska::Segment> parseSegment() const;
      void parseInfo(Matroska::Properties *properties) const;
      void parseTracks(Matroska::Properties *properties) const;

    private:
      std::unique_ptr<MkTags> tags;
      std::unique_ptr<MkAttachments> attachments;
      std::unique_ptr<MkChapters> chapters;
      std::unique_ptr<MkSeekHead> seekHead;
      std::unique_ptr<MkCues> cues;
      std::unique_ptr<MkInfo> info;
      std::unique_ptr<MkTracks> tracks;
    };
  }
}

#endif
#endif
