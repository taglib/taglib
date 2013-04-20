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

#ifndef TAGLIB_CHAPTERFRAME
#define TAGLIB_CHAPTERFRAME

#include "id3v2frame.h"

namespace TagLib {

  namespace ID3v2 {

    /*!
     * This is an implementation of ID3v2 chapter frames.  The purpose of this 
     * frame is to describe a single chapter within an audio file.
     */

    //! An implementation of ID3v2 chapter frames

    class TAGLIB_EXPORT ChapterFrame : public ID3v2::Frame
    {
      friend class FrameFactory;

    public:
      /*!
       * Creates a chapter frame based on \a data.
       */
      ChapterFrame(const ByteVector &data);

      /*!
       * Creates a chapter frame with the element ID \a eID,
       * start time \a sT, end time \a eT, start offset \a sO
       * and end offset \a eO.
       */
      ChapterFrame(const ByteVector &eID, const int &sT, const int &eT, const int &sO, const int &eO);

      /*!
       * Destroys the frame.
       */
      ~ChapterFrame();
      
      /*!
       * Returns the elementID of the frame. Element ID
       * is a null terminated string, however it's not human-readable.
       * 
       * \see setElementID()
       */
      ByteVector elementID() const;
      
      /*!
       * Returns time of chapter's start (in miliseconds).
       * 
       * \see setStartTime()
       */
      uint startTime() const;
      
      /*!
       * Returns time of chapter's end (in miliseconds).
       * 
       * \see setEndTime()
       */
      uint endTime() const;
      
      /*!
       * Returns zero based byte offset (count of bytes from the beginning
       * of the audio file) of chapter's start.
       * 
       * \see setStartOffset()
       */
      uint startOffset() const;
      
      /*!
       * Returns zero based byte offset (count of bytes from the beginning
       * of the audio file) of chapter's end.
       * 
       * \see setEndOffset()
       */
      uint endOffset() const;

      /*!
       * Sets the elementID of the frame to \a eID. 
       * 
       * \warning Element ID must be null terminated.
       * \see elementID()
       */
      void setElementID(const ByteVector &eID);
      
      /*!
       * Sets time of chapter's start (in miliseconds) to \a sT.
       * 
       * \see startTime()
       */
      void setStartTime(const uint &sT);
      
      /*!
       * Sets time of chapter's end (in miliseconds) to \a eT.
       * 
       * \see endTime()
       */
      void setEndTime(const uint &eT);
      
      /*!
       * Sets zero based byte offset (count of bytes from the beginning
       * of the audio file) of chapter's start to \a sO.
       * 
       * \see startOffset()
       */
      void setStartOffset(const uint &sO);
      
      /*!
       * Sets zero based byte offset (count of bytes from the beginning
       * of the audio file) of chapter's end to \a eO.
       * 
       * \see endOffset()
       */
      void endOffset(const uint &eO);

      virtual String toString() const;

      PropertyMap asProperties() const;

      /*!
       * CHAP frames each have a unique element ID. This searches for a CHAP
       * frame with the element ID \a eID and returns a pointer to it.
       *
       * \see elementID()
       */
      static ChapterFrame *findByElementID(const Tag *tag, const ByteVector &eID);

    protected:
      virtual void parseFields(const ByteVector &data);
      virtual ByteVector renderFields() const;

    private:
      ChapterFrame(const ChapterFrame &);
      ChapterFrame &operator=(const ChapterFrame &);

      ChapterFrame(const ByteVector &data, Header *h);

      class ChapterFramePrivate;
      ChapterFramePrivate *d;
    };
  }
}

#endif
