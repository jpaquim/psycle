//-----------------------------------------------------------------------------
// boost mpl/aux_/range_c/O1_size.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2000-02
// Aleksey Gurtovoy
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee, 
// provided that the above copyright notice appears in all copies and 
// that both the copyright notice and this permission notice appear in 
// supporting documentation. No representations are made about the 
// suitability of this software for any purpose. It is provided "as is" 
// without express or implied warranty.

#ifndef BOOST_MPL_AUX_RANGE_C_O1_SIZE_HPP_INCLUDED
#define BOOST_MPL_AUX_RANGE_C_O1_SIZE_HPP_INCLUDED

#include "boost/mpl/O1_size_fwd.hpp"
#include "boost/mpl/aux_/range_c/size.hpp"
#include "boost/mpl/aux_/range_c/tag.hpp"

namespace boost {
namespace mpl {

template<>
struct O1_size_traits< aux::half_open_range_tag >
    : size_traits< aux::half_open_range_tag >
{
};

} // namespace mpl
} // namespace boost

#endif // BOOST_MPL_AUX_RANGE_C_O1_SIZE_HPP_INCLUDED
