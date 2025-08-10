#include <memory>
#include "matroskaattachedfile.h"
#include "tstring.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::AttachedFile::AttachedFilePrivate
{
public:
  AttachedFilePrivate() = default;
  ~AttachedFilePrivate() = default;
  AttachedFilePrivate(const AttachedFilePrivate &) = delete;
  AttachedFilePrivate &operator=(const AttachedFilePrivate &) = delete;
  String fileName;
  String description;
  String mediaType;
  ByteVector data;
  UID uid = 0;
};

Matroska::AttachedFile::AttachedFile() :
  d(std::make_unique<AttachedFilePrivate>())
{
}
Matroska::AttachedFile::~AttachedFile() = default;

void Matroska::AttachedFile::setFileName(const String &fileName)
{
  d->fileName = fileName;
}

const String &Matroska::AttachedFile::fileName() const
{
  return d->fileName;
}

void Matroska::AttachedFile::setDescription(const String &description)
{
  d->description = description;
}

const String &Matroska::AttachedFile::description() const
{
  return d->description;
}

void Matroska::AttachedFile::setMediaType(const String &mediaType)
{
  d->mediaType = mediaType;
}

const String &Matroska::AttachedFile::mediaType() const
{
  return d->mediaType;
}

void Matroska::AttachedFile::setData(const ByteVector &data)
{
  d->data = data;
}

const ByteVector &Matroska::AttachedFile::data() const
{
  return d->data;
}

void Matroska::AttachedFile::setUID(UID uid)
{
  d->uid = uid;
}

Matroska::AttachedFile::UID Matroska::AttachedFile::uid() const
{
  return d->uid;
}
