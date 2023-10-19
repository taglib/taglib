#include <memory>
#include "matroskaelement.h"
#include "tlist.h"
#include "tbytevector.h"

using namespace TagLib;

class Matroska::Element::ElementPrivate
{
public:
  ElementPrivate() {}
  ~ElementPrivate() = default;
  ElementPrivate(const ElementPrivate &) = delete;
  ElementPrivate &operator=(const ElementPrivate &) = delete;
  offset_t size = 0;
  offset_t offset = 0;

};

Matroska::Element::Element()
: e(std::make_unique<ElementPrivate>())
{
}
Matroska::Element::~Element() = default;

offset_t Matroska::Element::size() const
{
  return e->size;
}

offset_t Matroska::Element::offset() const
{
  return e->offset;
}

void Matroska::Element::setOffset(offset_t offset)
{
  e->offset = offset;
}

void Matroska::Element::setSize(offset_t size)
{
  e->size = size;
}