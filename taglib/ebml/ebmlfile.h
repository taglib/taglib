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

#ifndef TAGLIB_EBMLFILE_H
#define TAGLIB_EBMLFILE_H

#include "taglib_export.h"
#include "tfile.h"

#include "ebmlconstants.h"

namespace TagLib {

  //! A namespace for the classes used by EBML-based metadata files
  namespace EBML {
  
    class Element;
  
    /*!
     * Represents an EBML file. It offers access to the root element which can
     * be used to obtain the necessary information and to change the file
     * according to changes.
     */
    class TAGLIB_EXPORT File : public TagLib::File
    {
    public:
      //! Destroys the instance of the file.
      virtual ~File();
      
      /*!
       * Returns a pointer to the document root element of the EBML file.
       */
      Element *getDocumentRoot();
      
    protected:
      /*!
       * Constructs an instance of an EBML file from \a file.
       *
       * This constructor is protected since an object should be created
       * through a specific subclass.
       */
      explicit File(FileName file);
      
      /*!
       * Constructs an instance of an EBML file from an IOStream.
       *
       * This constructor is protected since an object should be created
       * through a specific subclass.
       */
      explicit File(IOStream *stream);
    
    private:
      //! Non-copyable
      File(const File&);
      File &operator=(const File &);
      
      class FilePrivate;
      FilePrivate *d;
    };
  
  }
}

#endif
