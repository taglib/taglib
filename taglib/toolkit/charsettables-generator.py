#!/usr/bin/env python3

"""
Charset table generator. Run it like
   python3 taglib/toolkit/charsettables-generator.py
It invokes iconv to generate the mapping tables from codepage byte to unicode
for each codepage.

See also:
* Microsoft Code Pages https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__)) # folder where this script resides

REPLACEMENT_CHAR = 0xFFFD
DELIMITER = 0x0000                                # U+0000, see via_iconv()

# Table: Microsoft Codepage, Enum, Description, iconv(1) name
PAGES = [
    (37,    "EBCDIC037",   "IBM EBCDIC US-Canada",                        "IBM037"),
    (437,   "OEM437",      "IBM437 OEM US",                               "CP437"),
    (500,   "EBCDIC500",   "IBM EBCDIC International",                    "CP500"),
    (708,   "OEM708",      "ASMO-708 Arabic",                             "ASMO-708"),
    (737,   "OEM737",      "IBM737 OEM Greek",                            "CP737"),
    (775,   "OEM775",      "IBM775 OEM Baltic",                           "CP775"),
    (850,   "OEM850",      "IBM850 OEM Latin 1",                          "CP850"),
    (852,   "OEM852",      "IBM852 OEM Latin 2",                          "CP852"),
    (855,   "OEM855",      "IBM855 OEM Cyrillic",                         "CP855"),
    (857,   "OEM857",      "IBM857 OEM Turkish",                          "CP857"),
    (858,   "OEM858",      "IBM858 OEM Latin 1 with Euro",                "CP858"),
    (860,   "OEM860",      "IBM860 OEM Portuguese",                       "CP860"),
    (861,   "OEM861",      "IBM861 OEM Icelandic",                        "CP861"),
    (862,   "OEM862",      "DOS-862 OEM Hebrew",                          "CP862"),
    (863,   "OEM863",      "IBM863 OEM French Canadian",                  "CP863"),
    (864,   "OEM864",      "IBM864 OEM Arabic",                           "CP864"),
    (865,   "OEM865",      "IBM865 OEM Nordic",                           "CP865"),
    (866,   "OEM866",      "IBM866 OEM Russian",                          "CP866"),
    (869,   "OEM869",      "IBM869 OEM Modern Greek",                     "CP869"),
    (874,   "Thai874",     "Thai (Windows-874)",                          "CP874"),
    (1047,  "EBCDIC1047",  "IBM1047 EBCDIC Latin 1/Open System",          "IBM1047"), # https://public.dhe.ibm.com/software/globalization/gcoc/attachments/CP01047.txt
    (1250,  "Windows1250", "Windows-1250 Central European",               "CP1250"),
    (1251,  "Windows1251", "Windows-1251 Cyrillic",                       "CP1251"),
    (1252,  "Windows1252", "Windows-1252 Western European",               "CP1252"),
    (1253,  "Windows1253", "Windows-1253 Greek",                          "CP1253"),
    (1254,  "Windows1254", "Windows-1254 Turkish",                        "CP1254"),
    (1255,  "Windows1255", "Windows-1255 Hebrew",                         "CP1255"),
    (1256,  "Windows1256", "Windows-1256 Arabic",                         "CP1256"),
    (1257,  "Windows1257", "Windows-1257 Baltic",                         "CP1257"),
    (1258,  "Windows1258", "Windows-1258 Vietnamese",                     "CP1258"),
    (20127, "ASCII",       "US-ASCII",                                    "ASCII"),
    (20273, "EBCDIC273",   "IBM273 EBCDIC Germany",                       "IBM273"),
    (20277, "EBCDIC277",   "IBM277 EBCDIC Denmark-Norway",                "IBM277"),
    (20278, "EBCDIC278",   "IBM278 EBCDIC Finland-Sweden",                "IBM278"),
    (20280, "EBCDIC280",   "IBM280 EBCDIC Italy",                         "IBM280"),
    (20284, "EBCDIC284",   "IBM284 EBCDIC Latin America-Spain",           "IBM284"),
    (20285, "EBCDIC285",   "IBM285 EBCDIC United Kingdom",                "IBM285"),
    (20290, "EBCDIC290",   "IBM290 EBCDIC Japanese Katakana Extended",    "IBM290"),
    (20297, "EBCDIC297",   "IBM297 EBCDIC France",                        "IBM297"),
    (20420, "EBCDIC420",   "IBM420 EBCDIC Arabic",                        "IBM420"),
    (20423, "EBCDIC423",   "IBM423 EBCDIC Greek",                         "IBM423"),
    (20866, "KOI8R",       "KOI8-R",                                      "KOI8-R"),
    (20871, "EBCDIC871",   "IBM871 EBCDIC Icelandic",                     "IBM871"),
    (21025, "EBCDIC1025",  "IBM1025 EBCDIC Cyrillic Serbian-Bulgarian",   "CP1025"),
    (21866, "KOI8U",       "KOI8-U",                                      "KOI8-U"),
    (28591, "Latin1",      "ISO 8859-1",                                  "ISO-8859-1"),
    (28592, "Latin2",      "ISO 8859-2",                                  "ISO-8859-2"),
    (28593, "Latin3",      "ISO 8859-3",                                  "ISO-8859-3"),
    (28594, "Latin4",      "ISO 8859-4",                                  "ISO-8859-4"),
    (28595, "Latin5",      "ISO 8859-5",                                  "ISO-8859-5"),
    (28596, "Latin6",      "ISO 8859-6",                                  "ISO-8859-6"),
    (28597, "Latin7",      "ISO 8859-7",                                  "ISO-8859-7"),
    (28598, "Latin8",      "ISO 8859-8 Hebrew (logical)",                 "ISO-8859-8"),
    (28599, "Latin9",      "ISO 8859-9",                                  "ISO-8859-9"),
    (28603, "Latin13",     "ISO 8859-13",                                 "ISO-8859-13"),
    (28605, "Latin15",     "ISO 8859-15",                                 "ISO-8859-15"),

    # The "Euro" EBCDIC pages, which iconv(1) knows natively.
    (1140,  "EBCDIC1140",  "IBM037 EBCDIC US-Canada with Euro",           "CP1140"),
    (1141,  "EBCDIC1141",  "IBM273 EBCDIC Germany with Euro",             "CP1141"),
    (1142,  "EBCDIC1142",  "IBM277 EBCDIC Denmark-Norway with Euro",      "CP1142"),
    (1143,  "EBCDIC1143",  "IBM278 EBCDIC Finland-Sweden with Euro",      "CP1143"),
    (1144,  "EBCDIC1144",  "IBM280 EBCDIC Italy with Euro",               "CP1144"),
    (1145,  "EBCDIC1145",  "IBM284 EBCDIC Latin America-Spain with Euro", "CP1145"),
    (1146,  "EBCDIC1146",  "IBM285 EBCDIC UK with Euro",                  "CP1146"),
    (1147,  "EBCDIC1147",  "IBM297 EBCDIC France with Euro",              "CP1147"),
    (1148,  "EBCDIC1148",  "IBM500 EBCDIC International with Euro",       "CP1148"),
    (1149,  "EBCDIC1149",  "IBM871 EBCDIC Icelandic with Euro",           "CP1149"),
]

def via_iconv(name):
    """The whole page in one iconv(1) run.

    The input is the 256 bytes with a 0x00 in front of every one of them, and -c
    makes iconv(1) drop the bytes the page leaves undefined. 0x00 is what
    separates them because it is the one byte every page here maps to U+0000 and
    the only one that does, so a byte that got dropped leaves its two 0x00 next
    to each other and the run comes back as one U+0000 per byte with the code
    point of every byte the page does define in between. Byte 0x00 itself is the
    one byte the probe cannot ask about, since it is the delimiter: it is NUL in
    every page here, and finalize() below checks that it is the only byte that
    maps to U+0000. A page that gave some other byte U+0000 as well would lose
    that value to the delimiter without complaint, so every table the generator
    writes asserts in finalize() that byte 0x00 is its only U+0000.
    """
    probe = b"".join(bytes((0x00, b)) for b in range(1, 256))
    r = subprocess.run(["iconv","-f",name,"-t","UCS-4LE","-c"],
                       input=probe, capture_output=True)
    if r.returncode != 0:
        sys.exit("iconv(1) does not know the code page %s: %s"
                 % (name, r.stderr.decode("utf-8", "replace").strip()))
    assert len(r.stdout) % 4 == 0, (name, len(r.stdout))
    units = [int.from_bytes(r.stdout[i:i + 4], "little")
             for i in range(0, len(r.stdout), 4)]
    t = [DELIMITER] + [None]*255                   # byte 0x00 is NUL, the delimiter's own value
    pos = 0
    for b in range(1, 256):
        assert pos < len(units) and units[pos] == DELIMITER, (name, hex(b))
        pos += 1
        if pos < len(units) and units[pos] != DELIMITER:
            t[b] = units[pos]
            pos += 1
    assert pos == len(units), (name, "trailing code points", pos, len(units))
    return t

_CACHE = {}
def build(name):
    if name not in _CACHE:
        _CACHE[name] = via_iconv(name)
    return list(_CACHE[name])

def finalize(t):
    """Fill in the bytes a code page leaves empty, so that every byte becomes a
    character and no table has a gap.

    A code page need not give a meaning to every byte. A byte with no meaning
    in 0x80-0x9f becomes the character with the same number, so 0x81 becomes
    U+0081. Windows does this, and so do browsers: the standard that defines
    how browsers read these pages fills the gaps the same way. See section 9.1
    at https://encoding.spec.whatwg.org , and for a full table
    https://encoding.spec.whatwg.org/index-windows-1252.txt

    An empty byte anywhere else becomes U+FFFD, the replacement character that
    stands for input we could not read.
    """
    out = []
    for b in range(256):
        v = t[b]
        if v is None:
            v = b if 0x80 <= b <= 0x9f else REPLACEMENT_CHAR
        assert (v == 0) == (b == 0), (b, hex(v))
        assert v <= 0xFFFF, (b, hex(v))
        out.append(v)
    return out

LIC = """
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

