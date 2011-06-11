// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\\interface psycle::front_ends:gui:color
#ifndef PSYCLE__FRONT_ENDS__GUI__COLOR__INCLUDED
#define PSYCLE__FRONT_ENDS__GUI__COLOR__INCLUDED
#pragma once
#include <psycle/detail/project.hpp>
#include <cassert>
namespace psycle { namespace front_ends { namespace gui {
	class color {
		public:
			typedef uint32_t rgba; // meant to match glib/gdk's ::guint32
			typedef uint8_t channel;
			
			color(rgba const & rgba) : rgba_(rgba) {}
			operator rgba const & () const { return rgba_; }

			channel r() const { return (rgba_ & 0xff000000) >> 24; }
			channel g() const { return (rgba_ & 0x00ff0000) >> 16; }
			channel b() const { return (rgba_ & 0x0000ff00) >>  8; }
			channel a() const { return (rgba_ & 0x000000ff) >>  0; }

			color   r(channel const & r) const { color copy(*this); copy.r(r); return copy; }
			color   g(channel const & g) const { color copy(*this); copy.g(g); return copy; }
			color   b(channel const & b) const { color copy(*this); copy.b(b); return copy; }
			color   a(channel const & a) const { color copy(*this); copy.a(a); return copy; }

			color & r(channel const & r)       { uint32_t shift(r); shift <<= 24; rgba_ &= 0x00ffffff; rgba_ |= shift; assert(this->r() == r); return *this; }
			color & g(channel const & g)       { uint32_t shift(g); shift <<= 16; rgba_ &= 0xff00ffff; rgba_ |= shift; assert(this->g() == g); return *this; }
			color & b(channel const & b)       { uint16_t shift(b); shift <<=  8; rgba_ &= 0xffff00ff; rgba_ |= shift; assert(this->b() == b); return *this; }
			color & a(channel const & a)       {                                  rgba_ &= 0xffffff00; rgba_ |= a    ; assert(this->a() == a); return *this; }

		private:
			rgba rgba_;
	};
}}}
#endif
