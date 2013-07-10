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

#include "tag.h"
#include "tmap.h"
#include "tstring.h"
#include "tstringhandler.h"
#include "tbytevector.h"
#include "taglib_export.h"

namespace TagLib {

  class File;

  //! A RIFF Info tag implementation. 

  namespace RIFF {
  namespace Info {

    typedef Map<ByteVector, String> FieldMap;

    //! A abstraction for the string to data encoding in Info tags.

    /*!
     * RIFF Info tag has no clear definitions about character encodings.
     * In practice, local encoding of each system is largely used and UTF-8 is
     * popular too.
     *
     * Here is an option to read and write tags in your preferred encoding 
     * by subclassing this class, reimplementing parse() and render() and setting 
     * your reimplementation as the default with Info::Tag::setStringHandler().
     *
     * \see ID3v1::Tag::setStringHandler()
     */

    class TAGLIB_EXPORT StringHandler : public TagLib::StringHandler
    {
    public:
      StringHandler();

      /*!
       * Decode a string from \a data.  The default implementation assumes that
       * \a data is an UTF-8 character array.
       */
      virtual String parse(const ByteVector &data) const;

      /*!
       * Encode a ByteVector with the data from \a s.  The default implementation
       * assumes that \a s is an UTF-8 string. 
       */
      virtual ByteVector render(const String &s) const;
    };

    //! The main class in the ID3v2 implementation

    /*!
     * This is the main class in the INFO tag implementation.  RIFF INFO tag is a 
     * metadata format found in WAV audio and AVI video files.  Though it is a part 
     * of Microsoft/IBM's RIFF specification, the author could not find the official 
     * documents about it.  So, this implementation is referring to unofficial documents 
     * online and some applications' behaviors especially Windows Explorer.
     *
     * This is used for 
     */
    class TAGLIB_EXPORT Tag : public TagLib::Tag
    {
    public:
      /*!
       * Constructs an empty Info tag.
       */
      Tag();

      /*!
       * Constructs an Info tag read from \a data which is contents of "LIST" chunk.
       */
      Tag(const ByteVector &data);

      virtual ~Tag();

      // Reimplementations

      /*!
       * Returns the track name.  Corresponding to the "INAM" field. 
       */
      virtual String title() const;

      /*!
       * Returns the artist name.  Corresponding to the "IART" field. 
       */
      virtual String artist() const;

      /*!
       * Returns the album name.  Corresponding to the "IPRD" field. 
       */
      virtual String album() const;

      /*!
       * Returns the track comment.  Corresponding to the "ICMT" field. 
       */
      virtual String comment() const;
      
      /*!
       * Returns the genre name.  Corresponding to the "IGNR" field. 
       */
      virtual String genre() const;

      /*!
       * Returns the year part of the creation date.  Corresponding to the "ICRD" field,
       * but converts the first four chars of the "ICRD" field to uint. 
       */
      virtual uint year() const;

      /*!
       * Returns the track number.  Corresponding to the "ITRK" field. 
       */
      virtual uint track() const;

      /*!
       * Sets the track name.  Corresponding to the "INAM" field. 
       */
      virtual void setTitle(const String &s);

      /*!
       * Sets the artist name.  Corresponding to the "IART" field. 
       */
      virtual void setArtist(const String &s);

      /*!
       * Sets the album name.  Corresponding to the "IPRD" field. 
       */
      virtual void setAlbum(const String &s);

      /*!
       * Sets the track comment.  Corresponding to the "ICMT" field. 
       */
      virtual void setComment(const String &s);

      /*!
       * Sets the genre name.  Corresponding to the "IGNR" field. 
       */
      virtual void setGenre(const String &s);

      /*!
       * Sets the year part of the creation date.  Corresponding to the "ICRD" field,
       * but modifies the first four chars of the "ICRD" field. 
       * If \a i is zero, removes the "ICRD" field.
       */
      virtual void setYear(uint i);

      /*!
       * Sets the track number.  Corresponding to the "ITRK" field. 
       * If \a i is zero, removes the "ITRK" field.
       */
      virtual void setTrack(uint i);

      virtual bool isEmpty() const;

      /*!
       * Returns a map that represents all of the fields of the Tag.  
       *
       * \note Modifying the returned map doesn't affect the Tag.  
       * Use setFieldText() and removeField() to modify it.
       *
       * \see setFieldText()
       * \see removeField()
       */
      FieldMap fieldMap() const;
      
      /*
       * Returns the value of the field with the ID \a id.
       */
      String fieldText(const ByteVector &id) const;
        
      /*
       * Sets the value of the field with the ID \a id to \a text.
       * If the field does not exist, it is created.  If \text is empty, the field 
       * is removed.
       *
       * \note fieldId must be four-byte long pure ASCII string.  This function 
       * performs nothing if fieldId is invalid.
       */
      void setFieldText(const ByteVector &id, const String &text);

      /*
       * Removes the field with the ID \a id.
       */
      void removeField(const ByteVector &id);

      /*!
       * Render the tag back to binary data, suitable to be written to disk.
       * If \a createIID3Field is true, an ID3v1 tag will be appended as an
       * "IID3" field at the end of the data.
       *
       * \note Returns empty ByteVector is the tag contains no fields. 
       */
      ByteVector render(bool createIID3Field = false) const;

      /*!
       * Sets the string handler that decides how the text data will be
       * converted to and from binary data.
       * If the parameter \a handler is null, the previous handler is
       * released and default UTF-8 handler is restored.
       *
       * \note The caller is responsible for deleting the previous handler
       * as needed after it is released.
       *
       * \see StringHandler
       */
      static void setStringHandler(const TagLib::StringHandler *handler);
    
    protected:
      /*!
       * Pareses the body of the tag in \a data.
       */
      void parse(const ByteVector &data);

    private:
      class TagPrivate;
      TagPrivate *d;
    };
  }}
}

#endif
