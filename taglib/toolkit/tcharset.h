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

#ifndef TAGLIB_TCHARSET_H
#define TAGLIB_TCHARSET_H

#include "taglib_export.h"

#include <optional>

namespace TagLib {

  class ByteVector;
  class String;

  /*!
   * Conversions between the byte encodings found in files and TagLib's
   * internal string representation, which is UTF-16 based and therefore
   * independent of all of them.  A decoded string is a normal String from
   * there on; no charset is remembered beyond the call.
   *
   * These are kept out of String because String::Type only knows about the
   * codecs that TagLib's own formats need, whereas the Windows code page
   * numbering and the EBCDIC and DOS pages are used by none of them and only
   * show up because a foreign container format asks for them by number.
   */
  namespace Charset {

    /*!
     * One of the supported encodings.  Every value is the Microsoft code page
     * number of a single byte encoding, which is the same numbering a CSET
     * chunk of a RIFF file carries, so a caller can hand in the number a file
     * states.  UTF8 is the one exception: it is not a single byte code page,
     * so it takes 65001, the number Windows uses for UTF-8 in place of a code
     * page, and has no byte table of its own.
     *
     * \note The values are code page numbers and are therefore neither
     * contiguous nor ordered; they are 37, 437, 1250, ...  Nothing may use a
     * value as an index.
     */
    enum class Type : unsigned int
    {
      EBCDIC037  = 37,  //!< IBM EBCDIC US-Canada
      OEM437     = 437,  //!< IBM437 OEM US
      EBCDIC500  = 500,  //!< IBM EBCDIC International
      OEM708     = 708,  //!< ASMO-708 Arabic
      OEM737     = 737,  //!< IBM737 OEM Greek
      OEM775     = 775,  //!< IBM775 OEM Baltic
      OEM850     = 850,  //!< IBM850 OEM Latin 1
      OEM852     = 852,  //!< IBM852 OEM Latin 2
      OEM855     = 855,  //!< IBM855 OEM Cyrillic
      OEM857     = 857,  //!< IBM857 OEM Turkish
      OEM858     = 858,  //!< IBM858 OEM Latin 1 with Euro
      OEM860     = 860,  //!< IBM860 OEM Portuguese
      OEM861     = 861,  //!< IBM861 OEM Icelandic
      OEM862     = 862,  //!< DOS-862 OEM Hebrew
      OEM863     = 863,  //!< IBM863 OEM French Canadian
      OEM864     = 864,  //!< IBM864 OEM Arabic
      OEM865     = 865,  //!< IBM865 OEM Nordic
      OEM866     = 866,  //!< IBM866 OEM Russian
      OEM869     = 869,  //!< IBM869 OEM Modern Greek
      Thai874    = 874,  //!< Thai (Windows-874)
      EBCDIC1047 = 1047,  //!< IBM1047 EBCDIC Latin 1/Open System
      EBCDIC1140 = 1140,  //!< IBM037 EBCDIC US-Canada with Euro
      EBCDIC1141 = 1141,  //!< IBM273 EBCDIC Germany with Euro
      EBCDIC1142 = 1142,  //!< IBM277 EBCDIC Denmark-Norway with Euro
      EBCDIC1143 = 1143,  //!< IBM278 EBCDIC Finland-Sweden with Euro
      EBCDIC1144 = 1144,  //!< IBM280 EBCDIC Italy with Euro
      EBCDIC1145 = 1145,  //!< IBM284 EBCDIC Latin America-Spain with Euro
      EBCDIC1146 = 1146,  //!< IBM285 EBCDIC UK with Euro
      EBCDIC1147 = 1147,  //!< IBM297 EBCDIC France with Euro
      EBCDIC1148 = 1148,  //!< IBM500 EBCDIC International with Euro
      EBCDIC1149 = 1149,  //!< IBM871 EBCDIC Icelandic with Euro
      Windows1250= 1250,  //!< Windows-1250 Central European
      Windows1251= 1251,  //!< Windows-1251 Cyrillic
      Windows1252= 1252,  //!< Windows-1252 Western European
      Windows1253= 1253,  //!< Windows-1253 Greek
      Windows1254= 1254,  //!< Windows-1254 Turkish
      Windows1255= 1255,  //!< Windows-1255 Hebrew
      Windows1256= 1256,  //!< Windows-1256 Arabic
      Windows1257= 1257,  //!< Windows-1257 Baltic
      Windows1258= 1258,  //!< Windows-1258 Vietnamese
      ASCII      = 20127,  //!< US-ASCII
      EBCDIC273  = 20273,  //!< IBM273 EBCDIC Germany
      EBCDIC277  = 20277,  //!< IBM277 EBCDIC Denmark-Norway
      EBCDIC278  = 20278,  //!< IBM278 EBCDIC Finland-Sweden
      EBCDIC280  = 20280,  //!< IBM280 EBCDIC Italy
      EBCDIC284  = 20284,  //!< IBM284 EBCDIC Latin America-Spain
      EBCDIC285  = 20285,  //!< IBM285 EBCDIC United Kingdom
      EBCDIC290  = 20290,  //!< IBM290 EBCDIC Japanese Katakana Extended
      EBCDIC297  = 20297,  //!< IBM297 EBCDIC France
      EBCDIC420  = 20420,  //!< IBM420 EBCDIC Arabic
      EBCDIC423  = 20423,  //!< IBM423 EBCDIC Greek
      KOI8R      = 20866,  //!< KOI8-R
      EBCDIC871  = 20871,  //!< IBM871 EBCDIC Icelandic
      EBCDIC1025 = 21025,  //!< IBM1025 EBCDIC Cyrillic Serbian-Bulgarian
      KOI8U      = 21866,  //!< KOI8-U
      Latin1     = 28591,  //!< ISO 8859-1
      Latin2     = 28592,  //!< ISO 8859-2
      Latin3     = 28593,  //!< ISO 8859-3
      Latin4     = 28594,  //!< ISO 8859-4
      Latin5     = 28595,  //!< ISO 8859-5
      Latin6     = 28596,  //!< ISO 8859-6
      Latin7     = 28597,  //!< ISO 8859-7
      Latin8     = 28598,  //!< ISO 8859-8 Hebrew (logical)
      Latin9     = 28599,  //!< ISO 8859-9
      Latin13    = 28603,  //!< ISO 8859-13
      Latin15    = 28605,  //!< ISO 8859-15
      UTF8       = 65001   //!< UTF-8
    };

