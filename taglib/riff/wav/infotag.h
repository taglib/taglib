/***************************************************************************
    copyright            : (C) 2012 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
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

#ifndef TAGLIB_INFOTAG_H
#define TAGLIB_INFOTAG_H

#include "tmap.h"
#include "tstring.h"
#include "tbytevector.h"
#include "taglib_export.h"
#include "tag.h"

namespace TagLib {

  class File;

  namespace RIFF {
  //! A RIFF INFO tag implementation.
  namespace Info {

    using FieldListMap = Map<ByteVector, String>;

    //! An abstraction for the string to data encoding in Info tags.

    /*!
     * Per the RIFF specification, the character encoding of the INFO tag is
     * declared with a top-level CSET chunk; when it is absent, the default is
     * the ISO 8859-1 (Latin-1) ANSI code page. A declared code page is applied
     * as the tag is read, so the text is a normal in-memory String from there
     * on, and render() writes UTF-8.
     * https://www.robotplanet.dk/audio/wav_meta_data/riff_mci.pdf#page=25
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
       * \a data is an ISO 8859-1 (Latin-1) character array, which is the RIFF
       * default when the file declares no CSET code page.
       */
      virtual String parse(const ByteVector &data) const;

      /*!
       * Encode a ByteVector with the data from \a s.  The default implementation
       * assumes that \a s is an UTF-8 string.
       */
      virtual ByteVector render(const String &s) const;

    private:
      class StringHandlerPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<StringHandlerPrivate> d;
    };

    //! The main class in the INFO tag implementation

    /*!
     * This is the main class in the INFO tag implementation.  RIFF INFO tag is a
     * metadata format found in WAV audio and AVI video files.  Though it is a part
     * of Microsoft/IBM's RIFF specification, the author could not find the official
     * documents about it.  So, this implementation is referring to unofficial documents
     * online and some applications' behaviors especially Windows Explorer.
     */
    class TAGLIB_EXPORT Tag : public TagLib::Tag
    {
    public:
      /*!
       * Constructs an empty INFO tag.
       */
      Tag();

      /*!
       * Constructs an INFO tag read from \a data which is the contents of the "LIST" chunk.
       *
       * \a data comes from a file that declares no code page, so the text is decoded
       * with assumedCodePage(), unless a handler installed with setStringHandler()
       * takes over.
       *
       * \see assumedCodePage()
       */
      Tag(const ByteVector &data);

      /*!
       * Constructs an INFO tag read from \a data which is the contents of the "LIST" chunk,
       * decoding the text with the code page \a codePage declared by the file's top-level
       * CSET chunk.
       *
       * \a codePage only decides how \a data is read. The text becomes a normal
       * in-memory String the moment it is read, and render() writes UTF-8, which is
       * what RIFF::WAV::File::save() declares in the CSET chunk it writes. A tag is
       * never converted back into the code page it was read from.
       *
       * Code page 0 is the RIFF "unspecified" value, used when a file has no CSET
       * chunk, and means the code page assumedCodePage() names, which is ISO 8859-1
       * (Latin-1) unless the caller has stated another one. A code page this library
       * has no table for also means ISO 8859-1: the number is a hint about how the
       * text was written, and the fields are still read rather than dropped over a
       * hint that could not be followed.
       *
       * \note A handler installed with setStringHandler() takes precedence over
       * \a codePage, because installing one is an explicit override.
       *
       * \see assumedCodePage()
       * \see setStringHandler()
       */
      Tag(const ByteVector &data, unsigned int codePage);

      ~Tag() override;

      Tag(const Tag &) = delete;
      Tag &operator=(const Tag &) = delete;

      // Reimplementations

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

      bool isEmpty() const override;

      PropertyMap properties() const override;
      void removeUnsupportedProperties(const StringList &props) override;
      PropertyMap setProperties(const PropertyMap &props) override;

      /*!
       * Returns a copy of the internal fields of the tag.  The returned map directly
       * reflects the contents of the "INFO" chunk.
       *
       * \note Modifying this map does not affect the tag's internal data.
       * Use setFieldText() and removeField() instead.
       *
       * \see setFieldText()
       * \see removeField()
       */
      FieldListMap fieldListMap() const;

      /*!
       * Gets the value of the field with the ID \a id.
       */
      String fieldText(const ByteVector &id) const;

      /*!
       * Sets the value of the field with the ID \a id to \a s.
       * If the field does not exist, it is created.
       * If \a s is empty, the field is removed.
       *
       * \note fieldId must be a four-byte long pure ASCII string.  This function
       * performs nothing if fieldId is invalid.
       */
      void setFieldText(const ByteVector &id, const String &s);

      /*!
       * Removes the field with the ID \a id.
       */
      void removeField(const ByteVector &id);

      /*!
       * Render the tag back to binary data, suitable to be written to disk.
       *
       * \note Returns an empty ByteVector if the tag contains no fields.
       */
      ByteVector render() const;

      /*!
       * Sets the string handler that decides how the text data will be
       * converted to and from binary data.
       * If the parameter \a handler is null, the previous handler is
       * released and the default (ISO 8859-1 decode, UTF-8 encode) handler
       * is restored.
       *
       * \note Installing a handler is an explicit override, so it is used for
       * every INFO tag, including those read from a file that declares a CSET
       * code page. Pass null to hand those files back to their own code page.
       *
       * \note The caller is responsible for deleting the previous handler
       * as needed after it is released.
       *
       * \see StringHandler
       */
      static void setStringHandler(const StringHandler *handler);

      /*!
       * Returns the code page assumed for an INFO tag read from a file that declares
       * none, which is 0 until setAssumedCodePage() states another one.
       *
       * \see setAssumedCodePage()
       */
      static unsigned int assumedCodePage();

      /*!
       * Sets the code page \a codePage assumed for an INFO tag whose file declares no
       * charset: a file without a CSET chunk, a CSET chunk whose wCodePage is zero, or
       * an INFO list read without its CSET chunk, as in Tag(const ByteVector &data).
       *
       * \a codePage is a Windows code page number, the same numbering the CSET chunk
       * uses: 65001 assumes UTF-8, 1252 assumes Windows-1252, and 0, the initial value,
       * assumes the ISO 8859-1 the RIFF specification prescribes for a file that says
       * nothing.
       * https://www.robotplanet.dk/audio/wav_meta_data/riff_mci.pdf#page=25
       *
       * The specification cannot tell a file holding UTF-8 or an ANSI code page without
       * a CSET chunk from one holding ISO 8859-1, because all three look the same on
       * disk. This is where the caller says which of them its files hold.
       *
       * \note This is a process wide setting, like setStringHandler(), and applies to
       * the tags read after the call. It never overrides a code page a file declares
       * itself, not even one this library has no table for.
       *
       * \note It only decides how text is read. save() writes UTF-8 and declares that
       * in the CSET chunk it writes, whatever is assumed here.
       *
       * \note A code page this library cannot convert is refused: the call then has no
       * effect and a debug message is emitted.
       *
       * \see assumedCodePage()
       * \see setStringHandler()
       */
      static void setAssumedCodePage(unsigned int codePage);

    protected:
      /*!
       * Parses the body of the tag in \a data.
       */
      void parse(const ByteVector &data);

    private:
      class TagPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<TagPrivate> d;
    };
  }  // namespace Info
}  // namespace RIFF
}  // namespace TagLib

#endif
