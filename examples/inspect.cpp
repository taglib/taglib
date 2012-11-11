/* Copyright (C) 2012 Lukas Lalinsky <lalinsky@gmail.com>
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
#include <stdlib.h>

#include <fileref.h>
#include <mp4file.h>

using namespace std;
using namespace TagLib;

#define MAYBE_PRINT_DESC(_Type) \
    if(dynamic_cast<_Type *>(f.file())) { \
      cout << dynamic_cast<_Type *>(f.file())->toString().to8Bit(true) << endl; \
      found = 1; \
    }

int main(int argc, char *argv[])
{
  // process the command line args

  for(int i = 1; i < argc; i++) {

    cout << "******************** \"" << argv[i] << "\"********************" << endl;

    FileRef f(argv[i]);

    bool found = 0;
    if(!f.isNull() && f.file()) {
      MAYBE_PRINT_DESC(MP4::File);
      MAYBE_PRINT_DESC(File);
    }

    if(!found) {
      cout << "could not find any information about the file" << endl;
    }

  }
}
