TagLib 2.0 (Jan 24, 2024)
=========================

 * New major version, binary incompatible, but source-compatible with the
   latest 1.x release if no deprecated features are used.
 * Requires a C++17 compiler and uses features of C++17.
 * Major code cleanup, fixed warnings issued by compilers and static analyzers.
 * Made methods virtual which should have been virtual but could not be
   changed to keep binary compatibility, remove related workarounds.
 * Removed deprecated functions:
   - APE::Item::Item(const String &, const String &)
   - APE::Item::toStringList(): Use values()
   - APE::Item::value(): Use binaryData()
   - ASF::Properties::setLength()
   - ByteVector::checksum()
   - ByteVector::isNull(): Use isEmpty()
   - ByteVector::null
   - FLAC::File::setID3v2FrameFactory()
   - FLAC::File::streamInfoData()
   - FLAC::File::streamLength()
   - FLAC::Properties::Properties(File *, ReadStyle)
   - FLAC::Properties::sampleWidth(): Use bitsPerSample()
   - File::isReadable(): Use system functions
   - File::isWritable(): Use system functions
   - FileName::str()
   - FileRef::create(): Use constructor
   - MP4::Tag::itemListMap(): Use itemMap()
   - MPC::File::remove(): Use strip()
   - MPC::Properties::Properties(const ByteVector &, long, ReadStyle)
   - MPEG::File::save(int, ...): Use overload
   - MPEG::File::setID3v2FrameFactory(): Use constructor
   - MPEG::ID3v2::Frame::Header::Header(const ByteVector &, bool)
   - MPEG::ID3v2::Frame::Header::frameAlterPreservation(): Use
     fileAlterPreservation()
   - MPEG::ID3v2::Frame::Header::setData(const ByteVector &, bool)
   - MPEG::ID3v2::Frame::Header::size(unsigned int): Use size()
   - MPEG::ID3v2::Frame::Header::unsycronisation(): use unsynchronisation()
   - MPEG::ID3v2::Frame::checkEncoding(const StringList &, String::Type): Use
     checkTextEncoding(const StringList &, String::Type)
   - MPEG::ID3v2::Frame::headerSize(): Use Header::size()
   - MPEG::ID3v2::Frame::headerSize(unsigned int): Use
     Header::size(unsigned int)
   - MPEG::ID3v2::FrameFactory::createFrame(const ByteVector &, bool)
   - MPEG::ID3v2::FrameFactory::createFrame(const ByteVector &, unsigned int):
     Use createFrame(const ByteVector &, const Header *)
   - MPEG::ID3v2::RelativeVolumeFrame::channelType()
   - MPEG::ID3v2::RelativeVolumeFrame::peakVolume(): Use peakVolume(ChannelType)
   - MPEG::ID3v2::RelativeVolumeFrame::setChannelType()
   - MPEG::ID3v2::RelativeVolumeFrame::setPeakVolume(const PeakVolume &): Use
     setPeakVolume(const PeakVolume &, ChannelType)
   - MPEG::ID3v2::RelativeVolumeFrame::setVolumeAdjustment(float): Use
     setVolumeAdjustment(float, ChannelType)
   - MPEG::ID3v2::RelativeVolumeFrame::setVolumeAdjustmentIndex(short): Use
     setVolumeAdjustmentIndex(short, ChannelType)
   - MPEG::ID3v2::RelativeVolumeFrame::volumeAdjustment(): Use
     volumeAdjustment(ChannelType)
   - MPEG::ID3v2::RelativeVolumeFrame::volumeAdjustmentIndex(): Use
     volumeAdjustmentIndex(ChannelType)
   - MPEG::ID3v2::Tag::footer()
   - MPEG::ID3v2::Tag::render(int): Use render(Version)
   - MPEG::XingHeader::xingHeaderOffset()
   - Ogg::Page::getCopyWithNewPageSequenceNumber()
   - Ogg::XiphComment::removeField(): Use removeFields()
   - PropertyMap::unsupportedData(): Returns now const reference, use
     addUnsupportedData() to add keys
   - RIFF::AIFF::Properties::Properties(const ByteVector &, ReadStyle)
   - RIFF::AIFF::Properties::Properties(const ByteVector &, int, ReadStyle)
   - RIFF::AIFF::Properties::sampleWidth(): Use bitsPerSample()
   - RIFF::WAV::File::save(TagTypes, bool, int): Use
     save(TagTypes, StripTags, Version)
   - RIFF::WAV::File::tag(): Returns now a TagUnion, use ID3v2Tag() to get an
     ID3v2::Tag
   - String::isNull(): Use isEmpty()
   - String::null
   - TrueAudio::File::setID3v2FrameFactory(): Use constructor
   - WavPack::Properties::Properties(const ByteVector &, long, ReadStyle)
