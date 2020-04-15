/***************************************************************************
    copyright            : (C) 2003 by Scott Wheeler
    email                : wheeler@kde.org
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <attachedpictureframe.h>
#include <id3v2tag.h>
#include <tbytevector.h>
#include <fileref.h>
#include <tfile.h>
#include <tlist.h>
#include <asffile.h>
#include <vorbisfile.h>
#include <mpegfile.h>
#include <flacfile.h>
#include <oggflacfile.h>
#include <mpcfile.h>
#include <wavpackfile.h>
#include <speexfile.h>
#include <trueaudiofile.h>
#include <mp4file.h>
#include <tag.h>
#include <string.h>
#include <id3v2framefactory.h>
#include <id3v2frame.h>

#include "tag_c.h"

using namespace TagLib;

namespace
{
  List<char *> strings;
  bool unicodeStrings = true;
  bool stringManagementEnabled = true;

  char *stringToCharArray(const String &s)
  {
    const std::string str = s.to8Bit(unicodeStrings);

#ifdef HAVE_ISO_STRDUP

    return ::_strdup(str.c_str());

#else

    return ::strdup(str.c_str());

#endif
  }

  String charArrayToString(const char *s)
  {
    return String(s, unicodeStrings ? String::UTF8 : String::Latin1);
  }
}

// Image file
class Image : public File
{
public:
  Image(const char *filename): File(filename) { }
  ~Image() = default;

  ByteVector data()
  {
    return readBlock(length());
  }
private:
  virtual Tag* tag() const
  {
    return 0;
  }

  virtual AudioProperties* audioProperties() const
  {
    return 0;
  }

  virtual bool save()
  {
    return 0;
  }
};

void taglib_set_strings_unicode(BOOL unicode)
{
  unicodeStrings = (unicode != 0);
}

void taglib_set_string_management_enabled(BOOL management)
{
  stringManagementEnabled = (management != 0);
}

void taglib_free(void* pointer)
{
  free(pointer);
}


////////////////////////////////////////////////////////////////////////////////
// Cover stuff
////////////////////////////////////////////////////////////////////////////////

TagLib_Mpeg_File *taglib_mpeg_file_new(const char *filename)
{
  return reinterpret_cast<TagLib_Mpeg_File *>(new MPEG::File(filename));
}

void taglib_mpef_file_free(TagLib_Mpeg_File *file)
{
  MPEG::File *f = reinterpret_cast<MPEG::File *>(file);
  free(f);
  file = NULL;
}

BOOL taglib_mpeg_file_save(TagLib_Mpeg_File *file)
{
  MPEG::File *song_file = reinterpret_cast<MPEG::File *>(file);
  return song_file->save();
}


TagLib_ID3v2_Tag *taglib_id3v2_tag_new(TagLib_Mpeg_File *file)
{
  MPEG::File *song_file = reinterpret_cast<MPEG::File *>(file);
  return reinterpret_cast<TagLib_ID3v2_Tag *>(song_file->ID3v2Tag());
}

void taglib_id3v2_tag_add_frame(TagLib_ID3v2_Tag *tag, TagLib_ID3v2_AttachedPictureFrame *frame)
{
  ID3v2::Tag *t = reinterpret_cast<ID3v2::Tag *>(tag);
  ID3v2::AttachedPictureFrame *f = reinterpret_cast<ID3v2::AttachedPictureFrame *>(frame);

  t->addFrame(f);
  tag = reinterpret_cast<TagLib_ID3v2_Tag *>(t);
}


TagLib_ID3v2_Image *taglib_id3v2_image_new(const char *filename)
{
  return reinterpret_cast<TagLib_ID3v2_Image *>(new Image(filename));
}

void taglib_id3v2_image_free(TagLib_ID3v2_Image *image)
{
  Image *img = reinterpret_cast<Image *>(image);
  delete img;
  img = NULL;
  image = reinterpret_cast<TagLib_ID3v2_Image *>(img);
}


int taglib_is_picture_frame_list_empty(TagLib_ID3v2_Tag *tag)
{
  ID3v2::Tag *t = reinterpret_cast<ID3v2::Tag *>(tag);
  ID3v2::FrameList f = t->frameListMap()["APIC"];

  if (f.isEmpty())
    return 0;
  else
    return 1;
}

void taglib_remove_picture_frame_lists(TagLib_ID3v2_Tag *tag)
{
  ID3v2::Tag *t = reinterpret_cast<ID3v2::Tag *>(tag);
  ID3v2::FrameList f = t->frameListMap()["APIC"];

  for (ID3v2::FrameList::ConstIterator it = t->frameList().begin(); it != t->frameList().end(); ++it)
  {
    ByteVector frame_id = (*it)->frameID();
    std::string frame_name(frame_id.data(), frame_id.size());

    if (frame_name.compare("APIC") == 0)
    {
      t->removeFrame((*it));
      it = t->frameList().begin();
    }
  }

  tag = reinterpret_cast<TagLib_ID3v2_Tag *>(t);
}


TagLib_ID3v2_AttachedPictureFrame *taglib_id3v2_attached_picture_frame_new()
{
  return reinterpret_cast<TagLib_ID3v2_AttachedPictureFrame *>(new ID3v2::AttachedPictureFrame);
}

void taglib_id3v2_attached_picture_frame_free(TagLib_ID3v2_AttachedPictureFrame *picture_frame)
{
  ID3v2::AttachedPictureFrame *pf = reinterpret_cast<ID3v2::AttachedPictureFrame *>(picture_frame);
  delete pf;
  pf = NULL;
  picture_frame = NULL;
}

void taglib_id3v2_attached_picture_frame_set_picture(TagLib_ID3v2_AttachedPictureFrame *picture_frame, TagLib_ID3v2_Image *image)
{
  ID3v2::AttachedPictureFrame *pf = reinterpret_cast<ID3v2::AttachedPictureFrame *>(picture_frame);
  Image *img = reinterpret_cast<Image *>(image);
  pf->setPicture(img->data());
  picture_frame = reinterpret_cast<TagLib_ID3v2_AttachedPictureFrame *>(pf);
}

int taglib_id3v2_attached_picture_frame_set_type(TagLib_ID3v2_AttachedPictureFrame *picture_frame, TagLib_Img_Type type)
{
  int result = 0;
  ID3v2::AttachedPictureFrame *pf = reinterpret_cast<ID3v2::AttachedPictureFrame *>(picture_frame);
  switch (type)
  {
    case TagLib_Img_Front_Cover:
      pf->setType(ID3v2::AttachedPictureFrame::FrontCover);
      picture_frame = reinterpret_cast<TagLib_ID3v2_AttachedPictureFrame *>(pf);
      result = 0;
      break;
    default:
      result = -1;
      break;
  }

  return result;
}


/** Ignore
////////////////////////////////////////////////////////////////////////////////
// Simple, basic, generic wrapper for covers
////////////////////////////////////////////////////////////////////////////////

int taglib_is_cover_empty(const char *filename, TagLib_File_Type type)
{
  switch (type) {
  case TagLib_File_MPEG:
    break;
  default:
    return -1;
  }

  int result = 1;

  MPEG::File *f = new MPEG::File(filename);
  ID3v2::Tag *tag = f->ID3v2Tag();

  const ID3v2::FrameList fl = tag->frameListMap()["APIC"];
  if (fl.isEmpty()) {
    result = 0;
  }

  free(tag);
  free(f);

  return result;
}

int taglib_remove_cover(const char *filename)
{
  int result = 1;

  MPEG::File *f = new MPEG::File(filename);
  ID3v2::Tag *tag = f->ID3v2Tag();

  ID3v2::FrameList fl = tag->frameListMap()["APIC"];

  for (auto it = tag->frameList().begin(); it != tag->frameList().end(); ++it) {
    auto frameID = (*it)->frameID();
	std::string fsv(frameID.data(), frameID.size());

	if (fsv.compare("APIC") == 0) {
	  tag->removeFrame(*it);
	  it = tag->frameList().begin();
	  result = 0;
	}
  }

  free(tag);
  free(f);

  return result;
}

int taglib_update_cover(const char *filename, const char *img_path, TagLib_Img_Type type)
{
  int result = 1;
  switch (type) {
  case TagLib_Img_Front_Cover:
    break;
  default:
	return -1;
  }

  MPEG::File *f = new MPEG::File(filename);
  ID3v2::Tag *tag = f->ID3v2Tag();

  TagLib::ID3v2::AttachedPictureFrame *picFrame = new TagLib::ID3v2::AttachedPictureFrame;
  Image img(img_path);
  picFrame->setPicture(img.data());
  picFrame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);

  tag->addFrame(picFrame);

  f->save();
  result = 0;

  free(picFrame);
  free(tag);
  free(f);

  return result;
}
*/

