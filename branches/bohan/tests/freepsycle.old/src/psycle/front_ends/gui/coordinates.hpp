// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>


/********* UNUSED FILE **********/

///\file
///\interface
///\n psycle::front_ends::gui::coordinates::cartesian.
///\n psycle::front_ends::gui::dimensions
///\n psycle::front_ends::gui::position
///\n psycle::front_ends::gui::bounds
#pragma once
#include "forward_declarations.hpp"
namespace psycle { namespace front_ends { namespace gui {

/// coordinates in several types of referentials.
/// \todo non cartesian, i.e. polar.
namespace coordinates {

/// coordinates in a cartesian referential.
template<typename Derived, typename Coordinate>
class cartesian {
	public:
		/// returns the type of coordinate in a instance of this template class.
		typedef                   Coordinate      coordinate_type;
		typedef typename Derived::coordinate_type coordinate_type;

		///\name constructors
		//\{
			/// default constructor.
			cartesian() {}
			/// copy constructor.
			cartesian(Derived const & other) : x_(other.x()), y_(other.y()) {}
			/// constructor from separate cartesian coordinates.
			cartesian(Coordinate const & x, Coordinate const & y) : x_(x), y_(y) {}
		//\}

		///\name operators
		//\{
			/// assignment operator.
			Derived & operator= (Derived const & other) throw() { this->x() = other.x(); this->y() = other.y(); return *this; }
			/// addition operator.
			Derived operator+ (Derived const & other) const throw() { return Derived(this->x() + other.x(), this->y() + other.y()); }
			/// assignment addition operator.
			Derived & operator+=(Derived const & other) throw() { this->x() += other.x(); this->y() += other.y(); return *this; }
			/// negation operator.
			Derived operator-() const throw() { return Derived(-x(), -y()); }
			/// substraction operator.
			Derived operator- (Derived const & other) const throw() { return Derived(this->x() - other.x(), this->y() - other.y()); }
			/// assignment substraction operator.
			Derived & operator-=(Derived const & other) throw() { this->x() -= other.x(); this->y() -= other.y(); return *this; }
			/// cartesian product operator.
			Coordinate operator*(Derived const & other ) const throw() { return this->x() * other.y() - this->y() * other.x(); }
			/// scalar product operator.
			Derived operator*(Coordinate const & factor) const throw() { return Derived(this->x() * factor, this->y() * factor); }
			/// assignment scalar product operator.
			Derived & operator*=(Coordinate const & factor) throw() { this->x() *= factor; this->y() *= factor; return *this; }
			/// scalar division operator.
			Coordinate operator/(Coordinate const & factor) const throw() { assert(factor); return (*this) * (1 / factor); }
			/// assignment scalar division operator.
			Derived & operator/=(Coordinate const & factor) throw() { assert(factor); return (*this) *= (1 / factor); }
		//\}

	protected:
		///\name x
		//\{
			/// returns const x.
			Coordinate const & x() const throw() { return x_; }
			/// returns mutable x.
			Coordinate & x() throw() { return x_; }
			/// sets x.
			void x(Coordinate const & x) throw() { this->x_ = x; }
		//\}

		///\name y
		//\{
			/// returns const y.
			Coordinate const & y() const throw() { return y_; }
			/// returns mutable y.
			Coordinate const & y() throw() { return y_; }
			/// sets y.
			void y(Coordinate const & y) throw() { this->y_ = y; }
		//\}

	private:
		Coordinate x_, y_;
};
}

/// dimensions.
template<typename Coordinate>
class dimensions : public coordinates::cartesian<dimensions<Coordinate>, Coordinate> {
	private:
		typedef coordinates::cartesian<dimensions<Coordinate>, Coordinate> coordinates_type;
		dimensions(coordinates_type const & coordinates) : coordinates_type(coordinates) {}

	public:
		///\name constructors
		//\{
			/// default constructor.
			dimensions() {}
			/// copy constructor.
			dimensions(dimensions const & other) : coordinates_type(other) {}
			/// constructor from separate cartesian coordinates.
			dimensions(Coordinate const & w, Coordinate const & h) : coordinates_type(w, h) {}
		//\}

		///\name operators
		//\{
			/// assignment operator.
			dimensions & operator=(dimensions const & other) throw() { coordinates_type::operator=(other); return *this; }
			/// addition operator.
			dimensions operator+(dimensions const & other) const throw() { return coordinates_type::operator+(other); }
			/// assignement addition operator.
			dimensions & operator+=(dimensions const & other) throw() { coordinates_type::operator+=(other); return *this; }
			/// negation operator.
			dimensions operator-() const throw() { return coordinates_type::operator-(); }
			/// substraction operator.
			dimensions operator-(dimensions const & other) const throw() { return coordinates_type::operator-(other); }
			/// assignement substraction operator.
			dimensions & operator-=(dimensions const & other) throw() { coordinates_type::operator-=(other); return *this; }
			/// cartesian product.
			Coordinate operator* (dimensions const & other) throw() { return coordinates_type::operator*(other); }
		//\}

