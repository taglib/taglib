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

#ifndef TAGLIB_EBMLELEMENT_H
#define TAGLIB_EBMLELEMENT_H
#ifndef DO_NOT_DOCUMENT

#include "tfile.h"
#include "tutils.h"
#include "taglib.h"

namespace TagLib::EBML {
  class Element
  {
  public:
    using Id = unsigned int;
    Element(Id id, int sizeLength, offset_t dataSize) :
      id(id), sizeLength(sizeLength), dataSize(dataSize)
    {
    }
    virtual ~Element() = default;
    virtual bool read(File &file)
    {
      skipData(file);
      return true;
    }
    void skipData(File &file);
    Id getId() const { return id; }
    offset_t headSize() const;
    offset_t getSize() const { return headSize() + dataSize; }
    int getSizeLength() const { return sizeLength; }
    int64_t getDataSize() const { return dataSize; }
    ByteVector renderId() const;
    virtual ByteVector render();
    static Element *factory(File &file);
    static Id readId(File &file);

  protected:
    Id id;
    int sizeLength;
    offset_t dataSize;
  };

  namespace ElementIDs {
    inline constexpr Element::Id EBMLHeader                = 0x1A45DFA3;
    inline constexpr Element::Id VoidElement               = 0xEC;
    inline constexpr Element::Id MkSegment                 = 0x18538067;
    inline constexpr Element::Id MkTags                    = 0x1254C367;
    inline constexpr Element::Id MkTag                     = 0x7373;
    inline constexpr Element::Id MkTagTargets              = 0x63C0;
    inline constexpr Element::Id MkTagTargetTypeValue      = 0x68CA;
    inline constexpr Element::Id MkSimpleTag               = 0x67C8;
    inline constexpr Element::Id MkTagName                 = 0x45A3;
    inline constexpr Element::Id MkTagLanguage             = 0x447A;
    inline constexpr Element::Id MkTagString               = 0x4487;
    inline constexpr Element::Id MkTagsTagLanguage         = 0x447A;
    inline constexpr Element::Id MkTagsLanguageDefault     = 0x4484;
    inline constexpr Element::Id MkAttachments             = 0x1941A469;
    inline constexpr Element::Id MkAttachedFile            = 0x61A7;
    inline constexpr Element::Id MkAttachedFileDescription = 0x467E;
    inline constexpr Element::Id MkAttachedFileName        = 0x466E;
    inline constexpr Element::Id MkAttachedFileMediaType   = 0x4660;
    inline constexpr Element::Id MkAttachedFileData        = 0x465C;
    inline constexpr Element::Id MkAttachedFileUID         = 0x46AE;
    inline constexpr Element::Id MkSeekHead                = 0x114D9B74;
    inline constexpr Element::Id MkSeek                    = 0x4DBB;
    inline constexpr Element::Id MkSeekID                  = 0x53AB;
    inline constexpr Element::Id MkSeekPosition            = 0x53AC;
    inline constexpr Element::Id MkCluster                 = 0x1F43B675;
    inline constexpr Element::Id MkCodecState              = 0xA4;
    inline constexpr Element::Id MkCues                    = 0x1C53BB6B;
    inline constexpr Element::Id MkCuePoint                = 0xBB;
    inline constexpr Element::Id MkCueTime                 = 0xB3;
    inline constexpr Element::Id MkCueTrackPositions       = 0xB7;
    inline constexpr Element::Id MkCueTrack                = 0xF7;
    inline constexpr Element::Id MkCueClusterPosition      = 0xF1;
    inline constexpr Element::Id MkCueRelativePosition     = 0xF0;
    inline constexpr Element::Id MkCueDuration             = 0xB2;
    inline constexpr Element::Id MkCueBlockNumber          = 0x5378;
    inline constexpr Element::Id MkCueCodecState           = 0xEA;
    inline constexpr Element::Id MkCueReference            = 0xDB;
    inline constexpr Element::Id MkCueRefTime              = 0x96;
  }
}

#endif
#endif
