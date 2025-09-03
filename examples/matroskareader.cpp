#include <cstdio>
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskasimpletag.h"
#include "matroskaattachments.h"
#include "matroskaattachedfile.h"
#include "tstring.h"
#include "tutils.h"
#include "tbytevector.h"
#define GREEN_TEXT(s) "[1;32m" s "[0m"
#define PRINT_PRETTY(label, value) printf("  " GREEN_TEXT(label) ": %s\n", value)

int main(int argc, char *argv[])
{
  if(argc != 2) {
    printf("Usage: matroskareader FILE\n");
    return 1;
  }
  TagLib::Matroska::File file(argv[1]);
  if(!file.isValid()) {
    printf("File is not valid\n");
    return 1;
  }
  auto tag = dynamic_cast<TagLib::Matroska::Tag*>(file.tag());
  if(!tag) {
    printf("File has no tag\n");
    return 0;
  }

  const TagLib::Matroska::SimpleTagsList &list = tag->simpleTagsList();
  printf("Found %u tag(s):\n", list.size());

  for(const TagLib::Matroska::SimpleTag &t : list) {
    PRINT_PRETTY("Tag Name", t.name().toCString(true));

    if(t.type() == TagLib::Matroska::SimpleTag::StringType)
      PRINT_PRETTY("Tag Value", t.toString().toCString(true));
    else if(t.type() == TagLib::Matroska::SimpleTag::BinaryType)
      PRINT_PRETTY("Tag Value",
        TagLib::Utils::formatString("Binary with size %i", t.toByteVector().size()).toCString(false)
      );

    auto targetTypeValue = static_cast<unsigned int>(t.targetTypeValue());
    PRINT_PRETTY("Target Type Value",
      targetTypeValue == 0 ? "None" : TagLib::Utils::formatString("%i", targetTypeValue).toCString(false)
    );
    if(auto trackUid = t.trackUid()) {
      PRINT_PRETTY("Track UID",
        TagLib::Utils::formatString("%llu",trackUid).toCString(false)
      );
    }
    const TagLib::String &language = t.language();
    PRINT_PRETTY("Language", !language.isEmpty() ? language.toCString(false) : "Not set");

    printf("\n");
  }

  TagLib::Matroska::Attachments *attachments = file.attachments();
  if(attachments) {
    const TagLib::Matroska::Attachments::AttachedFileList &list = attachments->attachedFileList();
    printf("Found %u attachment(s)\n", list.size());
    for(const auto &attachedFile : list) {
      PRINT_PRETTY("Filename", attachedFile.fileName().toCString(true));
      const TagLib::String &description = attachedFile.description();
      PRINT_PRETTY("Description", !description.isEmpty() ? description.toCString(true) : "None");
      const TagLib::String &mediaType = attachedFile.mediaType();
      PRINT_PRETTY("Media Type", !mediaType.isEmpty() ? mediaType.toCString(false) : "None");
      PRINT_PRETTY("Data Size",
        TagLib::Utils::formatString("%u byte(s)",attachedFile.data().size()).toCString(false)
      );
      PRINT_PRETTY("UID",
        TagLib::Utils::formatString("%llu",attachedFile.uid()).toCString(false)
      );
    }
  }
  else
    printf("File has no attachments\n");

  if(auto properties = dynamic_cast<const TagLib::Matroska::Properties *>(file.audioProperties())) {
    printf("Properties:\n");
    PRINT_PRETTY("Doc Type", properties->docType().toCString(false));
    PRINT_PRETTY("Doc Type Version", TagLib::String::number(properties->docTypeVersion()).toCString(false));
    PRINT_PRETTY("Codec Name", properties->codecName().toCString(true));
    PRINT_PRETTY("Bitrate", TagLib::String::number(properties->bitrate()).toCString(false));
    PRINT_PRETTY("Sample Rate", TagLib::String::number(properties->sampleRate()).toCString(false));
    PRINT_PRETTY("Channels", TagLib::String::number(properties->channels()).toCString(false));
    PRINT_PRETTY("Length [ms]", TagLib::String::number(properties->lengthInMilliseconds()).toCString(false));
  }
  return 0;
}
