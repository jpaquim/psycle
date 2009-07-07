// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__WINDOW
#define RAYTRACE__WINDOW

#include "view.hpp"
#include "render.hpp"
#include "lock.hpp"

#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>

namespace raytrace {

class window : public view, public Gtk::Window {
	public:
		window(render &);
		void update() /*override*/ {
			//std::cout << "update\n";
			lock lock;
			image_.set(image_.get_pixbuf());
		}
	private:
		render & render_;
		Gtk::VBox v_box_;
		Gtk::Image image_;
		Gtk::Button button_;
		void on_button_clicked();
};

}

#endif
