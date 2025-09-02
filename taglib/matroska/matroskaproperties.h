/***************************************************************************
    copyright            : (C) 2025 by Urs Fleisch
    email                : ufleisch@users.sourceforge.net
 ***************************************************************************/

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

#ifndef TAGLIB_MATROSKAPROPERTIES_H
#define TAGLIB_MATROSKAPROPERTIES_H

#include "taglib_export.h"
#include "audioproperties.h"

namespace TagLib::EBML {
  class MkTracks;
  class MkInfo;
}
namespace TagLib::Matroska {
  class File;

  //! An implementation of Matroska audio properties
  class TAGLIB_EXPORT Properties : public AudioProperties
  {
  public:
    /*!
     * Creates an instance of Matroska::Properties.
     */
    explicit Properties(File *file, ReadStyle style = Average);

    /*!
     * Destroys this Matroska::Properties instance.
     */
    ~Properties() override;

    Properties(const Properties &) = delete;
    Properties &operator=(const Properties &) = delete;

    /*!
     * Returns the length of the file in milliseconds.
     *
     * \see lengthInSeconds()
     */
    int lengthInMilliseconds() const override;

    /*!
     * Returns the average bit rate of the file in kb/s.
     */
    int bitrate() const override;

    /*!
     * Returns the sample rate in Hz.
     */
    int sampleRate() const override;

    /*!
     * Returns the number of audio channels.
     */
    int channels() const override;

    /*!
     * Returns the number of bits per audio sample.
     */
    int bitsPerSample() const;

    /*!
     * Returns the concrete codec name, for example "A_MPEG/L3"
     * used in the file if available, otherwise an empty string.
     */
    String codecName() const;

    /*!
     * Returns the general name of the segment.
     * Some applications store the title of the file here, but players should
     * prioritize the tag title over the segment title.
     */
    String title() const;

  private:
    class PropertiesPrivate;
    friend class EBML::MkInfo;
    friend class EBML::MkTracks;

    void setLengthInMilliseconds(int length);
    void setBitrate(int bitrate);
    void setSampleRate(int sampleRate);
    void setChannels(int channels);
    void setBitsPerSample(int bitsPerSample);
    void setCodecName(const String &codecName);
    void setTitle(const String &title);

    TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
    std::unique_ptr<PropertiesPrivate> d;
  };
}

#endif
