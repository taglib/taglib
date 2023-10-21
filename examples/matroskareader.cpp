#include <cstdio>
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskasimpletag.h"
#include "tstring.h"
#include "tutils.h"
#include "tbytevector.h"
#define GREEN_TEXT(s) "[1;32m" s "[0m"
#define PRINT_PRETTY(label, value) printf("" GREEN_TEXT(label) ": %s\n", value)

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("Usage: matroskareader FILE\n");
    return 1;
  }
  TagLib::Matroska::File file(argv[1]);
  if (!file.isValid()) {
    printf("File is not valid\n");
    return 1;
  }
  auto tag = dynamic_cast<TagLib::Matroska::Tag*>(file.tag());
  if (!tag) {
    printf("File has no tag\n");
    return 0;
  }

  const TagLib::Matroska::SimpleTagsList &list = tag->simpleTagsList();
  printf("Found %i tags:\n\n", list.size());
  
  for (TagLib::Matroska::SimpleTag *t : list) {
    PRINT_PRETTY("Tag Name", t->name().toCString(true));

    TagLib::Matroska::SimpleTagString *tString = nullptr;
    TagLib::Matroska::SimpleTagBinary *tBinary = nullptr;
    if ((tString = dynamic_cast<TagLib::Matroska::SimpleTagString*>(t)))
      PRINT_PRETTY("Tag Value", tString->value().toCString(true));
    else if ((tBinary = dynamic_cast<TagLib::Matroska::SimpleTagBinary*>(t)))
      PRINT_PRETTY("Tag Value", 
        TagLib::Utils::formatString("Binary with size %i", tBinary->value().size()).toCString(false)
      );

    auto targetTypeValue = static_cast<unsigned int>(t->targetTypeValue());
    PRINT_PRETTY("Target Type Value",
      targetTypeValue == 0 ? "None" : TagLib::Utils::formatString("%i", targetTypeValue).toCString(false)
    );
    const TagLib::String &language = t->language();
    PRINT_PRETTY("Language", !language.isEmpty() ? language.toCString(false) : "Not set");

    printf("\n");
  }

  return 0;
}
