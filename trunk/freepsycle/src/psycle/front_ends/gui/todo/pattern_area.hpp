// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for ...
#pragma once
#include <gdkmm/gc.h>
#include <gdkmm/color.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/drawingarea.h>
#include <glibmm/signalproxy.h>
#include <iostream>
#include <psycle/engine/Pattern.hpp>
#include <psycle/engine/Event.hpp>
#include "BitmappedFont.hpp"
namespace psycle
{
	class pattern_area : public Gtk::DrawingArea
	{
	public:
		pattern_area() throw(std::exception);
		virtual ~pattern_area() throw();
		virtual bool button_press(GdkEventButton* event);
		virtual void clear();
		virtual void invalidate(const Gdk::Rectangle& update_rect);
		void draw_pattern(const int& line_start, const int& line_end, const int& col_start, const int& col_end, const int& x, const int& y);
	
	protected:
		//Overridden default signal handlers:
		virtual void on_realize() throw(std::exception);
		virtual bool on_expose_event(GdkEventExpose* event);
		void draw_highlights();
		void draw_borders(const int& left, int right, const int& top, int bottom);
		void draw_line_numbers();

	protected:
		Glib::RefPtr<Gdk::GC> gc_;
		Gdk::Color blue_, red_;
		Glib::RefPtr<Gdk::Window> window;
		Glib::RefPtr<Gdk::Pixmap> pixmap_pattern_area;
		BitmappedFont* the_font;
		int pattern_area_width;
		int pattern_area_height;
		int line_height;
		int channel_width;
		int line_number_width;
		int title_height;
		Gdk::Color highlight_colour;
		Gdk::Color border_colour;
		/// [alk] temporary, until patterns are put int the proper place
		Pattern * pat1;
		/// [alk] temporary, till the engine in made
		int bpm;
	};
}
