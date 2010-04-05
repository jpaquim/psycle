// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include "window.hpp"
#include "lock.hpp"
#include <gtkmm/main.h>
#include <boost/bind.hpp>

namespace raytrace {

window::window(render & render)
:
	gl_scene_(render),
	button_("Quit")
{
	render.update_signal().connect(boost::bind(&window::update, this));
	button_.signal_clicked().connect(sigc::mem_fun(*this, &window::on_button_clicked));
	set_title("raytracer");
	set_size_request(render.pixels().width(), render.pixels().height());
	v_box_.pack_start(gl_scene_);
	v_box_.pack_start(button_, Gtk::PACK_SHRINK);
	add(v_box_);
	show_all_children();
}

void window::update() {
	lock lock;
	gl_scene_.queue_draw();
}

void window::on_button_clicked() {
	hide();
}

gl_scene::gl_scene(render & render) :
	render_(render)
{
	// configure OpenGL-capable visual

	// try double-buffered visual
	Glib::RefPtr<Gdk::GL::Config> glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB | Gdk::GL::MODE_DOUBLE);
	if(!glconfig) {
		std::cerr <<
			"*** Cannot find the double-buffered visual.\n"
			"*** Trying single-buffered visual.\n";

		// try single-buffered visual
		glconfig = Gdk::GL::Config::create(Gdk::GL::MODE_RGB);
		if(!glconfig) {
			std::cerr << "*** Cannot find any OpenGL-capable visual.\n";
			std::exit(1);
		}
	}
	// set OpenGL-capability to the widget.
	set_gl_capability(glconfig);
}

void gl_scene::on_realize()	{
	Gtk::DrawingArea::on_realize();
	Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

	if(!glwindow->gl_begin(get_gl_context())) return;

	{ // set orthographic coordinate system with 0,0 in lower-left corner
		int const width = get_width(), height = get_height();
		GLint v[2];
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, v);
		glLoadIdentity();
		glViewport(width - v[0], height - v[1], v[0], v[1]);
		glOrtho(width - v[0], width, height - v[1], height, -1, 1);
	}

	glwindow->gl_end();
}

bool gl_scene::on_configure_event(GdkEventConfigure* event) {
	Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

	if(!glwindow->gl_begin(get_gl_context())) return false;

	glViewport(0, 0, get_width(), get_height());

	glwindow->gl_end();

	return true;
}

bool gl_scene::on_expose_event(GdkEventExpose* event) {
	Glib::RefPtr<Gdk::GL::Window> glwindow = get_gl_window();

	if(!glwindow->gl_begin(get_gl_context())) return false;

	// Disable stuff that's likely to slow down glDrawPixels.
	// (Omit as much of	this as possible, when you know in advance that the OpenGL state will already be set correctly.)
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_LOGIC_OP);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
	glPixelTransferi(GL_RED_SCALE, 1);
	glPixelTransferi(GL_RED_BIAS, 0);
	glPixelTransferi(GL_GREEN_SCALE, 1);
	glPixelTransferi(GL_GREEN_BIAS, 0);
	glPixelTransferi(GL_BLUE_SCALE, 1);
	glPixelTransferi(GL_BLUE_BIAS, 0);
	glPixelTransferi(GL_ALPHA_SCALE, 1);
	glPixelTransferi(GL_ALPHA_BIAS, 0);

	// Disable extensions that could slow down glDrawPixels.
	// (Actually, you should check for the presence of the proper extension before making these calls.
	// I've omitted that code for simplicity.)
	#ifdef GL_EXT_convolution
		glDisable(GL_CONVOLUTION_1D_EXT);
		glDisable(GL_CONVOLUTION_2D_EXT);
		glDisable(GL_SEPARABLE_2D_EXT);
	#endif
	#ifdef GL_EXT_histogram
		glDisable(GL_HISTOGRAM_EXT);
		glDisable(GL_MINMAX_EXT);
	#endif
	#ifdef GL_EXT_texture3D
		glDisable(GL_TEXTURE_3D_EXT);
	#endif
	
	int const width = render_.pixels().width(), height = render_.pixels().height();
	char const * const pixels = render_.pixels();
	GLenum const format = GL_RGBA;
	glRasterPos2i(0,0);
	glDrawPixels(width, height, format, GL_UNSIGNED_BYTE, pixels);

	if(glwindow->is_double_buffered()) glwindow->swap_buffers(); else glFlush();

	glwindow->gl_end();
	
	return true;
}

}
