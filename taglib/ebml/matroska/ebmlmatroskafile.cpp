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

#include "ebmlmatroskaconstants.h"
#include "ebmlmatroskaaudio.h"

#include "tpropertymap.h"

using namespace TagLib;

class EBML::Matroska::File::FilePrivate
{
public:
  // Returns true if simpleTag has a TagName and TagString child and writes
  // their contents into name and value.
  bool extractContent(Element *simpleTag, String &name, String &value)
  {
    Element *n = simpleTag->getChild(Constants::TagName);
    Element *v = simpleTag->getChild(Constants::TagString);
    if(!n || !v)
      return false;
    
    name = n->getAsString();
    value = v->getAsString();
    return true;
  }
  
  FilePrivate(File *p_document) : tag(0), document(p_document)
  {
    // Just get the first segment, because "Typically a Matroska file is
    // composed of 1 segment."
    Element* elem = document->getDocumentRoot()->getChild(Constants::Segment);
    
    // We take the first tags element (there shouldn't be more), if there is
    // non such element, consider the file as not compatible.
    if(!elem || !(elem = elem->getChild(Constants::Tags))) {
      document->setValid(false);
      return;
    }
    
    // Load all Tag entries
    List<Element *> entries = elem->getChildren(Constants::Tag);
    for(List<Element *>::Iterator i = entries.begin(); i != entries.end(); ++i) {
      Element *target = (*i)->getChild(Constants::Targets);
      ulli ttvalue = 0;
      if(target && (target = (*i)->getChild(Constants::TargetTypeValue)))
        ttvalue = target->getAsUnsigned();
      
      // Load all SimpleTags
      PropertyMap tagEntries;
      List<Element *> simpleTags = (*i)->getChildren(Constants::SimpleTag);
      for(List<Element *>::Iterator j = simpleTags.begin(); j != simpleTags.end();
        ++j) {
        String name, value;
        if(!extractContent(*j, name, value))
          continue;
        tagEntries.insert(name, StringList(value));
      }
      
      tags.append(std::pair<PropertyMap, std::pair<Element *, ulli> >(tagEntries, std::pair<Element *, ulli>(*i, ttvalue)));
    }
  }
  
  // Creates Tag and AudioProperties. Late creation because both require a fully
  // functional FilePrivate (well AudioProperties doesn't...)
  void lateCreate()
  {
    tag = new Tag(document);
    audio = new AudioProperties(document);
  }
  
  // Checks the EBML header and creates the FilePrivate.
  static FilePrivate *checkAndCreate(File *document)
  {
    Element *elem = document->getDocumentRoot()->getChild(Header::EBML);
    Element *child = elem->getChild(Header::DocType);
    if(child) {
      String dt = child->getAsString();
      if (dt == Constants::DocTypeMatroska || dt == Constants::DocTypeWebM) {
        FilePrivate *fp = new FilePrivate(document);
        return fp;
      }
    }
    
    return 0;
  }
  
  // The tags with their Element and TargetTypeValue
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > > tags;
  
  // The tag
  Tag *tag;
  
  // The audio properties
  AudioProperties *audio;
  
