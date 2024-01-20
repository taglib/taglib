/***************************************************************************
    copyright            : (C) 2015 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
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

#include "tagutils.h"

#include "tfile.h"

#include "id3v1tag.h"
#include "id3v2header.h"
#include "apetag.h"

using namespace TagLib;

offset_t Utils::findID3v1(File *file)
{
  if(!file->isValid())
    return -1;

  // Differentiate between a match of APEv2 magic and a match of ID3v1 magic.

  if (file->length() >= 131) {
    file->seek(-131, File::End);
    const offset_t p = file->tell() + 3;

    if(const TagLib::ByteVector data = file->readBlock(8);
       data.containsAt(ID3v1::Tag::fileIdentifier(), 3) && data != APE::Tag::fileIdentifier())
      return p;
  } else {
    file->seek(-128, File::End);
    const offset_t p = file->tell();

    if(file->readBlock(3) == ID3v1::Tag::fileIdentifier())
      return p;
  }

  return -1;
}

offset_t Utils::findID3v2(File *file)
{
  if(!file->isValid())
    return -1;

  file->seek(0);

  if(file->readBlock(3) == ID3v2::Header::fileIdentifier())
    return 0;

  return -1;
}

offset_t Utils::findAPE(File *file, offset_t id3v1Location)
{
  if(!file->isValid())
    return -1;

  if(id3v1Location >= 0)
    file->seek(id3v1Location - 32, File::Beginning);
  else
    file->seek(-32, File::End);

  const offset_t p = file->tell();

  if(file->readBlock(8) == APE::Tag::fileIdentifier())
    return p;

  return -1;
}

ByteVector TagLib::Utils::readHeader(IOStream *stream, unsigned int length,
                                     bool skipID3v2, offset_t *headerOffset)
{
  if(!stream || !stream->isOpen())
    return ByteVector();

  const offset_t originalPosition = stream->tell();
  offset_t bufferOffset = 0;

  if(skipID3v2) {
    stream->seek(0);
    if(const ByteVector data = stream->readBlock(ID3v2::Header::size());
       data.startsWith(ID3v2::Header::fileIdentifier()))
      bufferOffset = ID3v2::Header(data).completeTagSize();
  }

  stream->seek(bufferOffset);
  const ByteVector header = stream->readBlock(length);
  stream->seek(originalPosition);

  if(headerOffset)
    *headerOffset = bufferOffset;

  return header;
}
