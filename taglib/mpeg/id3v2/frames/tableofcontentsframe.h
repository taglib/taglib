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

#ifndef TAGLIB_TABLEOFCONTENTSFRAME
#define TAGLIB_TABLEOFCONTENTSFRAME

#include "id3v2frame.h"

namespace TagLib {

  namespace ID3v2 {

    /*!
     * This is an implementation of ID3v2 table of contents frames.  Purpose
     * of this frame is to allow a table of contents to be defined.
     */

    //! An implementation of ID3v2 table of contents frames

    class TAGLIB_EXPORT TableOfContentsFrame : public ID3v2::Frame
    {
      friend class FrameFactory;

    public:
      /*!
       * Creates a table of contents frame based on \a data.
       */
      TableOfContentsFrame(const ByteVector &data);

      /*!
       * Creates a table of contents frame with the element ID \a eID and
       * the child elements \a ch.
       */
      TableOfContentsFrame(const ByteVector &eID, const ByteVectorList &ch);

      /*!
       * Destroys the frame.
       */
      ~TableOfContentsFrame();
      
      /*!
       * Returns the elementID of the frame. Element ID
       * is a null terminated string, however it's not human-readable.
       * 
       * \see setElementID()
       */
      ByteVector elementID() const;
      
      /*!
       * Returns true, if the frame is top-level (doen't have
       * any parent CTOC frame).
       * 
       * \see setIsTopLevel()
       */
      bool isTopLevel() const;
      
      /*!
       * Returns true, if the child elements list entries
       * are ordered.
       * 
       * \see setIsOrdered()
       */
      bool isOrdered() const;
      
      /*!
       * Returns count of child elements of the frame. It allways
       * corresponds to size of child elements list.
       * 
       * \see childElements()
       */
      uint entryCount() const;
      
      /*!
       * Returns list of child elements of the frame.
       *
       * \see setChildElements()
       */
      ByteVectorList childElements() const;

      /*!
       * Sets the elementID of the frame to \a eID. If \a eID isn't
       * null terminated, a null char is appended automatically.
       * 
       * \see elementID()
       */
      void setElementID(const ByteVector &eID);
      
      /*!
       * Sets, if the frame is top-level (doen't have
       * any parent CTOC frame).
       * 
       * \see isTopLevel()
       */
      void setIsTopLevel(const bool &t);
      
      /*!
       * Sets, if the child elements list entries
       * are ordered.
       * 
       * \see isOrdered()
       */
      void setIsOrdered(const bool &o);
      
      /*!
       * Sets list of child elements of the frame to \a l.
       *
       * \see childElements()
       */
      void setChildElements(const ByteVectorList &l);

      virtual String toString() const;

      PropertyMap asProperties() const;

      /*!
       * CTOC frames each have a unique element ID. This searches for a CTOC
       * frame with the element ID \a eID and returns a pointer to it. This 
       * can be used to link together parent and child CTOC frames.
       *
       * \see elementID()
       */
      static TableOfContentsFrame *findByElementID(const Tag *tag, const ByteVector &eID);
      
      /*!
       * CTOC frames each contain a flag that indicates, if CTOC frame is top-level (there isn't
       * any frame, which contains this frame in its child elements list). Only a single frame 
       * within tag can be top-level. This searches for a top-level CTOC frame.
       *
       * \see isTopLevel()
       */
      static TableOfContentsFrame *findTopLevel(const Tag *tag);

    protected:
      virtual void parseFields(const ByteVector &data);
      virtual ByteVector renderFields() const;

    private:
      TableOfContentsFrame(const TableOfContentsFrame &);
      TableOfContentsFrame &operator=(const TableOfContentsFrame &);

      TableOfContentsFrame(const ByteVector &data, Header *h);

      class TableOfContentsFramePrivate;
      TableOfContentsFramePrivate *d;
    };
  }
}

#endif
