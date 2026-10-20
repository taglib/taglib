/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#ifndef TAGLIB_ID3V1TAG_H
#define TAGLIB_ID3V1TAG_H

#include "tbytevector.h"
#include "taglib.h"
#include "taglib_export.h"
#include "tag.h"

namespace TagLib {

  class File;

  //! An ID3v1 implementation

  namespace ID3v1 {

    //! An abstraction for the string to data encoding in ID3v1 tags.

    /*!
     * ID3v1 should in theory always contain ISO-8859-1 (Latin1) data.  In
     * practice it does not.  TagLib by default only supports ISO-8859-1 data
     * in ID3v1 tags.
     *
     * However by subclassing this class and reimplementing parse() and render()
     * and setting your reimplementation as the default with
     * ID3v1::Tag::setStringHandler() you can define how you would like these
     * transformations to be done.
     *
     * An alternative for reading is a Windows code page number, which
     * ID3v1::Tag supports directly and which needs no subclass; see
     * ID3v1::Tag::setAssumedCodePage() and the ID3v1::Tag constructor taking one.
     *
     * \warning It is advisable <b>not</b> to write non-ISO-8859-1 data to ID3v1
     * tags.  Please consider disabling the writing of ID3v1 tags in the case
     * that the data is not ISO-8859-1.
     *
     * \see ID3v1::Tag::setStringHandler()
     * \see ID3v1::Tag::setAssumedCodePage()
     */

    class TAGLIB_EXPORT StringHandler
    {
    public:
      StringHandler();

      virtual ~StringHandler();

      StringHandler(const StringHandler &) = delete;
      StringHandler &operator=(const StringHandler &) = delete;

      /*!
       * Decode a string from \a data.  The default implementation assumes that
       * \a data is an ISO-8859-1 (Latin1) character array.
       */
      virtual String parse(const ByteVector &data) const;

      /*!
       * Encode a ByteVector with the data from \a s.  The default implementation
       * assumes that \a s is an ISO-8859-1 (Latin1) string.  If the string is
       * does not conform to ISO-8859-1, no value is written.
       *
       * \warning It is recommended that you <b>not</b> override this method, but
       * instead do not write an ID3v1 tag in the case that the data is not
       * ISO-8859-1.
       */
      virtual ByteVector render(const String &s) const;

    private:
      class StringHandlerPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<StringHandlerPrivate> d;
    };

    //! The main class in the ID3v1 implementation

    /*!
     * This is an implementation of the ID3v1 format.  ID3v1 is both the simplest
     * and most common of tag formats but is rather limited.  Because of its
     * pervasiveness and the way that applications have been written around the
     * fields that it provides, the generic TagLib::Tag API is a mirror of what is
     * provided by ID3v1.
     *
     * ID3v1 tags should generally only contain Latin1 information.  However because
     * many applications do not follow this rule there is now support for overriding
     * the ID3v1 string handling using the ID3v1::StringHandler class.  Please see
     * the documentation for that class for more information.
     *
     * \see StringHandler
     *
     * \note Most fields are truncated to a maximum of 28-30 bytes.  The
     * truncation happens automatically when the tag is rendered.
     */

    class TAGLIB_EXPORT Tag : public TagLib::Tag
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
      Tag(File *file, offset_t tagOffset);

      /*!
       * Create an ID3v1 tag and parse the data in \a file starting at
       * \a tagOffset, decoding the text with the Windows code page \a codePage.
       *
       * \a codePage is a Windows code page number in the same numbering a CSET
       * chunk of a RIFF file uses: 1252 reads Windows-1252, 866 reads IBM866,
       * 65001 reads UTF-8, and 0 is the default, which is the code page
       * assumedCodePage() names and ISO 8859-1 unless the caller has stated
       * another one.
       *
       * An ID3v1 tag has no place to declare a charset, so unlike a RIFF CSET
       * chunk this number is nothing but what the caller states about its files.
       * A code page this library has no table for falls back to
       * assumedCodePage(), and the fields are still read rather than dropped
       * over a number that could not be followed.
       *
       * \a codePage only decides how the text is read; it becomes a normal
       * in-memory String the moment it is read. render() is not affected by it,
       * as it writes through the StringHandler, which produces ISO 8859-1.
       *
       * \note A handler installed with setStringHandler() takes precedence over
       * \a codePage, because installing one is an explicit override.
       *
       * \see assumedCodePage()
       * \see setStringHandler()
       */
      Tag(File *file, offset_t tagOffset, unsigned int codePage);