* Behavioral changes:
   - The basic tag methods (e.g. genre()) separate multiple values with " / "
     instead of " ".
   - The stream operator for String uses UTF-8 instead of ISO-8859-1 encoding.
   - MP4 property ORIGINALDATE is mapped to "----:com.apple.iTunes:ORIGINALDATE"
     instead of "----:com.apple.iTunes:originaldate".
 * Unified interface for complex properties like pictures.
 * Simplified the unified properties interface by providing its methods on
   FileRef.
 * C bindings: Support for properties (taglib_property_...) and complex
   properties like cover art (taglib_complex_property_...), memory I/O streams.
 * Support for Direct Stream Digital (DSD) stream files (DSF) and interchange
   file format (DSDIFF, DFF), ADTS (AAC) files.
 * The runtime version can be queried.
 * Additional utility functions ByteVector::fromUShort(),
   ByteVector::fromULongLong(), ByteVector::toULongLong(),
   ByteVector::toULongLong(), List::sort().
 * Fixed List::setAutoDelete() affecting implicitly shared copies.
 * Build system: Direct support for CMake, find_package(TagLib) exports target
   TagLib::tag.
 * Build system: Fixed PackageConfig to support both relative and absolute paths.
 * Build system: utf8cpp is no longer included, it can be provided via a system
   package or a Git submodule.
 * ASF: Support additional properties ARTISTWEBPAGE, ENCODING, INITIALKEY,
   ORIGINALALBUM, ORIGINALARTIST, ORIGINALFILENAME, ORIGINALLYRICIST.
 * ID3v2: Fixed extensibility of FrameFactory, use it also for WAV and AIFF
   files.
 * MP4: Support additional properties OWNER, RELEASEDATE.
 * MP4: Introduced ItemFactory allowing clients to support new atom types.
 * MP4: Detect duration from mvhd atom if not present in mdhd atom.
 * MP4: Fixed type of hdvd atom to be  integer instead of boolean.
 * MP4: Tolerate trailing garbage in M4A files.
 * MPC: Fixed content check in presence of an ID3v2 tag.
 * MPEG: Do not scan full file for ID3v2 tag when ReadStyle Fast is used.
 * RIFF: Support properties ALBUM, ARRANGER, ARTIST, ARTISTWEBPAGE, BPM,
   COMMENT, COMPOSER, COPYRIGHT, DATE, DISCSUBTITLE, ENCODEDBY, ENCODING,
   ENCODINGTIME, GENRE, ISRC, LABEL, LANGUAGE, LYRICIST, MEDIA, PERFORMER,
   RELEASECOUNTRY, REMIXER, TITLE, TRACKNUMBER.
 * WAV: Fixed crash with files having the "id3 " chunk as the only valid chunk.
 * Windows: Fixed support for files larger than 2GB.

TagLib 1.13.1 (Jul 1, 2023)
===========================

 * Fixed parsing of TXXX frames without description.
 * Detect MP4 atoms with invalid length or type.
 * Do not miss ID3v2 frames when an extended header is present.
 * Use property "DISCSUBTITLE" for ID3v2 "TSST" frame.
 * Build system improvements: Use absolute path for macOS dylib install name,
   support --define-prefix when using pkg-config, fixed minimum required
   CppUnit version.
 * Code clean up using clang-tidy.