  // The corresponding file.
  File *document;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

EBML::Matroska::File::~File()
{
  delete d->tag;
  delete d->audio;
  delete d;
}

EBML::Matroska::File::File(FileName file) : EBML::File(file), d(0)
{
  if(isValid() && isOpen()) {
    d = FilePrivate::checkAndCreate(this);
    if(!d)
      setValid(false);
    else
    d->lateCreate();
  }
}

EBML::Matroska::File::File(IOStream *stream) : EBML::File(stream), d(0)
{
  if(isValid() && isOpen()) {
    d = FilePrivate::checkAndCreate(this);
    if(!d)
      setValid(false);
    else
      d->lateCreate();
  }
}

Tag *EBML::Matroska::File::tag() const
{
  return d->tag;
}

PropertyMap EBML::Matroska::File::properties() const
{
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator best = d->tags.end();
  for(List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator i = d->tags.begin();
    i != d->tags.end(); ++i) {
    if(best == d->tags.end() || best->second.second > i->second.second)
      best = i;
  }
  return best->first;
}

PropertyMap EBML::Matroska::File::setProperties(const PropertyMap &properties)
{
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator best = d->tags.end();
  for(List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator i = d->tags.begin();
    i != d->tags.end(); ++i) {
    if(best == d->tags.end() || best->second.second > i->second.second)
      best = i;
  }
  
  std::pair<PropertyMap, std::pair<Element *, ulli> > replace(properties, best->second);
  d->tags.erase(best);
  d->tags.prepend(replace);
  
  return PropertyMap();
}

AudioProperties *EBML::Matroska::File::audioProperties() const
{
  return d->audio;
}

bool EBML::Matroska::File::save()
{
  if(readOnly())
    return false;
  
  // C++11 features would be nice: for(auto &i : d->tags) { /* ... */ }
  // Well, here we just iterate over each extracted element.
  for(List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator i = d->tags.begin();
    i != d->tags.end(); ++i) {
    
    for(PropertyMap::Iterator j = i->first.begin(); j != i->first.end(); ++j) {
      
      // No element? Create it!
      if(!i->second.first) {
        // Should be save, since we already checked, when creating the object.
        Element *container = d->document->getDocumentRoot()
          ->getChild(Constants::Segment)->getChild(Constants::Tags);
        
        // Create Targets container
        i->second.first = container->addElement(Constants::Tag);
        Element *target = i->second.first->addElement(Constants::Targets);
        
        if(i->second.second == Constants::MostCommonPartValue)
          target->addElement(Constants::TargetType, Constants::TRACK);
        else if(i->second.second == Constants::MostCommonGroupingValue)
          target->addElement(Constants::TargetType, Constants::ALBUM);
          
        target->addElement(Constants::TargetTypeValue, i->second.second);
      }
      
      // Find entries
      List<Element *> simpleTags = i->second.first->getChildren(Constants::SimpleTag);
      StringList::Iterator str = j->second.begin();
      List<Element *>::Iterator k = simpleTags.begin();
      for(; k != simpleTags.end(); ++k) {
        
        String name, value;
        if(!d->extractContent(*k, name, value))
          continue;
        
        // Write entry from StringList
        if(name == j->first) {
          if(str == j->second.end()) {
            // We have all StringList elements but still found another element
            // with the same name? Let's delete it!
            i->second.first->removeChild(*k);
          }
          else {
             if(value != *str) {
              // extractContent already checked for availability
              (*k)->getChild(Constants::TagString)->setAsString(*str);
            }
            ++str;
          }
        }
      }
      
      // If we didn't write the complete StringList, we have to write the rest.
      for(; str != j->second.end(); ++str) {
        Element *stag = i->second.first->addElement(Constants::SimpleTag);
        stag->addElement(Constants::TagName, j->first);
        stag->addElement(Constants::TagString, *str);
      }
    }
    
    // Finally, we have to find elements that are not in the PropertyMap and
    // remove them.
    List<Element *> simpleTags = i->second.first->getChildren(Constants::SimpleTag);
    for(List<Element *>::Iterator j = simpleTags.begin(); j != simpleTags.end(); ++j) {
      
      String name, value;
      if(!d->extractContent(*j, name, value))
        continue;
      
      if(i->first.find(name) == i->first.end()){
        i->second.first->removeChild(*j);}
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// Tag
//
////////////////////////////////////////////////////////////////////////////////


class EBML::Matroska::File::Tag::TagPrivate
{
public:
  // Creates a TagPrivate instance
  TagPrivate(File *p_document) :
    document(p_document),
    title(document->d->tags.end()),
    artist(document->d->tags.end()),
    album(document->d->tags.end()),
    comment(document->d->tags.end()),
    genre(document->d->tags.end()),
    year(document->d->tags.end()),
    track(document->d->tags.end())
  {
    
    for(List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator i =
      document->d->tags.begin(); i != document->d->tags.end(); ++i) {
      
      // Just save it, if the title is more specific, or there is no title yet.
      if(i->first.find(Constants::TITLE) != i->first.end() &&
        (title == document->d->tags.end() ||
        title->second.second > i->second.second ||
        i->second.second == Constants::MostCommonPartValue)) {
        
        title = i;
      }
      
      // Same goes for artist.
      if(i->first.find(Constants::ARTIST) != i->first.end() &&
        (artist == document->d->tags.end() ||
        artist->second.second > i->second.second ||
        i->second.second == Constants::MostCommonPartValue)) {
        
        artist = i;
      }
      
      // Here, we also look for a title (the album title), but since we
      // specified the granularity, we have to search for it exactly.
      // Therefore it is possible, that title and album are the same (if only
      // the title of the album is given).
      if(i->first.find(Constants::TITLE) != i->first.end() &&
        i->second.second == Constants::MostCommonGroupingValue) {
        
        album = i;
      }
      
      // Again the same as title and artist.
      if(i->first.find(Constants::COMMENT) != i->first.end() &&
        (comment == document->d->tags.end() ||
        comment->second.second > i->second.second ||
        i->second.second == Constants::MostCommonPartValue)) {
        
        comment = i;
      }
      
      // Same goes for genre.
      if(i->first.find(Constants::GENRE) != i->first.end() &&
        (genre == document->d->tags.end() ||
        genre->second.second > i->second.second ||
        i->second.second == Constants::MostCommonPartValue)) {
        
        genre = i;
      }
      
      // And year (in our case: DATE_REALEASE)
      if(i->first.find(Constants::DATE_RELEASE) != i->first.end() &&
        (year == document->d->tags.end() ||
        year->second.second > i->second.second ||
        i->second.second == Constants::MostCommonPartValue)) {
        
        year = i;
      }
      
      // And track (in our case: PART_NUMBER)
      if(i->first.find(Constants::PART_NUMBER) != i->first.end() &&
        (track == document->d->tags.end() ||
        track->second.second > i->second.second ||
        i->second.second == Constants::MostCommonPartValue)) {
        
        track = i;
      }
    }
  }
  
  // Searches for the Tag with given TargetTypeValue (returns the first one)
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator
  find(ulli ttv)
  {
    for(List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator i =
      document->d->tags.begin(); i != document->d->tags.end(); ++i) {
      
      if(i->second.second == ttv)
        return i;
    }
  }
  
  // Updates the given information
  void update(
    List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator t,
    const String &tagname,
    const String &s
    )
  {
    t->first.find(tagname)->second.front() = s;
  }
  
  // Inserts a tag with given information
  void insert(const String &tagname, const ulli ttv, const String &s)
  {
    for(List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator i =
      document->d->tags.begin(); i != document->d->tags.end(); ++i) {
      
      if(i->second.second == ttv) {
        i->first.insert(tagname, StringList(s));
        return;
      }
    }
    
    // Not found? Create new!
    PropertyMap pm;
    pm.insert(tagname, StringList(s));
    document->d->tags.append(
      std::pair<PropertyMap, std::pair<Element *, ulli> >(pm,
        std::pair<Element *, ulli>(0, ttv)
      )
    );
  }
  
  // The PropertyMap from the Matroska::File
  File *document;
  
  // Iterators to the tags.
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator title;
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator artist;
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator album;
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator comment;
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator genre;
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator year;
  List<std::pair<PropertyMap, std::pair<Element *, ulli> > >::Iterator track;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

EBML::Matroska::File::Tag::~Tag()
{
  delete e;
}

EBML::Matroska::File::Tag::Tag(EBML::Matroska::File *document) :
  e(new EBML::Matroska::File::Tag::TagPrivate(document))
{
}

String EBML::Matroska::File::Tag::title() const
{
  if(e->title != e->document->d->tags.end())
    return e->title->first.find(Constants::TITLE)->second.front();
  else
    return String::null;
}

String EBML::Matroska::File::Tag::artist() const
{
  if(e->artist != e->document->d->tags.end())
    return e->artist->first.find(Constants::ARTIST)->second.front();
  else
    return String::null;
}

String EBML::Matroska::File::Tag::album() const
{
  if(e->album != e->document->d->tags.end())
    return e->album->first.find(Constants::TITLE)->second.front();
  else
    return String::null;
}

String EBML::Matroska::File::Tag::comment() const
{
  if(e->comment != e->document->d->tags.end())
    return e->comment->first.find(Constants::COMMENT)->second.front();
  else
    return String::null;
}

String EBML::Matroska::File::Tag::genre() const
{
  if(e->genre != e->document->d->tags.end())
    return e->genre->first.find(Constants::GENRE)->second.front();
  else
    return String::null;
}

uint EBML::Matroska::File::Tag::year() const
{
  if(e->year != e->document->d->tags.end())
    return e->year->first.find(Constants::DATE_RELEASE)->second.front().toInt();
  else
    return 0;
}

uint EBML::Matroska::File::Tag::track() const
{
  if(e->track != e->document->d->tags.end())
    return e->track->first.find(Constants::PART_NUMBER)->second.front().toInt();
  else
    return 0;
}

void EBML::Matroska::File::Tag::setTitle(const String &s)
{
  if(e->title != e->document->d->tags.end())
    e->update(e->title, Constants::TITLE, s);
  else
    e->insert(Constants::TITLE, Constants::MostCommonPartValue, s);
}

void EBML::Matroska::File::Tag::setArtist(const String &s)
{
  if(e->artist != e->document->d->tags.end())
    e->update(e->artist, Constants::ARTIST, s);
  else
    e->insert(Constants::ARTIST, Constants::MostCommonPartValue, s);
}

void EBML::Matroska::File::Tag::setAlbum(const String &s)
{
  if(e->album != e->document->d->tags.end())
    e->update(e->album, Constants::TITLE, s);
  else
    e->insert(Constants::TITLE, Constants::MostCommonGroupingValue, s);
}

void EBML::Matroska::File::Tag::setComment(const String &s)
{
  if(e->comment != e->document->d->tags.end())
    e->update(e->comment, Constants::COMMENT, s);
  else
    e->insert(Constants::COMMENT, Constants::MostCommonPartValue, s);
}

void EBML::Matroska::File::Tag::setGenre(const String &s)
{
  if(e->genre != e->document->d->tags.end())
    e->update(e->genre, Constants::GENRE, s);
  else
    e->insert(Constants::GENRE, Constants::MostCommonPartValue, s);
}

void EBML::Matroska::File::Tag::setYear(uint i)
{
  String s = String::number(i);
  if(e->year != e->document->d->tags.end())
    e->update(e->year, Constants::DATE_RELEASE, s);
  else
    e->insert(Constants::DATE_RELEASE, Constants::MostCommonPartValue, s);
}

void EBML::Matroska::File::Tag::setTrack(uint i)
{
  String s = String::number(i);
  if(e->track != e->document->d->tags.end())
    e->update(e->track, Constants::PART_NUMBER, s);
  else
    e->insert(Constants::PART_NUMBER, Constants::MostCommonPartValue, s);
}
