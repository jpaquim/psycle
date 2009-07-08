// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include "window.hpp"
#include "lock.hpp"
#include <gtkmm/main.h>
#include <boost/bind.hpp>

namespace raytrace {

window::window(render & render)
:
	render_(render),
	image_(render.pixels()),
	button_("Quit")
{
	render.update_signal().connect(boost::bind(&window::update, this));
	button_.signal_clicked().connect(sigc::mem_fun(*this, &window::on_button_clicked));
	set_title("raytracer");
	v_box_.pack_start(image_);
	v_box_.pack_start(button_, Gtk::PACK_SHRINK);
	add(v_box_);
	show_all_children();
}

void window::update() {
	lock lock;
	image_.set(image_.get_pixbuf());
}

void window::on_button_clicked() {
	/// XXX may deadlock if we're responding to a gui action and a thread is processing the update signal, which acquires the gui lock.
	render_.stop();
	Gtk::Main::quit();
}

}
