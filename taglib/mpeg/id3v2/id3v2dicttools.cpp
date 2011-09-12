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

#include "frames/textidentificationframe.h"
#include "frames/commentsframe.h"
#include "frames/urllinkframe.h"
#include "frames/uniquefileidentifierframe.h"
#include "frames/unsynchronizedlyricsframe.h"
#include "id3v1genres.h"

namespace TagLib {
  namespace ID3v2 {

    // list of deprecated frames and their successors
    static const uint deprecatedFramesSize = 4;
    static const char *deprecatedFrames[][2] = {
      {"TRDA", "TDRC"}, // 2.3 -> 2.4 (http://en.wikipedia.org/wiki/ID3)
      {"TDAT", "TDRC"}, // 2.3 -> 2.4
      {"TYER", "TDRC"}, // 2.3 -> 2.4
      {"TIME", "TDRC"}, // 2.3 -> 2.4
    };

    FrameIDMap &deprecationMap()
    {
      static FrameIDMap depMap;
      if (depMap.isEmpty())
        for(uint i = 0; i < deprecatedFramesSize; ++i)
          depMap[deprecatedFrames[i][0]] = deprecatedFrames[i][1];
      return depMap;
    }

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

    Map<ByteVector, String> &idMap()
    {
      static Map<ByteVector, String> m;
        if (m.isEmpty())
          for (size_t i = 0; i < numid3frames; ++i)
            m[id3frames[i][0]] = id3frames[i][1];
      return m;
    }

    String frameIDToTagName(const ByteVector &id)
    {
      Map<ByteVector, String> &m = idMap();
      if (m.contains(id))
        return m[id];
      if (deprecationMap().contains(id))
        return m[deprecationMap()[id]];
      debug("unknown frame ID in frameIDToTagName(): " + id);
      return "UNKNOWNID3TAG#" + String(id) + "#"; //TODO: implement this nicer
    }

    ByteVector tagNameToFrameID(const String &s)
    {
      static Map<String, ByteVector> m;
      if (m.isEmpty())
        for (size_t i = 0; i < numid3frames; ++i)
          m[id3frames[i][1]] = id3frames[i][0];
      if (m.contains(s.upper()))
        return m[s];
      return "TXXX";
    }

    bool ignored(const ByteVector& id)
    {
      return !(id == "TXXX") && !idMap().contains(id) && !deprecated(id);
    }

    bool deprecated(const ByteVector& id)
    {
      return deprecationMap().contains(id);
    }

    String prepareTagName(const String &s) {
      int pos = s.find("::");
      return ((pos != -1) ? s.substr(pos+2) : s).upper();
    }
    /*
     * The following _parseXXX functions are to be replaced by implementations of a virtual
     * function in ID3v2::Frame ASAP.
     */
    KeyValuePair _parseUserTextIdentificationFrame(const UserTextIdentificationFrame *frame)
    {
      String tagName = frame->description();
      StringList l(frame->fieldList());
      // this is done because taglib stores the description also as first entry
      // in the field list. (why?)
      if (l.contains(tagName))
         l.erase(l.find(tagName));
      return KeyValuePair(prepareTagName(tagName), l);
    }

    Frame *_createUserTextIdentificationFrame(const String &tag, const StringList &values)
    {
      UserTextIdentificationFrame* frame = new UserTextIdentificationFrame();
      frame->setDescription(tag);
      frame->setText(values);
      return frame;
    }

    KeyValuePair _parseTextIdentificationFrame(const TextIdentificationFrame *frame)
    {
      String tagName = frameIDToTagName(frame->frameID());
      StringList l = frame->fieldList();
      if (tagName == "GENRE") {
        // Special case: Support ID3v1-style genre numbers. They are not officially supported in
        // ID3v2, however it seems that still a lot of programs use them.
        //
        for (StringList::Iterator lit = l.begin(); lit != l.end(); ++lit) {
          bool ok = false;
          int test = lit->toInt(&ok); // test if the genre value is an integer
          if (ok)
            *lit = ID3v1::genre(test);
        }
      }
      else if (tagName == "DATE") {
        for (StringList::Iterator lit = l.begin(); lit != l.end(); ++lit) {
          // ID3v2 specifies ISO8601 timestamps which contain a 'T' as separator between date and time.
          // Since this is unusual in other formats, the T is removed.
          //
          int tpos = lit->find("T");
          if (tpos != -1)
            (*lit)[tpos] = ' ';
        }
      }
      return KeyValuePair(tagName, l);
    }

