/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#include "textidentificationframe.h"
#include "tbytevectorlist.h"
#include "id3v2tag.h"
#include "tpropertymap.h"
#include "id3v1genres.h"

#include <array>

using namespace TagLib;
using namespace ID3v2;

class TextIdentificationFrame::TextIdentificationFramePrivate
{
public:
  TextIdentificationFramePrivate() : textEncoding(String::Latin1) {}
  String::Type textEncoding;
  StringList fieldList;
};

class UserTextIdentificationFrame::UserTextIdentificationFramePrivate
{
};

////////////////////////////////////////////////////////////////////////////////
// TextIdentificationFrame public members
////////////////////////////////////////////////////////////////////////////////

TextIdentificationFrame::TextIdentificationFrame(const ByteVector &type, String::Type encoding) :
  Frame(type),
  d(std::make_unique<TextIdentificationFramePrivate>())
{
  d->textEncoding = encoding;
}

TextIdentificationFrame::TextIdentificationFrame(const ByteVector &data) :
  Frame(data),
  d(std::make_unique<TextIdentificationFramePrivate>())
{
  setData(data);
}

TextIdentificationFrame *TextIdentificationFrame::createTIPLFrame(const PropertyMap &properties) // static
{
  auto frame = new TextIdentificationFrame("TIPL");
  StringList l;
  for(auto it = properties.begin(); it != properties.end(); ++it){
    const String role = involvedPeopleMap()[it->first];
    if(role.isEmpty()) // should not happen
      continue;
    l.append(role);
    l.append(it->second.toString(",")); // comma-separated list of names
  }
  frame->setText(l);
  return frame;
}

TextIdentificationFrame *TextIdentificationFrame::createTMCLFrame(const PropertyMap &properties) // static
{
  auto frame = new TextIdentificationFrame("TMCL");
  StringList l;
  for(auto it = properties.begin(); it != properties.end(); ++it){
    if(!it->first.startsWith(instrumentPrefix)) // should not happen
      continue;
    l.append(it->first.substr(instrumentPrefix.size()));
    l.append(it->second.toString(","));
  }
  frame->setText(l);
  return frame;
}

TextIdentificationFrame::~TextIdentificationFrame() = default;

void TextIdentificationFrame::setText(const StringList &l)
{
  d->fieldList = l;
}

void TextIdentificationFrame::setText(const String &s)
{
  d->fieldList = s;
}

String TextIdentificationFrame::toString() const
{
  return d->fieldList.toString();
}

StringList TextIdentificationFrame::fieldList() const
{
  return d->fieldList;
}

String::Type TextIdentificationFrame::textEncoding() const
{
  return d->textEncoding;
}

void TextIdentificationFrame::setTextEncoding(String::Type encoding)
{
  d->textEncoding = encoding;
}

namespace
{
  // array of allowed TIPL prefixes and their corresponding key value
  constexpr std::array involvedPeople {
    std::pair("ARRANGER", "ARRANGER"),
    std::pair("ENGINEER", "ENGINEER"),
    std::pair("PRODUCER", "PRODUCER"),
    std::pair("DJ-MIX", "DJMIXER"),
    std::pair("MIX", "MIXER"),
  };
}  // namespace

const KeyConversionMap &TextIdentificationFrame::involvedPeopleMap() // static
{
  static KeyConversionMap m;
  if(m.isEmpty()) {
    for(const auto &[o, t] : involvedPeople)
      m.insert(t, o);
  }
  return m;
}

