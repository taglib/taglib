/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
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
 ***************************************************************************/

#ifndef TAGLIB_MPCFILE_H
#define TAGLIB_MPCFILE_H

#include <tfile.h>

#include "mpcproperties.h"

namespace TagLib {

  class Tag;

  //! An implementation of MPC metadata

  /*!
   * This is implementation of MPC metadata.
   *
   * This supports ID3v1 and APE (v1 and v2) style comments as well as reading stream
   * properties from the file. ID3v2 tags will be skipped and ignored.
   */

  namespace MPC {

    //! An implementation of TagLib::File with MPC specific methods

    /*!
     * This implements and provides an interface for MPC files to the
     * TagLib::Tag and TagLib::AudioProperties interfaces by way of implementing
     * the abstract TagLib::File API as well as providing some additional
     * information specific to MPC files.
     */

    class File : public TagLib::File
    {
    public:
      /*!
       * Contructs an MPC file from \a file.  If \a readProperties is true the
       * file's audio properties will also be read using \a propertiesStyle.  If
       * false, \a propertiesStyle is ignored.
       */
      File(const char *file, bool readProperties = true,
           Properties::ReadStyle propertiesStyle = Properties::Average);

      /*!
       * Destroys this instance of the File.
       */
      virtual ~File();

      /*!
       * Returns the Tag for this file.  This will be either an APE or
       * ID3v1 tag.
       */
      virtual TagLib::Tag *tag() const;

      /*!
       * Returns the MPC::Properties for this file.  If no audio properties
       * were read then this will return a null pointer.
       */
      virtual Properties *audioProperties() const;

      /*!
       * Save the file.
       */
      virtual void save();

    private:
      File(const File &);
      File &operator=(const File &);

      void read(bool readProperties, Properties::ReadStyle propertiesStyle);
      void scan();
      bool findAPE();
      long findID3v1();
      long findID3v2();

      class FilePrivate;
      FilePrivate *d;
    };
  }
}

#endif