    Frame *_createTextIdentificationFrame(const String &tag, const StringList &values)
    {
      StringList newValues(values); // create a copy because the following might modify
      // the easiest case: a normal text frame
      if (tag == "DATE") {
        // Handle ISO8601 date format
        for (StringList::Iterator lit = newValues.begin(); lit != newValues.end();  ++lit)
          if (lit->length() > 10 && (*lit)[10] == ' ')
            (*lit)[10] = 'T';
      }
      TextIdentificationFrame *frame = new TextIdentificationFrame(tagNameToFrameID(tag));
      frame->setText(newValues);
      return frame;
    }

    KeyValuePair _parseUserUrlLinkFrame(const UserUrlLinkFrame *frame)
    {
      String tagName = frame->description().upper();
      if (tagName == "")
         tagName = "URL";
      return KeyValuePair(tagName, frame->url());
    }

    /*!
     * Create a UserUrlLinkFrame. Note that this is valid only if values.size() == 1.
     */
    Frame *_createUserUrlLinkFrame(const String &tag, const StringList &values)
    {
      UserUrlLinkFrame* frame = new UserUrlLinkFrame();
      frame->setDescription(tag);
      frame->setUrl(values[0]);
      return frame;
    }

    KeyValuePair _parseUrlLinkFrame(const UrlLinkFrame *frame)
    {
      return KeyValuePair(frameIDToTagName(frame->frameID()) , frame->url());
    }

    /*!
     * Create a rUrlLinkFrame. Note that this is valid only if values.size() == 1.
     */
    Frame *_createUrlLinkFrame(const String &tag, const StringList &values)
    {
      UrlLinkFrame *frame = new UrlLinkFrame(tagNameToFrameID(tag));
      frame->setUrl(values[0]);
      return frame;
    }

    KeyValuePair _parseCommentsFrame(const CommentsFrame *frame)
    {
      String tagName = frame->description().upper();
      if (tagName.isEmpty())
        tagName = "COMMENT";
      return KeyValuePair(tagName, frame->text());
    }

    Frame *_createCommentsFrame(const String &tag, const StringList &values)
    {
      CommentsFrame *frame = new CommentsFrame(String::UTF8);
      frame->setText(values[0]);
      return frame;
    }

    KeyValuePair _parseUnsynchronizedLyricsFrame(const UnsynchronizedLyricsFrame *frame)
    {
      return KeyValuePair("LYRICS", frame->text());
    }

    Frame *_createUnsynchronizedLyricsFrame(const String &tag, const StringList &values)
    {
      UnsynchronizedLyricsFrame* frame = new UnsynchronizedLyricsFrame();
      frame->setDescription("");
      frame->setText(values[0]);
      return frame;
    }

    KeyValuePair parseFrame(const Frame *frame)
    {
      const ByteVector &id = frame->frameID();
      if (id == "TXXX")
        return _parseUserTextIdentificationFrame(dynamic_cast< const UserTextIdentificationFrame* >(frame));
      else if (id[0] == 'T')
        return _parseTextIdentificationFrame(dynamic_cast<const TextIdentificationFrame* >(frame));
      else if (id == "WXXX")
        return _parseUserUrlLinkFrame(dynamic_cast< const UserUrlLinkFrame* >(frame));
      else if (id[0] == 'W')
        return _parseUrlLinkFrame(dynamic_cast< const UrlLinkFrame* >(frame));
      else if (id == "COMM")
        return _parseCommentsFrame(dynamic_cast< const CommentsFrame* >(frame));
      else if (id == "USLT")
        return _parseUnsynchronizedLyricsFrame(dynamic_cast< const UnsynchronizedLyricsFrame* >(frame));
      else {
        debug("parsing unknown ID3 frame: " + id);
        return KeyValuePair("UNKNOWNID3TAG", frame->toString());
      }
    }

    Frame *createFrame(const String &tag, const StringList &values)
    {
      ByteVector id = tagNameToFrameID(tag);
      if (id == "TXXX" ||
               ((id[0] == 'W' || id == "COMM" || id == "USLT") && values.size() > 1))
        return _createUserTextIdentificationFrame(tag, values);
      else if (id[0] == 'T')
        return _createTextIdentificationFrame(tag, values);
      else if (id == "WXXX")
        return _createUserUrlLinkFrame(tag, values);
      else if (id[0] == 'W')
        return _createUrlLinkFrame(tag, values);
      else if (id == "COMM")
        return _createCommentsFrame(tag, values);
      else if (id == "USLT")
        return _createUnsynchronizedLyricsFrame(tag, values);
      return 0;
    }
  }
}
