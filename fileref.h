/***************************************************************************
    copyright            : (C) 2003 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#ifndef TAGLIB_FILEREF_H
#define TAGLIB_FILEREF_H

#include "audioproperties.h"

namespace TagLib {

  class String;
  class File;
  class Tag;

  //! This class provides a simple abstraction for creating and handling files

  /*!
   * FileRef exists to provide a minimal, generic and value-based wrapper around
   * a File.  It is lightweight and implicitly shared, and as such suitable for
   * pass-by-value use.  This hides some of the uglier details of TagLib::File
   * and the non-generic portions of the concrete file implementations.
   *
   * This class is useful in a "simple usage" situation where it is desirable
   * to be able to get and set some of the tag information that is similar
   * across file types.
   *
   * Also note that it is probably a good idea to plug this into your mime
   * type system rather than using the constructor that accepts a file name.
   *
   * For example in KDE this could be done with:
   *
   * \code
   *
   * TagLib::FileRef createFileRef( const QString &fileName )
   * {
   *   KMimeType::Ptr result = KMimeType::findByPath( fileName, 0, true );
   *
   *   if( result->name() == "audio/x-mp3" )
   *     return FileRef( new MPEG::File( QFile::encodeName( fileName ).data() ) );
   *
   *   if( result->name() == "application/ogg" )
   *     return FileRef( new Vorbis::File( QFile::encodeName( fileName ).data() ) );
   *
   *   return FileRef( 0 );
   * }
   *
   * \endcode
   */

  class FileRef
  {
  public:

    FileRef();

    /*!
     * Create a FileRef from \a fileName.  If \a readAudioProperties is true then
     * the audio properties will be read using \a audioPropertiesStyle.  If
     * \a readAudioProperties is false then \a audioPropertiesStyle will be
     * ignored.
     *
     * Also see the note in the class documentation about why you may not want to
     * use this method in your application.
     */
    explicit FileRef(const char *fileName,
                     bool readAudioProperties = true,
                     AudioProperties::ReadStyle
                     audioPropertiesStyle = AudioProperties::Average);

    /*!
     * Contruct a FileRef using \a file.  The FileRef now takes ownership of the
     * pointer and will delete the File when it passes out of scope.
     */
    explicit FileRef(File *file);

    /*!
     * Make a copy of \a ref.
     */
    FileRef(const FileRef &ref);

    /*!
     * Destroys this FileRef instance.
     */
    virtual ~FileRef();

    /*!
     * Returns a pointer to represented file's tag.
     *
     * \warning This pointer will become invalid when this FileRef and all
     * copies pass out of scope.
     *
     * \see File::tag()
     */
    Tag *tag() const;

    /*!
     * Returns the audio properties for this FileRef.  If no audio properties
     * were read then this will returns a null pointer.
     */
    AudioProperties *audioProperties() const;

    /*!
     * Returns a pointer to the file represented by this handler class.
     *
     * As a general rule this call should be avoided since if you need to work
     * with file objects directly, you are probably better served instantiating
     * the File subclasses (i.e. MPEG::File) manually and working with their APIs.
     *
     * This <i>handle</i> exists to provide a minimal, generic and value-based
     * wrapper around a File.  Accessing the file directly generally indicates
     * a moving away from this simplicity (and into things beyond the scope of
     * FileRef).
     *
     * \warning This pointer will become invalid when this FileRef and all
     * copies pass out of scope.
     */
    File *file() const;

    /*!
     * Saves the file.  Returns true on success.
     */
    bool save();

    /*!
     * Returns true if the file (and as such other pointers) are null.
     */
    bool isNull() const;

    /*!
     * Assign the file pointed to by \a ref to this FileRef.
     */
    FileRef &operator=(const FileRef &ref);

    /*!
     * Returns true if this FileRef and \a ref point to the same File object.
     */
    bool operator==(const FileRef &ref) const;

    /*!
     * Returns true if this FileRef and \a ref do not point to the same File
     * object.
     */
    bool operator!=(const FileRef &ref) const;

    /*!
     * A simple implementation of file type guessing.  If \a readAudioProperties
     * is true then the audio properties will be read using
     * \a audioPropertiesStyle.  If \a readAudioProperties is false then
     * \a audioPropertiesStyle will be ignored.
     *
     * \note You generally shouldn't use this method, but instead the constructor
     * directly.
     */
    static File *create(const char *fileName,
                        bool readAudioProperties = true,
                        AudioProperties::ReadStyle audioPropertiesStyle = AudioProperties::Average);

  private:
    class FileRefPrivate;
    FileRefPrivate *d;
  };

} // namespace TagLib

#endif
