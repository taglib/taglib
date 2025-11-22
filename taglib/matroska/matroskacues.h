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

#ifndef TAGLIB_MATROSKACUES_H
#define TAGLIB_MATROSKACUES_H
#ifndef DO_NOT_DOCUMENT

#include <optional>

#include "tlist.h"
#include "matroskaelement.h"

namespace TagLib {
  class File;

  namespace EBML {
    class MkCues;
  }

  namespace Matroska {
    class CuePoint;
    class CueTrack;

    class Cues : public Element
    {
    public:
      using CuePointList = std::list<std::unique_ptr<CuePoint>>;
      explicit Cues(offset_t segmentDataOffset);
      ~Cues() override;
      bool isValid(TagLib::File &file) const;
      void addCuePoint(std::unique_ptr<CuePoint> &&cuePoint);
      const CuePointList &cuePointList();
      bool sizeChanged(Element &caller, offset_t delta) override;
      void write(TagLib::File &file) override;

    private:
      friend class EBML::MkCues;
      ByteVector renderInternal() override;

      CuePointList cuePoints;
      const offset_t segmentDataOffset;
    };

    class CuePoint
    {
    public:
      using CueTrackList = std::list<std::unique_ptr<CueTrack>>;
      using Time = unsigned long long;
      CuePoint();
      ~CuePoint();
      bool isValid(TagLib::File &file, offset_t segmentDataOffset) const;
      void addCueTrack(std::unique_ptr<CueTrack> &&cueTrack);
      const CueTrackList &cueTrackList() const;
      void setTime(Time timestamp);
      Time getTime() const;
      bool adjustOffset(offset_t offset, offset_t delta);

    private:
      CueTrackList cueTracks;
      Time time = 0;
    };

    class CueTrack
    {
    public:
      using ReferenceTimeList = List<unsigned long long>;
      CueTrack();
      ~CueTrack();
      bool isValid(TagLib::File &file, offset_t segmentDataOffset) const;
      void setTrackNumber(unsigned long long trackNr);
      unsigned long long getTrackNumber() const;
      void setClusterPosition(offset_t clusterPos);
      offset_t getClusterPosition() const;
      void setRelativePosition(std::optional<offset_t> relativePos);
      std::optional<offset_t> getRelativePosition() const;
      void setCodecState(std::optional<offset_t> codecStatePos);
      std::optional<offset_t> getCodecState() const;
      void setBlockNumber(std::optional<unsigned long long> blockNr);
      std::optional<unsigned long long> getBlockNumber() const;
      void setDuration(std::optional<unsigned long long> segmentTicks);
      std::optional<unsigned long long> getDuration() const;
      void addReferenceTime(unsigned long long refTime);
      const ReferenceTimeList &referenceTimes() const;
      bool adjustOffset(offset_t offset, offset_t delta);

    private:
      unsigned long long trackNumber = 0;
      offset_t clusterPosition = 0;
      std::optional<offset_t> relativePosition;
      std::optional<unsigned long long> blockNumber;
      std::optional<unsigned long long> duration;
      std::optional<offset_t> codecState;
      ReferenceTimeList refTimes;
    };
  }
}

#endif
#endif
