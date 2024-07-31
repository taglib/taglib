/* Copyright (C) 2004 Scott Wheeler <wheeler@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

#include "tlist.h"
#include "tfile.h"
#include "tpropertymap.h"
#include "tvariant.h"
#include "fileref.h"
#include "tag.h"

bool isArgument(const char *s)
{
  return strlen(s) == 2 && s[0] == '-';
}

bool isFile(const char *s)
{
#ifdef _WIN32
  struct _stat64 st;
  return ::_stat64(s, &st) == 0 && (st.st_mode & S_IFREG);
#else
  struct stat st;
  return ::stat(s, &st) == 0 && (st.st_mode & (S_IFREG | S_IFLNK));
#endif
}

void usage()
{
  std::cout << std::endl;
  std::cout << "Usage: tagwriter <fields> <files>" << std::endl;
  std::cout << std::endl;
  std::cout << "Where the valid fields are:" << std::endl;
  std::cout << "  -t <title>"   << std::endl;
  std::cout << "  -a <artist>"  << std::endl;
  std::cout << "  -A <album>"   << std::endl;
  std::cout << "  -c <comment>" << std::endl;
  std::cout << "  -g <genre>"   << std::endl;
  std::cout << "  -y <year>"    << std::endl;
  std::cout << "  -T <track>"   << std::endl;
  std::cout << "  -R <tagname> <tagvalue>"   << std::endl;
  std::cout << "  -I <tagname> <tagvalue>"   << std::endl;
  std::cout << "  -D <tagname>"   << std::endl;
  std::cout << "  -p <picturefile> <description> (\"\" \"\" to remove)" << std::endl;
  std::cout << std::endl;

  exit(1);
}

void checkForRejectedProperties(const TagLib::PropertyMap &tags)
{ // stolen from tagreader.cpp
  if(tags.size() > 0) {
    unsigned int longest = 0;
    for(auto i = tags.begin(); i != tags.end(); ++i) {
      if(i->first.size() > longest) {
        longest = i->first.size();
      }
    }
    std::cout << "-- rejected TAGs (properties) --" << std::endl;
    for(auto i = tags.begin(); i != tags.end(); ++i) {
      for(auto j = i->second.begin(); j != i->second.end(); ++j) {
        std::cout << std::left << std::setw(longest) << i->first << " - " << '"' << *j << '"' << std::endl;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  TagLib::List<TagLib::FileRef> fileList;

  while(argc > 0 && isFile(argv[argc - 1])) {

    TagLib::FileRef f(argv[argc - 1]);

    if(!f.isNull() && f.tag())
      fileList.append(f);

    argc--;
  }

  if(fileList.isEmpty())
    usage();

  int i = 1;
  while(i < argc - 1) {

    if(isArgument(argv[i]) && i + 1 < argc && !isArgument(argv[i + 1])) {

      char field = argv[i][1];
      TagLib::String value = argv[i + 1];
      int numArgsConsumed = 2;

      for(auto &f : fileList) {

        TagLib::Tag *t = f.tag();

        switch (field) {
        case 't':
          t->setTitle(value);
          break;
        case 'a':
          t->setArtist(value);
          break;
        case 'A':
          t->setAlbum(value);
          break;
        case 'c':
          t->setComment(value);
          break;
        case 'g':
          t->setGenre(value);
          break;
        case 'y':
          t->setYear(value.toInt());
          break;
        case 'T':
          t->setTrack(value.toInt());
          break;
        case 'R':
        case 'I':
          if(i + 2 < argc) {
            TagLib::PropertyMap map = f.properties();
            if(field == 'R') {
              map.replace(value, TagLib::String(argv[i + 2]));
            }
            else {
              map.insert(value, TagLib::String(argv[i + 2]));
            }
            numArgsConsumed = 3;
            checkForRejectedProperties(f.setProperties(map));
          }
          else {
            usage();
          }
          break;
        case 'D': {
          TagLib::PropertyMap map = f.properties();
          map.erase(value);
          checkForRejectedProperties(f.setProperties(map));
          break;
        }
        case 'p': {
          if(i + 2 < argc) {
            numArgsConsumed = 3;
            if(!value.isEmpty()) {
              if(!isFile(value.toCString())) {
                std::cout << value.toCString() << " not found." << std::endl;
                return 1;
              }
              std::ifstream picture;
              picture.open(value.toCString(), std::ios::in | std::ios::binary);
              std::stringstream buffer;
              buffer << picture.rdbuf();
              picture.close();
              TagLib::String buf(buffer.str());
              TagLib::ByteVector data(buf.data(TagLib::String::Latin1));
              TagLib::String mimeType = data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")
                ? "image/png" : "image/jpeg";
              TagLib::String description(argv[i + 2]);
              f.setComplexProperties("PICTURE", {
                {
                  {"data", data},
                  {"pictureType", "Front Cover"},
                  {"mimeType", mimeType},
                  {"description", description}
                }
              });
            }
            else {
              // empty value, remove pictures
              f.setComplexProperties("PICTURE", {});
            }
          }
          else {
            usage();
          }
          break;
        }
        default:
          usage();
          break;
        }
      }
      i += numArgsConsumed;
    }
    else
      usage();
  }

  for(auto &f : fileList)
    f.save();

  return 0;
}
