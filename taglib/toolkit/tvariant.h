/***************************************************************************
    copyright            : (C) 2023 by Urs Fleisch
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

#ifndef TAGLIB_VARIANT_H
#define TAGLIB_VARIANT_H

#include <memory.h>
#include <iostream>

#include "tlist.h"
#include "tmap.h"
#include "taglib_export.h"

// Forward declaration needed for friend function
namespace TagLib { class Variant; }

/*!
 * \relates TagLib::Variant
 *
 * Send the variant to an output stream.
 */
TAGLIB_EXPORT std::ostream &operator<<(std::ostream &s, const TagLib::Variant &v);

namespace TagLib {

  class String;
  class StringList;
  class ByteVector;
  class ByteVectorList;

  /*!
   * This is an implicitly shared discriminated union.
   *
   * The use of implicit sharing means that copying a variant is cheap.
   * These Variant objects are immutable (have only const methods).
   */
  class TAGLIB_EXPORT Variant
  {
  public:
    /*!
     * Types which can be stored in a variant.
     */
    // The number and order of these types must correspond to the template
    // parameters for StdVariantType in tvariant.cpp!
    enum Type {
      Void,
      Bool,
      Int,
      UInt,
      LongLong,
      ULongLong,
      Double,
      String,
      StringList,
      ByteVector,
      ByteVectorList,
      VariantList,
      VariantMap
    };

    /*!
     * Constructs an empty Variant.
     */
    Variant();

    /*!
     * Constructs a Variant from an integer value.
     */
    Variant(int val);

    Variant(unsigned int val);
    Variant(long long val);
    Variant(unsigned long long val);
    Variant(bool val);
    Variant(double val);
    Variant(const char *val);
    Variant(const TagLib::String &val);
    Variant(const TagLib::StringList &val);
    Variant(const TagLib::ByteVector &val);
    Variant(const TagLib::ByteVectorList &val);
    Variant(const TagLib::List<TagLib::Variant> &val);
    Variant(const TagLib::Map<TagLib::String, TagLib::Variant> &val);

    /*!
     * Make a shallow, implicitly shared, copy of \a v.  Because this is
     * implicitly shared, this method is lightweight and suitable for
     * pass-by-value usage.
     */
    Variant(const Variant &v);

    /*!
     * Destroys this Variant instance.
     */
    ~Variant();

    /*!
     * Get the type which is currently stored in this Variant.
     */
    Type type() const;

    /*!
     * Returns true if the Variant is empty.
     */
    bool isEmpty() const;

    /*!
     * Extracts an integer value from the Variant.
     * If \a ok is passed, its boolean variable will be set to true if the
     * Variant contains the correct type, and the returned value is the value
     * of the Variant.  Otherwise, the \a ok variable is set to false and
     * a dummy default value is returned.
     */
    int toInt(bool *ok = nullptr) const;

    unsigned int toUInt(bool *ok = nullptr) const;
    long long toLongLong(bool *ok = nullptr) const;
    unsigned long long toULongLong(bool *ok = nullptr) const;
    bool toBool(bool *ok = nullptr) const;
    double toDouble(bool *ok = nullptr) const;
    TagLib::String toString(bool *ok = nullptr) const;
    TagLib::StringList toStringList(bool *ok = nullptr) const;
    TagLib::ByteVector toByteVector(bool *ok = nullptr) const;
    TagLib::ByteVectorList toByteVectorList(bool *ok = nullptr) const;
    TagLib::List<TagLib::Variant> toList(bool *ok = nullptr) const;
    TagLib::Map<TagLib::String, TagLib::Variant> toMap(bool *ok = nullptr) const;

    /*!
     * Extracts value of type \a T from the Variant.
     * If \a ok is passed, its boolean variable will be set to true if the
     * Variant contains the correct type, and the returned value is the value
     * of the Variant.  Otherwise, the \a ok variable is set to false and
     * a dummy default value is returned.
     */
    template<typename T>
    T value(bool *ok = nullptr) const;

    /*!
     * Returns true it the Variant and \a v are of the same type and contain the
     * same value.
     */
    bool operator==(const Variant &v) const;

    /*!
     * Returns true it the Variant and \a v  differ in type or value.
     */
    bool operator!=(const Variant &v) const;

    /*!
     * Performs a shallow, implicitly shared, copy of \a v, overwriting the
     * Variant's current data.
     */
    Variant &operator=(const Variant &v);

  private:
    friend std::ostream& ::operator<<(std::ostream &s, const TagLib::Variant &v);
    class VariantPrivate;
    std::shared_ptr<VariantPrivate> d;
  };

  /*! A list of Variant elements. */
  using VariantList = TagLib::List<TagLib::Variant>;

  /*! A map with String keys and Variant values. */
  using VariantMap = TagLib::Map<TagLib::String, TagLib::Variant>;

  extern template TAGLIB_EXPORT bool Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT int Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT unsigned int Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT long long Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT unsigned long long Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT double Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT String Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT StringList Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT ByteVector Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT ByteVectorList Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT VariantList Variant::value(bool *ok) const;
  extern template TAGLIB_EXPORT VariantMap Variant::value(bool *ok) const;
}  // namespace TagLib

#endif