TagLib 1.13 (Oct 27, 2022)
==========================

 * Added interface StreamTypeResolver to support streams which cannot be
   fopen()'ed, e.g. network files.
 * Added MP4::File::strip() to remove meta atom from MP4 file.
 * Added Map::value() to look up without creating entry.
 * Use property "WORK" instead of "CONTENTGROUP" for ID3v2 "TIT1" frame,
   use property "WORK" for ASF "WM/ContentGroupDescription",
   use property "COMPILATION" for ID3v2 "TCMP" frame.
 * Build system improvements: option WITH_ZLIB, BUILD_TESTING instead of
   BUILD_TESTS, GNUInstallDirs, FeatureSummary, tests with BUILD_SHARED_LIBS,
   cross compilation with Buildroot, systems without HAVE_GCC_ATOMIC, Clang.
 * Fixed heap-buffer-overflows when handling ASF, APE, FLAC, ID3v2, MP4, MPC
   tags.
 * Fixed detection of invalid file by extension when correct type can be
   detected by contents.
 * Fixed unnecessary creation of map entries in APE and FLAC tags if looked up
   tag does not exist.
 * Fixed parsing of MP4 non-full meta atoms.
 * Fixed potential ID3v1 false positive in the presence of an APE tag.
 * Fixed ID3v2 version handling for frames embedded in CHAP or CTOC frames.
 * Fixed parsing of multiple strings with a single BOM in ID3v2.4.0.
 * Fixed several smaller issues reported by clang-tidy.

TagLib 1.12 (Feb 16, 2021)
==========================

 * Added support for WinRT.
 * Added support for Linux on POWER.
 * Added support for classical music tags of iTunes 12.5.
 * Added support for file descriptor to FileStream.
 * Added support for 'cmID', 'purl', 'egid' MP4 atoms.
 * Added support for 'GRP1' ID3v2 frame.
 * Added support for extensible WAV subformat.
 * Enabled FileRef to detect file types based on the stream content.
 * Dropped support for Windows 9x and NT 4.0 or older.
 * Check for mandatory header objects in ASF files.
 * More tolerant handling of RIFF padding, WAV files, broken MPEG streams.
 * Improved calculation of Ogg, Opus, Speex, WAV, MP4 bitrates.
 * Improved Windows compatibility by storing FLAC picture after comments.
 * Fixed numerical genres in ID3v2.3.0 'TCON' frames.
 * Fixed consistency of API removing MP4 items when empty values are set.
 * Fixed consistency of API preferring COMM frames with no description.
 * Fixed OOB read on invalid Ogg FLAC files (CVE-2018-11439).
 * Fixed handling of empty MPEG files.
 * Fixed parsing MP4 mdhd timescale.
 * Fixed reading MP4 atoms with zero length.
 * Fixed reading FLAC files with zero-sized seektables.
 * Fixed handling of lowercase field names in Vorbis Comments.
 * Fixed handling of 'rate' atoms in MP4 files.
 * Fixed handling of invalid UTF-8 sequences.
 * Fixed possible file corruptions when saving Ogg files.
 * Fixed handling of non-audio blocks, sampling rates, DSD audio in WavPack files.
 * TableOfContentsFrame::toString() improved.
 * UserTextIdentificationFrame::toString() improved.
 * Marked FileRef::create() deprecated.
 * Marked MPEG::File::save() with boolean parameters deprecated,
   provide overloads with enum parameters.
 * Several smaller bug fixes and performance improvements.

TagLib 1.11.1 (Oct 24, 2016)
============================

 * Fixed binary incompatible change in TagLib::String.
 * Fixed reading ID3v2 CTOC frames with a lot of entries.
 * Fixed seeking ByteVectorStream from the end.

TagLib 1.11 (Apr 29, 2016)
==========================

1.11:

 * Fixed reading APE items with long keys.
 * Fixed reading ID3v2 SYLT frames when description is empty.

1.11 BETA 2:

 * Better handling of PCM WAV files with a 'fact' chunk.
 * Better handling of corrupted APE tags.
 * Efficient decoding of unsynchronized ID3v2 frames.
 * Fixed text encoding when saving certain frames in ID3v2.3 tags.
 * Fixed updating the size of RIFF files when removing chunks.
 * Several smaller bug fixes and performance improvements.

