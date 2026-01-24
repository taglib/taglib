#include <cstdio>
#include "matroskafile.h"
#include "matroskatag.h"
#include "matroskasimpletag.h"
#include "matroskaattachments.h"
#include "matroskaattachedfile.h"
#include "tfilestream.h"
#include "tstring.h"
#include "tutils.h"

int main(int argc, char *argv[])
{
  if(argc != 3) {
    printf("Usage: matroskawriter FILE ARTWORK\n");
    return 1;
  }
  TagLib::Matroska::File file(argv[1]);
  if(!file.isValid()) {
    printf("File is not valid\n");
    return 1;
  }
  auto tag = file.tag(true);
  tag->clearSimpleTags();

  tag->addSimpleTag(TagLib::Matroska::SimpleTag(
    "Test Name 1", TagLib::String("Test Value 1"),
    TagLib::Matroska::SimpleTag::TargetTypeValue::Track, "en"));

  tag->addSimpleTag(TagLib::Matroska::SimpleTag(
    "Test Name 2", TagLib::String("Test Value 2"),
    TagLib::Matroska::SimpleTag::TargetTypeValue::Album));
  tag->setTitle("Test title");
  tag->setArtist("Test artist");
  tag->setYear(1969);

  TagLib::FileStream image(argv[2]);
  auto data = image.readBlock(image.length());
  auto attachments = file.attachments(true);
  attachments->addAttachedFile(TagLib::Matroska::AttachedFile(
    data, "cover.jpg", "image/jpeg"));

  file.save();

  return 0;
}
