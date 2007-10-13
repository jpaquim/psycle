// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief \interface psycle::front_ends::gui::contraption
#pragma once
#include "forward_declarations.hpp"
#include "color.hpp"
#include <boost/signal.hpp>
#include <libgnomecanvasmm/group.h>
#include <libgnomecanvasmm/rect.h>
#include <libgnomecanvasmm/text.h>
#include <universalis/compiler/numeric.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PACKAGENERIC__MODULE__SOURCE__PSYCLE__FRONT_ENDS__GUI__CONTRAPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
			typedef ::guint time;
			
			typedef universalis::compiler::numeric<>::floating_point real;

			class UNIVERSALIS__COMPILER__DYNAMIC_LINK contraption : public Gnome::Canvas::Group
			{
				public:
					contraption(Gnome::Canvas::Group & parent, real const & x, real const & y, color const &, std::string const & text);
					
				protected:
					Gnome::Canvas::Rect inline & rectangle() throw() { return rectangle_; }
				private:
					Gnome::Canvas::Rect rectangle_;

				///\name text
				///\{
					public:
						void text(std::string const & text);
					protected:
						Gnome::Canvas::Text inline & text_shadow() throw() { return text_shadow_; }
						Gnome::Canvas::Text inline & text_highlight() throw() { return text_highlight_; }
					private:
						Gnome::Canvas::Text text_shadow_, text_highlight_;
				///\}

				///\name colors
				///\{
					public:
						color inline & fill_color() { return fill_color_; }
						color inline & outline_color() { return outline_color_; }
						color inline & highlight_fill_color() { return highlight_fill_color_; }
						color inline & highlight_outline_color() { return highlight_outline_color_; }
					private:
						color fill_color_, outline_color_, highlight_fill_color_, highlight_outline_color_;
				///\}

				///\name signals
				///\{
					public:
						boost::signal<void (contraption &)> inline & signal_enter()  { return signal_enter_; }
						boost::signal<void (contraption &)> inline & signal_select() { return signal_select_; }
						boost::signal<void (contraption &)> inline & signal_move()   { return signal_move_; }
						boost::signal<void (contraption &)> inline & signal_leave()  { return signal_leave_; }
					private:
						boost::signal<void (contraption &)> signal_enter_, signal_move_, signal_select_, signal_leave_;
					protected:
						bool on_event(GdkEvent *) /*override*/;
				///\}

				///\name dragging
				///\{
					protected:
						void dragging_start(real const & x, real const & y, time const & time);
						void dragging(real const & x, real const & y);
						void dragging_stop(time const & time);
						bool inline const & dragging() { return dragging_; }
					private:
						bool dragging_;
						real dragging_x_, dragging_y_;
				///\}
			};
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