1.11 BETA:

 * New API for creating FileRef from IOStream.
 * Added support for ID3v2 PCST and WFED frames.
 * Added support for pictures in XiphComment.
 * Added String::clear().
 * Added FLAC::File::strip() for removing non-standard tags.
 * Added alternative functions to XiphComment::removeField().
 * Added BUILD_BINDINGS build option.
 * Added ENABLE_CCACHE build option.
 * Replaced ENABLE_STATIC build option with BUILD_SHARED_LIBS.
 * Better handling of duplicate ID3v2 tags in all kinds of files.
 * Better handling of duplicate tag chunks in WAV files.
 * Better handling of duplicate tag chunks in AIFF files.
 * Better handling of duplicate Vorbis comment blocks in FLAC files.
 * Better handling of broken MPEG audio frames.
 * Fixed crash when calling File::properties() after strip().
 * Fixed crash when parsing certain MPEG files.
 * Fixed crash when saving Ogg files.
 * Fixed possible file corruptions when saving ASF files.
 * Fixed possible file corruptions when saving FLAC files.
 * Fixed possible file corruptions when saving MP4 files.
 * Fixed possible file corruptions when saving MPEG files.
 * Fixed possible file corruptions when saving APE files.
 * Fixed possible file corruptions when saving Musepack files.
 * Fixed possible file corruptions when saving WavPack files.
 * Fixed updating the comment field of Vorbis comments.
 * Fixed reading date and time in ID3v2.3 tags.
 * Marked ByteVector::null and ByteVector::isNull() deprecated.
 * Marked String::null and String::isNull() deprecated.
 * Marked XiphComment::removeField() deprecated.
 * Marked Ogg::Page::getCopyWithNewPageSequenceNumber() deprecated. It returns null.
 * Marked custom integer types deprecated.
 * Many smaller bug fixes and performance improvements.

TagLib 1.10 (Nov 11, 2015)
==========================

1.10:

 * Added new options to the tagwriter example.
 * Fixed self-assignment operator in some types.
 * Fixed extraction of MP4 tag keys with an empty list.

1.10 BETA:

 * New API for the audio length in milliseconds.
 * Added support for ID3v2 ETCO and SYLT frames.
 * Added support for album artist in PropertyMap API of MP4 files.
 * Added support for embedded frames in ID3v2 CHAP and CTOC frames.
 * Added support for AIFF-C files.
 * Better handling of duplicate ID3v2 tags in MPEG files.
 * Allowed generating taglib.pc on Windows.
 * Added ZLIB_SOURCE build option.
 * Fixed backwards-incompatible change in TagLib::String when constructing UTF16 strings.
 * Fixed crash when parsing certain FLAC files.
 * Fixed crash when encoding empty strings.
 * Fixed saving of certain XM files on OS X.
 * Changed Xiph and APE generic getters to return space-concatenated values.
 * Fixed possible file corruptions when removing tags from WAV files.
 * Added support for MP4 files with 64-bit atoms in certain 64-bit environments.
 * Prevented ID3v2 padding from being too large.
 * Fixed crash when parsing corrupted APE files.
 * Fixed crash when parsing corrupted WAV files.
 * Fixed crash when parsing corrupted Ogg FLAC files.
 * Fixed crash when parsing corrupted MPEG files.
 * Fixed saving empty tags in WAV files.
 * Fixed crash when parsing corrupted Musepack files.
 * Fixed possible memory leaks when parsing AIFF and WAV files.
 * Fixed crash when parsing corrupted MP4 files.
 * Stopped writing empty ID3v2 frames.
 * Fixed possible file corruptions when saving WMA files.
 * Added TagLib::MP4::Tag::isEmpty().
 * Added accessors to manipulate MP4 tags.
 * Fixed crash when parsing corrupted WavPack files.
 * Fixed seeking MPEG frames.
 * Fixed reading FLAC files with zero-sized padding blocks.
 * Added support for reading the encoder information of WMA files.
 * Added support for reading the codec of WAV files.
 * Added support for multi channel WavPack files.
 * Added support for reading the nominal bitrate of Ogg Speex files.
 * Added support for VBR headers in MPEG files.
 * Marked FLAC::File::streamInfoData() deprecated. It returns an empty ByteVector.
 * Marked FLAC::File::streamLength() deprecated. It returns zero.
 * Fixed possible file corruptions when adding an ID3v1 tag to FLAC files.
 * Many smaller bug fixes and performance improvements.

TagLib 1.9.1 (Oct 8, 2013)
==========================

 * Fixed binary incompatible change in TagLib::Map and TagLib::List.
 * Fixed constructing String from ByteVector.
 * Fixed compilation on MSVC with the /Zc:wchar_t- option.
 * Fixed detecting of RIFF files with invalid chunk sizes.
 * Added TagLib::MP4::Properties::codec().

