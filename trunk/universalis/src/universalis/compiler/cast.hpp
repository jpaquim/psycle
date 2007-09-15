// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2007 johan boule <bohan@jabber.org>
// copyright 2002-2007 psycledelics http://psycle.sourceforge.net

///\file
///\interface universalis::compiler::cast
#pragma once
#include <universalis/detail/project.hpp>
#include <universalis/operating_system/loggers.hpp>
#include "typenameof.hpp"
#include "template_constructors.hpp"
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#if !defined NDEBUG
	#include <typeinfo>
#endif
namespace universalis { namespace compiler { namespace cast {
	template<typename Derived, typename Base>
	Derived inline & derive(Base & base)
		#if !defined NDEBUG
			throw(std::bad_cast)
		#endif
	{
		BOOST_STATIC_ASSERT((boost::is_base_and_derived<Base, Derived>::value));
		#if 0 && !defined NDEBUG
			if(operating_system::loggers::trace()())
			{
				std::ostringstream s;
				s << "downcasting from " << typenameof(base) << " down to " << typenameof(static_cast<Derived*>(0));
				operating_system::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
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
	class derived
	{
		public:
			inline operator Derived const & () const throw() { return /*derive<Derived const>*/ static_cast<Derived const &>(*this); }
			inline operator Derived       & ()       throw() { return /*derive<Derived      >*/ static_cast<Derived       &>(*this); }
	};

	template<typename Derived_Underlying, typename Derived, typename Underlying>
	class derived_underlying //: public derived< Derived_Underlying, derived< Underlying, derived<Derived> > >
	{
		private:
			BOOST_STATIC_ASSERT((boost::is_base_and_derived<Underlying, Derived_Underlying>::value));
		public:
			inline operator Derived_Underlying const & () const throw()
			{
				#if 0 && !defined NDEBUG
					if(operating_system::loggers::trace()())
					{
						std::ostringstream s;
						s
							<< "derived underlying: "
							<<                                                                                                              this
							<< " "
							<<                                                                           static_cast<Derived const * const>(this)
							<< " "
							<<                                         &static_cast<Underlying const &>(*static_cast<Derived const * const>(this))
							<< " "
							<< &static_cast<Derived_Underlying const &>(static_cast<Underlying const &>(*static_cast<Derived const * const>(this)));
						operating_system::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
				//return derive<Derived_Underlying const>(static_cast<Underlying const &>(derive<Derived const>(*this)));
				return static_cast<Derived_Underlying const &>(static_cast<Underlying const &>(static_cast<Derived const &>(*this)));
			}
			inline operator Derived_Underlying       & ()       throw()
			{
				#if 0 && !defined NDEBUG
					if(operating_system::loggers::trace()())
					{
						std::ostringstream s;
						s
							<< "derived underlying: "
							<<                                                                                                              this
							<< " "
							<<                                                                           static_cast<Derived       * const>(this)
							<< " "
							<<                                         &static_cast<Underlying       &>(*static_cast<Derived       * const>(this))
							<< " "
							<< &static_cast<Derived_Underlying       &>(static_cast<Underlying       &>(*static_cast<Derived       * const>(this)));
						operating_system::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
				//return derive<Derived_Underlying      >(static_cast<Underlying       &>(derive<Derived       &>(*this));
				return static_cast<Derived_Underlying       &>(static_cast<Underlying       &>(static_cast<Derived       &>(*this)));
			}
	};
	
	template<typename Derived_Underlying, typename Base_Wrapper = void>
	class underlying_wrapper
	:
		public Base_Wrapper,
		public derived_underlying
		<
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
			underlying_type inline const & underlying() const throw() { return *this; }
			underlying_type inline       & underlying()       throw() { return *this; }
	};

	template<typename Underlying>
	class underlying_wrapper<Underlying>
	{
		public:
			typedef Underlying underlying_type;
			
			Underlying const & underlying() const throw() { return *this; }
			Underlying       & underlying()       throw() { return *this; }
			
			operator Underlying const & () const throw() { return this->underlying_; }
			operator Underlying       & ()       throw() { return this->underlying_; }
		private:
			Underlying & underlying_;
		protected:
			typedef underlying_wrapper underlying_wrapper_type;
			underlying_wrapper(Underlying & underlying) : underlying_(underlying)
			{
				#if 0 && !defined NDEBUG
					if(operating_system::loggers::trace()())
					{
						std::ostringstream s;
						s << "underlying wrapper: " << this << " -> " << &underlying;
						operating_system::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
			}
	};

	template<typename Underlying>
	class underlying_value_wrapper
	{
		public:
			typedef Underlying underlying_type;
			
			Underlying const & underlying() const throw() { return *this; }
			Underlying       & underlying()       throw() { return *this; }
			
			operator Underlying const & () const throw() { return this->underlying_; }
			operator Underlying       & ()       throw() { return this->underlying_; }
		private:
			Underlying underlying_;
		protected:
			typedef underlying_value_wrapper underlying_wrapper_type;
			underlying_value_wrapper() {}
			underlying_value_wrapper(Underlying const & underlying) : underlying_(underlying)
			{
				#if 0 && !defined NDEBUG
					if(operating_system::loggers::trace()())
					{
						std::ostringstream s;
						s << "underlying value wrapper: " << this << " -> " << &underlying;
						operating_system::loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				#endif
			}
	};
}}}
