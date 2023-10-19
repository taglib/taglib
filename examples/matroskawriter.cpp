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

  auto simpleTag = new TagLib::Matroska::SimpleTagString();
  simpleTag->setName("Test Name 1");
  simpleTag->setTargetTypeValue(TagLib::Matroska::SimpleTag::TargetTypeValue::Track);
  simpleTag->setValue("Test Value 1");
  simpleTag->setLanguage("en");
  tag->addSimpleTag(simpleTag);

  simpleTag = new TagLib::Matroska::SimpleTagString();
  simpleTag->setName("Test Name 2");
  simpleTag->setTargetTypeValue(TagLib::Matroska::SimpleTag::TargetTypeValue::Album);
  simpleTag->setValue("Test Value 2");
  tag->addSimpleTag(simpleTag);
  tag->setTitle("Test title");
  tag->setArtist("Test artist");
  tag->setYear(1969);

  TagLib::FileStream image(argv[2]);
  auto data = image.readBlock(image.length());
  auto attachments = file.attachments(true);
  auto attachedFile = new TagLib::Matroska::AttachedFile();
  attachedFile->setFileName("cover.jpg");
  attachedFile->setMediaType("image/jpeg");
  attachedFile->setData(data);
  //attachedFile->setUID(5081000385627515072ull);
  attachments->addAttachedFile(attachedFile);

  file.save();

  return 0;
}