TagLib 1.9 (Oct 6, 2013)
========================

 * Added support for the Ogg Opus file format.
 * Added support for INFO tags in WAV files.
 * Changed FileStream to use Windows file API.
 * Included taglib-config.cmd script for Windows.
 * New ID3v1::Tag methods for working directly with genre numbers.
 * New MPEG::File methods for checking which tags are saved in the file.
 * Added support for the PropertyMap API to ASF and MP4 files.
 * Added MusicBrainz identifiers to the PropertyMap API.
 * Allowed reading of MP4 cover art without an explicitly specified format.
 * Better parsing of corrupted FLAC files.
 * Fixed saving of PropertyMap comments without description into ID3v2 tags.
 * Fixed crash when parsing certain XM files.
 * Fixed compilation of unit test with clang.
 * Better handling of files that can't be open or have read-only permissions.
 * Improved atomic reference counting.
 * New hookable API for debug messages.
 * More complete Windows install instructions.
 * Many smaller bug fixes and performance improvements.

TagLib 1.8 (Sep 6, 2012)
========================

1.8:

 * Added support for OWNE ID3 frames.
 * Changed key validation in the new PropertyMap API.
 * ID3v1::Tag::setStringHandler will no longer delete the previous handler,
   the caller is responsible for this.
 * File objects will also no longer delete the passed IOStream objects. It
   should be done in the caller code after the File object is no longer
   used.
 * Added ID3v2::Tag::setLatin1StringHandler for custom handling of
   latin1-encoded text in ID3v2 frames.
 * Fixed validation of ID3v2 frame IDs (IDs with '0' were ignored).

1.8 BETA:

 * New API for accessing tags by name.
 * New abstract I/O stream layer to allow custom I/O handlers.
 * Support for writing ID3v2.3 tags.
 * Support for various module file formats (MOD, S3M, IT, XM).
 * Support for MP4 and ASF is now enabled by default.
 * Started using atomic int operations for reference counting.
 * Added methods for checking if WMA and MP4 files are DRM-protected.
 * Added taglib_free to the C bindings.
 * New method to allow removing pictures from FLAC files.
 * Support for reading audio properties from ALAC and Musepack SV8 files.
 * Added replay-gain information to Musepack audio properties.
 * Support for APEv2 binary tags.
 * Many AudioProperties subclasses now provide information about the total number of samples.
 * Various small bug fixes.

TagLib 1.7.2 (Apr 20, 2012)
===========================

 * Fixed division by zero while parsing corrupted MP4 files (CVE-2012-2396).
 * Fixed compilation on Haiku.

TagLib 1.7.1 (Mar 17, 2012)
===========================

 * Improved parsing of corrupted WMA, RIFF and OGG files.
 * Fixed a memory leak in the WMA parser.
 * Fixed a memory leak in the FLAC parser.
 * Fixed a possible division by zero in the APE parser.
 * Added detection of TTA2 files.
 * Fixed saving of multiple identically named tags to Vorbis Comments.

TagLib 1.7 (Mar 11, 2011)
=========================

1.7:

 * Fixed memory leaks in the FLAC file format parser.
 * Fixed bitrate calculation for WAV files.

1.7 RC1:

 * Support for reading/writing tags from Monkey's Audio files. (BUG:210404)
 * Support for reading/writing embedded pictures from WMA files.
 * Support for reading/writing embedded pictures from FLAC files (BUG:218696).
 * Implemented APE::Tag::isEmpty() to check for all APE tags, not just the
   basic ones.
 * Added reading of WAV audio length. (BUG:116033)
 * Exposed FLAC MD5 signature of the uncompressed audio stream via
   FLAC::Properties::signature(). (BUG:160172)
 * Added function ByteVector::toHex() for hex-encoding of byte vectors.
 * WavPack reader now tries to get the audio length by finding the final
   block, if the header doesn't have the information. (BUG:258016)
 * Fixed a memory leak in the ID3v2.2 PIC frame parser. (BUG:257007)
 * Fixed writing of RIFF files with even chunk sizes. (BUG:243954)
 * Fixed compilation on MSVC 2010.
 * Removed support for building using autoconf/automake.
 * API docs can be now built using "make docs".