    /*!
     * Returns the encoding identified by the Microsoft code page \a codePage,
     * or nothing if this module cannot convert it.
     *
     * Note that code page 0 is not a code page.  It is conventionally used to
     * mean "the encoding was not specified", so substituting a default for it
     * belongs to whoever read the number, not here.
     */
    TAGLIB_EXPORT std::optional<Type> typeForCodePage(unsigned int codePage);

    /*!
     * Returns the Microsoft code page identifying \a type.
     */
    TAGLIB_EXPORT unsigned int codePageForType(Type type);

    /*!
     * Decodes \a data according to \a type.  Decoding stops at the first NUL,
     * which is how the single byte code pages terminate a string.
     */
    TAGLIB_EXPORT String decode(const ByteVector &data, Type type);

    /*!
     * Encodes \a s as UTF-8, which is the only encoding this module produces.
     * A string is decoded into TagLib's internal representation, which is UTF-16
     * and not a byte encoding at all, so a caller that stores the output has to
     * say what it is, and the only thing this module can make it is UTF-8.
     *
     * There is deliberately no way to encode into a legacy code page. Doing so
     * would have to invent a byte for every character the page cannot represent,
     * and the usual choice, '?', throws away text that the internal
     * representation is still holding perfectly intact.
     */
    TAGLIB_EXPORT ByteVector encode(const String &s);
  }  // namespace Charset
}  // namespace TagLib

#endif
