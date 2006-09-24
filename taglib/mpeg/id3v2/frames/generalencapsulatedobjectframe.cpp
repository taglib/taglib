/***************************************************************************
    copyright            : (C) 2004 by Scott Wheeler
    email                : wheeler@kde.org
    copyright            : (C) 2006 by Aaron VonderHaar
    email                : avh4@users.sourceforge.net
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

#include <tdebug.h>

#include "generalencapsulatedobjectframe.h"

using namespace TagLib;
using namespace ID3v2;

class GeneralEncapsulatedObjectFrame::GeneralEncapsulatedObjectFramePrivate
{
public:
  GeneralEncapsulatedObjectFramePrivate() : textEncoding(String::Latin1) {}

  String::Type textEncoding;
  String mimeType;
  String fileName;
  String description;
  ByteVector data;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

GeneralEncapsulatedObjectFrame::GeneralEncapsulatedObjectFrame() : Frame("GEOB")
{
    d = new GeneralEncapsulatedObjectFramePrivate;
}

GeneralEncapsulatedObjectFrame::GeneralEncapsulatedObjectFrame(const ByteVector &data) : Frame(data)
{
  setData(data);
  d = new GeneralEncapsulatedObjectFramePrivate;
}

GeneralEncapsulatedObjectFrame::~GeneralEncapsulatedObjectFrame()
{
  delete d;
}

String GeneralEncapsulatedObjectFrame::toString() const
{
  String text = "[" + d->mimeType + "]";

  if(!d->fileName.isEmpty())
    text += " " + d->fileName;

  if(!d->description.isEmpty())
    text += " \"" + d->description + "\"";

  return text;
}

String::Type GeneralEncapsulatedObjectFrame::textEncoding() const
{
  return d->textEncoding;
}

void GeneralEncapsulatedObjectFrame::setTextEncoding(String::Type encoding)
{
  d->textEncoding = encoding;
}

String GeneralEncapsulatedObjectFrame::mimeType() const
{
  return d->mimeType;
}

void GeneralEncapsulatedObjectFrame::setMimeType(const String &type)
{
  d->mimeType = type;
}

String GeneralEncapsulatedObjectFrame::fileName() const
{
  return d->fileName;
}

void GeneralEncapsulatedObjectFrame::setFileName(const String &name)
{
  d->fileName = name;
}

String GeneralEncapsulatedObjectFrame::description() const
{
  return d->description;
}

void GeneralEncapsulatedObjectFrame::setDescription(const String &desc)
{
  d->description = desc;
}

ByteVector GeneralEncapsulatedObjectFrame::object() const
{
  return d->data;
}

void GeneralEncapsulatedObjectFrame::setObject(const ByteVector &data)
{
  d->data = data;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void GeneralEncapsulatedObjectFrame::parseFields(const ByteVector &data)
{
  if(data.size() < 4) {
    debug("An object frame must contain at least 4 bytes.");
    return;
  }

  int fieldStart = 0;

  d->textEncoding = String::Type(data[fieldStart]);
  fieldStart += 1;

  int fieldEnd = data.find(textDelimiter(String::Latin1), fieldStart);

  if(fieldEnd < fieldStart)
    return;

  d->mimeType = String(data.mid(fieldStart, fieldEnd - fieldStart), String::Latin1);
  fieldStart = fieldEnd + 1;

  fieldEnd = data.find(textDelimiter(d->textEncoding), fieldStart);

  if(fieldEnd < fieldStart)
    return;

  d->fileName = String(data.mid(fieldStart, fieldEnd - fieldStart), d->textEncoding);
  fieldStart = fieldEnd + 1;

  fieldEnd = data.find(textDelimiter(d->textEncoding), fieldStart);

  if(fieldEnd < fieldStart)
    return;

  d->description = String(data.mid(fieldStart, fieldEnd - fieldStart), d->textEncoding);
  fieldStart = fieldEnd + 1;

  d->data = data.mid(fieldStart);
}

ByteVector GeneralEncapsulatedObjectFrame::renderFields() const
{
  ByteVector data;

  data.append(char(d->textEncoding));
  data.append(d->mimeType.data(String::Latin1));
  data.append(textDelimiter(String::Latin1));
  data.append(d->fileName.data(d->textEncoding));
  data.append(textDelimiter(d->textEncoding));
  data.append(d->description.data(d->textEncoding));
  data.append(textDelimiter(d->textEncoding));
  data.append(d->data);

  return data;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

GeneralEncapsulatedObjectFrame::GeneralEncapsulatedObjectFrame(const ByteVector &data, Header *h) : Frame(h)
{
  d = new GeneralEncapsulatedObjectFramePrivate;
  parseFields(fieldData(data));
}
