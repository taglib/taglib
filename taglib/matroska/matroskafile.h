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

#ifndef TAGLIB_MATROSKAFILE_H
#define TAGLIB_MATROSKAFILE_H

#include "taglib_export.h"
#include "tfile.h"
#include "tag.h"
#include "matroskaproperties.h"

namespace TagLib::Matroska {
  class Properties;
  class Tag;
  class Attachments;

  /*!
   * Implementation of TagLib::File for Matroska.
   */
  class TAGLIB_EXPORT File : public TagLib::File
  {
  public:
    /*!
     * Constructs a Matroska file from \a file.  If \a readProperties is \c true the
     * file's audio properties will also be read.
     *
     * If \a readStyle is \c Accurate all seek head and cues segment positions
     * are verified for the isValid() state of the file.
     */
    explicit File(FileName file, bool readProperties = true,
                  Properties::ReadStyle readStyle = Properties::Average);

    /*!
     * Constructs a Matroska file from \a stream.  If \a readProperties is \c true the
     * file's audio properties will also be read.
     *
     * If \a readStyle is \c Accurate all seek head and cues segment positions
     * are verified for the isValid() state of the file.
     */
    explicit File(IOStream *stream, bool readProperties = true,
                  Properties::ReadStyle readStyle = Properties::Average);

    /*!
     * Destroys this instance of the File.
     */
    ~File() override;

    File(const File &) = delete;
    File &operator=(const File &) = delete;

    /*!
     * Returns a pointer to the tag of the file.
     *
     * It will create a tag if one does not exist and returns a valid pointer.
     *
     * \note The tag <b>is still</b> owned by the Matroska::File and should not
     * be deleted by the user.  It will be deleted when the file (object) is
     * destroyed.
     */
    TagLib::Tag *tag() const override;

    /*!
     * Returns a pointer to the Matroska tag of the file.
     *
     * If \a create is \c false this may return a null pointer if there is no tag.
     * If \a create is \c true it will create a tag if one does not exist and
     * returns a valid pointer.
     *
     * \note The tag <b>is still</b> owned by the Matroska::File and should not
     * be deleted by the user.  It will be deleted when the file (object)
     * destroyed.
     */
    Tag *tag(bool create) const;

    /*!
     * Implements the reading part of the unified property interface.
     */
    PropertyMap properties() const override;

    void removeUnsupportedProperties(const StringList &properties) override;

    /*!
     * Implements the writing part of the unified tag dictionary interface.
     */
    PropertyMap setProperties(const PropertyMap &) override;

    /*!
     * Returns the keys for attached files, "PICTURE" for images, the media
     * type, file name or UID for other attached files.
     * The names of the binary simple tags are included too.
     */
    StringList complexPropertyKeys() const override;

    /*!
     * Get the pictures stored in the attachments as complex properties
     * for \a key "PICTURE". Other attached files can be retrieved, by
     * media type, file name or UID.
     * The attached files are returned as maps with keys "data", "mimeType",
     * "description", "fileName, "uid".
     * Binary simple tags can be retrieved as maps with keys "data", "name",
     * "targetTypeValue", "language", "defaultLanguage".
     */
    List<VariantMap> complexProperties(const String &key) const override;

    /*!
     * Set attached files as complex properties \a value, e.g. pictures for
     * \a key "PICTURE" with the maps in \a value having keys "data", "mimeType",
     * "description", "fileName, "uid". For other attached files, the mime type,
     * file name or UID can be used as the \a key.
     * Maps with keys "name" (with the same value as \a key) and "data" are
     * stored as binary simple tags with additional keys "targetTypeValue",
     * "language", "defaultLanguage".
     */
    bool setComplexProperties(const String &key, const List<VariantMap> &value) override;

    /*!
     * Returns the Matroska::Properties for this file.  If no audio properties
     * were read then this will return a null pointer.
     */
    AudioProperties *audioProperties() const override;

    /*!
     * Save the file.
     *
     * This returns \c true if the save was successful.
     */
    bool save() override;

    Attachments *attachments(bool create = false) const;

    /*!
     * Returns whether or not the given \a stream can be opened as a Matroska
     * file.
     *
     * \note This method is designed to do a quick check.  The result may
     * not necessarily be correct.
     */
    static bool isSupported(IOStream *stream);

  private:
    void read(bool readProperties, Properties::ReadStyle readStyle);
    class FilePrivate;
    friend class Properties;
    TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
    std::unique_ptr<FilePrivate> d;
  };
}

#endif
