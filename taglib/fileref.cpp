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

#include "fileref.h"

#include <cstring>
#include <utility>

#include "tfilestream.h"
#include "tpropertymap.h"
#include "tstringlist.h"
#include "tvariant.h"
#include "tdebug.h"
#include "aifffile.h"
#include "apefile.h"
#include "asffile.h"
#include "flacfile.h"
#include "itfile.h"
#include "modfile.h"
#include "mp4file.h"
#include "mpcfile.h"
#include "mpegfile.h"
#include "oggflacfile.h"
#include "opusfile.h"
#include "s3mfile.h"
#include "speexfile.h"
#include "trueaudiofile.h"
#include "vorbisfile.h"
#include "wavfile.h"
#include "wavpackfile.h"
#include "xmfile.h"
#include "dsffile.h"
#include "dsdifffile.h"
#include "shortenfile.h"

using namespace TagLib;

class FileRef::FileTypeResolver::FileTypeResolverPrivate
{
};

class FileRef::StreamTypeResolver::StreamTypeResolverPrivate
{
};

namespace
{
  List<const FileRef::FileTypeResolver *> fileTypeResolvers;

  // Detect the file type by user-defined resolvers.

  File *detectByResolvers(FileName fileName, bool readAudioProperties,
                          AudioProperties::ReadStyle audioPropertiesStyle)
  {
#ifdef _WIN32
    if(::wcslen(fileName) == 0)
      return nullptr;
#else
    if(::strlen(fileName) == 0)
      return nullptr;
#endif
    for(const auto &resolver : std::as_const(fileTypeResolvers)) {
      File *file = resolver->createFile(fileName, readAudioProperties, audioPropertiesStyle);
      if(file)
        return file;
    }

    return nullptr;
  }

  File *detectByResolvers(IOStream* stream, bool readAudioProperties,
                          AudioProperties::ReadStyle audioPropertiesStyle)
  {
    for(const auto &resolver : std::as_const(fileTypeResolvers)) {
      if(auto streamResolver = dynamic_cast<const FileRef::StreamTypeResolver *>(resolver)) {
        if(File *file = streamResolver->createFileFromStream(
             stream, readAudioProperties, audioPropertiesStyle))
          return file;
      }
    }

    return nullptr;
  }

  // Detect the file type based on the file extension.

