/***************************************************************************
    copyright           : (C) 2011 by Mathias Panzenböck
    email               : grosser.meister.morti@gmx.net
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#ifndef TAGLIB_S3MFILE_H
#define TAGLIB_S3MFILE_H

#include <stdint.h>

#include "tfile.h"
#include "audioproperties.h"
#include "taglib_export.h"
#include "modfile.h"
#include "modtag.h"
#include "s3mproperties.h"

namespace TagLib {

	namespace S3M {

		class TAGLIB_EXPORT File : public Mod::File {
			public:
				/*!
				 * Contructs a ScreamTracker III file from \a file. If \a readProperties
				 * is true the file's audio properties will also be read using
				 * \a propertiesStyle. If false, \a propertiesStyle is ignored.
				 */
				File(FileName file, bool readProperties = true,
				     AudioProperties::ReadStyle propertiesStyle =
				     AudioProperties::Average);

				/*!
				 * Contructs a ScreamTracker III file from \a stream. If \a readProperties
				 * is true the file's audio properties will also be read using
				 * \a propertiesStyle. If false, \a propertiesStyle is ignored.
				 */
				File(IOStream *stream, bool readProperties = true,
				     AudioProperties::ReadStyle propertiesStyle =
				     AudioProperties::Average);

				/*!
				 * Destroys this instance of the File.
				 */
				virtual ~File();

				Mod::Tag *tag() const;

				/*!
				 * Returns the S3M::Properties for this file. If no audio properties
				 * were read then this will return a null pointer.
				 */
				S3M::Properties *audioProperties() const;

				/*!
				 * Save the file.
				 * This is the same as calling save(AllTags);
				 *
				 * \note Saving ScreamTracker III tags is not supported.
				 */
				bool save();

			private:
				File(const File &);
				File &operator=(const File &);

				void read(bool readProperties);

				class FilePrivate;
				FilePrivate *d;
		};
	}
}

#endif
