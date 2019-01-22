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

#include <tfile.h>
#include <tfilestream.h>
#include <tstring.h>
#include <tdebug.h>
#include <tsmartptr.h>

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
#include "dsffile.h"
#include "dsdifffile.h"
#include "ebmlmatroskafile.h"

using namespace TagLib;

namespace
{
  typedef List<const FileRef::FileTypeResolver *> ResolverList;
  ResolverList fileTypeResolvers;

  // Detect the file type by user-defined resolvers.

  File *detectByResolvers(FileName fileName, bool readAudioProperties,
                          AudioProperties::ReadStyle audioPropertiesStyle)
  {
    ResolverList::ConstIterator it = fileTypeResolvers.begin();
    for(; it != fileTypeResolvers.end(); ++it) {
      File *file = (*it)->createFile(fileName, readAudioProperties, audioPropertiesStyle);
      if(file)
        return file;
    }

    return 0;
  }

  // Detect the file type based on the file extension.

  File* detectByExtension(IOStream *stream, bool readAudioProperties,
                          AudioProperties::ReadStyle audioPropertiesStyle)
  {
#ifdef _WIN32
    const String s(stream->name().wstr());
#else
    const String s(stream->name());
#endif

    String ext;
    const size_t pos = s.rfind(".");
    if(pos != String::npos())
      ext = s.substr(pos + 1).upper();

    // If this list is updated, the method defaultFileExtensions() should also be
    // updated.  However at some point that list should be created at the same time
    // that a default file type resolver is created.

    if(ext.isEmpty())
      return 0;

    // .oga can be any audio in the Ogg container. So leave it to content-based detection.

    if(ext == "MP3")
      return new MPEG::File(stream, ID3v2::FrameFactory::instance(), readAudioProperties, audioPropertiesStyle);
    if(ext == "OGG")
      return new Ogg::Vorbis::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "FLAC")
      return new FLAC::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "MPC")
      return new MPC::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "WV")
      return new WavPack::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "SPX")
      return new Ogg::Speex::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "OPUS")
      return new Ogg::Opus::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "TTA")
      return new TrueAudio::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "M4A" || ext == "M4R" || ext == "M4B" || ext == "M4P" || ext == "MP4" || ext == "3G2" || ext == "M4V")
      return new MP4::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "WMA" || ext == "ASF")
      return new ASF::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "AIF" || ext == "AIFF" || ext == "AFC" || ext == "AIFC")
      return new RIFF::AIFF::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "WAV")
      return new RIFF::WAV::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "APE")
      return new APE::File(stream, readAudioProperties, audioPropertiesStyle);
    // module, nst and wow are possible but uncommon extensions
    if(ext == "MOD" || ext == "MODULE" || ext == "NST" || ext == "WOW")
      return new Mod::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "S3M")
      return new S3M::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "IT")
      return new IT::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "XM")
      return new XM::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "DFF" || ext == "DSDIFF")
      return new DSDIFF::File(stream, readAudioProperties, audioPropertiesStyle);
    if(ext == "DSF")
      return new DSF::File(stream, readAudioProperties, audioPropertiesStyle);
    if (ext == "MKA" || ext == "MKV") {
      return new EBML::Matroska::File(stream, readAudioProperties, audioPropertiesStyle);
    }

    return 0;
  }

  // Detect the file type based on the actual content of the stream.

  File *detectByContent(IOStream *stream, bool readAudioProperties,
                        AudioProperties::ReadStyle audioPropertiesStyle)
  {
    File *file = 0;

    if(MPEG::File::isSupported(stream))
      file = new MPEG::File(stream, ID3v2::FrameFactory::instance(), readAudioProperties, audioPropertiesStyle);
    else if(Ogg::Vorbis::File::isSupported(stream))
      file = new Ogg::Vorbis::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(Ogg::FLAC::File::isSupported(stream))
      file = new Ogg::FLAC::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(FLAC::File::isSupported(stream))
      file = new FLAC::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(MPC::File::isSupported(stream))
      file = new MPC::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(WavPack::File::isSupported(stream))
      file = new WavPack::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(Ogg::Speex::File::isSupported(stream))
      file = new Ogg::Speex::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(Ogg::Opus::File::isSupported(stream))
      file = new Ogg::Opus::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(TrueAudio::File::isSupported(stream))
      file = new TrueAudio::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(MP4::File::isSupported(stream))
      file = new MP4::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ASF::File::isSupported(stream))
      file = new ASF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(RIFF::AIFF::File::isSupported(stream))
      file = new RIFF::AIFF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(RIFF::WAV::File::isSupported(stream))
      file = new RIFF::WAV::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(APE::File::isSupported(stream))
      file = new APE::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(DSDIFF::File::isSupported(stream))
      file = new DSDIFF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(DSF::File::isSupported(stream))
      file = new DSF::File(stream, readAudioProperties, audioPropertiesStyle);

    // isSupported() only does a quick check, so double check the file here.

    if(file) {
      if(file->isValid())
        return file;
      else
        delete file;
    }

    return 0;
  }

  struct FileRefData
  {
    FileRefData() :
      file(0),
      stream(0) {}

    ~FileRefData() {
      delete file;
      delete stream;
    }

    File     *file;
    IOStream *stream;
  };
}