TagLib 1.6.3 (Apr 17, 2010)
===========================

 * Fixed definitions of the TAGLIB_WITH_MP4 and TAGLIB_WITH_ASF macros.
 * Fixed upgrading of ID3v2.3 genre frame with ID3v1 code 0 (Blues).
 * New method `int String::toInt(bool *ok)` which can return whether the
   conversion to a number was successful.
 * Fixed parsing of incorrectly written lengths in ID3v2 (affects mainly
   compressed frames). (BUG:231075)

TagLib 1.6.2 (Apr 9, 2010)
==========================

 * Read Vorbis Comments from the first FLAC metadata block, if there are
   multiple ones. (BUG:211089)
 * Fixed a memory leak in FileRef's OGA format detection.
 * Fixed compilation with the Sun Studio compiler. (BUG:215225)
 * Handle WM/TrackNumber attributes with DWORD content in WMA files.
   (BUG:218526)
 * More strict check if something is a valid MP4 file. (BUG:216819)
 * Correctly save MP4 int-pair atoms with flags set to 0.
 * Fixed compilation of the test runner on Windows.
 * Store ASF attributes larger than 64k in the metadata library object.
 * Ignore trailing non-data atoms when parsing MP4 covr atoms.
 * Don't upgrade ID3v2.2 frame TDA to TDRC. (BUG:228968)

TagLib 1.6.1 (Oct 31, 2009)
===========================

 * Better detection of the audio codec of .oga files in FileRef.
 * Fixed saving of Vorbis comments to Ogg FLAC files. TagLib tried to
   include the Vorbis framing bit, which is only correct for Ogg Vorbis.
 * Public symbols now have explicitly set visibility to "default" on GCC.
 * Added missing exports for static ID3v1 functions.
 * Fixed a typo in taglib_c.pc
 * Fixed a failing test on ppc64.
 * Support for binary 'covr' atom in MP4 files. TagLib 1.6 treated them
   as text atoms, which corrupted them in some cases.
 * Fixed ID3v1-style genre to string conversion in MP4 files.

TagLib 1.6 (Sep 13, 2009)
=========================

1.6:

 * New CMake option to build a static version - ENABLE_STATIC.
 * Added support for disabling dllimport/dllexport on Windows using the
   TAGLIB_STATIC macro.
 * Support for parsing the obsolete 'gnre' MP4 atom.
 * New cpp macros TAGLIB_WITH_MP4 and TAGLIB_WITH_ASF to determine if
   TagLib was built with MP4/ASF support.

1.6 RC1:

 * Split Ogg packets larger than 64k into multiple pages. (BUG:171957)
 * TagLib can now use FLAC padding block. (BUG:107659)
 * ID3v2.2 frames are now not incorrectly saved. (BUG:176373)
 * Support for ID3v2.2 PIC frames. (BUG:167786)
 * Fixed a bug in ByteVectorList::split().
 * XiphComment::year() now falls back to YEAR if DATE doesn't exist
   and XiphComment::year() falls back to TRACKNUM if TRACKNUMBER doesn't
   exist. (BUG:144396)
 * Improved ID3v2.3 genre parsing. (BUG:188578)
 * Better checking of corrupted ID3v2 APIC data. (BUG:168382)
 * Bitrate calculating using the Xing header now uses floating point
   numbers. (BUG:172556)
 * New TagLib::String method rfind().
 * Added support for MP4 file format with iTunes-style metadata [optional].
 * Added support for ASF (WMA) file format [optional].
 * Fixed crash when saving a Locator APEv2 tag. (BUG:169810)
 * Fixed a possible crash in the non-const version of String::operator[]
   and in String::operator+=. (BUG:169389)
 * Added support for PRIV ID3v2 frames.
 * Empty ID3v2 genres are no longer treated as numeric ID3v1 genres.
 * Added support for the POPM (rating/play count) ID3v2 frame.
 * Generic RIFF file format support:
   * Support for AIFF files with ID3v2 tags.
   * Support for WAV files with ID3v2 tags.
 * Fixed crash on handling unsupported ID3v2 frames, e.g. on encrypted
   frames. (BUG:161721)
 * Fixed overflow while calculating bitrate of FLAC files with a very
   high bitrate.
