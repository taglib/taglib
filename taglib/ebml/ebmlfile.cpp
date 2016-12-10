/***************************************************************************
    copyright            : (C) 2013 by Sebastian Rachuj
    email                : rachus@web.de
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

#include "ebmlelement.h"

using namespace TagLib;

class EBML::File::FilePrivate
{
public:
  explicit FilePrivate(File *document) : root(document)
  {
  }
  
  // Performs a few basic checks and creates the FilePrivate if they were
  // successful.
  static FilePrivate *checkAndCreate(File* document)
  {
    document->seek(0);
    ByteVector magical = document->readBlock(4);
    if(static_cast<ulli>(magical.toUInt32BE(0)) != Header::EBML)
      return 0;
    FilePrivate *d = new FilePrivate(document);
    Element *head = d->root.getChild(Header::EBML);
    Element *p;
    if(!head ||
      !((p = head->getChild(Header::EBMLVersion)) && p->getAsUnsigned() == 1L) ||
      !((p = head->getChild(Header::EBMLReadVersion)) && p->getAsUnsigned() == 1L) ||
      // Actually 4 is the current maximum of the EBML spec, but we support up to 8
      !((p = head->getChild(Header::EBMLMaxIDWidth)) && p->getAsUnsigned() <= 8) ||
      !((p = head->getChild(Header::EBMLMaxSizeWidth)) && p->getAsUnsigned() <= 8)
    ) {
      delete d;
      return 0;
	}
	return d;
  }
  
  Element root;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

EBML::File::~File()
{
  delete d;
}

EBML::Element *EBML::File::getDocumentRoot()
{
  if(!d && isValid())
    d = new FilePrivate(this);
  return &d->root;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

EBML::File::File(FileName file) :
  TagLib::File(file)
{
  if(isOpen()) {
    d = FilePrivate::checkAndCreate(this);
    if(!d)
      setValid(false);
  }
}

EBML::File::File(IOStream *stream) :
  TagLib::File(stream)
{
  if(isOpen()) {
    d = FilePrivate::checkAndCreate(this);
    if(!d)
      setValid(false);
  }
}
