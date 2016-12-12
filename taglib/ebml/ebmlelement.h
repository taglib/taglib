/***************************************************************************
    copyright            : (C) 2013 by Sebastian Rachuj
    email                : rachus@web.de
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

#ifndef TAGLIB_EBMLELEMENT_H
#define TAGLIB_EBMLELEMENT_H

#include "tlist.h"
#include "tbytevector.h"
#include "tstring.h"

#include "ebmlfile.h"

namespace TagLib {

  namespace EBML {

    /*!
     * Represents an element of the EBML. The only instance of this child, that
     * is directly used is the root element. Every other element is accessed
     * via pointers to the elements within the root element.
     *
     * Just create one root instance per file to prevent race conditions.
     *
     * Changes of the document tree will be directly written back to the file.
     * Invalid values (exceeding the maximal value defined in the RFC) will be
     * truncated.
     *
     * This class should not be used by library users since the proper file
     * class should handle the internals.
     *
     * NOTE: Currently does not adjust CRC32 values.
     */
    class TAGLIB_EXPORT Element
    {
    public:
      //! Destroys the instance of the element.
      ~Element();

      /*!
       * Creates an root element using document.
       */
      explicit Element(File *document);

      /*!
       * Returns the first found child element with the given id. Returns a null
       * pointer if the child does not exist.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       */
      Element *getChild(const ulli id);

      /*!
       * Returns a list of all child elements with the given id. Returns an
       * empty list if no such element exists.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       */
      List<Element *> getChildren(const ulli id);

      /*!
       * Returns a list of every child elements available. Returns an empty list
       * if there are no children.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       */
      List<Element *> getChildren();

      /*!
       * Returns the parent element or null if no such element exists.
       */
      Element *getParent();

      /*!
       * Returns the raw content of the element.
       */
      ByteVector getAsBinary();

      /*!
       * Returns the content of this element interpreted as a string.
       */
      String getAsString();

      /*!
       * Returns the content of this element interpreted as an signed integer.
       *
       * Do not call this method if *this element is not an INT element (see
       * corresponding DTD)
       */
      long long getAsInt();

      /*!
       * Returns the content of this element interpreted as an unsigned integer.
       *
       * Do not call this method if *this element is not an UINT element (see
       * corresponding DTD)
       */
      ulli getAsUnsigned();

      /*!
       * Returns the content of this element interpreted as a floating point
       * type.
       *
       * Do not call this method if *this element is not an FLOAT element (see
       * corresponding DTD)
       *
       * NOTE: There are 10 byte floats defined, therefore we might need a long
       * double to store the value.
       */
      long double getAsFloat();

      /*!
       * Adds an empty element with given id to this element. Returns a pointer
       * to the new element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       */
      Element *addElement(ulli id);

      /*!
       * Adds a new element, containing the given binary, to this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       */
      Element *addElement(ulli id, const ByteVector &binary);

      /*!
       * Adds a new element, containing the given string, to this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       */
      Element *addElement(ulli id, const String &string);

      /*!
       * Adds a new element, containing the given integer, to this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       */
      Element *addElement(ulli id, signed long long number);

      /*!
       * Adds a new element, containing the given unsigned integer, to this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       */
      Element *addElement(ulli id, ulli number);

      /*!
       * Adds a new element, containing the given floating point value, to this element.
       * Returns a pointer to the new element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       *
       * This method is not implemented!
       */
      Element *addElement(ulli id, long double number);

      /*!
       * Removes all children with the given id. Returns false if there was no
       * such element.
       * If useVoid is true, the element will be changed to a void element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       *
       * Every pointer to a removed element is invalidated.
       */
      bool removeChildren(ulli id, bool useVoid = true);

      /*!
       * Removes all children. Returns false if this element had no children.
       * If useVoid ist rue, the element will be changed to a void element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       *
       * Every pointer to a removed element is invalidated.
       */
      bool removeChildren(bool useVoid = true);

      /*!
       * Removes the given element.
       * If useVoid is true, the element will be changed to a void element.
       *
       * Do not call this method if *this element is not a container element (see
       * corresponding DTD)
       *
       * The pointer to the given element is invalidated.
       */
      bool removeChild(Element *element, bool useVoid = true);

      /*!
       * Writes the given binary to this element.
       */
      void setAsBinary(const ByteVector &binary);

      /*!
       * Writes the given string to this element.
       */
      void setAsString(const String &string);

      /*!
       * Writes the given integer to this element.
       */
      void setAsInt(signed long long number);

      /*!
       * Writes the given unsigned integer to this element.
       */
      void setAsUnsigned(ulli number);

      /*!
       * Writes the given floating point variable to this element.
       *
       * This method is not implemented!
       */
      void setAsFloat(long double number);

    private:
      //! Non-copyable
      Element(const Element &);
      //! Non-copyable
      Element &operator=(const File &);

      //! Lazy parsing. This method will be triggered when trying to access
      //! children.
      void populate();

      class ElementPrivate;
      ElementPrivate *d;

      //! Creates a new Element from an ElementPrivate. (The constructor takes
      //! ownership of the pointer and will delete it when the element is
      //! destroyed.
      Element(ElementPrivate *pe);
    };

  }
}


#endif
