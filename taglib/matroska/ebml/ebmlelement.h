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

#include <cstdint>
#include <memory>
#include "taglib.h"

namespace TagLib
{
  class File;
  class ByteVector;

  namespace EBML {

    class Element
    {
    public:
      enum class Id : unsigned int
      {
        EBMLHeader                = 0x1A45DFA3,
        DocType                   = 0x4282,
        DocTypeVersion            = 0x4287,
        VoidElement               = 0xEC,
        MkSegment                 = 0x18538067,
        MkTags                    = 0x1254C367,
        MkTag                     = 0x7373,
        MkTagTargets              = 0x63C0,
        MkTagTargetTypeValue      = 0x68CA,
        MkTagTrackUID             = 0x63C5,
        MkTagEditionUID           = 0x63C9,
        MkTagChapterUID           = 0x63C4,
        MkTagAttachmentUID        = 0x63C6,
        MkSimpleTag               = 0x67C8,
        MkTagName                 = 0x45A3,
        MkTagLanguage             = 0x447A,
        MkTagBinary               = 0x4485,
        MkTagString               = 0x4487,
        MkTagsTagLanguage         = 0x447A,
        MkTagsLanguageDefault     = 0x4484,
        MkAttachments             = 0x1941A469,
        MkAttachedFile            = 0x61A7,
        MkAttachedFileDescription = 0x467E,
        MkAttachedFileName        = 0x466E,
        MkAttachedFileMediaType   = 0x4660,
        MkAttachedFileData        = 0x465C,
        MkAttachedFileUID         = 0x46AE,
        MkSeekHead                = 0x114D9B74,
        MkSeek                    = 0x4DBB,
        MkSeekID                  = 0x53AB,
        MkSeekPosition            = 0x53AC,
        MkCluster                 = 0x1F43B675,
        MkCodecState              = 0xA4,
        MkCues                    = 0x1C53BB6B,
        MkCuePoint                = 0xBB,
        MkCueTime                 = 0xB3,
        MkCueTrackPositions       = 0xB7,
        MkCueTrack                = 0xF7,
        MkCueClusterPosition      = 0xF1,
        MkCueRelativePosition     = 0xF0,
        MkCueDuration             = 0xB2,
        MkCueBlockNumber          = 0x5378,
        MkCueCodecState           = 0xEA,
        MkCueReference            = 0xDB,
        MkCueRefTime              = 0x96,
        MkInfo                    = 0x1549A966,
        MkTimestampScale          = 0x2AD7B1,
        MkDuration                = 0x4489,
        MkTitle                   = 0x7BA9,
        MkTracks                  = 0x1654AE6B,
        MkTrackEntry              = 0xAE,
        MkCodecID                 = 0x86,
        MkAudio                   = 0xE1,
        MkSamplingFrequency       = 0xB5,
        MkBitDepth                = 0x6264,
        MkChannels                = 0x9F,
        MkChapters                = 0x1043A770,
        MkEditionEntry            = 0x45B9,
        MkEditionUID              = 0x45BC,
        MkEditionFlagDefault      = 0x45DB,
        MkEditionFlagOrdered      = 0x45DD,
        MkChapterAtom             = 0xB6,
        MkChapterUID              = 0x73C4,
        MkChapterTimeStart        = 0x91,
        MkChapterTimeEnd          = 0x92,
        MkChapterFlagHidden       = 0x98,
        MkChapterDisplay          = 0x80,
        MkChapString              = 0x85,
        MkChapLanguage            = 0x437C,
      };

      Element(Id id, int sizeLength, offset_t dataSize);
      Element(Id id, int sizeLength, offset_t dataSize, offset_t);
      virtual ~Element();

      virtual bool read(File &file);
      void skipData(File &file);
      Id getId() const;
      offset_t headSize() const;
      offset_t getSize() const;
      int getSizeLength() const;
      int64_t getDataSize() const;
      ByteVector renderId() const;
      virtual ByteVector render();
      static std::unique_ptr<Element> factory(File &file);
      static unsigned int readId(File &file);

    protected:
      Id id;
      int sizeLength;
      offset_t dataSize;
    };

    // Template specializations to ensure that elements for the different IDs
    // are consistently created and cast. They provide a mapping between IDs
    // and Element subclasses. The switch in factory() makes sure that the
    // template for all IDs are instantiated, i.e. that every ID has its Element
    // subclass mapped.
    class MasterElement;
    class UIntElement;
    class BinaryElement;
    class FloatElement;
    class MkSegment;
    class MkInfo;
    class MkTracks;
    class MkTags;
    class MkAttachments;
    class MkSeekHead;
    class MkChapters;
    class MkCues;
    class VoidElement;
    class UTF8StringElement;
    class Latin1StringElement;

