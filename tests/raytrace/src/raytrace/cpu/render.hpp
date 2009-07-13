// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__RENDER
#define RAYTRACE__RENDER
#pragma once

#include "scene.hpp"
#include "pixels.hpp"
#include "typenames.hpp"
#include <boost/signal.hpp>
#include <thread>
#include <mutex>
#include <condition>
#include <list>

namespace raytrace {

class render {
	public:
		render(typenames::scene const & scene, typenames::view const & view, typenames::pixels & pixels);
		void start();
		void process();
		void wait();
		void stop();

	public:
		typenames::scene const & scene() const { return scene_; }
	private:
		typenames::scene const & scene_;
		
	public:
		typenames::view const & view() const { return view_; }
		void view(typenames::view & view) { view_ = view; }
	private:
		typenames::view view_;
		void compute_view();
		real x_offset_, y_offset_, z_offset_;
		real xx_ratio_, xy_ratio_, xz_ratio_;
		real yx_ratio_, yy_ratio_, yz_ratio_;

	public:
		typenames::pixels & pixels() { return pixels_; }
	private:
		typenames::pixels & pixels_;

	public:
		boost::signal<void ()> & update_signal() throw() { return update_signal_; }
	private:
		boost::signal<void ()> update_signal_;

	private:
		typedef std::scoped_lock<std::mutex> scoped_lock;
		std::mutex mutable mutex_, update_signal_mutex_, done_mutex_;
		std::condition<scoped_lock> mutable condition_, condition_done_;
		bool process_requested_, done_, stop_requested_;
		unsigned int count_, update_signal_count_;
		std::size_t thread_count_, thread_done_count_, thread_done_count2_;
		typedef std::list<std::thread *> threads_type;
		threads_type threads_;
		void process_loop(std::size_t i, unsigned int min_x, unsigned int max_x, unsigned int min_y, unsigned int max_y, unsigned int y_step);
		void trace(unsigned int x, unsigned int y);
};

}

#endif