////////////////////////////////////////////////////////////////////////////////
// TagLib::File wrapper
////////////////////////////////////////////////////////////////////////////////

TagLib_File *taglib_file_new(const char *filename)
{
  return reinterpret_cast<TagLib_File *>(FileRef::create(filename));
}

TagLib_File *taglib_file_new_type(const char *filename, TagLib_File_Type type)
{
  switch(type) {
  case TagLib_File_MPEG:
    return reinterpret_cast<TagLib_File *>(new MPEG::File(filename));
  case TagLib_File_OggVorbis:
    return reinterpret_cast<TagLib_File *>(new Ogg::Vorbis::File(filename));
  case TagLib_File_FLAC:
    return reinterpret_cast<TagLib_File *>(new FLAC::File(filename));
  case TagLib_File_MPC:
    return reinterpret_cast<TagLib_File *>(new MPC::File(filename));
  case TagLib_File_OggFlac:
    return reinterpret_cast<TagLib_File *>(new Ogg::FLAC::File(filename));
  case TagLib_File_WavPack:
    return reinterpret_cast<TagLib_File *>(new WavPack::File(filename));
  case TagLib_File_Speex:
    return reinterpret_cast<TagLib_File *>(new Ogg::Speex::File(filename));
  case TagLib_File_TrueAudio:
    return reinterpret_cast<TagLib_File *>(new TrueAudio::File(filename));
  case TagLib_File_MP4:
    return reinterpret_cast<TagLib_File *>(new MP4::File(filename));
  case TagLib_File_ASF:
    return reinterpret_cast<TagLib_File *>(new ASF::File(filename));
  default:
    return 0;
  }
}