    template <Element::Id ID>
    struct GetElementTypeById;

    template <> struct GetElementTypeById<Element::Id::EBMLHeader> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::DocType> { using type = Latin1StringElement; };
    template <> struct GetElementTypeById<Element::Id::DocTypeVersion> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkSegment> { using type = MkSegment; };
    template <> struct GetElementTypeById<Element::Id::MkInfo> { using type = MkInfo; };
    template <> struct GetElementTypeById<Element::Id::MkTracks> { using type = MkTracks; };
    template <> struct GetElementTypeById<Element::Id::MkTags> { using type = MkTags; };
    template <> struct GetElementTypeById<Element::Id::MkAttachments> { using type = MkAttachments; };
    template <> struct GetElementTypeById<Element::Id::MkTag> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagTargets> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkSimpleTag> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkAttachedFile> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkSeek> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkTrackEntry> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkAudio> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkCuePoint> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueTrackPositions> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueReference> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkCluster> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkCues> { using type = MkCues; };
    template <> struct GetElementTypeById<Element::Id::MkTagName> { using type = UTF8StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagString> { using type = UTF8StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkAttachedFileName> { using type = UTF8StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkAttachedFileDescription> { using type = UTF8StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagLanguage> { using type = Latin1StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkAttachedFileMediaType> { using type = Latin1StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkCodecID> { using type = Latin1StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagTargetTypeValue> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagTrackUID> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagEditionUID> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagChapterUID> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagAttachmentUID> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkAttachedFileUID> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkSeekPosition> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkTimestampScale> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkBitDepth> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkChannels> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueTime> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueTrack> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueClusterPosition> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueRelativePosition> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueDuration> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueBlockNumber> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueCodecState> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkCueRefTime> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagsLanguageDefault> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkAttachedFileData> { using type = BinaryElement; };
    template <> struct GetElementTypeById<Element::Id::MkSeekID> { using type = BinaryElement; };
    template <> struct GetElementTypeById<Element::Id::MkTagBinary> { using type = BinaryElement; };
    template <> struct GetElementTypeById<Element::Id::MkCodecState> { using type = BinaryElement; };
    template <> struct GetElementTypeById<Element::Id::MkDuration> { using type = FloatElement; };
    template <> struct GetElementTypeById<Element::Id::MkTitle> { using type = UTF8StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkSamplingFrequency> { using type = FloatElement; };
    template <> struct GetElementTypeById<Element::Id::MkSeekHead> { using type = MkSeekHead; };
    template <> struct GetElementTypeById<Element::Id::MkChapters> { using type = MkChapters; };
    template <> struct GetElementTypeById<Element::Id::MkEditionEntry> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkEditionUID> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkEditionFlagDefault> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkEditionFlagOrdered> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkChapterAtom> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkChapterUID> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkChapterTimeStart> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkChapterTimeEnd> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkChapterFlagHidden> { using type = UIntElement; };
    template <> struct GetElementTypeById<Element::Id::MkChapterDisplay> { using type = MasterElement; };
    template <> struct GetElementTypeById<Element::Id::MkChapString> { using type = UTF8StringElement; };
    template <> struct GetElementTypeById<Element::Id::MkChapLanguage> { using type = Latin1StringElement; };
    template <> struct GetElementTypeById<Element::Id::VoidElement> { using type = VoidElement; };

    template <Element::Id ID, typename T=typename GetElementTypeById<ID>::type>
    const T *element_cast(const std::unique_ptr<Element> &ptr)
    {
      return static_cast<const T *>(ptr.get());
    }

    template <Element::Id ID, typename T=typename GetElementTypeById<ID>::type>
    std::unique_ptr<T> element_cast(std::unique_ptr<Element> &&ptr)
    {
      return std::unique_ptr<T>(static_cast<T *>(ptr.release()));
    }

    template <Element::Id ID, typename T=typename GetElementTypeById<ID>::type>
    std::unique_ptr<T> make_unique_element(Element::Id id, int sizeLength, offset_t dataSize, offset_t offset)
    {
      return std::make_unique<T>(id, sizeLength, dataSize, offset);
    }

    template <Element::Id ID, typename T=typename GetElementTypeById<ID>::type>
    std::unique_ptr<T> make_unique_element()
    {
      return std::make_unique<T>(ID, 0, 0, 0);
    }

  }
}

#endif
#endif
