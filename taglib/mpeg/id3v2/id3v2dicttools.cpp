/***************************************************************************
    copyright            : (C) 2011 by Michael Helmling
    email                : supermihi@web.de
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
#include "tdebug.h"
#include "id3v2dicttools.h"
#include "tmap.h"
namespace TagLib {
  namespace ID3v2 {

    /*!
     * A map of translations frameID <-> tag used by the unified dictionary interface.
     */
    static const uint numid3frames = 54;
    static const char *id3frames[][2] = {
      // Text information frames
      { "TALB", "ALBUM"},
      { "TBPM", "BPM" },
      { "TCOM", "COMPOSER" },
      { "TCON", "GENRE" },
      { "TCOP", "COPYRIGHT" },
      { "TDEN", "ENCODINGTIME" },
      { "TDLY", "PLAYLISTDELAY" },
      { "TDOR", "ORIGINALRELEASETIME" },
      { "TDRC", "DATE" },
      // { "TRDA", "DATE" }, // id3 v2.3, replaced by TDRC in v2.4
      // { "TDAT", "DATE" }, // id3 v2.3, replaced by TDRC in v2.4
      // { "TYER", "DATE" }, // id3 v2.3, replaced by TDRC in v2.4
      // { "TIME", "DATE" }, // id3 v2.3, replaced by TDRC in v2.4
      { "TDRL", "RELEASETIME" },
      { "TDTG", "TAGGINGTIME" },
      { "TENC", "ENCODEDBY" },
      { "TEXT", "LYRICIST" },
      { "TFLT", "FILETYPE" },
      { "TIPL", "INVOLVEDPEOPLE" },
      { "TIT1", "CONTENTGROUP" },
      { "TIT2", "TITLE"},
      { "TIT3", "SUBTITLE" },
      { "TKEY", "INITIALKEY" },
      { "TLAN", "LANGUAGE" },
      { "TLEN", "LENGTH" },
      { "TMCL", "MUSICIANCREDITS" },
      { "TMED", "MEDIATYPE" },
      { "TMOO", "MOOD" },
      { "TOAL", "ORIGINALALBUM" },
      { "TOFN", "ORIGINALFILENAME" },
      { "TOLY", "ORIGINALLYRICIST" },
      { "TOPE", "ORIGINALARTIST" },
      { "TOWN", "OWNER" },
      { "TPE1", "ARTIST"},
      { "TPE2", "PERFORMER" },
      { "TPE3", "CONDUCTOR" },
      { "TPE4", "ARRANGER" },
      { "TPOS", "DISCNUMBER" },
      { "TPRO", "PRODUCEDNOTICE" },
      { "TPUB", "PUBLISHER" },
      { "TRCK", "TRACKNUMBER" },
      { "TRSN", "RADIOSTATION" },
      { "TRSO", "RADIOSTATIONOWNER" },
      { "TSOA", "ALBUMSORT" },
      { "TSOP", "ARTISTSORT" },
      { "TSOT", "TITLESORT" },
      { "TSRC", "ISRC" },
      { "TSSE", "ENCODING" },

      // URL frames
      { "WCOP", "COPYRIGHTURL" },
      { "WOAF", "FILEWEBPAGE" },
      { "WOAR", "ARTISTWEBPAGE" },
      { "WOAS", "AUDIOSOURCEWEBPAGE" },
      { "WORS", "RADIOSTATIONWEBPAGE" },
      { "WPAY", "PAYMENTWEBPAGE" },
      { "WPUB", "PUBLISHERWEBPAGE" },
      { "WXXX", "URL"},

      // Other frames
      { "COMM", "COMMENT" },
      { "USLT", "LYRICS" },
    };

    // list of frameIDs that are ignored by the unified dictionary interface
    static const uint ignoredFramesSize = 7;
    static const char *ignoredFrames[] = {
      "TCMP", // illegal 'Part of Compilation' frame set by iTunes (see http://www.id3.org/Compliance_Issues)
      "GEOB", // no way to handle a general encapsulated object by the dict interface
      "PRIV", // private frames
      "APIC", // attached picture -- TODO how could we do this?
      "POPM", // popularimeter
      "RVA2", // relative volume
      "UFID", // unique file identifier
    };

    // list of deprecated frames and their successors
    static const uint deprecatedFramesSize = 4;
    static const char *deprecatedFrames[][2] = {
      {"TRDA", "TDRC"}, // 2.3 -> 2.4 (http://en.wikipedia.org/wiki/ID3)
      {"TDAT", "TDRC"}, // 2.3 -> 2.4
      {"TYER", "TDRC"}, // 2.3 -> 2.4
      {"TIME", "TDRC"}, // 2.3 -> 2.4
    };

    String frameIDToTagName(const ByteVector &id) {
      static Map<ByteVector, String> m;
      if (m.isEmpty())
        for (size_t i = 0; i < numid3frames; ++i)
          m[id3frames[i][0]] = id3frames[i][1];

      if (m.contains(id))
        return m[id];
      if (deprecationMap().contains(id))
        return m[deprecationMap()[id]];
      debug("unknown frame ID: " + id);
      return "UNKNOWNID3TAG"; //TODO: implement this nicer
    }

    ByteVector tagNameToFrameID(const String &s) {
      static Map<String, ByteVector> m;
      if (m.isEmpty())
        for (size_t i = 0; i < numid3frames; ++i)
          m[id3frames[i][1]] = id3frames[i][0];
      if (m.contains(s.upper()))
        return m[s];
      return "TXXX";
    }

    bool isIgnored(const ByteVector& id) {
      List<ByteVector> ignoredList;
      if (ignoredList.isEmpty())
        for (uint i = 0; i < ignoredFramesSize; ++i)
          ignoredList.append(ignoredFrames[i]);
      return ignoredList.contains(id);
    }

    FrameIDMap deprecationMap() {
      static FrameIDMap depMap;
      if (depMap.isEmpty())
        for(uint i = 0; i < deprecatedFramesSize; ++i)
          depMap[deprecatedFrames[i][0]] = deprecatedFrames[i][1];
      return depMap;
    }

    bool isDeprecated(const ByteVector& id) {
      return deprecationMap().contains(id);
    }
  }
}
