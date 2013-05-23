/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org

    copyright            : (C) 2010 by Alex Novichkov
    email                : novichko@atnet.ru
                           (added APE file support)
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

#include "taglib_config.h"

#ifdef _WIN32
# include <Shlwapi.h>
#endif 

#include <tfile.h>
#include <tstring.h>
#include <tdebug.h>

#include "fileref.h"
#include "asffile.h"
#include "mpegfile.h"
#include "vorbisfile.h"
#include "flacfile.h"
#include "oggflacfile.h"
#include "mpcfile.h"
#include "mp4file.h"
#include "wavpackfile.h"
#include "speexfile.h"
#include "opusfile.h"
#include "trueaudiofile.h"
#include "aifffile.h"
#include "wavfile.h"
#include "apefile.h"
#include "modfile.h"
#include "s3mfile.h"
#include "itfile.h"
#include "xmfile.h"
#include "ebmlmatroskafile.h"

using namespace TagLib;

namespace
{
  typedef List<const FileRef::FileTypeResolver *> ResolverList;
  typedef ResolverList::ConstIterator ResolverConstIterator;

  ResolverList fileTypeResolvers;

  RefCountPtr<File> create(
    FileName fileName, 
    bool readAudioProperties,
    AudioProperties::ReadStyle audioPropertiesStyle)
  {
    RefCountPtr<File> file;
    for(ResolverConstIterator it = fileTypeResolvers.begin(); it != fileTypeResolvers.end(); ++it) 
    {
      file.reset((*it)->createFile(fileName, readAudioProperties, audioPropertiesStyle));
      if(file)
        return file;
    }

    String ext;

#ifdef _WIN32
    // Avoids direct conversion from FileName to String
    // because String can't decode strings in local encodings properly.

    if(!fileName.wstr().empty()) {
      const wchar_t *pext = PathFindExtensionW(fileName.wstr().c_str());
      if(*pext == L'.')
        ext = String(pext + 1).upper();
    }
    else {
      const char *pext = PathFindExtensionA(fileName.str().c_str());
      if(*pext == '.')
        ext = String(pext + 1).upper();
    }
#else
    {
      String s = fileName;
      const size_t pos = s.rfind(".");
      if(pos != String::npos)
        ext = s.substr(pos + 1).upper();
    }
#endif

    // If this list is updated, the method defaultFileExtensions() should also be
    // updated.  However at some point that list should be created at the same time
    // that a default file type resolver is created.

    if(!ext.isEmpty()) {
      if(ext == "MP3")
        file.reset(new MPEG::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "OGG")
        file.reset(new Ogg::Vorbis::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "OGA") {
        /* .oga can be any audio in the Ogg container. First try FLAC, then Vorbis. */
        file.reset(new Ogg::FLAC::File(fileName, readAudioProperties, audioPropertiesStyle));
        if(!file->isValid())
          file.reset(new Ogg::Vorbis::File(fileName, readAudioProperties, audioPropertiesStyle));
      }
      else if(ext == "FLAC")
        file.reset(new FLAC::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "MPC")
        file.reset(new MPC::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "WV")
        file.reset(new WavPack::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "SPX")
        file.reset(new Ogg::Speex::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "OPUS")
        file.reset(new Ogg::Opus::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "TTA")
        file.reset(new TrueAudio::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "M4A" || ext == "M4R" || ext == "M4B" || ext == "M4P" || ext == "MP4" || ext == "3G2")
        file.reset(new MP4::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "WMA" || ext == "ASF")
        file.reset(new ASF::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "AIF" || ext == "AIFF")
        file.reset(new RIFF::AIFF::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "WAV")
        file.reset(new RIFF::WAV::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "APE")
        file.reset(new APE::File(fileName, readAudioProperties, audioPropertiesStyle));
      // module, nst and wow are possible but uncommon extensions
      else if(ext == "MOD" || ext == "MODULE" || ext == "NST" || ext == "WOW")
        file.reset(new Mod::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "S3M")
        file.reset(new S3M::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "IT")
        file.reset(new IT::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "XM")
        file.reset(new XM::File(fileName, readAudioProperties, audioPropertiesStyle));
      else if(ext == "MKA")
        file.reset(new EBML::Matroska::File(fileName, readAudioProperties, audioPropertiesStyle));
    }

    return file;
  }
}

