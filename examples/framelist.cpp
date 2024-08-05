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
#include <cstdlib>

#include "tbytevector.h"
#include "mpegfile.h"
#include "id3v2tag.h"
#include "id3v2frame.h"
#include "id3v2header.h"
#include "commentsframe.h"
#include "id3v1tag.h"
#include "apetag.h"

using namespace TagLib;

int main(int argc, char *argv[])
{
  // process the command line args


  for(int i = 1; i < argc; i++) {

    std::cout << "******************** \"" << argv[i] << "\"********************" << std::endl;

    MPEG::File f(argv[i]);

    ID3v2::Tag *id3v2tag = f.ID3v2Tag();

    if(id3v2tag) {

      std::cout << "ID3v2."
           << id3v2tag->header()->majorVersion()
           << "."
           << id3v2tag->header()->revisionNumber()
           << ", "
           << id3v2tag->header()->tagSize()
           << " bytes in tag"
           << std::endl;

      const auto &frames = id3v2tag->frameList();
      for(auto it = frames.begin(); it != frames.end(); it++) {
        std::cout << (*it)->frameID();

        if(auto comment = dynamic_cast<ID3v2::CommentsFrame *>(*it))
          if(!comment->description().isEmpty())
            std::cout << " [" << comment->description() << "]";

        std::cout << " - \"" << (*it)->toString() << "\"" << std::endl;
      }
    }
    else
      std::cout << "file does not have a valid id3v2 tag" << std::endl;

    std::cout << std::endl << "ID3v1" << std::endl;

    ID3v1::Tag *id3v1tag = f.ID3v1Tag();

    if(id3v1tag) {
      std::cout << "title   - \"" << id3v1tag->title()   << "\"" << std::endl;
      std::cout << "artist  - \"" << id3v1tag->artist()  << "\"" << std::endl;
      std::cout << "album   - \"" << id3v1tag->album()   << "\"" << std::endl;
      std::cout << "year    - \"" << id3v1tag->year()    << "\"" << std::endl;
      std::cout << "comment - \"" << id3v1tag->comment() << "\"" << std::endl;
      std::cout << "track   - \"" << id3v1tag->track()   << "\"" << std::endl;
      std::cout << "genre   - \"" << id3v1tag->genre()   << "\"" << std::endl;
    }
    else
      std::cout << "file does not have a valid id3v1 tag" << std::endl;

    APE::Tag *ape = f.APETag();

    std::cout << std::endl << "APE" << std::endl;

    if(ape) {
      const auto &items = ape->itemListMap();
      for(auto it = items.begin(); it != items.end(); ++it)
      {
        if((*it).second.type() != APE::Item::Binary)
          std::cout << (*it).first << " - \"" << (*it).second.toString() << "\"" << std::endl;
        else
          std::cout << (*it).first << " - Binary data (" << (*it).second.binaryData().size() << " bytes)" << std::endl;
      }
    }
    else
      std::cout << "file does not have a valid APE tag" << std::endl;

    std::cout << std::endl;
  }
}
