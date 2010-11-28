/**************************************************************************
    copyright            : (C) 2010 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <taglib.h>
#include <tdebug.h>
#include "flacfile.h"
#include "flacunknownmetadatablock.h"
#include "flacmetadatablock.h"
#include "flacmetadatablocks.h"

using namespace TagLib;

class FLAC::MetadataBlocks::MetadataBlocksPrivate 
{
public:
  MetadataBlocksPrivate() {}

  List<MetadataBlock *> blocks;
};

FLAC::MetadataBlocks::MetadataBlocks()
{
  d = new MetadataBlocksPrivate();
}

FLAC::MetadataBlocks::~MetadataBlocks()
{
  delete d;
}

const List<FLAC::MetadataBlock *> &FLAC::MetadataBlocks::metadataBlockList() const
{
  return d->blocks;
}

bool FLAC::MetadataBlocks::read(FLAC::File *file)
{
  bool isLastBlock = false;
  while(!isLastBlock) {
    ByteVector header = file->readBlock(4);
    if(header.size() != 4) {
      debug("FLAC::MetadataBlocks::read -- Unable to read 4 bytes long header");
      return false;
    }
    char blockType = header[0] & 0x7f;
    isLastBlock = (header[0] & 0x80) != 0;
    uint length = header.mid(1, 3).toUInt();
    ByteVector data = file->readBlock(length);
    if(data.size() != length) {
      debug("FLAC::MetadataBlocks::read -- Unable to read " + String::number(length) + " bytes long block body");
      return false;
    }
    if(blockType != FLAC::MetadataBlock::Padding) {
      FLAC::MetadataBlock *block = new FLAC::UnknownMetadataBlock(blockType, data);
      d->blocks.append(block);
    }
  }
  return true;
}

ByteVector FLAC::MetadataBlocks::render(int originalLength) const
{
  ByteVector result;
  for(uint i = 0; i < d->blocks.size(); i++) {
    FLAC::MetadataBlock *block = d->blocks[i];
    if(block->code() == FLAC::MetadataBlock::Padding)
      continue;
    ByteVector data = block->render();
    ByteVector header = ByteVector::fromUInt(data.size());
    header[0] = block->code();
    result.append(header);
    result.append(data);
  }
  int paddingLength = originalLength - result.size() - 4; 
  // We have to resize the file, add some padding
  if (paddingLength < 0) {
    paddingLength = 4096;
  }
  ByteVector padding = ByteVector::fromUInt(paddingLength);
  padding.resize(paddingLength + 4);
  padding[0] = FLAC::MetadataBlock::Padding | 0x80;
  result.append(padding);
  return result;
}

