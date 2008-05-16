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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef TAGLIB_RIFFFILE_H
#define TAGLIB_RIFFFILE_H

#include "taglib_export.h"
#include "tfile.h"

namespace TagLib {

  //! An implementation of TagLib::File with RIFF specific methods

  namespace RIFF {

    //! An RIFF file class with some useful methods specific to RIFF

    /*!
     * This implements the generic TagLib::File API and additionally provides
     * access to properties that are distinct to RIFF files, notably access
     * to the different ID3 tags.
     */

    class TAGLIB_EXPORT File : public TagLib::File
    {
    public:
      /*!
       * Destroys this instance of the File.
       */
      virtual ~File();

    protected:
      File(FileName file);

    private:
      File(const File &);
      File &operator=(const File &);

      void read();

      class FilePrivate;
      FilePrivate *d;
    };
  }
}

#endif