void taglib_file_free(TagLib_File *file)
{
  delete reinterpret_cast<File *>(file);
}

BOOL taglib_file_is_valid(const TagLib_File *file)
{
  return reinterpret_cast<const File *>(file)->isValid();
}

TagLib_Tag *taglib_file_tag(const TagLib_File *file)
{
  const File *f = reinterpret_cast<const File *>(file);
  return reinterpret_cast<TagLib_Tag *>(f->tag());
}

const TagLib_AudioProperties *taglib_file_audioproperties(const TagLib_File *file)
{
  const File *f = reinterpret_cast<const File *>(file);
  return reinterpret_cast<const TagLib_AudioProperties *>(f->audioProperties());
}

BOOL taglib_file_save(TagLib_File *file)
{
  return reinterpret_cast<File *>(file)->save();
}

////////////////////////////////////////////////////////////////////////////////
// TagLib::Tag wrapper
////////////////////////////////////////////////////////////////////////////////

char *taglib_tag_title(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->title());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_artist(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->artist());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_album(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->album());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_comment(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->comment());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

char *taglib_tag_genre(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  char *s = stringToCharArray(t->genre());
  if(stringManagementEnabled)
    strings.append(s);
  return s;
}

unsigned int taglib_tag_year(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  return t->year();
}

unsigned int taglib_tag_track(const TagLib_Tag *tag)
{
  const Tag *t = reinterpret_cast<const Tag *>(tag);
  return t->track();
}

void taglib_tag_set_title(TagLib_Tag *tag, const char *title)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setTitle(charArrayToString(title));
}

void taglib_tag_set_artist(TagLib_Tag *tag, const char *artist)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setArtist(charArrayToString(artist));
}

void taglib_tag_set_album(TagLib_Tag *tag, const char *album)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setAlbum(charArrayToString(album));
}

void taglib_tag_set_comment(TagLib_Tag *tag, const char *comment)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setComment(charArrayToString(comment));
}

void taglib_tag_set_genre(TagLib_Tag *tag, const char *genre)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setGenre(charArrayToString(genre));
}

void taglib_tag_set_year(TagLib_Tag *tag, unsigned int year)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setYear(year);
}

void taglib_tag_set_track(TagLib_Tag *tag, unsigned int track)
{
  Tag *t = reinterpret_cast<Tag *>(tag);
  t->setTrack(track);
}

void taglib_tag_free_strings()
{
  if(!stringManagementEnabled)
    return;

  for(List<char *>::ConstIterator it = strings.begin(); it != strings.end(); ++it)
    free(*it);
  strings.clear();
}

////////////////////////////////////////////////////////////////////////////////
// TagLib::AudioProperties wrapper
////////////////////////////////////////////////////////////////////////////////

int taglib_audioproperties_length(const TagLib_AudioProperties *audioProperties)
{
  const AudioProperties *p = reinterpret_cast<const AudioProperties *>(audioProperties);
  return p->length();
}

int taglib_audioproperties_bitrate(const TagLib_AudioProperties *audioProperties)
{
  const AudioProperties *p = reinterpret_cast<const AudioProperties *>(audioProperties);
  return p->bitrate();
}

int taglib_audioproperties_samplerate(const TagLib_AudioProperties *audioProperties)
{
  const AudioProperties *p = reinterpret_cast<const AudioProperties *>(audioProperties);
  return p->sampleRate();
}

int taglib_audioproperties_channels(const TagLib_AudioProperties *audioProperties)
{
  const AudioProperties *p = reinterpret_cast<const AudioProperties *>(audioProperties);
  return p->channels();
}

void taglib_id3v2_set_default_text_encoding(TagLib_ID3v2_Encoding encoding)
{
  String::Type type = String::Latin1;

  switch(encoding)
  {
  case TagLib_ID3v2_Latin1:
    type = String::Latin1;
    break;
  case TagLib_ID3v2_UTF16:
    type = String::UTF16;
    break;
  case TagLib_ID3v2_UTF16BE:
    type = String::UTF16BE;
    break;
  case TagLib_ID3v2_UTF8:
    type = String::UTF8;
    break;
  }

  ID3v2::FrameFactory::instance()->setDefaultTextEncoding(type);
}