      /*!
       * Destroys this Tag instance.
       */
      ~Tag() override;

      Tag(const Tag &) = delete;
      Tag &operator=(const Tag &) = delete;

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

      String title() const override;
      String artist() const override;
      String album() const override;
      String comment() const override;
      String genre() const override;
      unsigned int year() const override;
      unsigned int track() const override;

      void setTitle(const String &s) override;
      void setArtist(const String &s) override;
      void setAlbum(const String &s) override;
      void setComment(const String &s) override;
      void setGenre(const String &s) override;
      void setYear(unsigned int i) override;
      void setTrack(unsigned int i) override;

      /*!
       * Returns the genre in number.
       *
       * \note Normally 255 indicates that this tag contains no genre.
       */
      unsigned int genreNumber() const;

      /*!
       * Sets the genre in number to \a i.
       *
       * \note Valid value is from 0 up to 255. Normally 255 indicates that
       * this tag contains no genre.
       */
      void setGenreNumber(unsigned int i);

      /*!
       * Sets the string handler that decides how the ID3v1 data will be
       * converted to and from binary data.
       * If the parameter \a handler is null, the previous handler is
       * released and default ISO-8859-1 handler is restored.
       *
       * \note Installing a handler is an explicit override, so it is used for
       * every ID3v1 tag, including those read with a code page. Pass null to
       * hand those tags back to their own code page.
       *
       * \note The caller is responsible for deleting the previous handler
       * as needed after it is released.
       *
       * \see StringHandler
       */
      static void setStringHandler(const StringHandler *handler);

      /*!
       * Returns the code page assumed for an ID3v1 tag read without an explicit
       * one, which is 0 until setAssumedCodePage() states another one.
       *
       * \see setAssumedCodePage()
       */
      static unsigned int assumedCodePage();

      /*!
       * Sets the code page \a codePage assumed for an ID3v1 tag read without an
       * explicit one: a tag read through the two argument constructor, or one
       * read with a \a codePage of 0 or of a value this library cannot convert.
       *
       * \a codePage is a Windows code page number, the same numbering a CSET
       * chunk of a RIFF file uses: 65001 assumes UTF-8, 1252 assumes
       * Windows-1252, and 0, the initial value, assumes the ISO 8859-1 the
       * ID3v1 specification prescribes.
       *
       * ID3v1 has no way to declare a charset, so a file that holds UTF-8 or
       * any Windows code page looks exactly like one holding ISO 8859-1.  This
       * is where the caller says which of them its files hold.
       *
       * \note This is a process wide setting, like setStringHandler(), and
       * applies to the tags read after the call.
       *
       * \note It only decides how text is read. render() writes through the
       * StringHandler, which produces ISO 8859-1, whatever is assumed here.
       *
       * \note A code page this library cannot convert is refused: the call then
       * has no effect and a debug message is emitted.
       *
       * \see assumedCodePage()
       * \see setStringHandler()
       */
      static void setAssumedCodePage(unsigned int codePage);

    protected:
      /*!
       * Reads from the file specified in the constructor.
       */
      void read();
      /*!
       * Parses the body of the tag in \a data.
       */
      void parse(const ByteVector &data);

    private:
      class TagPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<TagPrivate> d;
    };
  }  // namespace ID3v1
}  // namespace TagLib

#endif
