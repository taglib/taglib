/***************************************************************************
    copyright            : (C) 2002 by Scott Wheeler
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

#ifndef TAGLIB_H
#define TAGLIB_H

#define TAGLIB_MAJOR_VERSION 1
#define TAGLIB_MINOR_VERSION 3

#include <string>

//! A namespace for all TagLib related classes and functions

/*!
 * This namespace contains everything in TagLib.  For projects working with
 * TagLib extensively it may be conveniten to add a
 * \code
 * using namespace TagLib;
 * \endcode
 */

namespace TagLib {

  class String;

  typedef wchar_t wchar;
  typedef unsigned char uchar;
  typedef unsigned int  uint;
  typedef unsigned long ulong;

  /*!
   * Unfortunately std::wstring isn't defined on some systems, (i.e. GCC < 3)
   * so I'm providing something here that should be constant.
   */
  typedef std::basic_string<wchar> wstring;

#ifndef DO_NOT_DOCUMENT // Tell Doxygen to skip this class.
  /*!
   * \internal
   * This is just used as a base class for shared classes in TagLib.
   *
   * \warning This <b>is not</b> part of the TagLib public API!
   */

  class RefCounter
  {
  public:
    RefCounter() : refCount(1) {}
    void ref() { refCount++; }
    bool deref() { return ! --refCount ; }
    int count() { return refCount; }
  private:
    uint refCount;
  };

#endif // DO_NOT_DOCUMENT

}

/*!
 * \mainpage TagLib
 * \section intro Introduction
 * TagLib, is well, a library for reading and editing audio meta data, commonly know as \e tags.
 *
 * Some goals of TagLib:
 * - A clean, high level, C++ API to handling audio meta data.
 * - Support for at least ID3v1, ID3v2 and Ogg Vorbis \e comments.
 * - A generic, \e simple API for the most common tagging related functions.
 * - Binary compatibility between minor releases using the standard KDE/Qt techniques for C++ binary compatibility.
 * - Make the tagging framework extensible by library users; i.e. it will be possible for libarary users to implement
 *   additional ID3v2 frames, without modifying the TagLib source (through the use of <i>Abstract Factories</i> and
 *   such.
 *
 * Because TagLib desires to be toolkit agnostic, in hope of being widely adopted and the most flexible in licensing
 * TagLib provides many of its own toolkit classes; in fact the only external dependancy that TagLib has, it a
 * semi-sane STL implementation.
 *
 * \section why Why TagLib?
 *
 * TagLib was written to fill a gap in the Open Source/Free Software community.  Currently there is a lack in the
 * OSS/FS for a homogenous API to the most common music types.  In fact the only semi-portable implementation of the
 * ID3v2 standard available on Linux is id3lib, which unfortunately is poorly written, poorly documented and which
 * cycles through maintainers at least once a year (I took my turn some time ago.).
 *
 * As TagLib will be initially injected into the KDE community, while I am not linking to any of the KDE or Qt libraries
 * I have tried to follow the coding style of those libraries.  Again, this is in sharp contrast to id3lib, which
 * basically provides a hybrid C/C++ API and uses a dubious object model.
 *
 * I get asked rather frequently why I am replacing id3lib (mostly by people that have never worked with id3lib), if
 * you are concerned about this please email me; I can provide my lengthy standard rant.  :-)
 *
 * \section examples Examples:
 *
 * I've talked a lot about the \e homogenous API to common music formats.  Here's an example of how things (will) work:
 *
 * \code
 *
 * TagLib::FileRef f("Latex Solar Beef.mp3");
 * TagLib::String artist = f.tag()->artist(); // artist == "Frank Zappa"
 *
 * f.tag()->setAlbum("Fillmore East");
 * f.save();
 *
 * TagLib::FileRef g("Free City Rhymes.ogg");
 * TagLib::String album = g.tag()->album(); // album == "NYC Ghosts & Flowers"
 *
 * g.tag()->setTrack(1);
 * g.save();
 *
 * \endcode
 *
 * Notice that these high level functions work for both Ogg \e and MP3.  For this high level API, which is suitable for
 * most applications, the differences between ID3v2, ID3v1, MPEG and Ogg Vorbis can all be ignored.
 *
 * \author Scott Wheeler <wheeler@kde.org>
 */

#endif
