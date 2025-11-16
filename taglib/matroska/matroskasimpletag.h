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

#ifndef TAGLIB_MATROSKASIMPLETAG_H
#define TAGLIB_MATROSKASIMPLETAG_H

#include <memory>

#include "tstring.h"

namespace TagLib {
  class String;
  class ByteVector;

  namespace Matroska {
    //! Attribute of Matroska metadata.
    class TAGLIB_EXPORT SimpleTag
    {
    public:
      //! Specifies the level of other elements the tag value applies to.
      enum TargetTypeValue {
        None = 0,       //!< Empty or omitted, everything in the segment
        Shot = 10,      //!< Shot
        Subtrack = 20,  //!< Subtrack / movement / scene
        Track = 30,     //!< Track / song / chapter
        Part = 40,      //!< Part / session
        Album = 50,     //!< Album / opera / concert / movie / episode
        Edition = 60,   //!< Edition / issue / volume / opus / season / sequel
        Collection = 70 //!< Collection
      };

      //! The types the value can have.
      enum ValueType {
        StringType = 0, //!< Item contains text information coded in UTF-8
        BinaryType = 1  //!< Item contains binary information
      };

      /*!
       * Construct a string simple tag.
       */
      SimpleTag(const String &name, const String &value,
                TargetTypeValue targetTypeValue = None,
                const String &language = String(), bool defaultLanguage = true,
                unsigned long long trackUid = 0);

      /*!
       * Construct a binary simple tag.
       */
      SimpleTag(const String &name, const ByteVector &value,
        TargetTypeValue targetTypeValue = None,
        const String &language = String(), bool defaultLanguage = true,
        unsigned long long trackUid = 0);

      /*!
       * Construct a simple tag as a copy of \a other.
       */
      SimpleTag(const SimpleTag &other);

      /*!
       * Construct a simple tag moving from \a other.
       */
      SimpleTag(SimpleTag &&other) noexcept;

      /*!
       * Destroys this simple tag.
       */
      ~SimpleTag();

      /*!
       * Copies the contents of \a other into this item.
       */
      SimpleTag &operator=(const SimpleTag &other);

      /*!
       * Moves the contents of \a other into this item.
       */
      SimpleTag &operator=(SimpleTag &&other) noexcept;

      /*!
       * Exchanges the content of the simple tag with the content of \a other.
       */
      void swap(SimpleTag &other) noexcept;

      /*!
       * Returns the name of the simple tag.
       */
      const String &name() const;

      /*!
       * Returns the logical level of the target.
       */
      TargetTypeValue targetTypeValue() const;

      /*!
       * Returns the language of the tag.
       */
      const String &language() const;

      /*!
       * Returns if this is the default/original language to use for the tag.
       */
      bool defaultLanguageFlag() const;

      /*!
       * Returns the UID that identifies the track that the tags belong to,
       * zero if not defined, the tag applies to all tracks
       */
      unsigned long long trackUid() const;

      /*!
       * Returns the type of the value.
       */
      ValueType type() const;

      /*!
       * Returns the StringType value.
       */
      String toString() const;

      /*!
       * Returns the BinaryType value.
       */
      ByteVector toByteVector() const;

    private:
      class SimpleTagPrivate;
      TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
      std::unique_ptr<SimpleTagPrivate> d;
    };

  }
}

#endif
