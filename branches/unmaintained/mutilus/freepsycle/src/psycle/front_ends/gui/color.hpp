// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief \interface psycle::front_ends:gui:color
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/compiler/numeric.hpp> // \todo <cstdint> or <boost/stdint.hpp>
//#include <glib/gtypes.h> // for ::guint32
#include <cassert>
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
			class color
			{
				public:
					//typedef ::guint32 rgba;
					typedef universalis::compiler::numeric<32>::unsigned_int    rgba;
					typedef universalis::compiler::numeric< 8>::unsigned_int channel;
					
					inline color(rgba const & rgba) : rgba_(rgba) {}
					inline operator rgba const & () const throw() { return rgba_; }

					channel inline r() const throw() { return (rgba_ & 0xff000000) >> 24; }
					channel inline g() const throw() { return (rgba_ & 0x00ff0000) >> 16; }
					channel inline b() const throw() { return (rgba_ & 0x0000ff00) >>  8; }
					channel inline a() const throw() { return (rgba_ & 0x000000ff) >>  0; }

					color inline   r(channel const & r) const throw() { color copy(*this); copy.r(r); return copy; }
					color inline   g(channel const & g) const throw() { color copy(*this); copy.g(g); return copy; }
					color inline   b(channel const & b) const throw() { color copy(*this); copy.b(b); return copy; }
					color inline   a(channel const & a) const throw() { color copy(*this); copy.a(a); return copy; }

					color inline & r(channel const & r)       throw() { universalis::compiler::numeric<32>::unsigned_int shift(r); shift <<= 24; rgba_ &= 0x00ffffff; rgba_ |= shift; assert(this->r() == r); return *this; }
					color inline & g(channel const & g)       throw() { universalis::compiler::numeric<32>::unsigned_int shift(g); shift <<= 16; rgba_ &= 0xff00ffff; rgba_ |= shift; assert(this->g() == g); return *this; }
					color inline & b(channel const & b)       throw() { universalis::compiler::numeric<16>::unsigned_int shift(b); shift <<=  8; rgba_ &= 0xffff00ff; rgba_ |= shift; assert(this->b() == b); return *this; }
					color inline & a(channel const & a)       throw() {                                                                          rgba_ &= 0xffffff00; rgba_ |= a    ; assert(this->a() == a); return *this; }

				private:
					rgba rgba_;
			};
		}
	}
}
