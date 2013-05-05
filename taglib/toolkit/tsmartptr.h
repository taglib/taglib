/***************************************************************************
    copyright            : (C) 2013 by Tsuda Kageyu
    email                : tsuda.kageyu@gmail.com
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

#ifndef TAGLIB_SMARTPTR_H
#define TAGLIB_SMARTPTR_H

#include "taglib_config.h"

#if defined(TAGLIB_USE_STD_SHARED_PTR)
# include <memory>
#elif defined(TAGLIB_USE_TR1_SHARED_PTR) 
# include <tr1/memory>
#elif defined(TAGLIB_USE_BOOST_SHARED_PTR) 
# include <boost/shared_ptr.hpp>
#else
# include "trefcountptr.h"
#endif

#if defined(TAGLIB_USE_STD_SHARED_PTR) || defined(TAGLIB_USE_TR1_SHARED_PTR) 

# define TAGLIB_SHARED_PTR std::tr1::shared_ptr

#elif defined(TAGLIB_USE_BOOST_SHARED_PTR)

# define TAGLIB_SHARED_PTR boost::shared_ptr

#else

# define TAGLIB_SHARED_PTR TagLib::RefCountPtr

#endif
#endif