  File* detectByExtension(IOStream *stream, bool readAudioProperties,
                          AudioProperties::ReadStyle audioPropertiesStyle)
  {
#ifdef _WIN32
    const String s = stream->name().toString();
#else
    const String s(stream->name());
#endif

    String ext;
    if(const int pos = s.rfind("."); pos != -1)
      ext = s.substr(pos + 1).upper();

    // If this list is updated, the method defaultFileExtensions() should also be
    // updated.  However at some point that list should be created at the same time
    // that a default file type resolver is created.

    if(ext.isEmpty())
      return nullptr;

    // .oga can be any audio in the Ogg container. So leave it to content-based detection.

    File *file = nullptr;

    if(ext == "MP3" || ext == "MP2" || ext == "AAC")
      file = new MPEG::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "OGG")
      file = new Ogg::Vorbis::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "OGA") {
      /* .oga can be any audio in the Ogg container. First try FLAC, then Vorbis. */
      file = new Ogg::FLAC::File(stream, readAudioProperties, audioPropertiesStyle);
      if(!file->isValid()) {
        delete file;
        file = new Ogg::Vorbis::File(stream, readAudioProperties, audioPropertiesStyle);
      }
    }
    else if(ext == "FLAC")
      file = new FLAC::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "MPC")
      file = new MPC::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "WV")
      file = new WavPack::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "SPX")
      file = new Ogg::Speex::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "OPUS")
      file = new Ogg::Opus::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "TTA")
      file = new TrueAudio::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "M4A" || ext == "M4R" || ext == "M4B" || ext == "M4P" || ext == "MP4" || ext == "3G2" || ext == "M4V")
      file = new MP4::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "WMA" || ext == "ASF")
      file = new ASF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "AIF" || ext == "AIFF" || ext == "AFC" || ext == "AIFC")
      file = new RIFF::AIFF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "WAV")
      file = new RIFF::WAV::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "APE")
      file = new APE::File(stream, readAudioProperties, audioPropertiesStyle);
    // module, nst and wow are possible but uncommon extensions
    else if(ext == "MOD" || ext == "MODULE" || ext == "NST" || ext == "WOW")
      file = new Mod::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "S3M")
      file = new S3M::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "IT")
      file = new IT::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "XM")
      file = new XM::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "DSF")
      file = new DSF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "DFF" || ext == "DSDIFF")
      file = new DSDIFF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(ext == "SHN")
      file = new Shorten::File(stream, readAudioProperties, audioPropertiesStyle);

    // if file is not valid, leave it to content-based detection.

    if(file) {
      if(file->isValid())
        return file;
      delete file;
    }

    return nullptr;
  }

  // Detect the file type based on the actual content of the stream.

  File *detectByContent(IOStream *stream, bool readAudioProperties,
                        AudioProperties::ReadStyle audioPropertiesStyle)
  {
    File *file = nullptr;

    if(MPEG::File::isSupported(stream))
      file = new MPEG::File(stream, readAudioProperties, audioPropertiesStyle);
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
    else if(DSF::File::isSupported(stream))
      file = new DSF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(DSDIFF::File::isSupported(stream))
      file = new DSDIFF::File(stream, readAudioProperties, audioPropertiesStyle);
    else if(Shorten::File::isSupported(stream))
      file = new Shorten::File(stream, readAudioProperties, audioPropertiesStyle);

    // isSupported() only does a quick check, so double check the file here.

    if(file) {
      if(file->isValid())
        return file;
      delete file;
    }

    return nullptr;
  }

}  // namespace

class FileRef::FileRefPrivate
{
public:
  FileRefPrivate() = default;
  ~FileRefPrivate()
  {
    delete file;
    delete stream;
  }

  FileRefPrivate(const FileRefPrivate &) = delete;
  FileRefPrivate &operator=(const FileRefPrivate &) = delete;

  bool isNull() const
  {
    return !file || !file->isValid();
  }

  bool isNullWithDebugMessage(const String &methodName) const
  {
    if(isNull()) {
      debug("FileRef::" + methodName + "() - Called without a valid file.");
      return true;
    }
    return false;
  }

