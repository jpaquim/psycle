// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__WINDOW
#define RAYTRACE__WINDOW

#include "render.hpp"
#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkglmm.h>

namespace raytrace {

class gl_scene :
	public Gtk::DrawingArea,
	public Gtk::GL::Widget<gl_scene>
{
	public:
  		gl_scene(render &);

	protected:
  		virtual void on_realize();
  		virtual bool on_configure_event(GdkEventConfigure* event);
  		virtual bool on_expose_event(GdkEventExpose* event);

	private:
		friend class window;
		render & render_;
};

class window : public Gtk::Window {
	public:
		window(render &);
		void update();
	private:
		gl_scene gl_scene_;
		Gtk::VBox v_box_;
		Gtk::Button button_;
		void on_button_clicked();
};

}

#endif
