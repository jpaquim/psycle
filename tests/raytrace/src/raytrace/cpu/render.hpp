// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2010 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__RENDER
#define RAYTRACE__RENDER
#pragma once

#include "scene.hpp"
#include "pixels.hpp"
#include <boost/signal.hpp> // TODO they managed to break backward compatibility ... need to upgrade to "signals2" ... grrr
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition_variable.hpp>
#include <list>

namespace raytrace {

using namespace universalis::stdlib;

class render {
	public:
		render(class scene const & scene, class view const & view, class pixels & pixels);
		void start();
		void process();
		void wait();
		void stop();

	public:
		class scene const & scene() const { return scene_; }
	private:
		class scene const & scene_;
		
	public:
		class view const & view() const { return view_; }
		void view(class view & view) { view_ = view; }
	private:
		class view view_;
		void compute_view();
		real x_offset_, y_offset_, z_offset_;
		real xx_ratio_, xy_ratio_, xz_ratio_;
		real yx_ratio_, yy_ratio_, yz_ratio_;

	public:
		class pixels & pixels() { return pixels_; }
		void resize(unsigned int width, unsigned int height);
	private:
		class pixels & pixels_;

	public:
		boost::signal<void ()> & update_signal() throw() { return update_signal_; }
	private:
		boost::signal<void ()> update_signal_;

	private:
		typedef unique_lock<mutex> scoped_lock;
		mutex mutable mutex_, update_signal_mutex_, done_mutex_;
		condition_variable mutable condition_, condition_done_;
		bool process_requested_, done_, stop_requested_;
		unsigned int count_, update_signal_count_;
		std::size_t thread_count_, thread_done_count_, thread_done_count2_;
		typedef std::list<thread *> threads_type;
		threads_type threads_;
		void process_loop(std::size_t i, unsigned int min_x, unsigned int max_x, unsigned int min_y, unsigned int max_y, unsigned int y_step);
		void trace(unsigned int x, unsigned int y);
};

}

#endif
