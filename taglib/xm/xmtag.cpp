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

#include "xmtag.h"

using namespace TagLib;
using namespace XM;

class XM::Tag::TagPrivate
{
public:
	TagPrivate() {}

	String title;
	String comment;
	String trackerName;
};

XM::Tag::Tag() : TagLib::Tag()
{
	d = new TagPrivate;
}

XM::Tag::~Tag()
{
	delete d;
}

String XM::Tag::title() const
{
	return d->title;
}

String XM::Tag::artist() const
{
	return String::null;
}

String XM::Tag::album() const
{
	return String::null;
}

String XM::Tag::comment() const
{
	return d->comment;
}

String XM::Tag::genre() const
{
	return String::null;
}

uint XM::Tag::year() const
{
	return 0;
}

uint XM::Tag::track() const
{
	return 0;
}

String XM::Tag::trackerName() const
{
	return d->trackerName;
}

void XM::Tag::setTitle(const String &title)
{
	d->title = title;
}

void XM::Tag::setArtist(const String &)
{
}

void XM::Tag::setAlbum(const String &)
{
}

void XM::Tag::setComment(const String &comment)
{
	d->comment = comment;
}

void XM::Tag::setGenre(const String &)
{
}

void XM::Tag::setYear(uint)
{
}

void XM::Tag::setTrack(uint)
{
}

void XM::Tag::setTrackerName(const String &trackerName)
{
	d->trackerName = trackerName;
}
