/* Copyright (C) 2003 Scott Wheeler <wheeler@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>

#include "tag_c.h"

#ifndef FALSE
#define FALSE 0
#endif

int main(int argc, char *argv[])
{
  int i;

  taglib_set_strings_unicode(1);

  for(i = 1; i < argc; i++) {
    TagLib_File *file;
    TagLib_Tag *tag;
    const TagLib_AudioProperties *properties;
    char **propertiesMap;
    char **complexKeys;

    printf("******************** \"%s\" ********************\n", argv[i]);

    file = taglib_file_new(argv[i]);

    if(file == NULL)
      break;

    tag = taglib_file_tag(file);
    properties = taglib_file_audioproperties(file);
    propertiesMap = taglib_property_keys(file);
    complexKeys = taglib_complex_property_keys(file);

    if(tag != NULL) {
      printf("-- TAG (basic) --\n");
      printf("title   - \"%s\"\n", taglib_tag_title(tag));
      printf("artist  - \"%s\"\n", taglib_tag_artist(tag));
      printf("album   - \"%s\"\n", taglib_tag_album(tag));
      printf("year    - \"%u\"\n", taglib_tag_year(tag));
      printf("comment - \"%s\"\n", taglib_tag_comment(tag));
      printf("track   - \"%u\"\n", taglib_tag_track(tag));
      printf("genre   - \"%s\"\n", taglib_tag_genre(tag));
    }


    if(propertiesMap != NULL) {
      char **keyPtr = propertiesMap;
      int longest = 0;
      while(*keyPtr) {
        int len = (int)strlen(*keyPtr++);
        if(len > longest) {
          longest = len;
        }
      }
      keyPtr = propertiesMap;

      printf("-- TAG (properties) --\n");
      while(*keyPtr) {
        char **valPtr;
        char **propertyValues = valPtr = taglib_property_get(file, *keyPtr);
        while(valPtr && *valPtr)
        {
          printf("%-*s - \"%s\"\n", longest, *keyPtr, *valPtr++);
        }
        taglib_property_free(propertyValues);
        ++keyPtr;
      }
    }

    if(complexKeys != NULL) {
      char **keyPtr = complexKeys;
      while(*keyPtr) {
        TagLib_Complex_Property_Attribute*** props =
          taglib_complex_property_get(file, *keyPtr);
        if(props != NULL) {
          TagLib_Complex_Property_Attribute*** propPtr = props;
          while(*propPtr) {
            TagLib_Complex_Property_Attribute** attrPtr = *propPtr;
            printf("%s:\n", *keyPtr);
            while(*attrPtr) {
              TagLib_Complex_Property_Attribute *attr = *attrPtr;
              TagLib_Variant_Type type = attr->value.type;
              printf("  %-11s - ", attr->key);
              switch(type) {
              case TagLib_Variant_Void:
                printf("null\n");
                break;
              case TagLib_Variant_Bool:
                printf("%s\n", attr->value.value.boolValue ? "true" : "false");
                break;
              case TagLib_Variant_Int:
                printf("%d\n", attr->value.value.intValue);
                break;
              case TagLib_Variant_UInt:
                printf("%u\n", attr->value.value.uIntValue);
                break;
              case TagLib_Variant_LongLong:
                printf("%lld\n", attr->value.value.longLongValue);
                break;
              case TagLib_Variant_ULongLong:
                printf("%llu\n", attr->value.value.uLongLongValue);
                break;
              case TagLib_Variant_Double:
                printf("%f\n", attr->value.value.doubleValue);
                break;
              case TagLib_Variant_String:
                printf("\"%s\"\n", attr->value.value.stringValue);
                break;
              case TagLib_Variant_StringList:
                if(attr->value.value.stringListValue) {
                  char **strs = attr->value.value.stringListValue;
                  char **s = strs;
                  while(*s) {
                    if(s != strs) {
                      printf(" ");
                    }
                    printf("%s", *s++);
                  }
                }
                printf("\n");
                break;
              case TagLib_Variant_ByteVector:
                printf("(%u bytes)\n", attr->value.size);
                break;
              }
              ++attrPtr;
            }
            ++propPtr;
          }
          taglib_complex_property_free(props);
        }
        ++keyPtr;
      }
      taglib_complex_property_free_keys(complexKeys);
    }

    if(properties != NULL) {
      int seconds = taglib_audioproperties_length(properties) % 60;
      int minutes = (taglib_audioproperties_length(properties) - seconds) / 60;

      printf("-- AUDIO --\n");
      printf("bitrate     - %i\n", taglib_audioproperties_bitrate(properties));
      printf("sample rate - %i\n", taglib_audioproperties_samplerate(properties));
      printf("channels    - %i\n", taglib_audioproperties_channels(properties));
      printf("length      - %i:%02i\n", minutes, seconds);
    }

    taglib_property_free(propertiesMap);
    taglib_tag_free_strings();
    taglib_file_free(file);
  }

  return 0;
}
