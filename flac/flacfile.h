/***************************************************************************
    copyright            : (C) 2003 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#ifndef TAGLIB_FLACFILE_H
#define TAGLIB_FLACFILE_H

#include <tfile.h>

#include "flacproperties.h"

namespace TagLib {

  class Tag;
  namespace ID3v2 { class FrameFactory; }

  //! An implementation of FLAC metadata

  /*!
   * This is implementation of FLAC metadata for non-Ogg FLAC files.  At some
   * point when Ogg / FLAC is more common there will be a similar implementation
   * under the Ogg hiearchy.
   *
   * This supports ID3v1, ID3v2 and Xiph style comments as well as reading stream
   * properties from the file.
   */

  namespace FLAC {

    //! An implementation of TagLib::File with FLAC specific methods

    /*!
     * This implements and provides an interface for FLAC files to the
     * TagLib::Tag and TagLib::AudioProperties interfaces by way of implementing
     * the abstract TagLib::File API as well as providing some additional
     * information specific to FLAC files.
     */

    class File : public TagLib::File
    {
    public:
      /*!
       * Contructs an FLAC file from \a file.  If \a readProperties is true the
       * file's audio properties will also be read using \a propertiesStyle.  If
       * false, \a propertiesStyle is ignored.
       */
      File(const char *file, bool readProperties = true,
           Properties::ReadStyle propertiesStyle = Properties::Average);

      /*!
       * Destroys this instance of the File.
       */
      virtual ~File();

      /*!
       * Returns the Tag for this file.  This will be a union of XiphComment
       * with ID3v1 and ID3v2 tags.
       */
      virtual TagLib::Tag *tag() const;

      /*!
       * Returns the FLAC::Properties for this file.  If no audio properties
       * were read then this will return a null pointer.
       */
      virtual Properties *audioProperties() const;

      /*!
       * Save the file.  This will primarily save the XiphComment, but
       * will also keep any old ID3-tags up to date. If the file
       * has no XiphComment, one will be constructed from the ID3-tags.
       */
      virtual void save();

      /*!
       * Set the ID3v2::FrameFactory to something other than the default.  This
       * can be used to specify the way that ID3v2 frames will be interpreted
       * when 
       *
       * \see ID3v2FrameFactory
       */
      void setID3v2FrameFactory(const ID3v2::FrameFactory *factory);

      /*!
       * Returns the length of the audio-stream, used by FLAC::Properties for
       * calculating the bitrate.
       */
      long streamLength();

    private:
      File(const File &);
      File &operator=(const File &);

      void read(bool readProperties, Properties::ReadStyle propertiesStyle);
      void scan();
      long findID3v2();
      long findID3v1();
      ByteVector streamInfoData();
      ByteVector xiphCommentData();

      class FilePrivate;
      FilePrivate *d;
    };
  }
}

#endif