		///\name width
		//\{
			/// returns the const width.
			Coordinate const & w() const throw() { return coordinates_type::x(); }
			/// returns the mutable width.
			Coordinate & w() throw() { return coordinates_type::x(); }
			/// sets the width.
			void w(Coordinate const & w) throw() { coordinates_type::x(w); }
		//\}

		///\name height
		//\{
			/// returns the const height.
			Coordinate const & h() const throw() { return coordinates_type::y(); }
			/// returns the mutable height.
			Coordinate & h() throw() { return coordinates_type::y(); }
			/// sets the height.
			void h(Coordinate const & h) throw() { coordinates_type::y(h); }
		//\}
};

/// position.
template<typename Coordinate>
class position : public coordinates::cartesian<Coordinate> {
	private:
		typedef coordinates::cartesian<Coordinate> coordinates_type;
	public:
		///\name constructors
		//\{
			/// default constructor.
			position() {}
			/// copy constructor.
			position(position const & other) : coordinates_type(other) {}
			/// constructor from separate cartesian coordinates.
			position(Coordinate const & x, Coordinate const & y) : coordinates_type(x, y) {}
		//\}

		///\name operators
		//\{
			/// assignment operator.
			position const & operator=(position const & other) { coordinates_type::operator=(other); return *this; }
			/// addition operator.
			position const operator+(dimensions<Coordinate> const & delta) const { return coordinates_type::operator+(delta); }
			/// assignement addition operator.
			position const & operator+=(dimensions<Coordinate> const & delta) { coordinates_type::operator+=(delta); return *this; }
			/// substraction operator.
			dimensions<Coordinate> const operator-(position const & other) const { return coordinates_type::operator-(other); }
			/// substraction operator.
			position const operator-(dimensions<Coordinate> const & delta) const { return coordinates_type::operator-(delta); }
			/// assignement substraction operator.
			position const operator-=(position const & other) { coordinates_type::operator-=(other); return *this; }
			/// assignement substraction operator.
			position const operator-=(dimensions<Coordinate> const & delta) { coordinates_type::operator-=(delta); return *this; }
			/// cartesian product.
			Coordinate const operator*(dimensions<Coordinate> const & delta) { return coordinates_type::operator*(delta); }
		//\}

		///\name x
		//\{
			/// returns const x.
			Coordinate const & x() const { return coordinates_type::x(); }
			/// returns mutable x.
			Coordinate & x() { return coordinates_type::x(); }
			/// sets x.
			void x(Coordinate const & x) { coordinates_type::x(x); }
		//\}

		///\name y
		//\{
			/// returns const y.
			Coordinate const & y() const { return coordinates_type::y(); }
			/// returns mutable y.
			Coordinate & y() { return coordinates_type::y(); }
			/// sets y.
			void y(Coordinate const & y) throw() { coordinates_type::y(y); }
		//\}
};

/// bounds are the compound of a Position and Dimensions.
template<typename Position_Coordinate, typename Dimensions_Coordinate>
class bounds : public position<Position_Coordinate>, public dimensions<Dimensions_Coordinate> {
	public:
		/// returns the type of coordinate for positions in a instance of this template class.
		typedef Position_Coordinate position_coordinate_type;
		/// returns the type of coordinate for dimensions in a instance of this template class.
		typedef Dimensions_Coordinate dimensions_coordinate_type;

		///\name constructors
		//\{
			/// default constructor.
			bounds() {}
			/// constructor from a Position and Dimensions.
			bounds(Position_Coordinate const & p, Dimensions_Coordinate const & d) : position<Position_Coordinate>(p), dimensions<Dimensions_Coordinate>(d) {}
			/// constructor from a Position and another Position.
			bounds(Position_Coordinate const & top_left, Position_Coordinate const & bottom_right) : position<Position_Coordinate>(top_left), dimensions<Dimensions_Coordinate>(bottom_right - top_left) {}
			/// constructor from separate cartesian coordinates representing a position and dimensions.
			bounds(Position_Coordinate const & x, Position_Coordinate const & y, Dimensions_Coordinate const & w, Dimensions_Coordinate const & h) : position<Position_Coordinate>(x, y), dimensions<Dimensions_Coordinate>(w, h) {}
		//\}

		///\name position
		/// this class is a Position anyway.
		///
		/// you can conveniently call this when you need to distinguish between a position and a size,
		/// instead of calling static_cast<position>().
		//\{
			/// returns the position.
			position<Position_Coordinate> const & position() const { return *this; }
			/// returns the mutable position.
			position<Position_Coordinate> & position() { return *this; }
			/// sets the position.
			void position(position<Position_Coordinate> const & position) { position<Position_Coordinate>::operator=(position); }
		//\}

		///\name position x coordinate
		//\{
			/// returns x.
			Position_Coordinate const & x() const { return position<Position_Coordinate>::x(); }
			/// returns mutable x.
			Position_Coordinate & x() { return position<Position_Coordinate>::x(); }
			/// sets x.
			void x(Position_Coordinate const & x) { position<Position_Coordinate>::x(x); }
		//\}

