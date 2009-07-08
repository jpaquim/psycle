// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include "main.hpp"
#include "render.hpp"
#include "window.hpp"
#include "lock.hpp"
#include "matrix4.hpp"
#include <gtkmm/main.h>
#include <limits>

namespace raytrace {

#if 1
class object {
	public:
		bool virtual hit(vertex3 const & from, vertex3 const & to, vertex3 & pos, vertex3 & normal) = 0;
};

class quadric : public object {
	public:
		matrix4 matrix;
		bool virtual hit(vertex3 const & from, vertex3 const & to, vertex3 & pos, vertex3 & normal) /*override*/ {
			real a = to * matrix * to;
			real b = 2 * to * matrix * from;
			real c = from * matrix * from;
			if(std::abs(2 * a) <= std::numeric_limits<real>::epsilon()) return false;
			real root1 = (-b + std::sqrt(b * b - 4 * a * c)) / (2 * a);
			real root2 = (-b - std::sqrt(b * b - 4 * a * c)) / (2 * a);
			pos = from + std::min(root1, root2) * to;
			//normal = ...
			return true;
		}
};
#else
class plane : public object {
	public:
		vertex3 normal,
		real r;
};

class sphere : public object {
	public:
		vertex3 pos;
		real radius;
}
#endif

class scene0 : public scene {
	public:
		color trace(vertex3 const & from, vertex3 const & to) /*override*/ {
			#if 1
			color c(to.x, to.y, to.x * to.y);
			return c;
			#else
			#endif
		}
	#if 1
	#else
		public:
			scene0() {
				plane_.normal(0, 1, 0);
				plane_.r = 0;
				sphere_.pos(0, 1, 0);
				sphere_.radius = 1;
			};
		plane plane_;
		sphere sphere_;
	#endif
};

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	Gtk::Main main(argument_count, arguments);
	scene0 scene;
	view view; view.from(0, 0, 0);
	pixels pixels(800, 800);
	pixels.fill(color(1, 1, 1));
	render render(scene, view, pixels);
	window window(render);
	render.start();
	render.process();
	{ lock lock;
		main.run(window);
	}
	return 0;
}

}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	return raytrace::main(argument_count, arguments);
}
