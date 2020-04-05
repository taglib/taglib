#pragma once
#include <tstring.h>
#include <tpicturemap.h>

#include "flacpicture.h"
#include "flacfile.h"
namespace TagLib{
  /*
  * A Dummy tag just to provide flac pictures
  * All Other Methods take no effects
  */
  namespace FLAC {
    class FlacPictureTag : public TagLib::Tag
    {

      File* file;
    public:
      FlacPictureTag(File* file) :Tag(),file(file) {}
      virtual String title() const ;
      virtual String artist() const;
      virtual String album() const;
      virtual String comment() const;
      virtual String genre() const;
      virtual unsigned int year() const;
      virtual unsigned int track() const;
      virtual PictureMap pictures() const;
      virtual void setTitle(const String& s);
      virtual void setArtist(const String& s);
      virtual void setAlbum(const String& s);
      virtual void setComment(const String& s);
      virtual void setGenre(const String& s);
      virtual void setYear(unsigned int i);
      virtual void setTrack(unsigned int i);

      /*!
       *  Sets the list of pictures
       */
      virtual void setPictures(const PictureMap& l);
    };
  }
}

