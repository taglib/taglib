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

class EBML::Element::ElementPrivate
{
public:  
  // The id of this element.
  ulli id;
  
  // The position of the element, where the header begins.
  size_t position;
  
  // The size of the element as read from the header. Note: Actually an ulli but
  // due to the variable integer size limited, thus size_t is ok.
  size_t size;
  
  // The position of the element's data.
  size_t data;
  
  // The element's children.
  List<Element *> children;
  
  // True: Treated this element as container and read children.
  bool populated;
  
  // The parent element. If NULL (0) this is the document root.
  Element *parent;
  
  // The file used to read and write.
  File *document;
  
  // Destructor: Clean up all children.
  ~ElementPrivate()
  {
    for(List<Element *>::Iterator i = children.begin(); i != children.end(); ++i) {
      delete *i;
    }
  }
  
  // Reads a variable length integer from the file at the given position
  // and saves its value to result. If cutOne is true the first one of the
  // binary representation of the result is removed (required for size). If
  // cutOne is false the one will remain in the result (required for id).
  // This method returns the position directly after the read integer.
  size_t readVInt(size_t position, ulli *result, bool cutOne = true)
  {
    document->seek(position);
    
    // Determine the length of the integer
    char firstByte = document->readBlock(1)[0];
    uint byteSize = 1;
    for(uint i = 0; i < 8 && ((firstByte << i) & (1 << 7)) == 0; ++i)
      ++byteSize;
    
    // Load the integer
    document->seek(position);
    ByteVector vint = document->readBlock(byteSize);
    
    // Cut the one if requested
    if(cutOne)
      vint[0] = (vint[0] & (~(1 << (8 - byteSize))));
  
    // Store the result and return the current position
    if(result)
      *result = static_cast<ulli>(vint.toLongLong());
    return position + byteSize;
  }
  
  // Returns a BytVector containing the given number in the variable integer
  // format. Truncates numbers > 2^56 (^ means potency in this case).
  // If addOne is true, the ByteVector will remain the One to determine the
  // integer's length.
  // If shortest is true, the ByteVector will be as short as possible (required
  // for the id)
  ByteVector createVInt(ulli number, bool addOne = true, bool shortest = true)
  {
    ByteVector vint = ByteVector::fromLongLong(static_cast<signed long long>(number));
    
    // Do we actually need to calculate the length of the variable length
    // integer? If not, then prepend the 0b0000 0001 if necessary and return the
    // vint.
    if(!shortest) {
      if(addOne)
        vint[0] = 1;
      return vint;
    }
    
    // Calculate the minimal length of the variable length integer
    uint byteSize = vint.size();
    for(uint i = 0; byteSize > 0 && vint[i] == 0; ++i)
      --byteSize;
    
    if(!addOne)
      return ByteVector(vint.data() + vint.size() - byteSize, byteSize);
    
    ulli firstByte = (1 << (vint.size() - byteSize));
    // The most significant byte loses #bytSize bits for storing information.
    // Therefore, we might need to increase byteSize.
    if(number >= (firstByte << (8 * (byteSize - 1))) && byteSize < vint.size())
      ++byteSize;
    // Add the one at the correct position
    uint firstBytePosition = vint.size() - byteSize;
    vint[firstBytePosition] |= (1 << firstBytePosition);
    return ByteVector(vint.data() + firstBytePosition, byteSize);
  }
  
  // Returns a void element within this element which is at least "least" in
  // size. Uses best fit method. Returns a null pointer if no suitable element
  // was found.
  Element *searchVoid(size_t least = 0L)
  {
    Element *currentBest = 0;
    for(List<Element *>::Iterator i = children.begin(); i != children.end(); ++i) {
      if((*i)->d->id == Void &&
        // We need room for the header if we don't remove the element.
        ((((*i)->d->size + (*i)->d->data - (*i)->d->position) == least || ((*i)->d->size >= least)) &&
        // best fit
        (!currentBest || (*i)->d->size < currentBest->d->size))
        ) {
        currentBest = *i;
      }
    }
    return currentBest;
  }
  
  // Replaces this element by a Void element. Returns true on success and false
  // on error.
  bool makeVoid()
  {
    ulli realSize = size + data - position;
    ByteVector header(createVInt(Void, false));
    ulli leftSize = realSize - (header.size() + sizeof(ulli));
    // Does not make sense to create a Void element
    if (leftSize > realSize)
      return false;
    header.append(createVInt(leftSize, true, false));
    // Write to file
    document->seek(position);
    document->writeBlock(header);
    // Update data
    data = position + header.size();
    size = leftSize;
    return true;
    
    // XXX: We actually should merge Voids, if possible.
  }
  