class FileRef::FileRefPrivate
{
public:
  FileRefPrivate() :
    data(new FileRefData()) {}

  SHARED_PTR<FileRefData> data;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FileRef::FileRef() :
  d(new FileRefPrivate())
{
}

FileRef::FileRef(FileName fileName, bool readAudioProperties,
                 AudioProperties::ReadStyle audioPropertiesStyle) :
  d(new FileRefPrivate())
{
  parse(fileName, readAudioProperties, audioPropertiesStyle);
}

FileRef::FileRef(IOStream* stream, bool readAudioProperties, AudioProperties::ReadStyle audioPropertiesStyle) :
  d(new FileRefPrivate())
{
  parse(stream, readAudioProperties, audioPropertiesStyle);
}

FileRef::FileRef(File *file) :
  d(new FileRefPrivate())
{
  d->data->file = file;
}

FileRef::FileRef(const FileRef &ref) :
  d(new FileRefPrivate(*ref.d))
{
}

FileRef::~FileRef()
{
  delete d;
}

Tag *FileRef::tag() const
{
  if(isNull()) {
    debug("FileRef::tag() - Called without a valid file.");
    return 0;
  }
  return d->data->file->tag();
}

PropertyMap FileRef::properties() const
{
  if(isNull()) {
    debug("FileRef::properties() - Called without a valid file.");
    return PropertyMap();
  }

  return d->data->file->properties();
}

void FileRef::removeUnsupportedProperties(const StringList& properties)
{
  if(isNull()) {
    debug("FileRef::removeUnsupportedProperties() - Called without a valid file.");
    return;
  }

  d->data->file->removeUnsupportedProperties(properties);
}

PropertyMap FileRef::setProperties(const PropertyMap &properties)
{
  if(isNull()) {
    debug("FileRef::setProperties() - Called without a valid file.");
    return PropertyMap();
  }

  return d->data->file->setProperties(properties);
}

AudioProperties *FileRef::audioProperties() const
{
  if(isNull()) {
    debug("FileRef::audioProperties() - Called without a valid file.");
    return 0;
  }
  return d->data->file->audioProperties();
}

File *FileRef::file() const
{
  return d->data->file;
}

bool FileRef::save()
{
  if(isNull()) {
    debug("FileRef::save() - Called without a valid file.");
    return false;
  }
  return d->data->file->save();
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
  l.append("m4v");
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
  l.append("dsf");
  l.append("dff");
  l.append("dsdiff"); // alias for "dff"
  l.append("mka");
  l.append("mkv");

  return l;
}

bool FileRef::isValid() const
{
  return (d->data->file && d->data->file->isValid());
}

bool FileRef::isNull() const
{
  return !isValid();
}

FileRef &FileRef::operator=(const FileRef &ref)
{
  FileRef(ref).swap(*this);
  return *this;
}

void FileRef::swap(FileRef &ref)
{
  using std::swap;

  swap(d, ref.d);
}

bool FileRef::operator==(const FileRef &ref) const
{
  return (ref.d->data == d->data);
}

bool FileRef::operator!=(const FileRef &ref) const
{
  return (ref.d->data != d->data);
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void FileRef::parse(FileName fileName, bool readAudioProperties,
                    AudioProperties::ReadStyle audioPropertiesStyle)
{
  // Try user-defined resolvers.

  d->data->file = detectByResolvers(fileName, readAudioProperties, audioPropertiesStyle);
  if(d->data->file)
    return;

  // Try to resolve file types based on the file extension.

  d->data->stream = new FileStream(fileName);
  d->data->file = detectByExtension(d->data->stream, readAudioProperties, audioPropertiesStyle);
  if(d->data->file)
    return;

  // At last, try to resolve file types based on the actual content.

  d->data->file = detectByContent(d->data->stream, readAudioProperties, audioPropertiesStyle);
  if(d->data->file)
    return;

  // Stream have to be closed here if failed to resolve file types.

  delete d->data->stream;
  d->data->stream = 0;
}

void FileRef::parse(IOStream *stream, bool readAudioProperties,
                    AudioProperties::ReadStyle audioPropertiesStyle)
{
  // User-defined resolvers won't work with a stream.

  // Try to resolve file types based on the file extension.

  d->data->file = detectByExtension(stream, readAudioProperties, audioPropertiesStyle);
  if(d->data->file)
    return;

  // At last, try to resolve file types based on the actual content of the file.

  d->data->file = detectByContent(stream, readAudioProperties, audioPropertiesStyle);
}