  File *file { nullptr };
  IOStream *stream { nullptr };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FileRef::FileRef() :
  d(std::make_shared<FileRefPrivate>())
{
}

FileRef::FileRef(FileName fileName, bool readAudioProperties,
                 AudioProperties::ReadStyle audioPropertiesStyle) :
  d(std::make_shared<FileRefPrivate>())
{
  parse(fileName, readAudioProperties, audioPropertiesStyle);
}

FileRef::FileRef(IOStream *stream, bool readAudioProperties, AudioProperties::ReadStyle audioPropertiesStyle) :
  d(std::make_shared<FileRefPrivate>())
{
  parse(stream, readAudioProperties, audioPropertiesStyle);
}

FileRef::FileRef(File *file) :
  d(std::make_shared<FileRefPrivate>())
{
  d->file = file;
}

FileRef::FileRef(const FileRef &) = default;

FileRef::~FileRef() = default;

Tag *FileRef::tag() const
{
  if(d->isNullWithDebugMessage(__func__)) {
    return nullptr;
  }
  return d->file->tag();
}

PropertyMap FileRef::properties() const
{
  if(d->isNullWithDebugMessage(__func__)) {
    return PropertyMap();
  }
  return d->file->properties();
}

void FileRef::removeUnsupportedProperties(const StringList& properties)
{
  if(d->isNullWithDebugMessage(__func__)) {
    return;
  }
  return d->file->removeUnsupportedProperties(properties);
}

PropertyMap FileRef::setProperties(const PropertyMap &properties)
{
  if(d->isNullWithDebugMessage(__func__)) {
    return PropertyMap();
  }
  return d->file->setProperties(properties);
}

StringList FileRef::complexPropertyKeys() const
{
  if(d->isNullWithDebugMessage(__func__)) {
    return StringList();
  }
  return d->file->complexPropertyKeys();
}

List<VariantMap> FileRef::complexProperties(const String &key) const
{
  if(d->isNullWithDebugMessage(__func__)) {
    return List<VariantMap>();
  }
  return d->file->complexProperties(key);
}

bool FileRef::setComplexProperties(const String &key, const List<VariantMap> &value)
{
  if(d->isNullWithDebugMessage(__func__)) {
    return false;
  }
  return d->file->setComplexProperties(key, value);
}

AudioProperties *FileRef::audioProperties() const
{
  if(d->isNullWithDebugMessage(__func__)) {
    return nullptr;
  }
  return d->file->audioProperties();
}

File *FileRef::file() const
{
  return d->file;
}

bool FileRef::save()
{
  if(d->isNullWithDebugMessage(__func__)) {
    return false;
  }
  return d->file->save();
}

const FileRef::FileTypeResolver *FileRef::addFileTypeResolver(const FileRef::FileTypeResolver *resolver) // static
{
  fileTypeResolvers.prepend(resolver);
  return resolver;
}

void FileRef::clearFileTypeResolvers() // static
{
  fileTypeResolvers.clear();
}

StringList FileRef::defaultFileExtensions()
{
  StringList l;

  l.append("ogg");
  l.append("flac");
  l.append("oga");
  l.append("opus");
  l.append("mp3");
  l.append("mp2");
  l.append("mpc");
  l.append("wv");
  l.append("spx");
  l.append("tta");
  l.append("aac");
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
  l.append("afc");
  l.append("aifc");
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
  l.append("shn");

  return l;
}

bool FileRef::isNull() const
{
  return d->isNull();
}

FileRef &FileRef::operator=(const FileRef &) = default;

void FileRef::swap(FileRef &ref) noexcept
{
  using std::swap;

  swap(d, ref.d);
}

bool FileRef::operator==(const FileRef &ref) const
{
  return ref.d->file == d->file;
}

bool FileRef::operator!=(const FileRef &ref) const
{
  return ref.d->file != d->file;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void FileRef::parse(FileName fileName, bool readAudioProperties,
                    AudioProperties::ReadStyle audioPropertiesStyle)
{
  // Try user-defined resolvers.

  d->file = detectByResolvers(fileName, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // Try to resolve file types based on the file extension.

  d->stream = new FileStream(fileName);
  d->file = detectByExtension(d->stream, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // At last, try to resolve file types based on the actual content.

  d->file = detectByContent(d->stream, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // Stream have to be closed here if failed to resolve file types.

  delete d->stream;
  d->stream = nullptr;
}

void FileRef::parse(IOStream *stream, bool readAudioProperties,
                    AudioProperties::ReadStyle audioPropertiesStyle)
{
  // Try user-defined stream resolvers.

  d->file = detectByResolvers(stream, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // Try user-defined resolvers.

  d->file = detectByResolvers(stream->name(), readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // Try to resolve file types based on the file extension.

  d->file = detectByExtension(stream, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // At last, try to resolve file types based on the actual content of the file.

  d->file = detectByContent(stream, readAudioProperties, audioPropertiesStyle);
}

FileRef::FileTypeResolver::FileTypeResolver() = default;
FileRef::FileTypeResolver::~FileTypeResolver() = default;

FileRef::StreamTypeResolver::StreamTypeResolver() = default;
FileRef::StreamTypeResolver::~StreamTypeResolver() = default;
