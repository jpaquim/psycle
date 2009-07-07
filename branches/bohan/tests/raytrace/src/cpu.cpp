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

#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <mutex>
#include <condition>
#include <list>

#include <universalis/os/cpu_affinity.hpp>

namespace raytrace {

namespace typenames {
	using namespace raytrace;
}

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

class view {
	public:
		view(pixels & pixels) : pixels_(pixels) {}
		void virtual update() = 0;
	private:
		pixels & pixels_;
};

class scene {
	public:
		color virtual trace(unsigned int x, unsigned int y) = 0;
};

class render {
	public:
		typenames::scene & scene() { return scene_; }
	private:
		typenames::scene & scene_;
		
	public:
		typenames::pixels & pixels() { return pixels_; }
	private:
		typenames::pixels & pixels_;

	private:
		typedef std::scoped_lock<std::mutex> scoped_lock;
		std::mutex mutable mutex_;
		std::condition<scoped_lock> mutable condition_;
		bool process_requested_, stop_requested_;
		unsigned int count_, update_signal_count_;
		typedef std::list<std::thread *> threads_type;
		threads_type threads_;

	public:
		boost::signal<void ()> & update_signal() throw() { return update_signal_; }
	private:
		boost::signal<void ()> update_signal_;

	public:
		render(typenames::scene & scene, typenames::pixels & pixels)
		:
			scene_(scene),
			pixels_(pixels),
			process_requested_(),
			stop_requested_(),
			count_(),
			update_signal_count_(10000)
		{}

		void start() {
			std::size_t thread_count(universalis::os::cpu_affinity::cpu_count());
			{ // thread count env var
				char const * const env(std::getenv("THREADS"));
				if(env) {
					std::stringstream s;
					s << env;
					s >> thread_count;
				}
			}

			unsigned int const width(pixels_.width()), height(pixels_.height());
			unsigned int const x_stripe(width), y_stripe(height / thread_count);
			unsigned int x(0), y(0);
			try {
				for(std::size_t i(0); i < thread_count; ++i) {
					threads_.push_back(new std::thread(boost::bind(&render::process_loop, this, x, x + x_stripe, y, y + y_stripe)));
					x += x_stripe;
					if(x >= width) {
						x = 0;
						y += y_stripe;
						if(y >= height) y = 0;
					}
				}
			} catch(...) {
				{ scoped_lock lock(mutex_);
					stop_requested_ = true;
				}
				condition_.notify_all();
				for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
					(**i).join();
					delete *i;
				}
				threads_.clear();
				throw;
			}
		}
		
		void stop() {
			{ scoped_lock lock(mutex_);
				stop_requested_ = true;
			}
			condition_.notify_all();
			for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
				(**i).join();
				delete *i;
			}
			threads_.clear();
		}
		
		void process() {
			{ scoped_lock lock(mutex_);
				process_requested_ = true;
			}
			condition_.notify_all();
		}
		
		void process_loop(unsigned int min_x, unsigned int max_x, unsigned int min_y, unsigned int max_y) {
			std::cout << "part: " << min_x << ' ' << max_x << ' ' << min_y << ' ' << max_y << '\n';
			unsigned int const inc(max_x - min_x);
			while(true) {
				{ scoped_lock lock(mutex_);
					while(!stop_requested_ && !process_requested_) condition_.wait(lock);
					if(stop_requested_) return;
				}
				for(unsigned int y(min_y); y < max_y; ++y) {
					for(unsigned int x(min_x); x < max_x; ++x) {
						color c = scene_.trace(x, y);
						pixels_.put(x, y, c);
					}
					{ scoped_lock lock(mutex_);
						if(stop_requested_) return;
						count_ += inc;
						if(count_ > update_signal_count_) {
							count_ = 0;
							update_signal_();
						}
					}
				}
				update_signal_();
			}
		}
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
		void on_button_clicked() { render_.stop(); Gtk::Main::quit(); }
};

class scene0 : public scene {
	protected:
		color trace(unsigned int x, unsigned int y) /*override*/ {
			color c(0);
			for(unsigned int i(0); i < x * y; ++i) c += i;
			return c + x * y;
		}
};

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	Gtk::Main main(argument_count, arguments);
	scene0 scene;
	pixels pixels(800, 800);
	//pixels.fill(0xffffff00);
	render render(scene, pixels);
	window window(render);
	render.start();
	render.process();
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

window::window(render & render)
:
	view(render.pixels()),
	render_(render),
	image_(render.pixels().pixbuf()),
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
