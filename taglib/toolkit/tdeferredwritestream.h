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

#ifndef TAGLIB_DEFERREDWRITESTREAM_H
#define TAGLIB_DEFERREDWRITESTREAM_H

#include "tbytevector.h"
#include "tiostream.h"
#include "taglib_export.h"
#include "taglib.h"

namespace TagLib {

  //! A stream that keeps every change to another stream in memory until commit()

  /*!
   * Writes, insertions, removals and truncations are recorded against the
   * underlying stream instead of applied to it, and reads see the result.
   * commit() then writes the final content in one pass: bytes that end up
   * where they started are not written at all, and bytes that move are moved
   * once, however many insertions and removals moved them.
   *
   * A save that changes several blocks ahead of the audio data, each of which
   * would move all of the audio when written to a FileStream, moves it at most
   * once through this stream.
   *
   * Only changed bytes are held in memory; unchanged ranges are read from the
   * underlying stream on demand. The underlying stream must not be changed by
   * anything else until commit() returns.
   */
  class TAGLIB_EXPORT DeferredWriteStream : public IOStream
  {
  public:
    /*!
     * Construct a DeferredWriteStream over \a stream, which is not owned and
     * must outlive it.
     */
    DeferredWriteStream(IOStream *stream);

    /*!
     * Destroys this DeferredWriteStream instance. Changes not committed are
     * discarded.
     */
    ~DeferredWriteStream() override;

    DeferredWriteStream(const DeferredWriteStream &) = delete;
    DeferredWriteStream &operator=(const DeferredWriteStream &) = delete;

    /*!
     * Writes every recorded change to the underlying stream. Returns \c false
     * if the underlying stream is read only or not open, in which case nothing
     * is written. Afterwards the stream reads the underlying stream directly
     * again.
     */
    bool commit();

    /*!
     * Returns \c true if there are changes not yet committed.
     */
    bool hasChanges() const;

    /*!
     * Returns how many bytes of the underlying stream commit() would move to
     * a new position; 0 when it would only overwrite or append.
     */
    offset_t bytesToMove() const;

    /*!
     * Writes the content, changes included, to \a destination from its
     * start, truncating it to the content's length. The underlying stream and
     * the recorded changes are left as they are. Returns \c false if
     * \a destination is read only or not open.
     */
    bool writeTo(IOStream *destination);

    /*!
     * Returns the underlying stream's name.
     */
    FileName name() const override;

    /*!
     * Reads a block of size \a length at the current get pointer.
     */
    ByteVector readBlock(size_t length) override;

    /*!
     * Records \a data as written at the current get pointer.
     */
    void writeBlock(const ByteVector &data) override;

    /*!
     * Records \a data as inserted at position \a start, overwriting \a replace
     * bytes of the content.
     */
    void insert(const ByteVector &data, offset_t start = 0, size_t replace = 0) override;

    /*!
     * Records the removal of \a length bytes starting at \a start.
     */
    void removeBlock(offset_t start = 0, size_t length = 0) override;

    /*!
     * Returns the underlying stream's read only state.
     */
    bool readOnly() const override;

    /*!
     * Returns the underlying stream's open state.
     */
    bool isOpen() const override;

    /*!
     * Move the I/O pointer to \a offset in the stream from position \a p.  This
     * defaults to seeking from the beginning of the stream.
     *
     * \see Position
     */
    void seek(offset_t offset, Position p = Beginning) override;

    /*!
     * Resets the underlying stream's end-of-stream and error flags.
     */
    void clear() override;

    /*!
     * Returns the current offset within the stream.
     */
    offset_t tell() const override;

    /*!
     * Returns the length of the stream, changes included.
     */
    offset_t length() override;

    /*!
     * Records the stream as truncated, or extended with zero bytes, to
     * \a length.
     */
    void truncate(offset_t length) override;

  private:
    class DeferredWriteStreamPrivate;
    TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
    std::unique_ptr<DeferredWriteStreamPrivate> d;
  };

}  // namespace TagLib

#endif
