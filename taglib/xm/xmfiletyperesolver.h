/***************************************************************************
    copyright            : (C) 2011 by Mathias Panzenböck
    email                : grosser.meister.morti@gmx.net
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

#ifndef TAGLIB_XMFILETYPERESOLVER_H
#define TAGLIB_XMFILETYPERESOLVER_H

#include "fileref.h"
#include "taglib_export.h"

class TAGLIB_EXPORT XMFileTypeResolver : public TagLib::FileRef::FileTypeResolver {
	TagLib::File *createFile(TagLib::FileName fileName,
	        bool readAudioProperties,
	        TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const;
	~XMFileTypeResolver() {}
};

#endif