    // Reading constructor: Reads all unknown information from the file.
  ElementPrivate(File *p_document, Element *p_parent = 0, size_t p_position = 0) :
    id(0),
    position(p_position),
    data(0),
    populated(false),
    parent(p_parent),
    document(p_document)
  {
    if(parent) {
      ulli ssize;
      data = readVInt(readVInt(position, &id, false), &ssize);
      size = static_cast<size_t>(ssize);
    }
    else {
      document->seek(0, File::End);
      size = document->tell();
    }
  }
  
  // Writing constructor: Takes given information, calculates missing information
  // and writes everything to the file.
  // Tries to use void elements if available in the parent.
  ElementPrivate(ulli p_id, File *p_document, Element *p_parent,
    size_t p_position, size_t p_size) :
      id(p_id),
      position(p_position),
      size(p_size),
      populated(true), // It is a new element so we know, there are no children.
      parent(p_parent),
      document(p_document)
  {
    // header
    ByteVector content(createVInt(id, false).append(createVInt(size, true, false)));
    data = position + content.size();
    // space for children
    content.resize(data - position + size);
    
    Element *freeSpace;
    if (!(freeSpace = searchVoid(content.size()))) {
      // We have to make room
      document->insert(content, position);
      // Update parents
      for(Element *current = parent; current->d->parent; current = current->d->parent) {
        current->d->size += content.size();
        // Create new header and write it.
        ByteVector parentHeader(createVInt(current->d->id, false).append(createVInt(current->d->size, true, false)));
        uint oldHeaderSize = current->d->data - current->d->position;
        if(oldHeaderSize < parentHeader.size()) {
          ByteVector secondHeader(createVInt(current->d->id, false).append(createVInt(current->d->size)));
          if(oldHeaderSize == secondHeader.size()) {
            // Write the header where the old one was.
            document->seek(current->d->position);
            document->writeBlock(secondHeader);
            continue; // Very important here!
          }
        }
        // Insert the new header
        document->insert(parentHeader, current->d->position, oldHeaderSize);
        current->d->data = current->d->position + parentHeader.size();
      }
    }
    else {
      document->seek(freeSpace->d->position);
      if((freeSpace->d->size + freeSpace->d->data - freeSpace->d->position)
        == content.size()) {
        // Write to file
        document->writeBlock(content);
        // Update parent
        for(List<Element *>::Iterator i = parent->d->children.begin();
          i != parent->d->children.end(); ++i) {
          if(freeSpace == *i)
            parent->d->children.erase(i);
        }
        delete freeSpace;
      }
      else {
        ulli newSize = freeSpace->d->size - content.size();
        ByteVector newVoid(createVInt(Void, false).append(createVInt(newSize, true, false)));
        
        // Check if the original size of the size field was really 8 byte
        if (newVoid.size() != (freeSpace->d->data - freeSpace->d->position))
          newVoid = createVInt(Void, false).append(createVInt(newSize));
        // Update freeSpace
        freeSpace->d->size = newSize;
        freeSpace->d->data = freeSpace->d->position + newVoid.size();
        // Write to file
        document->writeBlock(newVoid.resize(newVoid.size() + newSize).append(content));
      }
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

EBML::Element::~Element()
{
  delete d;
}

EBML::Element::Element(EBML::File *document)
  : d(new EBML::Element::ElementPrivate(document))
{
}

EBML::Element *EBML::Element::getChild(EBML::ulli id)
{
  populate();
  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end();
    ++i) {
    if ((*i)->d->id == id)
      return *i;
  }
  return 0;
}

List<EBML::Element *> EBML::Element::getChildren(EBML::ulli id)
{
  populate();
  List<Element *> result;
  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end();
    ++i) {
    if ((*i)->d->id == id)
      result.append(*i);
  }
  return result;
}

List<EBML::Element *> EBML::Element::getChildren()
{
  populate();
  return d->children;
}

EBML::Element *EBML::Element::getParent()
{
  return d->parent;
}

ByteVector EBML::Element::getAsBinary()
{
  d->document->seek(d->data);
  return d->document->readBlock(d->size);
}

String EBML::Element::getAsString()
{
  return String(getAsBinary(), String::UTF8);
}

signed long long EBML::Element::getAsInt()
{
  // The debug note about returning 0 because of empty data is irrelevant. The
  // behavior is as expected.
  return getAsBinary().toLongLong();
}

EBML::ulli EBML::Element::getAsUnsigned()
{
  // The debug note about returning 0 because of empty data is irrelevant. The
  // behavior is as expected.
  return static_cast<ulli>(getAsBinary().toLongLong());
}

long double EBML::Element::getAsFloat()
{
  // Very dirty implementation!
  ByteVector bin = getAsBinary();
  uint size = bin.size();
  ulli sum = 0.0L;
  
  // For 0 byte floats and any float that is not defined in the ebml spec.
  if (size != 4 && size != 8 /*&& size() != 10*/) // XXX: Currently no support for 10 bit floats.
    return sum;
  
  // From toNumber; Might not be portable, since it requires IEEE floats.
  uint last = size - 1;
  for(uint i = 0; i <= last; i++)
    sum |= (ulli) uchar(bin[i]) << ((last - i) * 8);
  
  if (size == 4) {
    float result = *reinterpret_cast<float *>(&sum);
    return result;
  }
  else {
    double result = *reinterpret_cast<double *>(&sum);
    return result;
  }
}

EBML::Element *EBML::Element::addElement(EBML::ulli id)
{
  Element *elem = new Element(
    new ElementPrivate(id, d->document, this, d->data + d->size, 0)
  );
  d->children.append(elem);
  return elem;
}

EBML::Element *EBML::Element::addElement(EBML::ulli id, const ByteVector &binary)
{
  Element *elem = new Element(
    new ElementPrivate(id, d->document, this, d->data + d->size, binary.size())
  );
  d->document->seek(elem->d->data);
  d->document->writeBlock(binary);
  d->children.append(elem);
  return elem;
}

EBML::Element *EBML::Element::addElement(EBML::ulli id, const String &string)
{
  return addElement(id, string.data(String::UTF8));
}

EBML::Element *EBML::Element::addElement(EBML::ulli id, signed long long number)
{
  return addElement(id, ByteVector::fromLongLong(number));
}

EBML::Element *EBML::Element::addElement(EBML::ulli id, EBML::ulli number)
{
  return addElement(id, ByteVector::fromLongLong(static_cast<signed long long>(number)));
}

EBML::Element *EBML::Element::addElement(EBML::ulli id, long double number)
{
  // Probably, we will never need this method.
  return 0;
}

bool EBML::Element::removeChildren(EBML::ulli id, bool useVoid)
{
  bool result = false;
  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end(); ++i)
    if((*i)->d->id == id) {
      removeChild(*i, useVoid);
      result = true;
    }
  return result;
}

