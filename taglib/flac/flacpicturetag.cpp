#include "flacpicturetag.h"

using namespace TagLib;
String TagLib::FLAC::FlacPictureTag::title() const {
	return String();
}

String TagLib::FLAC::FlacPictureTag::artist() const {
	return String();
}

String TagLib::FLAC::FlacPictureTag::album() const {
	return String();
}

String TagLib::FLAC::FlacPictureTag::comment() const {
	return String();
}

String TagLib::FLAC::FlacPictureTag::genre() const {
	return String();
}

unsigned int TagLib::FLAC::FlacPictureTag::year() const {
	return 0;
}

unsigned int TagLib::FLAC::FlacPictureTag::track() const {
	return 0;
}

PictureMap TagLib::FLAC::FlacPictureTag::pictures() const {
	auto plist = file->pictureList();
	PictureMap map;
	for (auto it = plist.begin(); it != plist.end(); it++) {
		map.insert(TagLib::Picture((*it)->data(), (TagLib::Picture::Type)(int)(*it)->type(), (*it)->mimeType(), (*it)->description()));//copy all existing pictures
	}
	return map;
}

void TagLib::FLAC::FlacPictureTag::setTitle(const String& s) {}

void TagLib::FLAC::FlacPictureTag::setArtist(const String& s) {}

void TagLib::FLAC::FlacPictureTag::setAlbum(const String& s) {}

void TagLib::FLAC::FlacPictureTag::setComment(const String& s) {}

void TagLib::FLAC::FlacPictureTag::setGenre(const String& s) {}

void TagLib::FLAC::FlacPictureTag::setYear(unsigned int i) {}

void TagLib::FLAC::FlacPictureTag::setTrack(unsigned int i) {}

/*!
*  Sets the list of pictures
*/

void TagLib::FLAC::FlacPictureTag::setPictures(const PictureMap& l) {
	file->removePictures();//remove all existing pictures
	if (!l.isEmpty())
		for (auto it = l.begin(); it != l.end(); it++) {
			auto list = it->second;
			if (!list.isEmpty())
				for (auto picture = list.begin(); picture != list.end(); picture++)
					file->addPicture(new Picture(picture->data()));//add all new pictures
		}
}
