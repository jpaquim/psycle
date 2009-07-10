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
		vertex3 virtual normal(vertex3 const & pos) const = 0;
		bool virtual hit(vertex3 const & from, vertex3 const & to, real & distance) const = 0;
};

bool inline second_order_hit(real const a, real const b, real const c, real & root) {
	real const discriminant(b * b - 4 * a * c);
	if(discriminant < 0) return false;
	real const discriminant_sqrt(std::sqrt(discriminant));
	real const inversed_denominator(1 / (2 * a));
	real const opposed_b(-b);
	real const root1((opposed_b + discriminant_sqrt) * inversed_denominator);
	real const root2((opposed_b - discriminant_sqrt) * inversed_denominator);
	if(root1 < 0) {
		if(root2 < 0) return false;
		root = root2;
	} else if(root2 < 0) root = root1;
	else root = std::min(root1, root2);
	return true;
}

class quadric : public object, public /*symmetric_*/matrix4 {
	public:
		vertex3 normal(vertex3 const & pos) const /*override*/ {
			vertex3 result;
			vertex4 const n(*this * pos);
			result.x = n.x;
			result.y = n.y;
			result.z = n.z;
			return result;
		}

		bool hit(vertex3 const & from, vertex3 const & to, real & distance) const /*override*/ {
			vertex4 const to_matrix(to * *this);
			real const a(to_matrix * to);
			real const b(2 * (to_matrix * from));
			real const c(from * *this * from);
			return second_order_hit(a, b, c, distance);
		}
};

class sphere : public object {
	public:
		vertex3 pos;
		real radius2;
		color c;
		
		void radius(real radius) { radius2 = radius * radius; }

		vertex3 normal(vertex3 const & pos) const /*override*/ {
			vertex3 result(pos - this->pos);
			return result;
		}

		bool hit(vertex3 const & from, vertex3 const & to, real & distance) const /*override*/ {
			vertex3 const rel_pos(from - this->pos);
			real const a(to.mag2());
			real const b(2 * (to * rel_pos));
			real const c(rel_pos.mag2() - radius2);
			return second_order_hit(a, b, c, distance);
		}
};

class scene0 : public scene {
	public:
		vertex3 lamp;
		sphere s1, s2, s3;
		quadric q;

		scene0() {
			lamp(-5, 5, 0);
			
			s1.c(1, 0.65, 0.65);
			s1.pos(0, 0, -10);
			s1.radius(1);

			s2.c(0.5, 1, 0.5);
			s2.pos(1, 1, -10);
			s2.radius(1);

			s3.c(0.75, 0.75, 1);
			s3.pos(-1, -1, -10);
			s3.radius(1);

			q.identity();
			q(3, 3) = -1;
		}
		
		color trace(vertex3 const & from, vertex3 const & to) const /*override*/ {
			color c;
			real distance, min_distance;
			sphere const * s(0);
			if(s1.hit(from, to, min_distance)) s = &s1;
			if(s2.hit(from, to, distance) && (!s || distance < min_distance)) { s = &s2; min_distance = distance; }
			if(s3.hit(from, to, distance) && (!s || distance < min_distance)) { s = &s3; min_distance = distance; }
			if(!s) c(0, 0, 0);
			else {
				vertex3 const pos(from + min_distance * to);
				vertex3 normal(s->normal(pos));
				normal.normalize();
				vertex3 rel(lamp - pos);
				rel.normalize();
				real const angle(std::acos(rel * normal));
				real const light(1 - angle / pi);
				real const light2(light * light);
				real const amp = real(0.1) + (light + light2) * 0.65;
				c = s->c * amp;
				if(amp > 1) c = color(std::min(real(1), c.x), std::min(real(1), c.y), std::min(real(1), c.z));
			}
			return c;
		}
};

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	Gtk::Main main(argument_count, arguments);
	scene0 scene;
	view view;
	view.from(0, 0, 0);
	view.angles(0, 0, 0);
	view.x_fov = 50 * pi / 180;
	pixels pixels(1000, 800);
	pixels.fill(color(1, 1, 1));
	render render(scene, view, pixels);
	render.start();
	render.process();
	{ lock lock;
		window window(render);
		main.run(window);
	}
	render.stop();
	std::cout << "done\n";
	return 0;
}

}

int main(int /*const*/ argument_count, char /*const*/ * /*const*/ arguments[]) {
	return raytrace::main(argument_count, arguments);
}
