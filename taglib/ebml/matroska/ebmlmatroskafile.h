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


#ifndef TAGLIB_EBMLMATROSKAFILE_H
#define TAGLIB_EBMLMATROSKAFILE_H

#include "ebmlelement.h"

namespace TagLib {
  
  namespace EBML {
    
    //! Implementation for reading Matroska tags.
    namespace Matroska {
      
      /*!
       * Implements the TagLib::File API and offers access to the tags of the
       * matroska file.
       */
      class TAGLIB_EXPORT File : public EBML::File
      {
      public:
        //! Destroys the instance of the file.
        virtual ~File();
        
        /*!
         * Constructs a Matroska File from a file name.
         */
        File(FileName file);
        
        /*!
         *  Constructs a Matroska File from a stream.
         */
        File(IOStream *stream);
        
        /*!
         * Returns the pointer to a tag that allow access on common tags.
         */
        virtual TagLib::Tag *tag() const;
        
        /*!
         * Exports the tags to a PropertyMap. Due to the diversity of the
         * Matroska format (e.g. multiple media streams in one file, each with
         * its own tags), only the best fitting tags are taken into account.
         * There are no unsupported tags.
         */
        virtual PropertyMap properties() const;
        
        /*!
         * Sets the tags of the file to those specified in properties. The
         * returned PropertyMap is always empty.
         * Note: Only the best fitting tags are taken into account.
         */
        virtual PropertyMap setProperties(const PropertyMap &properties);
        
        /*!
         * Returns a pointer to this file's audio properties.
         * 
         * I'm glad about not having a setAudioProperties method ;)
         */
        virtual AudioProperties *audioProperties() const;
        
        /*!
         * Saves the file. Returns true on success.
         */
        bool save();
        
        /*!
         * Offers access to a few common tag entries.
         */
        class Tag : public TagLib::Tag
        {
         public:
          //! Destroys the tag.
          ~Tag();
          
          /*!
           * Creates a new Tag for Matroska files. The given properties are gained
           * by the Matroska::File.
           */
          Tag(File *document);
          
          /*!
           * Returns the track name; if no track name is present in the tag
           * String::null will be returned.
           */
          virtual String title() const;
        
          /*!
           * Returns the artist name; if no artist name is present in the tag
           * String::null will be returned.
           */
          virtual String artist() const;
          
          /*!
           * Returns the album name; if no album name is present in the tag
           * String::null will be returned.
           */
          virtual String album() const;
          
          /*!
           * Returns the track comment; if no comment is present in the tag
           * String::null will be returned.
           */
          virtual String comment() const;
          
          /*!
           * Returns the genre name; if no genre is present in the tag String::null
           * will be returned.
           */
          virtual String genre() const;
          
          /*!
           * Returns the year; if there is no year set, this will return 0.
           */
          virtual uint year() const;
          
          /*!
           * Returns the track number; if there is no track number set, this will
           * return 0.
           */
          virtual uint track() const;
          
          /*!
           * Sets the title to s. If s is String::null then this value will be
           * cleared.
           */
          virtual void setTitle(const String &s);
          
          /*!
           * Sets the artist to s. If s is String::null then this value will be
           * cleared.
           */
          virtual void setArtist(const String &s);
          
          /*!
           * Sets the album to s. If s is String::null then this value will be
           * cleared.
           */
          virtual void setAlbum(const String &s);
          
          /*!
           * Sets the comment to s. If s is String::null then this value will be
           * cleared.
           */
          virtual void setComment(const String &s);
          
          /*!
           * Sets the genre to s. If s is String::null then this value will be
           * cleared.
           */
          virtual void setGenre(const String &s);
          
          /*!
           * Sets the year to i. If s is 0 then this value will be cleared.
           */
          virtual void setYear(uint i);
          
          /*!
           * Sets the track to i. If s is 0 then this value will be cleared.
           */
          virtual void setTrack(uint i);
        
        private:
          class TagPrivate;
          TagPrivate *e;
        };
        
      private:
        class FilePrivate;
        FilePrivate *d;
      };
      
    }
  }
}

#endif
