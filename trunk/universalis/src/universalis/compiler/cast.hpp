// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2007 johan boule <bohan@jabber.org>
// copyright 2002-2007 psycledelics http://psycle.sourceforge.net

///\file
///\interface universalis::compiler::cast

#ifndef UNIVERSALIS__COMPILER__CAST__INCLUDED
#define UNIVERSALIS__COMPILER__CAST__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>
#include <universalis/os/loggers.hpp>
#include "typenameof.hpp"
#include "template_constructors.hpp"
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#if !defined NDEBUG
	#include <typeinfo>
#endif

namespace universalis { namespace compiler { namespace cast {

	template<typename Derived, typename Base>
	Derived & derive(Base & base)
		#if !defined NDEBUG
			throw(std::bad_cast)
		#endif
	{
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<Base, Derived>::value));
		#if 0 && !defined NDEBUG
			if(os::loggers::trace()()) {
				std::ostringstream s;
				s << "downcasting from " << typenameof(base) << " down to " << typenameof(static_cast<Derived*>(0));
				os::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
			}
		#endif
		return
			#if defined NDEBUG
				static_cast
			#else
				dynamic_cast
			#endif
				<Derived&>(base);
	}
	
	template<typename Derived>
	class derived {
		protected:
			typedef derived derived_type;
		public:
			operator Derived const & () const throw() { return /*derive<Derived const>*/ static_cast<Derived const &>(*this); }
			operator Derived       & ()       throw() { return /*derive<Derived      >*/ static_cast<Derived       &>(*this); }
			
			Derived const & derived_this() const throw() { return *this; }
			Derived       & derived_this()       throw() { return *this; }
	};

	template<typename Derived_Underlying, typename Derived, typename Underlying>
	class derived_underlying { //: public derived< Derived_Underlying, derived< Underlying, derived<Derived> > >
		private:
			BOOST_STATIC_ASSERT((boost::is_base_and_derived<Underlying, Derived_Underlying>::value));
		public:
			operator Derived_Underlying const & () const throw() {
				#if 0 && !defined NDEBUG
					if(os::loggers::trace()()) {
						std::ostringstream s;
						s
							<< "derived underlying: "
							<< this << " "
							<< static_cast<Derived const * const>(this) << " "
							<< &static_cast<Underlying const &>(
								*static_cast<Derived const * const>(this)) << " "
							<< &static_cast<Derived_Underlying const &>(
								static_cast<Underlying const &>(
									*static_cast<Derived const * const>(this)));
						os::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
				#if 0
					return derive<Derived_Underlying const>(
						static_cast<Underlying const &>(
							derive<Derived const>(*this)));
				#else
					return static_cast<Derived_Underlying const &>(
						static_cast<Underlying const &>(
							static_cast<Derived const &>(*this)));
				#endif
			}
			
			operator Derived_Underlying & () throw() {
				#if 0 && !defined NDEBUG
					if(os::loggers::trace()()) {
						std::ostringstream s;
						s
							<< "derived underlying: "
							<< this << " "
							<< static_cast<Derived * const>(this) << " "
							<< &static_cast<Underlying &>(
								*static_cast<Derived * const>(this)) << " "
							<< &static_cast<Derived_Underlying &>(
								static_cast<Underlying &>(
									*static_cast<Derived * const>(this)));
						os::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
				#if 0
					return derive<Derived_Underlying>(
						static_cast<Underlying &>(
							derive<Derived &>(*this));
				#else
					return static_cast<Derived_Underlying &>(
						static_cast<Underlying &>(
							static_cast<Derived &>(*this)));
				#endif
			}
	};
	
	template<typename Derived_Underlying, typename Base_Wrapper = void>
	class underlying_wrapper
	:
		public Base_Wrapper,
		public derived_underlying<
			Derived_Underlying,
			underlying_wrapper<Derived_Underlying, Base_Wrapper>,
			typename Base_Wrapper::underlying_type
		>
	{
		public:
			UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS(underlying_wrapper, Base_Wrapper, UNIVERSALIS__COMPILER__TEMPLATE_CONSTRUCTORS__ARITY)
		protected:
			typedef underlying_wrapper underlying_wrapper_type;
		public:
			typedef Derived_Underlying underlying_type;
			underlying_type const & underlying() const throw() { return *this; }
			underlying_type       & underlying()       throw() { return *this; }
	};

	template<typename Underlying>
	class underlying_wrapper<Underlying> {
		public:
			typedef Underlying underlying_type;
			
			operator Underlying const & () const throw() { return this->underlying_; }
			operator Underlying       & ()       throw() { return this->underlying_; }

			Underlying const & underlying() const throw() { return *this; }
			Underlying       & underlying()       throw() { return *this; }
			
		private:
			Underlying & underlying_;
		protected:
			typedef underlying_wrapper underlying_wrapper_type;
			underlying_wrapper(Underlying & underlying) : underlying_(underlying) {
				#if 0 && !defined NDEBUG
					if(os::loggers::trace()()) {
						std::ostringstream s;
						s << "underlying wrapper: " << this << " -> " << &underlying;
						os::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
			}
	};

	template<typename Underlying>
	class underlying_value_wrapper {
		public:
			typedef Underlying underlying_type;
			
			operator Underlying const & () const throw() { return this->underlying_; }
			operator Underlying       & ()       throw() { return this->underlying_; }

			Underlying const & underlying() const throw() { return *this; }
			Underlying       & underlying()       throw() { return *this; }
			
		private:
			Underlying underlying_;
		protected:
			typedef underlying_value_wrapper underlying_wrapper_type;
			underlying_value_wrapper() {}
			underlying_value_wrapper(Underlying const & underlying) : underlying_(underlying) {
				#if 0 && !defined NDEBUG
					if(os::loggers::trace()()) {
						std::ostringstream s;
						s << "underlying value wrapper: " << this << " -> " << &underlying;
						os::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
			}
	};
}}}

#endif
