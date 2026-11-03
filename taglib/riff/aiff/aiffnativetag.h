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

#ifndef TAGLIB_AIFFNATIVETAG_H
#define TAGLIB_AIFFNATIVETAG_H

#include "tmap.h"
#include "tstring.h"
#include "tbytevector.h"
#include "taglib_export.h"
#include "tag.h"

namespace TagLib::RIFF::AIFF {

  //! The historic native text fields of an AIFF file.

  /*!
   * Before ID3v2 existed, an AIFF file carried its metadata in four text
   * chunks of its own: NAME (the title), AUTH (the artist), "(c) " (the
   * copyright) and ANNO (the comment). The text fills the chunk from end
   * to end. A file may hold any subset of them, and this class is the
   * subset a file held, plus whatever the caller sets on it.
   *
   * The AIFF specification gives these chunks no charset: they are bytes
   * on a Macintosh filesystem, and the software that wrote them wrote Mac
   * OS Roman, which is therefore what this class assumes.  Because the
   * file cannot say otherwise, the charset is what both reading and
   * writing use - the same situation as ID3v1, and the same solution: the
   * code page is resolved once when the tag is read and kept.  A caller
   * whose files hold another charset says so with setAssumedCodePage()
   * before reading them, and a tag read and written under one code page
   * keeps its text byte for byte.
   *
   * album(), genre(), year() and track() have no native field behind them:
   * the getters are empty and the setters do nothing.
   */
  class TAGLIB_EXPORT NativeTag : public Tag
  {
  public:
    /*!
     * The raw chunks a NativeTag is read from: a chunk id in the four this
     * class understands, mapped to the contents of that chunk, which is
     * its text from end to end. Ids this class does not know are ignored.
     */
    using FieldDataMap = Map<ByteVector, ByteVector>;

    /*!
     * Constructs an empty native tag, whose text is read and written in
     * the code page assumedCodePage() names.
     *
     * \see assumedCodePage()
     */
    NativeTag();

    /*!
     * Constructs a native tag from the chunks \a fields, decoded with the
     * code page assumedCodePage() names.
     *
     * The code page is decided here and kept: render() writes the text
     * back in the same charset it was read with, which is what a file
     * that declares no charset anywhere needs. A tag whose files hold
     * another charset is read by naming that charset before this is
     * constructed, so that it is the one kept.
     *
     * \see assumedCodePage()
     * \see setAssumedCodePage()
     */
    explicit NativeTag(const FieldDataMap &fields);

    ~NativeTag() override;

    NativeTag(const NativeTag &) = delete;
    NativeTag &operator=(const NativeTag &) = delete;

    // Reimplementations

    String title() const override;
    String artist() const override;
    String album() const override;
    String comment() const override;
    String genre() const override;
    unsigned int year() const override;
    unsigned int track() const override;

    void setTitle(const String &s) override;
    void setArtist(const String &s) override;
    void setAlbum(const String &s) override;
    void setComment(const String &s) override;
    void setGenre(const String &s) override;
    void setYear(unsigned int i) override;
    void setTrack(unsigned int i) override;

    bool isEmpty() const override;

    PropertyMap properties() const override;
    void removeUnsupportedProperties(const StringList &props) override;
    PropertyMap setProperties(const PropertyMap &props) override;

    /*!
     * Whether \a id is one of the four native text chunks this class
     * reads and writes: "NAME", "AUTH", "(c) " or "ANNO".
     */
    static bool isNativeField(const ByteVector &id);

    /*!
     * Returns the value of the field with the id \a id, which has to be
     * one of "NAME", "AUTH", "(c) " or "ANNO"; any other id is not a
     * native field of this class and yields an empty string.
     */
    String fieldText(const ByteVector &id) const;

    /*!
     * Sets the value of the field with the id \a id to \a s, where \a id
     * has to be one of "NAME", "AUTH", "(c) " or "ANNO". An empty \a s
     * removes the field, and with it its chunk. Another id is a chunk
     * this class does not write and is ignored with a debug message.
     */
    void setFieldText(const ByteVector &id, const String &s);

    /*!
     * Removes the field with the id \a id.
     */
    void removeField(const ByteVector &id);

    /*!
     * Renders every native field back to binary data, ready to be written
     * to disk: each of the four chunk ids mapped to the text of its
     * chunk, encoded with the code page this tag was read with. An empty
     * payload is a field that holds no text, whose chunk has to go.
     *
     * RIFF::AIFF::File::save() writes these payloads to the file, which is
     * where the assumed charset reaches the disk: a tag read and written
     * under one code page keeps its text byte for byte.
     */
    FieldDataMap render() const;

    /*!
     * Returns the code page assumed for a native tag read from a file,
     * which is 0 - no code page stated, and therefore Mac OS Roman - until
     * setAssumedCodePage() states another one.
     *
     * \see setAssumedCodePage()
     */
    static unsigned int assumedCodePage();

    /*!
     * Sets the code page \a codePage assumed for a native tag read from a
     * file: the text of every chunk is decoded with it, and a tag read
     * under it writes its text back with it too.
     *
     * \a codePage is a Windows code page number, the same numbering a CSET
     * chunk of a RIFF file uses: 65001 assumes UTF-8 and 28591 assumes
     * ISO 8859-1, while 0, the value to go back to, states no code page
     * and therefore assumes Mac OS Roman, which is what a Macintosh
     * writer's bytes are. This is where the caller says which of them its
     * files hold, because an AIFF file has no place to say it itself.
     *
     * \note This is a process wide setting and applies to the tags read
     * after the call. The charset is decided when a tag is constructed, so
     * a tag already read keeps the one it was read with.
     *
     * \note A code page this library cannot convert is refused: the call
     * then has no effect and a debug message is emitted.
     *
     * \see assumedCodePage()
     */
    static void setAssumedCodePage(unsigned int codePage);

  private:
    class NativeTagPrivate;
    TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
    std::unique_ptr<NativeTagPrivate> d;
  };
} // namespace TagLib::RIFF::AIFF

#endif