		///\name position y coordinate
		//\{
			/// returns y.
			Position_Coordinate const & y() const throw() { return position<Position_Coordinate>::y(); }
			/// returns mutable y.
			Position_Coordinate & y() throw() { return position<Position_Coordinate>::y(); }
			/// sets y
			void y(const Position_Coordinate & y) { position<Position_Coordinate>::y(y); }
		//\}

		///\name dimensions
		/// this class is a Dimensions anyway.
		///
		/// you can conveniently call this when you need to distinguish between a Position and Dimensions,
		/// instead of calling static_cast<dimensions>().
		//\{
			/// returns the dimensions.
			dimensions<Dimensions_Coordinate> const & dimensions() const { return *this; }
			/// returns the mutable dimensions.
			dimensions<Dimensions_Coordinate> & dimensions() { return *this; }
			/// sets the dimensions.
			void dimensions(dimensions<Dimensions_Coordinate> const & dimensions) { dimensions::operator=(position); }
		//\}

		///\name width dimension coordinate
		//\{
			/// returns the width.
			Dimensions_Coordinate const & w() const { return dimensions<Dimensions_Coordinate>::w(); }
			/// returns the mutable width.
			Dimensions_Coordinate & w() { return dimensions<Dimensions_Coordinate>::w(); }
			/// sets the width.
			void w(Dimensions_Coordinate const & w) { dimensions<Dimensions_Coordinate>::w(w); }
		//\}

		///\name height dimension coordinate
		//\{
			/// returns the height.
			Dimensions_Coordinate const & h() const { return dimensions<Dimensions_Coordinate>::h(); }
			/// returns the mutable height.
			Dimensions_Coordinate & h() { return dimensions<Dimensions_Coordinate>::h(); }
			/// sets the height.
			void h(Dimensions_Coordinate const & h) { dimensions<Dimensions_Coordinate>::h(h); }
		//\}

		///\name left position coordinate
		//\{
			/// returns left.
			Position_Coordinate const & left() const { return position<Position_Coordinate>::x(); }
			/// sets left.
			/// \note this translates the left/right position bounds, rather than changing the width dimension.
			void left(Position_Coordinate const & left) { position<Position_Coordinate>::x(left); }
		//\}

		///\name right position coordinate
		//\{
			/// returns right.
			Position_Coordinate const right() const { return position<Position_Coordinate>::x() + dimensions<Dimensions_Coordinate>::w(); }
			/// sets right.
			/// \note this translates the left/right position bounds, rather than changing the width dimension.
			void right(Position_Coordinate const & right) { position<Position_Coordinate>::x(right - dimensions<Dimensions_Coordinate>::w()); }
		//\}

		///\name top position coordinate
		//\{
			/// returns top.
			Position_Coordinate const & top() const { return position<Position_Coordinate>::y(); }
			/// sets top.
			/// \note this translates the top/bottom position bounds, rather than changing the height dimension.
			void top(Position_Coordinate const & top) { position<Position_Coordinate>::y(top); }
		//\}

		///\name bottom position coordinate
		//\{
			/// returns the bottom
			Position_Coordinate const bottom() const { return position<Position_Coordinate>::y() + dimensions<Dimensions_Coordinate>::h(); }
			/// sets bottom.
			/// \note this translates the top/bottom position bounds, rather than changing the height dimension.
			void bottom(Position_Coordinate const & bottom) { position<Position_Coordinate>::y(bottom - dimensions<Dimensions_Coordinate>::h()); }
		//\}

		///\name top left position
		//\{
			/// returns the top left position.
			position<Position_Coordinate> const & top_left() const { return position(); }
			/// sets the top left position.
			/// \note this translates the position, rather than changing the dimensions.
			void top_left(position<Position_Coordinate> const & top_left) { position(top_left); }
		//\}

		///\name top right position
		//\{
			/// returns the top right position
			position<Position_Coordinate> const top_right() const { return position(right(), top()); }
			/// sets the top right position.
			/// \note this translates the position, rather than changing the dimensions.
			void top_right(position<Position_Coordinate> const & top_right) { top(top_right.y()); right(top_right.x()); }
		//\}

		///\name bottom left position
		//\{
			/// returns the bottom left position.
			position<Position_Coordinate> const bottom_left() const { return position(left(), bottom()); }
			/// sets the bottom left position.
			/// \note this translates the position, rather than changing the dimensions.
			void bottom_left(position<Position_Coordinate> const & bottom_left) { bottom(bottom_left.y()); left(bottom_left.x()); }
		//\}

		///\name bottom right position
		//\{
			/// returns the bottom right position.
			position<Position_Coordinate> const bottom_right() const { return position() + dimensions(); }
			/// sets the bottom right position.
			/// \note this translates the position, rather than changing the dimensions.
			void bottom_right(position<Position_Coordinate> const & bottom_right) { position(bottom_right - dimensions()); }
		//\}
};
}}}
