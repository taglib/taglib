/***************************************************************************
    copyright            : (C) 2004 by Scott Wheeler
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

#ifndef TAGLIB_UNIQUEFILEIDENTIFIERFRAME
#define TAGLIB_UNIQUEFILEIDENTIFIERFRAME

#include <id3v2frame.h>

namespace TagLib {

  namespace ID3v2 {

    class UniqueFileIdentifierFrame : public ID3v2::Frame
    {
      friend class FrameFactory;

    public:
      UniqueFileIdentifierFrame(const ByteVector &data);
      UniqueFileIdentifierFrame(const String &owner, const ByteVector &id);

      String owner() const;
      ByteVector identifier() const;

      void setOwner(const String &s);
      void setIdentifier(const ByteVector &v);

      virtual String toString() const;

    protected:
      virtual void parseFields(const ByteVector &data);
      virtual ByteVector renderFields() const;

    private:
      UniqueFileIdentifierFrame(const ByteVector &data, Header *h);

      class UniqueFileIdentifierFramePrivate;
      UniqueFileIdentifierFramePrivate *d;
    };
  }
}

#endif
