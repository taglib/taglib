/***************************************************************************
    copyright            : (C) 2004 by Allan Sandfeld Jensen
    email                : kde@carewolf.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
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
 ***************************************************************************/

#ifndef TAGLIB_APEITEM_H
#define TAGLIB_APEITEM_H

#include <tbytevector.h>
#include <tstring.h>
#include <tstringlist.h>

namespace TagLib {

  namespace APE {

    //! An implementation of APE-items

    /*!
     * This class provides the features of items in the APEv2 standard.
     */
    struct Item
    {
      enum ItemTypes {
        //! Item contains text information coded in UTF-8
        Text = 0,
        //! Item contains binary information
        Binary = 1,
        //! Item is a locator of external stored information
        Locator = 2
      };
      Item();
      explicit Item(ByteVector& bin);
      explicit Item(const String&, const String&);
      explicit Item(const String&, const StringList &);
      Item(const Item&);
      Item& operator=(const Item&);

      String key() const;
      ByteVector value() const;

      int size() const;

      String toString() const;
      StringList toStringList() const;

      ByteVector render();
      void parse(const ByteVector&);

      void setReadOnly(bool);

      bool isReadOnly() const;

      void setType(ItemTypes type);

      ItemTypes type() const;
/*
      void setValue(ByteVector);
      void setValue(const String&);
      void setValue(const StringList&);
 */
      bool isEmpty() const;

    private:
      class ItemPrivate;
      ItemPrivate *d;
    };
  }

}

#endif


