/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include "oggfile.h"

#include <utility>

#include "tdebug.h"
#include "tmap.h"
#include "oggpage.h"
#include "oggpageheader.h"

using namespace TagLib;

namespace
{
  // Returns the first packet index of the right next page to the given one.
  unsigned int nextPacketIndex(const Ogg::Page *page)
  {
    if(page->header()->lastPacketCompleted())
      return page->firstPacketIndex() + page->packetCount();
    return page->firstPacketIndex() + page->packetCount() - 1;
  }
}  // namespace

class Ogg::File::FilePrivate
{
public:
  FilePrivate()
  {
    pages.setAutoDelete(true);
  }

  List<Page *> pages;
  std::unique_ptr<PageHeader> firstPageHeader;
  std::unique_ptr<PageHeader> lastPageHeader;
  Map<unsigned int, ByteVector> dirtyPackets;

  // File offset of the next page to read while scanning the file.  This tracks
  // the physical position independently of the logical stream's pages so that
  // pages of other multiplexed streams can be skipped.
  offset_t currentPageOffset { -1 };

  // Serial number of the logical bitstream packets are read from.  In a
  // multiplexed Ogg stream only pages of this stream are considered.
  unsigned int streamSerialNumber { 0 };
  bool streamSerialNumberSet { false };
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Ogg::File::~File() = default;

ByteVector Ogg::File::packet(unsigned int i)
{
  // Check to see if we're called setPacket() for this packet since the last
  // save:

  if(d->dirtyPackets.contains(i))
    return d->dirtyPackets[i];

  // If we haven't indexed the page where the packet we're interested in starts,
  // begin reading pages until we have.

  if(!readPages(i)) {
    debug("Ogg::File::packet() -- Could not find the requested packet.");
    return ByteVector();
  }

  // Look for the first page in which the requested packet starts.

  auto it = d->pages.cbegin();
  while((*it)->containsPacket(i) == Page::DoesNotContainPacket)
    ++it;

  // If the packet is completely contained in the first page that it's in.

  // If the packet is *not* completely contained in the first page that it's a
  // part of then that packet trails off the end of the page.  Continue appending
  // the pages' packet data until we hit a page that either does not end with the
  // packet that we're fetching or where the last packet is complete.

  ByteVector packet = (*it)->packets()[i - (*it)->firstPacketIndex()];

  while(nextPacketIndex(*it) <= i) {
    ++it;
    packet.append((*it)->packets().front());
  }

  return packet;
}

void Ogg::File::setPacket(unsigned int i, const ByteVector &p)
{
  if(!readPages(i)) {
    debug("Ogg::File::setPacket() -- Could not set the requested packet.");
    return;
  }

  d->dirtyPackets[i] = p;
}

const Ogg::PageHeader *Ogg::File::firstPageHeader()
{
  if(!d->firstPageHeader) {
    const offset_t firstPageHeaderOffset = find("OggS");
    if(firstPageHeaderOffset < 0)
      return nullptr;

    d->firstPageHeader = std::make_unique<PageHeader>(this, firstPageHeaderOffset);
  }

  return d->firstPageHeader->isValid() ? d->firstPageHeader.get() : nullptr;
}

const Ogg::PageHeader *Ogg::File::lastPageHeader()
{
  if(!d->lastPageHeader) {
    const offset_t lastPageHeaderOffset = rfind("OggS");
    if(lastPageHeaderOffset < 0)
      return nullptr;

    d->lastPageHeader = std::make_unique<PageHeader>(this, lastPageHeaderOffset);
  }

  return d->lastPageHeader->isValid() ? d->lastPageHeader.get() : nullptr;
}

bool Ogg::File::save()
{
  if(readOnly()) {
    debug("Ogg::File::save() - Cannot save to a read only file.");
    return false;
  }

  for(const auto &[i, pkt] : std::as_const(d->dirtyPackets))
    writePacket(i, pkt);

  d->dirtyPackets.clear();

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

Ogg::File::File(FileName file) :
  TagLib::File(file),
  d(std::make_unique<FilePrivate>())
{
}

Ogg::File::File(IOStream *stream) :
  TagLib::File(stream),
  d(std::make_unique<FilePrivate>())
{
}

bool Ogg::File::selectStream(const ByteVector &magic)
{
  // All beginning-of-stream pages of a (possibly multiplexed) Ogg stream
  // appear at the very start of the file, before any secondary pages.  Inspect
  // each one's first packet and lock onto the first logical bitstream whose
  // identification header matches magic.

  offset_t offset = find("OggS");
  if(offset < 0)
    return false;

  while(true) {
    Page page(this, offset);
    if(!page.header()->isValid())
      return false;

    // Once the beginning-of-stream pages are exhausted there are no more
    // logical bitstreams to discover.
    if(!page.header()->firstPageOfStream())
      return false;

    const ByteVectorList packets = page.packets();
    if(!packets.isEmpty() && packets.front().startsWith(magic)) {
      d->streamSerialNumber = page.header()->streamSerialNumber();
      d->streamSerialNumberSet = true;
      return true;
    }

    offset += page.size();
  }
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

bool Ogg::File::readPages(unsigned int i)
{
  while(true) {

    // If we've already indexed the page containing packet i, we're done.

    if(!d->pages.isEmpty()) {
      const Page *page = d->pages.back();
      if(nextPacketIndex(page) > i)
        return true;
      if(page->header()->lastPageOfStream())
        return false;
    }

    // Locate the first page in the file if we haven't started scanning yet.

    if(d->currentPageOffset < 0) {
      d->currentPageOffset = find("OggS");
      if(d->currentPageOffset < 0)
        return false;
    }

    // Read pages until we find the next one belonging to our logical bitstream,
    // skipping pages of other streams in a multiplexed Ogg stream.

    Page *nextPage;
    while(true) {
      nextPage = new Page(this, d->currentPageOffset);
      if(!nextPage->header()->isValid()) {
        delete nextPage;
        return false;
      }

      d->currentPageOffset += nextPage->size();

      const unsigned int serial = nextPage->header()->streamSerialNumber();
      if(!d->streamSerialNumberSet) {
        d->streamSerialNumber = serial;
        d->streamSerialNumberSet = true;
      }

      if(serial == d->streamSerialNumber)
        break;

      delete nextPage;
    }

    const unsigned int packetIndex
      = d->pages.isEmpty() ? 0 : nextPacketIndex(d->pages.back());

    nextPage->setFirstPacketIndex(packetIndex);
    d->pages.append(nextPage);
  }
}

void Ogg::File::writePacket(unsigned int i, const ByteVector &packet)
{
  if(!readPages(i)) {
    debug("Ogg::File::writePacket() -- Could not find the requested packet.");
    return;
  }

  // Look for the pages where the requested packet should belong to.

  auto it = d->pages.cbegin();
  while((*it)->containsPacket(i) == Page::DoesNotContainPacket)
    ++it;

  const Page *firstPage = *it;

  while(nextPacketIndex(*it) <= i)
    ++it;

  const Page *lastPage = *it;

  // Replace the requested packet and create new pages to replace the located pages.

  ByteVectorList packets = firstPage->packets();
  packets[i - firstPage->firstPacketIndex()] = packet;

  if(firstPage != lastPage && lastPage->packetCount() > 1) {
    ByteVectorList lastPagePackets = lastPage->packets();
    lastPagePackets.erase(lastPagePackets.begin());
    packets.append(lastPagePackets);
  }

  // TODO: This pagination method isn't accurate for what's being done here.
  // This should account for real possibilities like non-aligned packets and such.

  List<Page *> pages = Page::paginate(packets,
                                      Page::SinglePagePerGroup,
                                      firstPage->header()->streamSerialNumber(),
                                      firstPage->pageSequenceNumber(),
                                      firstPage->header()->firstPacketContinued(),
                                      lastPage->header()->lastPacketCompleted());
  pages.setAutoDelete(true);

  // Write the pages.

  ByteVector data;
  for(const auto &page : pages)
    data.append(page->render());

  const offset_t originalOffset = firstPage->fileOffset();
  const offset_t originalLength = lastPage->fileOffset() + lastPage->size() - originalOffset;

  insert(data, originalOffset, originalLength);

  // Renumber the following pages if the pages have been split or merged.

  if(const int numberOfNewPages
      = pages.back()->pageSequenceNumber() - lastPage->pageSequenceNumber();
     numberOfNewPages != 0) {
    offset_t pageOffset = originalOffset + data.size();

    while(true) {
      Page page(this, pageOffset);
      if(!page.header()->isValid())
        break;

      page.setPageSequenceNumber(page.pageSequenceNumber() + numberOfNewPages);
      const ByteVector pageData = page.render();

      seek(pageOffset + 18);
      writeBlock(pageData.mid(18, 8));

      if(page.header()->lastPageOfStream())
        break;

      pageOffset += page.size();
    }
  }

  // Discard all the pages to keep them up-to-date by fetching them again.

  d->pages.clear();
  d->currentPageOffset = -1;
}
