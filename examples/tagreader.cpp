/* Copyright (C) 2003 Scott Wheeler <wheeler@kde.org>
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
#include <cstdio>

#include "tpropertymap.h"
#include "tstringlist.h"
#include "tvariant.h"
#include "fileref.h"
#include "tag.h"

int main(int argc, char *argv[])
{
  for(int i = 1; i < argc; i++) {

    std::cout << "******************** \"" << argv[i] << "\" ********************" << std::endl;

    TagLib::FileRef f(argv[i]);

    if(!f.isNull() && f.tag()) {

      TagLib::Tag *tag = f.tag();

      std::cout << "-- TAG (basic) --" << std::endl;
      std::cout << "title   - \"" << tag->title()   << "\"" << std::endl;
      std::cout << "artist  - \"" << tag->artist()  << "\"" << std::endl;
      std::cout << "album   - \"" << tag->album()   << "\"" << std::endl;
      std::cout << "year    - \"" << tag->year()    << "\"" << std::endl;
      std::cout << "comment - \"" << tag->comment() << "\"" << std::endl;
      std::cout << "track   - \"" << tag->track()   << "\"" << std::endl;
      std::cout << "genre   - \"" << tag->genre()   << "\"" << std::endl;

      TagLib::PropertyMap tags = f.properties();
      if(!tags.isEmpty()) {
        unsigned int longest = 0;
        for(auto j = tags.cbegin(); j != tags.cend(); ++j) {
          if (j->first.size() > longest) {
            longest = j->first.size();
          }
        }

        std::cout << "-- TAG (properties) --" << std::endl;
        for(auto j = tags.cbegin(); j != tags.cend(); ++j) {
          for(auto k = j->second.begin(); k != j->second.end(); ++k) {
            std::cout << std::left << std::setfill(' ') << std::setw(longest) << j->first << " - " << '"' << *k << '"' << std::endl;
          }
        }
      }

      TagLib::StringList names = f.complexPropertyKeys();
      for(const auto &name : names) {
        const auto& properties = f.complexProperties(name);
        for(const auto &property : properties) {
          std::cout << name << ":" << std::endl;
          for(const auto &[key, value] : property) {
            std::cout << "  " << std::left << std::setfill(' ') << std::setw(11) << key << " - ";
            if(value.type() == TagLib::Variant::ByteVector) {
              std::cout << "(" << value.value<TagLib::ByteVector>().size() << " bytes)" << std::endl;
              /* The picture could be extracted using:
              std::ofstream picture;
              TagLib::String fn(argv[i]);
              int slashPos = fn.rfind('/');
              int dotPos = fn.rfind('.');
              if(slashPos >= 0 && dotPos > slashPos) {
                fn = fn.substr(slashPos + 1, dotPos - slashPos - 1);
              }
              fn += ".jpg";
              picture.open(fn.toCString(), std::ios_base::out | std::ios_base::binary);
              picture << value.value<TagLib::ByteVector>();
              picture.close();
              */
            }
            else {
              std::cout << value << std::endl;
            }
          }
        }
      }
    }

    if(!f.isNull() && f.audioProperties()) {

      TagLib::AudioProperties *properties = f.audioProperties();

      int seconds = properties->lengthInSeconds() % 60;
      int minutes = (properties->lengthInSeconds() - seconds) / 60;

      std::cout << "-- AUDIO --" << std::endl;
      std::cout << "bitrate     - " << properties->bitrate() << std::endl;
      std::cout << "sample rate - " << properties->sampleRate() << std::endl;
      std::cout << "channels    - " << properties->channels() << std::endl;
      std::cout << "length      - " << minutes << ":" << std::setfill('0') << std::setw(2) << std::right << seconds << std::endl;
    }
  }
  return 0;
}

