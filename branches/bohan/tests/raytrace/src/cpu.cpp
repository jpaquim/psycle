// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/main.h>

/*#include <gdk/gdk.h>*/ extern "C" { void gdk_threads_enter(); void gdk_flush(); void gdk_threads_leave(); }
#include <cassert>

#include <boost/bind.hpp>
#include <thread>

namespace raytrace {

typedef guint32 color;

class pixels {
	public:
		pixels(unsigned int width, unsigned int height);
		Glib::RefPtr<Gdk::Pixbuf> pixbuf() { return pixbuf_; }
		unsigned int width() const { return pixbuf_->get_width(); }
		unsigned int height() const { return pixbuf_->get_height(); }
		void fill(color c) { pixbuf_->fill(c); }
		void inline put(unsigned int x, unsigned int y, color);
	private:
		Glib::RefPtr<Gdk::Pixbuf> pixbuf_;
};

class window : public Gtk::Window {
	public:
		window(Glib::RefPtr<Gdk::Pixbuf> pixbuf);
		Gtk::Image & image() { return image_; }
	private:
		Gtk::VBox v_box_;
		Gtk::Image image_;
		Gtk::Button button_;
		void on_button_clicked();
};

class lock {
	public:
		void static init();
		bool static initialized() { return initialized_; }
	private:
		bool static initialized_;

	public:
		lock() {
			class once { public: once() { init(); } };
			once static once;
			assert(initialized());
			::gdk_threads_enter();
		}
		
		~lock() throw() {
			::gdk_flush();
			::gdk_threads_leave();
		}
};

void stuff(pixels & pixels, window & window) {
	pixels.fill(0xffffff00);
	color c0 = 0;
	for(unsigned int i(0); i < 100; ++i) {
		for(unsigned int x(0); x < pixels.width(); ++x)
			for(unsigned int y(0); y < pixels.height(); ++y) {
				color c = c0 + x * y;
				pixels.put(x, y, c);
			}
		c0 += 1000;
		{ lock lock;
			window.image().set(pixels.pixbuf());
		}
	}
}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	Gtk::Main main(argument_count, arguments);
	pixels pixels(800, 800);
	window window(pixels.pixbuf());
	std::thread t(boost::bind(&stuff, boost::ref(pixels), boost::ref(window)));
	{ lock lock;
		main.run(window);
	}
	return 0;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////

#include <glibmm/thread.h>
/*#include <gdk/gdk.h>*/ extern "C" { void gdk_threads_init(); }

namespace raytrace {

pixels::pixels(unsigned int width, unsigned int height) {
	pixbuf_ = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, width, height);
}

void pixels::put(unsigned int x, unsigned int y, color c) {
	guint8 * p = pixbuf_->get_pixels();
	p += y * pixbuf_->get_rowstride();
	p += x * pixbuf_->get_n_channels();
	union u {
		guint8 r, g, b, x;
		color c;
	} u;
	u.c = c;
	u.x = *(p + pixbuf_->get_n_channels());
	*reinterpret_cast<color* >(p) = c;
}

window::window(Glib::RefPtr<Gdk::Pixbuf> pixbuf)
:
	image_(pixbuf),
	button_("Quit")
{
	button_.signal_clicked().connect(sigc::mem_fun(*this, &window::on_button_clicked));
	set_title("raytracer");
	v_box_.pack_start(image_);
	v_box_.pack_start(button_, Gtk::PACK_SHRINK);
	add(v_box_);
	show_all_children();
}

void window::on_button_clicked() {
	Gtk::Main::quit();
}

bool lock::initialized_(false);
void lock::init() {
	if(!initialized()) {
		if(!Glib::thread_supported()) Glib::thread_init();
		::gdk_threads_init();
		initialized_ = true;
	}
	assert(initialized());
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	return raytrace::main(argument_count, arguments);
}
