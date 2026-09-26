#!/usr/bin/env python3

"""
Charset table generator. Run it like
   python3 taglib/toolkit/charsettables-generator.py
It invokes iconv to generate the mapping tables from codepage byte to unicode
for each codepage, except for the few hardcoded ones, see below.

See also:
* Microsoft Code Pages https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__)) # folder where this script resides

REPLACEMENT_CHAR = 0xFFFD
DELIMITER = 0x0000                                # U+0000, see via_iconv()

# Table: Microsoft Codepage, Enum, Description, Generator
PAGES = [
    (37,    "EBCDIC037",   "IBM EBCDIC US-Canada",                        ("iconv","IBM037"     )),
    (437,   "OEM437",      "IBM437 OEM US",                               ("iconv","CP437"      )),
    (500,   "EBCDIC500",   "IBM EBCDIC International",                    ("iconv","CP500"      )),
    (708,   "OEM708",      "ASMO-708 Arabic",                             ("iconv","ASMO-708"   )),
    (720,   "OEM720",      "DOS-720 Arabic",                              ("table","CP00720"    )), # hardcoded, see below
    (737,   "OEM737",      "IBM737 OEM Greek",                            ("iconv","CP737"      )),
    (775,   "OEM775",      "IBM775 OEM Baltic",                           ("iconv","CP775"      )),
    (850,   "OEM850",      "IBM850 OEM Latin 1",                          ("iconv","CP850"      )),
    (852,   "OEM852",      "IBM852 OEM Latin 2",                          ("iconv","CP852"      )),
    (855,   "OEM855",      "IBM855 OEM Cyrillic",                         ("iconv","CP855"      )),
    (857,   "OEM857",      "IBM857 OEM Turkish",                          ("iconv","CP857"      )),
    (858,   "OEM858",      "IBM858 OEM Latin 1 with Euro",                ("iconv","CP858"      )),
    (860,   "OEM860",      "IBM860 OEM Portuguese",                       ("iconv","CP860"      )),
    (861,   "OEM861",      "IBM861 OEM Icelandic",                        ("iconv","CP861"      )),
    (862,   "OEM862",      "DOS-862 OEM Hebrew",                          ("iconv","CP862"      )),
    (863,   "OEM863",      "IBM863 OEM French Canadian",                  ("iconv","CP863"      )),
    (864,   "OEM864",      "IBM864 OEM Arabic",                           ("iconv","CP864"      )),
    (865,   "OEM865",      "IBM865 OEM Nordic",                           ("iconv","CP865"      )),
    (866,   "OEM866",      "IBM866 OEM Russian",                          ("iconv","CP866"      )),
    (869,   "OEM869",      "IBM869 OEM Modern Greek",                     ("iconv","CP869"      )),
    (874,   "Thai874",     "Thai (Windows-874)",                          ("iconv","CP874"      )),
    (875,   "OEM875",      "IBM875 IBM EBCDIC Greek",                     ("table","CP00875"    )), # hardcoded, see below
    (1026,  "EBCDIC1026",  "IBM1026 EBCDIC Turkish",                      ("table","CP01026"    )), # hardcoded, see below
    (1047,  "EBCDIC1047",  "IBM1047 EBCDIC Latin 1/Open System",          ("iconv","IBM1047"    )), # https://public.dhe.ibm.com/software/globalization/gcoc/attachments/CP01047.txt
    (1250,  "Windows1250", "Windows-1250 Central European",               ("iconv","CP1250"     )),
    (1251,  "Windows1251", "Windows-1251 Cyrillic",                       ("iconv","CP1251"     )),
    (1252,  "Windows1252", "Windows-1252 Western European",               ("iconv","CP1252"     )),
    (1253,  "Windows1253", "Windows-1253 Greek",                          ("iconv","CP1253"     )),
    (1254,  "Windows1254", "Windows-1254 Turkish",                        ("iconv","CP1254"     )),
    (1255,  "Windows1255", "Windows-1255 Hebrew",                         ("iconv","CP1255"     )),
    (1256,  "Windows1256", "Windows-1256 Arabic",                         ("iconv","CP1256"     )),
    (1257,  "Windows1257", "Windows-1257 Baltic",                         ("iconv","CP1257"     )),
    (1258,  "Windows1258", "Windows-1258 Vietnamese",                     ("iconv","CP1258"     )),
    (10000, "MacRoman",    "Macintosh Roman",                             ("table","CP10000"    )), # hardcoded, see below
    (10007, "MacCyrillic",  "Macintosh Cyrillic",                          ("table","CP10007"    )), # hardcoded, see below
    (20127, "ASCII",       "US-ASCII",                                    ("iconv","ASCII"      )),
    (20273, "EBCDIC273",   "IBM273 EBCDIC Germany",                       ("iconv","IBM273"     )),
    (20277, "EBCDIC277",   "IBM277 EBCDIC Denmark-Norway",                ("iconv","IBM277"     )),
    (20278, "EBCDIC278",   "IBM278 EBCDIC Finland-Sweden",                ("iconv","IBM278"     )),
    (20280, "EBCDIC280",   "IBM280 EBCDIC Italy",                         ("iconv","IBM280"     )),
    (20284, "EBCDIC284",   "IBM284 EBCDIC Latin America-Spain",           ("iconv","IBM284"     )),
    (20285, "EBCDIC285",   "IBM285 EBCDIC United Kingdom",                ("iconv","IBM285"     )),
    (20290, "EBCDIC290",   "IBM290 EBCDIC Japanese Katakana Extended",    ("iconv","IBM290"     )),
    (20297, "EBCDIC297",   "IBM297 EBCDIC France",                        ("iconv","IBM297"     )),
    (20420, "EBCDIC420",   "IBM420 EBCDIC Arabic",                        ("iconv","IBM420"     )),
    (20423, "EBCDIC423",   "IBM423 EBCDIC Greek",                         ("iconv","IBM423"     )),
    (20424, "EBCDIC424",   "IBM424 EBCDIC Hebrew",                        ("table","CP00424"    )), # hardcoded, see below
    (20866, "KOI8R",       "KOI8-R",                                      ("iconv","KOI8-R"     )),
    (20871, "EBCDIC871",   "IBM871 EBCDIC Icelandic",                     ("iconv","IBM871"     )),
    (21025, "EBCDIC1025",  "IBM1025 EBCDIC Cyrillic Serbian-Bulgarian",   ("iconv","CP1025"     )),
    (21866, "KOI8U",       "KOI8-U",                                      ("iconv","KOI8-U"     )),
    (28591, "Latin1",      "ISO 8859-1",                                  ("iconv","ISO-8859-1" )),
    (28592, "Latin2",      "ISO 8859-2",                                  ("iconv","ISO-8859-2" )),
    (28593, "Latin3",      "ISO 8859-3",                                  ("iconv","ISO-8859-3" )),
    (28594, "Latin4",      "ISO 8859-4",                                  ("iconv","ISO-8859-4" )),
    (28595, "Latin5",      "ISO 8859-5",                                  ("iconv","ISO-8859-5" )),
    (28596, "Latin6",      "ISO 8859-6",                                  ("iconv","ISO-8859-6" )),
    (28597, "Latin7",      "ISO 8859-7",                                  ("iconv","ISO-8859-7" )),
    (28598, "Latin8",      "ISO 8859-8 Hebrew (logical)",                 ("iconv","ISO-8859-8" )),
    (28599, "Latin9",      "ISO 8859-9",                                  ("iconv","ISO-8859-9" )),
    (28603, "Latin13",     "ISO 8859-13",                                 ("iconv","ISO-8859-13")),
    (28605, "Latin15",     "ISO 8859-15",                                 ("iconv","ISO-8859-15")),

    # The "Euro" EBCDIC pages are their base page with U+20AC at 0x9f.
    (1140,  "EBCDIC1140",  "IBM037 EBCDIC US-Canada with Euro",           ("euro",("iconv","IBM037"))),
    (1141,  "EBCDIC1141",  "IBM273 EBCDIC Germany with Euro",             ("euro",("iconv","IBM273"))),
    (1142,  "EBCDIC1142",  "IBM277 EBCDIC Denmark-Norway with Euro",      ("euro",("iconv","IBM277"))),
    (1143,  "EBCDIC1143",  "IBM278 EBCDIC Finland-Sweden with Euro",      ("euro",("iconv","IBM278"))),
    (1144,  "EBCDIC1144",  "IBM280 EBCDIC Italy with Euro",               ("euro",("iconv","IBM280"))),
    (1145,  "EBCDIC1145",  "IBM284 EBCDIC Latin America-Spain with Euro", ("euro",("iconv","IBM284"))),
    (1146,  "EBCDIC1146",  "IBM285 EBCDIC UK with Euro",                  ("euro",("iconv","IBM285"))),
    (1147,  "EBCDIC1147",  "IBM297 EBCDIC France with Euro",              ("euro",("iconv","IBM297"))),
    (1148,  "EBCDIC1148",  "IBM500 EBCDIC International with Euro",       ("euro",("iconv","CP500" ))),
    (1149,  "EBCDIC1149",  "IBM871 EBCDIC Icelandic with Euro",           ("euro",("iconv","IBM871"))),
    
    # hardcoded, see below
    (20924, "EBCDIC20924", "IBM924 EBCDIC Latin 9 (1047 with Euro)",      ("table","CP00924")),
]

TABLES = {

  # 720, DOS-720 Arabic: IBM 720 Arabic, a transparent ASMO-708 DOS page.
  # https://public.dhe.ibm.com/software/globalization/gcoc/attachments/CP00720.txt
  # IBM gives the GCGID and GCS name of 215 positions rather than a code point, so
  # the GCGIDs were resolved through IBM's own CP437 table joined with the Unicode
  # Consortium's CP437.TXT, and the 47 CP437 has no use for, the Arabic letters and
  # diacritics among them, from their names. The 41 positions the file leaves
  # without a GCGID are the C0 controls and 0x7f, plus 0x80, 0x81, 0x84, 0x86 and
  # 0x8d..0x90, which the page does not define. There is no euro anywhere on it,
  # and 0xf0, "Identity Symbol" next to "Nearly Equals Symbol" and "Product Dot",
  # is U+2261 IDENTICAL TO, not a soft hyphen.
  # * iconv has no name for this page, not CP720 nor any alias around it
  # * Perl's Encode has no mapping for it either
  # * Python does have cp720, and it agrees at all 256 mappings hardcoded here
  "CP00720": [
     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
     0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
     0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
     0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
     0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
     0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
     0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
     0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
     0x0080, 0x0081, 0x00E9, 0x00E2, 0x0084, 0x00E0, 0x0086, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x008D, 0x008E, 0x008F,
     0x0090, 0x0651, 0x0652, 0x00F4, 0x00A4, 0x0640, 0x00FB, 0x00F9, 0x0621, 0x0622, 0x0623, 0x0624, 0x00A3, 0x0625, 0x0626, 0x0627,
     0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F, 0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x00AB, 0x00BB,
     0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
     0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
     0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
     0x0636, 0x0637, 0x0638, 0x0639, 0x063A, 0x0641, 0x00B5, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649, 0x064A,
     0x2261, 0x064B, 0x064C, 0x064D, 0x064E, 0x064F, 0x0650, 0x2248, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
  ],

  # 875, IBM875 IBM EBCDIC Greek.
  # https://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/EBCDIC/CP875.TXT
  # The file gives a code point and the name of the character at each of the 256
  # positions, so every value in our table below is the one which the file names.
  # | byte | this table              | iconv CP875 | Python cp875 | Perl cp875 |
  # |------|-------------------------|-------------|--------------|------------|
  # | 0x3f | U+001A SUBSTITUTE       | U+001A      | U+001A       | undefined  |
  # | 0x6a | U+007C VERTICAL LINE    | undefined   | U+007C       | U+007C     |
  # | 0x74 | U+00A0 NO-BREAK SPACE   | U+2207      | U+00A0       | U+00A0     |
  # | 0xdc | U+001A SUBSTITUTE       | undefined   | U+001A       | undefined  |
  # | 0xdd | U+0387 GREEK ANO TELEIA | U+00B7      | U+0387       | U+0387     |
  # | 0xe1 | U+001A SUBSTITUTE       | undefined   | U+001A       | undefined  |
  # | 0xec | U+001A SUBSTITUTE       | undefined   | U+001A       | undefined  |
  # | 0xed | U+001A SUBSTITUTE       | undefined   | U+001A       | undefined  |
  # | 0xfc | U+001A SUBSTITUTE       | undefined   | U+001A       | undefined  |
  # | 0xfd | U+001A SUBSTITUTE       | undefined   | U+001A       | U+001A     |
  "CP00875": [
    0x0000, 0x0001, 0x0002, 0x0003, 0x009C, 0x0009, 0x0086, 0x007F, 0x0097, 0x008D, 0x008E, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x009D, 0x0085, 0x0008, 0x0087, 0x0018, 0x0019, 0x0092, 0x008F, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x000A, 0x0017, 0x001B, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x0005, 0x0006, 0x0007,
    0x0090, 0x0091, 0x0016, 0x0093, 0x0094, 0x0095, 0x0096, 0x0004, 0x0098, 0x0099, 0x009A, 0x009B, 0x0014, 0x0015, 0x009E, 0x001A,
    0x0020, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x005B, 0x002E, 0x003C, 0x0028, 0x002B, 0x0021,
    0x0026, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F, 0x03A0, 0x03A1, 0x03A3, 0x005D, 0x0024, 0x002A, 0x0029, 0x003B, 0x005E,
    0x002D, 0x002F, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x007C, 0x002C, 0x0025, 0x005F, 0x003E, 0x003F,
    0x00A8, 0x0386, 0x0388, 0x0389, 0x00A0, 0x038A, 0x038C, 0x038E, 0x038F, 0x0060, 0x003A, 0x0023, 0x0040, 0x0027, 0x003D, 0x0022,
    0x0385, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6,
    0x00B0, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC,
    0x00B4, 0x007E, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x03BD, 0x03BE, 0x03BF, 0x03C0, 0x03C1, 0x03C3,
    0x00A3, 0x03AC, 0x03AD, 0x03AE, 0x03CA, 0x03AF, 0x03CC, 0x03CD, 0x03CB, 0x03CE, 0x03C2, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8,
    0x007B, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x00AD, 0x03C9, 0x0390, 0x03B0, 0x2018, 0x2015,
    0x007D, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 0x00B1, 0x00BD, 0x001A, 0x0387, 0x2019, 0x00A6,
    0x005C, 0x001A, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00B2, 0x00A7, 0x001A, 0x001A, 0x00AB, 0x00AC,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x00B3, 0x00A9, 0x001A, 0x001A, 0x00BB, 0x009F,
  ],

  # 20924, IBM924 IBM EBCDIC Latin 9, the 1047 page with the euro.
  # https://public.dhe.ibm.com/software/globalization/gcoc/attachments/CP00924.txt
  # The 191 GCS names, resolved as for 1026 below. It is more than 1047 with 0x9f
  # swapped: it also takes the two caron pairs and the OE ligatures, and drops the
  # fraction signs, the broken bar, the acute accent, the diaeresis, the cedilla and
  # the international currency sign, with cent, logical not and Y acute moving
  # 0x4a -> 0xb0 -> 0xba -> 0x4a. So it is written out in full, not derived from
  # 1047. 0x9f is the only change Microsoft documents.
  # iconv, Python and Perl have no mapping for this page.
  "CP00924": [
    0x0000, 0x0001, 0x0002, 0x0003, 0x009C, 0x0009, 0x0086, 0x007F, 0x0097, 0x008D, 0x008E, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x009D, 0x0085, 0x0008, 0x0087, 0x0018, 0x0019, 0x0092, 0x008F, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x000A, 0x0017, 0x001B, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x0005, 0x0006, 0x0007,
    0x0090, 0x0091, 0x0016, 0x0093, 0x0094, 0x0095, 0x0096, 0x0004, 0x0098, 0x0099, 0x009A, 0x009B, 0x0014, 0x0015, 0x009E, 0x001A,
    0x0020, 0x00A0, 0x00E2, 0x00E4, 0x00E0, 0x00E1, 0x00E3, 0x00E5, 0x00E7, 0x00F1, 0x00DD, 0x002E, 0x003C, 0x0028, 0x002B, 0x007C,
    0x0026, 0x00E9, 0x00EA, 0x00EB, 0x00E8, 0x00ED, 0x00EE, 0x00EF, 0x00EC, 0x00DF, 0x0021, 0x0024, 0x002A, 0x0029, 0x003B, 0x005E,
    0x002D, 0x002F, 0x00C2, 0x00C4, 0x00C0, 0x00C1, 0x00C3, 0x00C5, 0x00C7, 0x00D1, 0x0160, 0x002C, 0x0025, 0x005F, 0x003E, 0x003F,
    0x00F8, 0x00C9, 0x00CA, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x0060, 0x003A, 0x0023, 0x0040, 0x0027, 0x003D, 0x0022,
    0x00D8, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x00AB, 0x00BB, 0x00F0, 0x00FD, 0x00FE, 0x00B1,
    0x00B0, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x00AA, 0x00BA, 0x00E6, 0x017E, 0x00C6, 0x20AC,
    0x00B5, 0x007E, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x00A1, 0x00BF, 0x00D0, 0x005B, 0x00DE, 0x00AE,
    0x00A2, 0x00A3, 0x00A5, 0x00B7, 0x00A9, 0x00A7, 0x00B6, 0x0152, 0x0153, 0x0178, 0x00AC, 0x0161, 0x00AF, 0x005D, 0x017D, 0x00D7,
    0x007B, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x00AD, 0x00F4, 0x00F6, 0x00F2, 0x00F3, 0x00F5,
    0x007D, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 0x00B9, 0x00FB, 0x00FC, 0x00F9, 0x00FA, 0x00FF,
    0x005C, 0x00F7, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00B2, 0x00D4, 0x00D6, 0x00D2, 0x00D3, 0x00D5,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x00B3, 0x00DB, 0x00DC, 0x00D9, 0x00DA, 0x009F,
  ],

  # 1026, IBM1026 IBM EBCDIC Turkish, IBM's Latin 5 Turkey.
  # https://public.dhe.ibm.com/software/globalization/gcoc/attachments/CP01026.txt
  # The 191 GCS names, resolved to code points. The file names no GCGID for
  # 0x00..0x3f or 0xff, which are the EBCDIC control layout that the 37 entry of
  # tests/test_wav_charset.inc records.
  # Python and Perl agree with each other and with this table on both. 0x9d is
  # U+00B8 , which the file calls "Cedilla or Sedila Accent", and 0xbc is
  # U+00AF MACRON, "Overline".
  #
  # | byte | this table     | iconv CP1026 | Python cp1026 | Perl cp1026 |
  # |------|----------------|--------------|---------------|-------------|
  # | 0x9d | U+00B8 CEDILLA | U+02DB       | U+00B8        | U+00B8      |
  # | 0xbc | U+00AF MACRON  | U+2014       | U+00AF        | U+00AF      |
  "CP01026": [
    0x0000, 0x0001, 0x0002, 0x0003, 0x009C, 0x0009, 0x0086, 0x007F, 0x0097, 0x008D, 0x008E, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x009D, 0x0085, 0x0008, 0x0087, 0x0018, 0x0019, 0x0092, 0x008F, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x000A, 0x0017, 0x001B, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x0005, 0x0006, 0x0007,
    0x0090, 0x0091, 0x0016, 0x0093, 0x0094, 0x0095, 0x0096, 0x0004, 0x0098, 0x0099, 0x009A, 0x009B, 0x0014, 0x0015, 0x009E, 0x001A,
    0x0020, 0x00A0, 0x00E2, 0x00E4, 0x00E0, 0x00E1, 0x00E3, 0x00E5, 0x007B, 0x00F1, 0x00C7, 0x002E, 0x003C, 0x0028, 0x002B, 0x0021,
    0x0026, 0x00E9, 0x00EA, 0x00EB, 0x00E8, 0x00ED, 0x00EE, 0x00EF, 0x00EC, 0x00DF, 0x011E, 0x0130, 0x002A, 0x0029, 0x003B, 0x005E,
    0x002D, 0x002F, 0x00C2, 0x00C4, 0x00C0, 0x00C1, 0x00C3, 0x00C5, 0x005B, 0x00D1, 0x015F, 0x002C, 0x0025, 0x005F, 0x003E, 0x003F,
    0x00F8, 0x00C9, 0x00CA, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x0131, 0x003A, 0x00D6, 0x015E, 0x0027, 0x003D, 0x00DC,
    0x00D8, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x00AB, 0x00BB, 0x007D, 0x0060, 0x00A6, 0x00B1,
    0x00B0, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x00AA, 0x00BA, 0x00E6, 0x00B8, 0x00C6, 0x00A4,
    0x00B5, 0x00F6, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x00A1, 0x00BF, 0x005D, 0x0024, 0x0040, 0x00AE,
    0x00A2, 0x00A3, 0x00A5, 0x00B7, 0x00A9, 0x00A7, 0x00B6, 0x00BC, 0x00BD, 0x00BE, 0x00AC, 0x007C, 0x00AF, 0x00A8, 0x00B4, 0x00D7,
    0x00E7, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x00AD, 0x00F4, 0x007E, 0x00F2, 0x00F3, 0x00F5,
    0x011F, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 0x00B9, 0x00FB, 0x005C, 0x00F9, 0x00FA, 0x00FF,
    0x00FC, 0x00F7, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00B2, 0x00D4, 0x0023, 0x00D2, 0x00D3, 0x00D5,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x00B3, 0x00DB, 0x0022, 0x00D9, 0x00DA, 0x009F,
  ],

  # 10000, Macintosh Roman: Apple's Mac OS Roman.
  # https://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/ROMAN.TXT
  # The file gives a code point and the Unicode name of the character at each of
  # the 223 graphic positions. It lists no 0x00..0x1f or 0x7f, as the standard
  # mapping tables do not, but its header says which characters those are: the
  # standard control characters. 0xf0 is U+F8FF, the Apple logo, a corporate zone
  # character that the file documents in its header and in CORPCHAR.TXT and the one
  # value here with no Unicode name to check it against; the note about 0xdb in that
  # same header is why 0xdb below is the euro, U+20AC, rather than U+00A4.
  #
  # | byte | this table        | iconv MACINTOSH | Python mac_roman | Perl macintosh |
  # |------|-------------------|-----------------|------------------|----------------|
  # | 0x7f | U+007F DELETE     | U+007F          | U+007F           | undefined      |
  # | 0xc6 | U+2206 INCREMENT  | U+0394          | U+2206           | U+2206         |
  # | 0xf0 | U+F8FF APPLE LOGO | U+E01E          | U+F8FF           | U+F8FF         |
  "CP10000": [
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1, 0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
    0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3, 0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
    0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF, 0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211, 0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
    0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB, 0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA, 0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
    0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC, 0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7,
  ],

  # 10007, Macintosh Cyrillic: Apple's Mac OS Cyrillic.
  # https://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/CYRILLIC.TXT
  # The file gives a code point and the Unicode name of the character at each of
  # the 223 graphic positions. It lists no 0x00..0x1f or 0x7f, as the standard
  # mapping tables do not, but its header says which characters those are: the
  # standard control characters. This is the "Euro sign" version, the one Mac OS 9.0
  # merged the two older Slavic pages into, and its header is what dates the three
  # positions that merge moved: 0xa2 U+0490, 0xb6 U+0491 and 0xff U+20AC, where the
  # currency sign variant had U+00A2, U+2202 and U+00A4.
  #
  # | byte | this table       | iconv MACCYRILLIC | Python mac_cyrillic | Perl maccyrillic |
  # |------|------------------|-------------------|---------------------|------------------|
  # | 0x7f | U+007F DELETE    | U+007F            | U+007F              | undefined        |
  # | 0xff | U+20AC EURO SIGN | U+00A4            | U+20AC              | U+20AC           |
  "CP10007": [
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x2020, 0x00B0, 0x0490, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x0406, 0x00AE, 0x00A9, 0x2122, 0x0402, 0x0452, 0x2260, 0x0403, 0x0453,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x0456, 0x00B5, 0x0491, 0x0408, 0x0404, 0x0454, 0x0407, 0x0457, 0x0409, 0x0459, 0x040A, 0x045A,
    0x0458, 0x0405, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB, 0x00BB, 0x2026, 0x00A0, 0x040B, 0x045B, 0x040C, 0x045C, 0x0455,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x201E, 0x040E, 0x045E, 0x040F, 0x045F, 0x2116, 0x0401, 0x0451, 0x044F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x20AC,
  ],

  # 20424, IBM424 EBCDIC Hebrew: the Hebrew EBCDIC page, which is what Microsoft calls 20424.
  # https://www.unicode.org/Public/MAPPINGS/VENDORS/MISC/CP424.TXT
  # A "Format A" table from 1999: a code point and the Unicode name at each of 218 positions,
  # and the word "UNDEFINED" with no code point at the other 38, which the file does not leave
  # to be guessed. Those 38 are spelled out below as finalize() computes them, U+FFFD or the
  # C1 control of the same number. All 153 graphic names the file prints are still the names
  # Unicode uses today, so nothing here is a casualty of its age; the 65 controls it maps are
  # the standard EBCDIC ones.
  # Python and Perl. 0x78 is U+2017  and 0x8f U+00B1 .
  #
  # | byte | this table             | iconv CP424 | Python cp424 | Perl cp424 |
  # |------|------------------------|-------------|--------------|------------|
  # | 0x78 | U+2017 DOUBLE LOW LINE | U+21D4      | U+2017       | U+2017     |
  # | 0x8f | U+00B1 PLUS-MINUS SIGN | undefined   | U+00B1       | U+00B1     |
  "CP00424": [
    0x0000, 0x0001, 0x0002, 0x0003, 0x009C, 0x0009, 0x0086, 0x007F, 0x0097, 0x008D, 0x008E, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x009D, 0x0085, 0x0008, 0x0087, 0x0018, 0x0019, 0x0092, 0x008F, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x000A, 0x0017, 0x001B, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x0005, 0x0006, 0x0007,
    0x0090, 0x0091, 0x0016, 0x0093, 0x0094, 0x0095, 0x0096, 0x0004, 0x0098, 0x0099, 0x009A, 0x009B, 0x0014, 0x0015, 0x009E, 0x001A,
    0x0020, 0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8, 0x00A2, 0x002E, 0x003C, 0x0028, 0x002B, 0x007C,
    0x0026, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF, 0x05E0, 0x05E1, 0x0021, 0x0024, 0x002A, 0x0029, 0x003B, 0x00AC,
    0x002D, 0x002F, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x00A6, 0x002C, 0x0025, 0x005F, 0x003E, 0x003F,
    0xFFFD, 0x05EA, 0xFFFD, 0xFFFD, 0x00A0, 0xFFFD, 0xFFFD, 0xFFFD, 0x2017, 0x0060, 0x003A, 0x0023, 0x0040, 0x0027, 0x003D, 0x0022,
    0x0080, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x00AB, 0x00BB, 0x008C, 0x008D, 0x008E, 0x00B1,
    0x00B0, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x009A, 0x009B, 0x009C, 0x00B8, 0x009E, 0x00A4,
    0x00B5, 0x007E, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x00AE,
    0x005E, 0x00A3, 0x00A5, 0x00B7, 0x00A9, 0x00A7, 0x00B6, 0x00BC, 0x00BD, 0x00BE, 0x005B, 0x005D, 0x00AF, 0x00A8, 0x00B4, 0x00D7,
    0x007B, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x00AD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x007D, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 0x00B9, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x005C, 0x00F7, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00B2, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x00B3, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x009F,
  ],
}

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
    that value to the delimiter without complaint, which is why the generated
    tables are checked against tests/test_wav_charset.inc, page by page.
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
def build(gen):
    if gen in _CACHE:
        return list(_CACHE[gen])
    kind, arg = gen
    if kind == "euro":
        t = build(arg); t[0x9f] = 0x20AC
    elif kind == "table":
        t = list(TABLES[arg])
    else:
        t = via_iconv(arg)
    _CACHE[gen] = list(t)
    return t

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
  each page in a single conversion of its 256 bytes. The few pages that iconv(1)
  gets wrong or does not know are written out in the generator instead, each
  transcribed from the vendor's own table for the page.
 */
"""

