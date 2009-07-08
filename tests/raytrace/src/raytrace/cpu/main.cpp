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

class object {
	public:
		bool virtual hit(vertex3 const & from, vertex3 const & to, vertex3 & pos) = 0;
};

class quadric : public object {
	public:
		matrix4 matrix;
		bool virtual hit(vertex3 const & from, vertex3 const & to, vertex3 & pos) /*override*/ {
			vertex4 const to_matrix(to * matrix);
			real const a(to_matrix * to);
			real const b(2 * (to_matrix * from));
			real const c(from * matrix * from);
			real const discriminant(b * b - 4 * a * c);
			if(discriminant < 0) return false;
			real const discriminant_sqrt(std::sqrt(discriminant));
			real const inversed_denominator(1 / (2 * a));
			real const opposed_b(-b);
			real const root1((opposed_b + discriminant_sqrt) * inversed_denominator);
			real const root2((opposed_b - discriminant_sqrt) * inversed_denominator);
			real root;
			if(root1 < 0) {
				if(root2 < 0) return false;
				root = root2;
			} else if(root2 < 0) root = root1;
			else root = std::min(root1, root2);
			pos = from + root * to;
			return true;
		}
};

class plane : public object {
	public:
		vertex3 normal;
		real r;
};

class sphere : public object {
	public:
		vertex3 pos;
		real radius;
};

class scene0 : public scene {
	public:
		scene0() {
			matrix4 & m(q.matrix);
			m.identity();
			m *= 0.1;
			m(3, 3) = -0.99;
		}
		
		color trace(vertex3 const & from, vertex3 const & to) /*override*/ {
			#if 0
			color c(to.x, to.y, to.x * to.y);
			return c;
			#else
			color c;
			vertex3 pos;
			if(!q.hit(from, to, pos)) c(0, 0, 0);
			else {
				real const mag(pos.mag());
				c(mag, mag, mag);
			}
			return c;
			#endif
		}
	#if 1
		quadric q;
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