PropertyMap TextIdentificationFrame::asProperties() const
{
  if(frameID() == "TIPL")
    return makeTIPLProperties();
  if(frameID() == "TMCL")
    return makeTMCLProperties();
  PropertyMap map;
  String tagName = frameIDToKey(frameID());
  if(tagName.isEmpty()) {
    map.unsupportedData().append(frameID());
    return map;
  }
  StringList values = fieldList();
  if(tagName == "GENRE") {
    // Special case: Support ID3v1-style genre numbers. They are not officially supported in
    // ID3v2, however it seems that still a lot of programs use them.
    for(auto it = values.begin(); it != values.end(); ++it) {
      bool ok = false;
      int test = it->toInt(&ok); // test if the genre value is an integer
      if(ok)
        *it = ID3v1::genre(test);
    }
  } else if(tagName == "DATE") {
    for(auto it = values.begin(); it != values.end(); ++it) {
      // ID3v2 specifies ISO8601 timestamps which contain a 'T' as separator between date and time.
      // Since this is unusual in other formats, the T is removed.
      int tpos = it->find("T");
      if(tpos != -1)
        (*it)[tpos] = ' ';
    }
  }
  PropertyMap ret;
  ret.insert(tagName, values);
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
// TextIdentificationFrame protected members
////////////////////////////////////////////////////////////////////////////////

void TextIdentificationFrame::parseFields(const ByteVector &data)
{
  // Don't try to parse invalid frames

  if(data.size() < 2)
    return;

  // read the string data type (the first byte of the field data)

  d->textEncoding = static_cast<String::Type>(data[0]);

  // split the byte array into chunks based on the string type (two byte delimiter
  // for unicode encodings)

  int byteAlign = d->textEncoding == String::Latin1 || d->textEncoding == String::UTF8 ? 1 : 2;

  // build a small counter to strip nulls off the end of the field

  int dataLength = data.size() - 1;

  while(dataLength > 0 && data[dataLength] == 0)
    dataLength--;

  while(dataLength % byteAlign != 0)
    dataLength++;

  const ByteVectorList l = ByteVectorList::split(data.mid(1, dataLength), textDelimiter(d->textEncoding), byteAlign);

  d->fieldList.clear();

  // append those split values to the list and make sure that the new string's
  // type is the same specified for this frame

  unsigned short firstBom = 0;
  for(auto it = l.begin(); it != l.end(); it++) {
    if(!it->isEmpty() || (it == l.begin() && frameID() == "TXXX")) {
      if(d->textEncoding == String::Latin1) {
        d->fieldList.append(Tag::latin1StringHandler()->parse(*it));
      }
      else {
        String::Type textEncoding = d->textEncoding;
        if(textEncoding == String::UTF16) {
          if(it == l.begin()) {
            firstBom = it->mid(0, 2).toUShort();
          }
          else {
            unsigned short subsequentBom = it->mid(0, 2).toUShort();
            if(subsequentBom != 0xfeff && subsequentBom != 0xfffe) {
              if(firstBom == 0xfeff) {
                textEncoding = String::UTF16BE;
              }
              else if(firstBom == 0xfffe) {
                textEncoding = String::UTF16LE;
              }
            }
          }
        }
        d->fieldList.append(String(*it, textEncoding));
      }
    }
  }
}

ByteVector TextIdentificationFrame::renderFields() const
{
  String::Type encoding = checkTextEncoding(d->fieldList, d->textEncoding);

  ByteVector v;

  v.append(static_cast<char>(encoding));

  for(auto it = d->fieldList.cbegin(); it != d->fieldList.cend(); it++) {

    // Since the field list is null delimited, if this is not the first
    // element in the list, append the appropriate delimiter for this
    // encoding.

    if(it != d->fieldList.cbegin())
      v.append(textDelimiter(encoding));

    v.append((*it).data(encoding));
  }

  return v;
}

////////////////////////////////////////////////////////////////////////////////
// TextIdentificationFrame private members
////////////////////////////////////////////////////////////////////////////////

TextIdentificationFrame::TextIdentificationFrame(const ByteVector &data, Header *h) :
  Frame(h),
  d(std::make_unique<TextIdentificationFramePrivate>())
{
  parseFields(fieldData(data));
}

PropertyMap TextIdentificationFrame::makeTIPLProperties() const
{
  PropertyMap map;
  if(fieldList().size() % 2 != 0){
    // according to the ID3 spec, TIPL must contain an even number of entries
    map.unsupportedData().append(frameID());
    return map;
  }
  const StringList l = fieldList();
  for(auto it = l.begin(); it != l.end(); ++it) {
    auto found = std::find_if(involvedPeople.begin(), involvedPeople.end(), [=](const auto &person) { return *it == person.first; });
    if(found != involvedPeople.end()) {
      map.insert(found->second, (++it)->split(","));
    }
    else {
      // invalid involved role -> mark whole frame as unsupported in order to be consistent with writing
      map.clear();
      map.unsupportedData().append(frameID());
      return map;
    }
  }
  return map;
}

PropertyMap TextIdentificationFrame::makeTMCLProperties() const
{
  PropertyMap map;
  if(fieldList().size() % 2 != 0){
    // according to the ID3 spec, TMCL must contain an even number of entries
    map.unsupportedData().append(frameID());
    return map;
  }
  const StringList l = fieldList();
  for(auto it = l.begin(); it != l.end(); ++it) {
    String instrument = it->upper();
    if(instrument.isEmpty()) {
      // instrument is not a valid key -> frame unsupported
      map.clear();
      map.unsupportedData().append(frameID());
      return map;
    }
    map.insert(L"PERFORMER:" + instrument, (++it)->split(","));
  }
  return map;
}

////////////////////////////////////////////////////////////////////////////////
// UserTextIdentificationFrame public members
////////////////////////////////////////////////////////////////////////////////

UserTextIdentificationFrame::~UserTextIdentificationFrame() = default;

UserTextIdentificationFrame::UserTextIdentificationFrame(String::Type encoding) :
  TextIdentificationFrame("TXXX", encoding)
{
  StringList l;
  l.append(String());
  l.append(String());
  setText(l);
}


UserTextIdentificationFrame::UserTextIdentificationFrame(const ByteVector &data) :
  TextIdentificationFrame(data)
{
  checkFields();
}

UserTextIdentificationFrame::UserTextIdentificationFrame(const String &description, const StringList &values, String::Type encoding) :
  TextIdentificationFrame("TXXX", encoding)
{
  setDescription(description);
  setText(values);
}

String UserTextIdentificationFrame::toString() const
{
  // first entry is the description itself, drop from values list
  StringList l = fieldList();
  if(!l.isEmpty())
    l.erase(l.begin());
  return "[" + description() + "] " + l.toString();
}

String UserTextIdentificationFrame::description() const
{
  return !TextIdentificationFrame::fieldList().isEmpty()
    ? TextIdentificationFrame::fieldList().front()
    : String();
}

StringList UserTextIdentificationFrame::fieldList() const
{
  // TODO: remove this function

  return TextIdentificationFrame::fieldList();
}

void UserTextIdentificationFrame::setText(const String &text)
{
  if(description().isEmpty())
    setDescription(String());

  TextIdentificationFrame::setText(StringList(description()).append(text));
}

void UserTextIdentificationFrame::setText(const StringList &fields)
{
  if(description().isEmpty())
    setDescription(String());

  TextIdentificationFrame::setText(StringList(description()).append(fields));
}

void UserTextIdentificationFrame::setDescription(const String &s)
{
  StringList l = fieldList();

  if(l.isEmpty())
    l.append(s);
  else
    l[0] = s;

  TextIdentificationFrame::setText(l);
}

PropertyMap UserTextIdentificationFrame::asProperties() const
{
  PropertyMap map;
  String tagName = txxxToKey(description());
  const StringList v = fieldList();
  for(auto it = v.begin(); it != v.end(); ++it)
    if(it != v.begin())
      map.insert(tagName, *it);
  return map;
}

UserTextIdentificationFrame *UserTextIdentificationFrame::find(
  ID3v2::Tag *tag, const String &description) // static
{
  const FrameList l = tag->frameList("TXXX");
  for(auto it = l.begin(); it != l.end(); ++it) {
    auto f = dynamic_cast<UserTextIdentificationFrame *>(*it);
    if(f && f->description() == description)
      return f;
  }
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// UserTextIdentificationFrame private members
////////////////////////////////////////////////////////////////////////////////

UserTextIdentificationFrame::UserTextIdentificationFrame(const ByteVector &data, Header *h) :
  TextIdentificationFrame(data, h)
{
  checkFields();
}

void UserTextIdentificationFrame::checkFields()
{
  int fields = fieldList().size();

  if(fields == 0)
    setDescription(String());
  if(fields <= 1)
    setText(String());
}
