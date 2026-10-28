/***************************************************************************
    copyright            : (C) 2026 by Ryan Francesconi
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

#include "tdeferredwritestream.h"

#include <algorithm>
#include <vector>

#include "tdebug.h"
#include "tstring.h"

using namespace TagLib;

namespace
{
  // The block size moved bytes are copied in when committing.
  constexpr offset_t moveBlockSize = 1024 * 1024;

  // A run of the content: either bytes of the underlying stream starting at
  // source, or bytes held in data.
  struct Piece
  {
    bool isOriginal;
    offset_t source;
    ByteVector data;
    offset_t size;
  };
}  // namespace

class DeferredWriteStream::DeferredWriteStreamPrivate
{
public:
  DeferredWriteStreamPrivate(IOStream *stream) :
    stream(stream)
  {
  }

  IOStream *stream;
  // Pieces in content order; starts[i] is where pieces[i] begins.
  std::vector<Piece> pieces;
  std::vector<offset_t> starts;
  offset_t position { 0 };
  bool changed { false };

  void reset()
  {
    pieces.clear();
    starts.clear();
    const offset_t length = stream->length();
    if(length > 0) {
      pieces.push_back({ true, 0, ByteVector(), length });
      starts.push_back(0);
    }
    changed = false;
  }

  offset_t contentLength() const
  {
    return pieces.empty() ? 0 : starts.back() + pieces.back().size;
  }

  void updateStarts(size_t from)
  {
    starts.resize(pieces.size());
    for(size_t i = from; i < pieces.size(); ++i)
      starts[i] = i == 0 ? 0 : starts[i - 1] + pieces[i - 1].size;
  }

  // Ensures a piece boundary at offset, which must not exceed the content
  // length; returns the index of the piece that starts there.
  size_t split(offset_t offset)
  {
    const auto it = std::upper_bound(starts.begin(), starts.end(), offset);
    if(it == starts.begin())
      return 0;

    const size_t index = static_cast<size_t>(it - starts.begin()) - 1;
    const offset_t inner = offset - starts[index];
    if(inner == 0)
      return index;
    if(inner >= pieces[index].size)
      return index + 1;

    Piece &piece = pieces[index];
    Piece tail { piece.isOriginal, piece.source + inner, ByteVector(), piece.size - inner };
    if(!piece.isOriginal) {
      tail.data = piece.data.mid(static_cast<unsigned int>(inner));
      piece.data.resize(static_cast<unsigned int>(inner));
    }
    piece.size = inner;

    pieces.insert(pieces.begin() + static_cast<std::ptrdiff_t>(index) + 1, tail);
    updateStarts(index + 1);
    return index + 1;
  }

  // Replaces the content in [start, start + length) with data; start may be
  // past the end, which is filled with zero bytes first.
  void replace(offset_t start, offset_t length, const ByteVector &data)
  {
    const offset_t total = contentLength();
    if(start > total) {
      replace(total, 0, ByteVector(static_cast<unsigned int>(start - total), '\0'));
    }

    const offset_t end = std::min(start + length, contentLength());
    const size_t first = split(start);
    const size_t last = split(end);
    pieces.erase(pieces.begin() + static_cast<std::ptrdiff_t>(first),
                 pieces.begin() + static_cast<std::ptrdiff_t>(last));

    if(!data.isEmpty()) {
      pieces.insert(pieces.begin() + static_cast<std::ptrdiff_t>(first),
                    { false, 0, data, static_cast<offset_t>(data.size()) });
    }

    updateStarts(first);
    merge(first);
    changed = true;
  }

  // Joins in-memory pieces around index, so repeated small writes do not
  // fragment the table.
  void merge(size_t index)
  {
    size_t from = index > 0 ? index - 1 : 0;
    while(from + 1 < pieces.size() && from <= index + 1) {
      Piece &a = pieces[from];
      Piece &b = pieces[from + 1];
      if(!a.isOriginal && !b.isOriginal) {
        a.data.append(b.data);
        a.size += b.size;
        pieces.erase(pieces.begin() + static_cast<std::ptrdiff_t>(from) + 1);
      }
      else if(a.isOriginal && b.isOriginal && a.source + a.size == b.source) {
        a.size += b.size;
        pieces.erase(pieces.begin() + static_cast<std::ptrdiff_t>(from) + 1);
      }
      else {
        ++from;
      }
    }
    updateStarts(index > 0 ? index - 1 : 0);
  }

  ByteVector read(offset_t offset, offset_t length)
  {
    ByteVector result;
    const offset_t end = std::min(offset + length, contentLength());
    if(offset >= end)
      return result;

    const auto it = std::upper_bound(starts.begin(), starts.end(), offset);
    size_t index = static_cast<size_t>(it - starts.begin()) - 1;

    while(offset < end && index < pieces.size()) {
      const Piece &piece = pieces[index];
      const offset_t inner = offset - starts[index];
      const offset_t count = std::min(piece.size - inner, end - offset);

      if(piece.isOriginal) {
        stream->seek(piece.source + inner);
        result.append(stream->readBlock(static_cast<size_t>(count)));
      }
      else {
        result.append(piece.data.mid(static_cast<unsigned int>(inner), static_cast<unsigned int>(count)));
      }

      offset += count;
      ++index;
    }
    return result;
  }

  // Copies count bytes of the underlying stream from source to destination,
  // in blocks ordered so that an overlapping range is read before it is
  // overwritten.
  void move(offset_t source, offset_t destination, offset_t count)
  {
    if(destination < source) {
      for(offset_t done = 0; done < count; done += moveBlockSize) {
        const offset_t block = std::min(moveBlockSize, count - done);
        stream->seek(source + done);
        const ByteVector bytes = stream->readBlock(static_cast<size_t>(block));
        stream->seek(destination + done);
        stream->writeBlock(bytes);
      }
    }
    else {
      for(offset_t remaining = count; remaining > 0;) {
        const offset_t block = std::min(moveBlockSize, remaining);
        remaining -= block;
        stream->seek(source + remaining);
        const ByteVector bytes = stream->readBlock(static_cast<size_t>(block));
        stream->seek(destination + remaining);
        stream->writeBlock(bytes);
      }
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DeferredWriteStream::DeferredWriteStream(IOStream *stream) :
  d(std::make_unique<DeferredWriteStreamPrivate>(stream))
{
  d->reset();
}

DeferredWriteStream::~DeferredWriteStream() = default;

bool DeferredWriteStream::commit()
{
  if(!d->changed)
    return true;

  if(!isOpen() || readOnly()) {
    debug("DeferredWriteStream::commit() -- stream is not writable.");
    return false;
  }

  // Insertions and removals never reorder the underlying bytes, so original
  // pieces appear in source order. That is what makes moving them in place
  // safe: those moving towards the start are copied first, front to back,
  // then those moving towards the end, back to front.
  offset_t previousEnd = 0;
  for(const auto &piece : d->pieces) {
    if(!piece.isOriginal)
      continue;
    if(piece.source < previousEnd) {
      debug("DeferredWriteStream::commit() -- pieces out of order.");
      return false;
    }
    previousEnd = piece.source + piece.size;
  }

  // Original pieces moving the same distance with only changed bytes between
  // them are moved as one run; the bytes carried along under the changes are
  // overwritten when the changes are written.
  struct Run { offset_t source; offset_t destination; offset_t size; };
  std::vector<Run> runs;
  for(size_t i = 0; i < d->pieces.size(); ++i) {
    const Piece &piece = d->pieces[i];
    if(!piece.isOriginal)
      continue;
    const offset_t destination = d->starts[i];
    if(!runs.empty()) {
      Run &last = runs.back();
      if(destination - piece.source == last.destination - last.source &&
         (i == 0 || !d->pieces[i - 1].isOriginal)) {
        last.size = piece.source + piece.size - last.source;
        continue;
      }
    }
    runs.push_back({ piece.source, destination, piece.size });
  }

  for(const auto &run : runs) {
    if(run.destination < run.source)
      d->move(run.source, run.destination, run.size);
  }

  for(auto it = runs.rbegin(); it != runs.rend(); ++it) {
    if(it->destination > it->source)
      d->move(it->source, it->destination, it->size);
  }

  for(size_t i = 0; i < d->pieces.size(); ++i) {
    const Piece &piece = d->pieces[i];
    if(!piece.isOriginal) {
      d->stream->seek(d->starts[i]);
      d->stream->writeBlock(piece.data);
    }
  }

  const offset_t length = d->contentLength();
  if(d->stream->length() > length)
    d->stream->truncate(length);

  d->reset();
  return true;
}

bool DeferredWriteStream::hasChanges() const
{
  return d->changed;
}

offset_t DeferredWriteStream::bytesToMove() const
{
  offset_t total = 0;
  for(size_t i = 0; i < d->pieces.size(); ++i) {
    if(d->pieces[i].isOriginal && d->starts[i] != d->pieces[i].source)
      total += d->pieces[i].size;
  }
  return total;
}

bool DeferredWriteStream::writeTo(IOStream *destination)
{
  if(!destination->isOpen() || destination->readOnly()) {
    debug("DeferredWriteStream::writeTo() -- destination is not writable.");
    return false;
  }

  const offset_t length = d->contentLength();
  destination->seek(0);
  for(offset_t done = 0; done < length; done += moveBlockSize) {
    destination->writeBlock(d->read(done, std::min(moveBlockSize, length - done)));
  }
  if(destination->length() > length)
    destination->truncate(length);
  return true;
}

FileName DeferredWriteStream::name() const
{
  return d->stream->name();
}

ByteVector DeferredWriteStream::readBlock(size_t length)
{
  ByteVector bytes = d->read(d->position, static_cast<offset_t>(length));
  d->position += bytes.size();
  return bytes;
}

void DeferredWriteStream::writeBlock(const ByteVector &data)
{
  d->replace(d->position, data.size(), data);
  d->position += data.size();
}

void DeferredWriteStream::insert(const ByteVector &data, offset_t start, size_t replace)
{
  d->replace(start, static_cast<offset_t>(replace), data);
  d->position = start + data.size();
}

void DeferredWriteStream::removeBlock(offset_t start, size_t length)
{
  d->replace(start, static_cast<offset_t>(length), ByteVector());
  d->position = d->contentLength();
}

bool DeferredWriteStream::readOnly() const
{
  return d->stream->readOnly();
}

bool DeferredWriteStream::isOpen() const
{
  return d->stream->isOpen();
}

void DeferredWriteStream::seek(offset_t offset, Position p)
{
  switch(p) {
  case Beginning:
    d->position = offset;
    break;
  case Current:
    d->position += offset;
    break;
  case End:
    d->position = d->contentLength() + offset;
    break;
  }
}

void DeferredWriteStream::clear()
{
  d->stream->clear();
}

offset_t DeferredWriteStream::tell() const
{
  return d->position;
}

offset_t DeferredWriteStream::length()
{
  return d->contentLength();
}

void DeferredWriteStream::truncate(offset_t length)
{
  const offset_t total = d->contentLength();
  if(length < total)
    d->replace(length, total - length, ByteVector());
  else if(length > total)
    d->replace(total, 0, ByteVector(static_cast<unsigned int>(length - total), '\0'));
}