def emit_cpp(pages):
    """pages: list of (code page, enum name, description, 256 values)."""
    out = [LIC, "", '#include "charset.h"', "", "namespace TagLib {",
           "  namespace Charset {", "    namespace detail {", ""]
    for num, name, desc, table in pages:
        out.append("      // %d %s" % (num, desc))
        out.append("      constexpr unsigned short %sTable[256] = {" % name)
        for row in range(16):
            cells = ", ".join("0x%04X" % table[row * 16 + col] for col in range(16))
            out.append("        %s," % cells)
        out.append("      };")
        out.append("")
    out.append("      constexpr CodePageEntry codePageTable[] = {")
    for num, name, desc, table in pages:
        out.append('        { Type::%s, %d, "%s", %sTable },' % (name, num, desc, name))
    out.append("      };")
    out.append("")
    out.append("      // Every Type except UTF8 has exactly one row, in Type order.")
    out.append("      static_assert(")
    out.append("        sizeof(codePageTable) / sizeof(codePageTable[0]) + 1 == TypeCount,")
    out.append('        "the code page table and Charset::Type disagree");')
    out.append("")
    out.append("      // Charset::entryFor() indexes this table by the numeric value of a")
    out.append("      // Type, so the order is not merely a convention: it is load bearing.")
    out.append("      static_assert([] {")
    out.append("        for(unsigned int i = 0; i < sizeof(codePageTable) / sizeof(codePageTable[0]); ++i) {")
    out.append("          if(codePageTable[i].type != static_cast<Type>(i))")
    out.append("            return false;")
    out.append("        }")
    out.append("        return true;")
    out.append("      }(), \"the code page table is not in Charset::Type order\");")
    out.append("")
    out.append("    }  // namespace detail")
    out.append("")
    out.append("    const detail::CodePageEntry *detail::codePageEntries(unsigned int *count)")
    out.append("    {")
    out.append("      *count = sizeof(detail::codePageTable) / sizeof(detail::codePageTable[0]);")
    out.append("      return detail::codePageTable;")
    out.append("    }")
    out.append("  }  // namespace Charset")
    out.append("}  // namespace TagLib")
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