bool EBML::Element::removeChildren(bool useVoid)
{
  // Maybe a better implementation, because we probably create a lot of voids
  // in a row where a huge Void would be more appropriate.
  if (d->children.isEmpty())
    return false;
  
  for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end(); ++i)
    removeChild(*i, useVoid);
  return true;
}

bool EBML::Element::removeChild(Element *element, bool useVoid)
{
  if (!d->children.contains(element))
    return false;
  
  if(!useVoid || !element->d->makeVoid()) {
    d->document->removeBlock(element->d->position, element->d->size);
    // Update parents
    for(Element* current = this; current; current = current->d->parent)
      current->d->size -= element->d->size;
    // Update this element
    for(List<Element *>::Iterator i = d->children.begin(); i != d->children.end(); ++i)
      if(element == *i)
        d->children.erase(i);
    delete element;
  }
  return true;
}

void EBML::Element::setAsBinary(const ByteVector &binary)
{
  // Maybe: Search for void element after this one
  d->document->insert(binary, d->data, d->size);
}

void EBML::Element::setAsString(const String &string)
{
  setAsBinary(string.data(String::UTF8));
}

void EBML::Element::setAsInt(signed long long number)
{
  setAsBinary(ByteVector::fromLongLong(number));
}

void EBML::Element::setAsUnsigned(EBML::ulli number)
{
  setAsBinary(ByteVector::fromLongLong(static_cast<signed long long>(number)));
}

void EBML::Element::setAsFloat(long double)
{
  // Probably, we will never need this method.
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void EBML::Element::populate()
{
  if(!d->populated) {
    d->populated = true;
    size_t end = d->data + d->size;
    
    for(size_t i = d->data; i < end;) {
      Element *elem = new Element(
        new ElementPrivate(d->document, this, i)
      );
      d->children.append(elem);
      i = elem->d->data + elem->d->size;
    }
  }
}

EBML::Element::Element(EBML::Element::ElementPrivate *pe) : d(pe)
{}
