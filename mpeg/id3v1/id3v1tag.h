/***************************************************************************
    copyright            : (C) 2002, 2003 by Scott Wheeler
    email                : wheeler@kde.org
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

#ifndef TAGLIB_ID3V1TAG_H
#define TAGLIB_ID3V1TAG_H

#include <tag.h>
#include <tbytevector.h>

namespace TagLib {

  class File;

  //! An ID3v1 implementation

  namespace ID3v1 {

    //! The main class in the ID3v1 implementation

    /*!
     * This is an implementation of the ID3v1 format.  ID3v1 is both the simplist
     * and most common of tag formats but is rather limited.  Because of its
     * pervasiveness and the way that applications have been written around the
     * fields that it provides, the generic TagLib::Tag API is a mirror of what is
     * provided by ID3v1.
     *
     * \note Most fields are truncated to a maximum of 28-30 bytes.  The
     * truncation happens automatically when the tag is rendered.
     */

    class Tag : public TagLib::Tag
    {
    public:
      /*!
       * Create an ID3v1 tag with default values.
       */
      Tag();

      /*!
       * Create an ID3v1 tag and parse the data in \a file starting at
       * \a tagOffset.
       */
      Tag(File *file, long tagOffset);

      /*!
       * Destroys this Tag instance.
       */
      virtual ~Tag();

      /*!
       * Renders the in memory values to a ByteVector suitable for writing to
       * the file.
       */
      ByteVector render() const;

      /*!
       * Returns the string "TAG" suitable for usage in locating the tag in a
       * file.
       */
      static ByteVector fileIdentifier();

      // Reimplementations.

      virtual String title() const;
      virtual String artist() const;
      virtual String album() const;
      virtual String comment() const;
      virtual String genre() const;
      virtual uint year() const;
      virtual uint track() const;

      virtual void setTitle(const String &s);
      virtual void setArtist(const String &s);
      virtual void setAlbum(const String &s);
      virtual void setComment(const String &s);
      virtual void setGenre(const String &s);
      virtual void setYear(uint i);
      virtual void setTrack(uint i);

    protected:
      /*!
       * Reads from the file specified in the constructor.
       */
      void read();
      /*!
       * Pareses the body of the tag in \a data.
       */
      void parse(const ByteVector &data);

    private:
      Tag(const Tag &);
      Tag &operator=(const Tag &);

      class TagPrivate;
      TagPrivate *d;
    };
  }
}

#endif