class FileRef::FileRefPrivate 
{
public:
  FileRefPrivate()
    : file()
  {
  }

  FileRefPrivate(File *f) 
    : file(f) 
  {
  }
  
  FileRefPrivate(RefCountPtr<File> f) 
    : file(f) 
  {
  }

  RefCountPtr<File> file;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FileRef::FileRef()
  : d(new FileRefPrivate())
{
}

FileRef::FileRef(FileName fileName, 
                 bool readAudioProperties, AudioProperties::ReadStyle audioPropertiesStyle)
  : d(new FileRefPrivate(create(fileName, readAudioProperties, audioPropertiesStyle)))
{
}

FileRef::FileRef(File *file)
  : d(new FileRefPrivate(file))
{
}

FileRef::FileRef(const FileRef &ref) 
  : d(ref.d)
{
}

#ifdef TAGLIB_USE_MOVE_SEMANTICS

FileRef::FileRef(FileRef &&ref) 
  : d(std::move(ref.d))
{
}

#endif

FileRef::~FileRef()
{
}

Tag *FileRef::tag() const
{
  if(isNull()) {
    debug("FileRef::tag() - Called without a valid file.");
    return 0;
  }
  return d->file->tag();
}

PropertyMap FileRef::properties() const
{
  if(isNull()) {
    debug("FileRef::properties() - Called without a valid file.");
    return PropertyMap();
  }

  return d->file->properties();
}

void FileRef::removeUnsupportedProperties(const StringList& properties)
{
  if(isNull()) {
    debug("FileRef::removeUnsupportedProperties() - Called without a valid file.");
    return;
  }

  d->file->removeUnsupportedProperties(properties);
}

PropertyMap FileRef::setProperties(const PropertyMap &properties)
{
  if(isNull()) {
    debug("FileRef::setProperties() - Called without a valid file.");
    return PropertyMap();
  }

  return d->file->setProperties(properties);
}

AudioProperties *FileRef::audioProperties() const
{
  if(isNull()) {
    debug("FileRef::audioProperties() - Called without a valid file.");
    return 0;
  }
  return d->file->audioProperties();
}

File *FileRef::file() const
{
  return d->file.get();
}

bool FileRef::save()
{
  if(isNull()) {
    debug("FileRef::save() - Called without a valid file.");
    return false;
  }
  return d->file->save();
}

const FileRef::FileTypeResolver *FileRef::addFileTypeResolver(const FileRef::FileTypeResolver *resolver) // static
{
  fileTypeResolvers.prepend(resolver);
  return resolver;
}

StringList FileRef::defaultFileExtensions()
{
  StringList l;

  l.append("ogg");
  l.append("flac");
  l.append("oga");
  l.append("mp3");
  l.append("mpc");
  l.append("wv");
  l.append("spx");
  l.append("tta");
  l.append("m4a");
  l.append("m4r");
  l.append("m4b");
  l.append("m4p");
  l.append("3g2");
  l.append("mp4");
  l.append("wma");
  l.append("asf");
  l.append("aif");
  l.append("aiff");
  l.append("wav");
  l.append("ape");
  l.append("mod");
  l.append("module"); // alias for "mod"
  l.append("nst"); // alias for "mod"
  l.append("wow"); // alias for "mod"
  l.append("s3m");
  l.append("it");
  l.append("xm");

  return l;
}

bool FileRef::isValid() const
{
  return (d->file && d->file->isValid());
}

bool FileRef::isNull() const
{
  return !isValid();
}

FileRef &FileRef::operator=(const FileRef &ref)
{
  d = ref.d;
  return *this;
}

#ifdef TAGLIB_USE_MOVE_SEMANTICS

FileRef &FileRef::operator=(FileRef &&ref)
{
  d = std::move(ref.d);
  return *this;
}

#endif

bool FileRef::operator==(const FileRef &ref) const
{
  return ref.d->file == d->file;
}

bool FileRef::operator!=(const FileRef &ref) const
{
  return ref.d->file != d->file;
}
