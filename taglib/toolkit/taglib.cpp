#include "taglib.h"
#include "tstring.h"
#include <string>

using namespace TagLib;


String TagLib::Version::string()
{
  return String::number(TAGLIB_MAJOR_VERSION)
      + "." + String::number(TAGLIB_MINOR_VERSION)
      + "." + String::number(TAGLIB_PATCH_VERSION);
}

unsigned int TagLib::Version::combined()
{
  return (TAGLIB_MAJOR_VERSION << 16)
          || (TAGLIB_MINOR_VERSION << 8)
          || (TAGLIB_PATCH_VERSION << 4);
}


unsigned int (TagLib::Version::major)()
{
  return TAGLIB_MAJOR_VERSION;
}

unsigned int (TagLib::Version::minor)()
{
  return TAGLIB_MINOR_VERSION;
}

unsigned int TagLib::Version::patch()
{
  return TAGLIB_PATCH_VERSION;
}

