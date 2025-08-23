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
#include "ebmlmkseekhead.h"
#include "ebmlmkcues.h"
#include "ebmlmkinfo.h"
#include "ebmlmktracks.h"
#include "taglib.h"

namespace TagLib {
  namespace Matroska {
    class Tag;
    class Attachments;
    class SeekHead;
    class Segment;
  }
  namespace EBML {
    class MkSegment : public MasterElement
    {
    public:
      MkSegment(int sizeLength, offset_t dataSize, offset_t offset) :
        MasterElement(Id::MkSegment, sizeLength, dataSize, offset)
      {
      }
      MkSegment(Id, int sizeLength, offset_t dataSize, offset_t offset) :
        MasterElement(Id::MkSegment, sizeLength, dataSize, offset)
      {
      }
      ~MkSegment() override;
      offset_t segmentDataOffset() const;
      bool read(File &file) override;
      std::unique_ptr<Matroska::Tag> parseTag();
      std::unique_ptr<Matroska::Attachments> parseAttachments();
      std::unique_ptr<Matroska::SeekHead> parseSeekHead();
      std::unique_ptr<Matroska::Cues> parseCues();
      std::unique_ptr<Matroska::Segment> parseSegment();
      void parseInfo(Matroska::Properties *properties);
      void parseTracks(Matroska::Properties *properties);

    private:
      std::unique_ptr<MkTags> tags;
      std::unique_ptr<MkAttachments> attachments;
      std::unique_ptr<MkSeekHead> seekHead;
      std::unique_ptr<MkCues> cues;
      std::unique_ptr<MkInfo> info;
      std::unique_ptr<MkTracks> tracks;
    };
  }
}

#endif
#endif
