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

using namespace std;

int main(int argc, char *argv[])
{
  for(int i = 1; i < argc; i++) {

    cout << "******************** \"" << argv[i] << "\" ********************" << endl;

    TagLib::FileRef f(argv[i]);

    if(!f.isNull() && f.tag()) {

      TagLib::Tag *tag = f.tag();

      cout << "-- TAG (basic) --" << endl;
      cout << "title   - \"" << tag->title()   << "\"" << endl;
      cout << "artist  - \"" << tag->artist()  << "\"" << endl;
      cout << "album   - \"" << tag->album()   << "\"" << endl;
      cout << "year    - \"" << tag->year()    << "\"" << endl;
      cout << "comment - \"" << tag->comment() << "\"" << endl;
      cout << "track   - \"" << tag->track()   << "\"" << endl;
      cout << "genre   - \"" << tag->genre()   << "\"" << endl;

      TagLib::PropertyMap tags = f.properties();
      if(!tags.isEmpty()) {
        unsigned int longest = 0;
        for(auto j = tags.cbegin(); j != tags.cend(); ++j) {
          if (j->first.size() > longest) {
            longest = j->first.size();
          }
        }

        cout << "-- TAG (properties) --" << endl;
        for(auto j = tags.cbegin(); j != tags.cend(); ++j) {
          for(auto k = j->second.begin(); k != j->second.end(); ++k) {
            cout << left << std::setfill(' ') << std::setw(longest) << j->first << " - " << '"' << *k << '"' << endl;
          }
        }
      }

      TagLib::StringList names = f.complexPropertyKeys();
      for(const auto &name : names) {
        const auto& properties = f.complexProperties(name);
        for(const auto &property : properties) {
          cout << name << ":" << endl;
          for(const auto &[key, value] : property) {
            cout << "  " << left << std::setfill(' ') << std::setw(11) << key << " - ";
            if(value.type() == TagLib::Variant::ByteVector) {
              cout << "(" << value.value<TagLib::ByteVector>().size() << " bytes)" << endl;
              /* The picture could be extracted using:
              ofstream picture;
              TagLib::String fn(argv[i]);
              int slashPos = fn.rfind('/');
              int dotPos = fn.rfind('.');
              if(slashPos >= 0 && dotPos > slashPos) {
                fn = fn.substr(slashPos + 1, dotPos - slashPos - 1);
              }
              fn += ".jpg";
              picture.open(fn.toCString(), ios_base::out | ios_base::binary);
              picture << value.value<TagLib::ByteVector>();
              picture.close();
              */
            }
            else {
              cout << value << endl;
            }
          }
        }
      }
    }

    if(!f.isNull() && f.audioProperties()) {

      TagLib::AudioProperties *properties = f.audioProperties();

      int seconds = properties->lengthInSeconds() % 60;
      int minutes = (properties->lengthInSeconds() - seconds) / 60;

      cout << "-- AUDIO --" << endl;
      cout << "bitrate     - " << properties->bitrate() << endl;
      cout << "sample rate - " << properties->sampleRate() << endl;
      cout << "channels    - " << properties->channels() << endl;
      cout << "length      - " << minutes << ":" << setfill('0') << setw(2) << right << seconds << endl;
    }
  }
  return 0;
}
