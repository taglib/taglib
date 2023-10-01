#include <cstdio>
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskasimpletag.h"
#include "tstring.h"
#include "tutils.h"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("Usage: matroskawriter FILE\n");
    return 1;
  }
  TagLib::Matroska::File file(argv[1]);
  if (!file.isValid()) {
    printf("File is not valid\n");
    return 1;
  }
  auto tag = file.tag(true);
  tag->clearSimpleTags();

  auto simpleTag = new TagLib::Matroska::SimpleTagString();
  simpleTag->setName("Test Name 1");
  simpleTag->setTargetTypeValue(TagLib::Matroska::SimpleTag::TargetTypeValue::Track);
  simpleTag->setValue("Test Value 1");
  tag->addSimpleTag(simpleTag);

  simpleTag = new TagLib::Matroska::SimpleTagString();
  simpleTag->setName("Test Name 2");
  simpleTag->setTargetTypeValue(TagLib::Matroska::SimpleTag::TargetTypeValue::Album);
  simpleTag->setValue("Test Value 2");
  tag->addSimpleTag(simpleTag);

  file.save();

  return 0;
}
