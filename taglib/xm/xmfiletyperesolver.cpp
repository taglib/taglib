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

#include "xmfiletyperesolver.h"
#include "xmfile.h"

TagLib::File *XMFileTypeResolver::createFile(
		TagLib::FileName fileName,
		bool readProperties,
		TagLib::AudioProperties::ReadStyle propertiesStyle) const {
	TagLib::XM::File *f = new TagLib::XM::File(fileName, readProperties, propertiesStyle);
	if (f->isValid()) {
		return f;
	}
	else {
		delete f;
		return 0;
	}
}
