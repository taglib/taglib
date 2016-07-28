#include "taglib.h"
#include "tstring.h"
#include <string>

using namespace TagLib;

String TagLib::GetVersionString()
{
  return String::number(TAGLIB_MAJOR_VERSION)
      + "." + String::number(TAGLIB_MINOR_VERSION)
      + "." + String::number(TAGLIB_PATCH_VERSION);
}

unsigned int TagLib::GetMajorVersion()
{
  return TAGLIB_MAJOR_VERSION;
}

unsigned int TagLib::GetMinorVersion()
{
  return TAGLIB_MINOR_VERSION;
}

unsigned int TagLib::GetPatchVersion()
{
  return TAGLIB_PATCH_VERSION;
}

unsigned int TagLib::GetVersion()
{
  return (TAGLIB_MAJOR_VERSION << 16)
          || (TAGLIB_MINOR_VERSION << 8)
          || (TAGLIB_PATCH_VERSION << 4);
}