/*
  THIS FILE IS NOT A PART OF THE TAGLIB API

  Generated data, one table per single byte code page. Each table maps all 256
  byte values to a Unicode code point, so decoding and encoding never has to ask
  whether a byte is defined: a byte the code page leaves undefined is mapped to
  the C1 control of the same value in 0x80-0x9f (which is what the Windows ANSI
  pages do) and to U+FFFD anywhere else.

  Generated by taglib/toolkit/charsettables-generator.py, which asks iconv(1) for
  each page in a single conversion of its 256 bytes.
 */
"""

def emit_cpp(pages):
    """pages: list of (code page, enum name, description, 256 values)."""
    out = [LIC, "", '#include "charsettables.h"', "", "#include <unordered_map>",
           "", "namespace TagLib::Charset::detail {", ""]
    for num, name, desc, table in pages:
        out.append("  // %d %s" % (num, desc))
        out.append("  constexpr unsigned short %sTable[256] = {" % name)
        for row in range(16):
            cells = ", ".join("0x%04X" % table[row * 16 + col] for col in range(16))
            out.append("    %s," % cells)
        out.append("  };")
        out.append("")
    out.append("  constexpr CodePageEntry codePageTable[] = {")
    for num, name, desc, table in pages:
        out.append('    { Type::%s, %d, %sTable },' % (name, num, name))
    out.append("  };")
    out.append("")
    out.append("  // Every Type but UTF8 has exactly one row.")
    out.append("  static_assert(")
    out.append("    std::size(codePageTable) == %d," % len(pages))
    out.append('    "the code page table and Charset::Type disagree");')
    out.append("")
    out.append("  // A Type and the CSET code page number name the same row, so the")
    out.append("  // code page of every row has to equal the value of its Type.")
    out.append("  static_assert([] {")
    out.append("    for(auto entry : codePageTable) {")
    out.append("      if(entry.codePage != static_cast<unsigned int>(entry.type))")
    out.append("        return false;")
    out.append("    }")
    out.append("    return true;")
    out.append("  }(), \"a row's code page does not match the value of its Type\");")
    out.append("")
    out.append("  // A row is found by its code page, which is also the value of its Type.")
    out.append("  // The index is built once, from the table above, and never changes, so")
    out.append("  // a lookup cannot move it and the pointer entryForCodePage() hands out")
    out.append("  // stays valid. The local static guards that one time build.")
    out.append("  static const std::unordered_map<unsigned int, const CodePageEntry *> &codePageIndex()")
    out.append("  {")
    out.append("    static const std::unordered_map<unsigned int, const CodePageEntry *> index = [] {")
    out.append("      std::unordered_map<unsigned int, const CodePageEntry *> m;")
    out.append("      m.reserve(std::size(codePageTable));")
    out.append("      for(const auto &entry : codePageTable)")
    out.append("        m.emplace(entry.codePage, &entry);")
    out.append("      return m;")
    out.append("    }();")
    out.append("")
    out.append("    return index;")
    out.append("  }")
    out.append("")
    out.append("  const CodePageEntry *entryForCodePage(unsigned int codePage)")
    out.append("  {")
    out.append("    const auto &index = codePageIndex();")
    out.append("    const auto it = index.find(codePage);")
    out.append("    return it != index.end() ? it->second : nullptr;")
    out.append("  }")
    out.append("")
    out.append("} // namespace TagLib::Charset::detail")
    out.append("")
    path = os.path.join(HERE, "charsettables.cpp")
    open(path, "w").write("\n".join(out))
    print("wrote taglib/toolkit/charsettables.cpp (%d code pages, %d values)"
          % (len(pages), len(pages) * 256))

def main():
    pages = [(num, name, desc, finalize(build(gen)))
             for num, name, desc, gen in PAGES]
    pages.sort(key=lambda p: p[0])
    emit_cpp(pages)

if __name__ == "__main__":
    main()
