// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\\interface psycle::front_ends::gui::contraption
#ifndef PSYCLE__FRONT_ENDS__GUI__CONTRAPTION__INCLUDED
#define PSYCLE__FRONT_ENDS__GUI__CONTRAPTION__INCLUDED
#pragma once
#include "forward_declarations.hpp"
#include "color.hpp"
#include <boost/signal.hpp>
#include <libgnomecanvasmm/group.h>
#include <libgnomecanvasmm/rect.h>
#include <libgnomecanvasmm/text.h>
#define PSYCLE__DECL PSYCLE__FRONT_ENDS__GUI
#include <psycle/detail/decl.hpp>
namespace psycle { namespace front_ends { namespace gui {

typedef ::guint time;

typedef double real;

class PSYCLE__DECL contraption : public Gnome::Canvas::Group {
	public:
		contraption(Gnome::Canvas::Group & parent, real const & x, real const & y, color const &, std::string const & text);
		
	protected:
		Gnome::Canvas::Rect & rectangle() { return rectangle_; }
	private:
		Gnome::Canvas::Rect rectangle_;

	///\name text
	///\{
		public:
			void text(std::string const & text);
		protected:
			Gnome::Canvas::Text & text_shadow() { return text_shadow_; }
			Gnome::Canvas::Text & text_highlight() { return text_highlight_; }
		private:
			Gnome::Canvas::Text text_shadow_, text_highlight_;
	///\}

	///\name colors
	///\{
		public:
			color & fill_color() { return fill_color_; }
			color & outline_color() { return outline_color_; }
			color & highlight_fill_color() { return highlight_fill_color_; }
			color & highlight_outline_color() { return highlight_outline_color_; }
		private:
			color fill_color_, outline_color_, highlight_fill_color_, highlight_outline_color_;
	///\}

	///\name signals
	///\{
		public:
			boost::signal<void (contraption &)> & signal_enter()  { return signal_enter_; }
			boost::signal<void (contraption &)> & signal_select() { return signal_select_; }
			boost::signal<void (contraption &)> & signal_move()   { return signal_move_; }
			boost::signal<void (contraption &)> & signal_leave()  { return signal_leave_; }
		private:
			boost::signal<void (contraption &)> signal_enter_, signal_move_, signal_select_, signal_leave_;
			bool on_canvas_event(GdkEvent *);
	///\}

	///\name dragging
	///\{
		protected:
			void dragging_start(real const & x, real const & y, time const & time);
			void dragging(real const & x, real const & y);
			void dragging_stop(time const & time);
			bool dragging() { return dragging_; }
		private:
			bool dragging_;
			real dragging_x_, dragging_y_;
	///\}
};
}}}
#include <psycle/detail/decl.hpp>
#endif
