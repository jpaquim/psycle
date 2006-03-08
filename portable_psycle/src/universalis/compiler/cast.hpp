// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief \interface universalis::compiler::cast
#pragma once
#include <universalis/detail/project.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
namespace universalis
{
	namespace compiler
	{
		namespace cast
		{
			template<typename Derived, typename Base>
			Derived inline & down_cast(Base & base)
			{
				BOOST_STATIC_ASSERT((boost::is_base_and_derived<Base, Derived>::value));
				#if defined NDEBUG
					return static_cast<Derived&>(base);
				#else
					Derived * const result(dynamic_cast<Derived * const>(&base));
					assert(result);
					return *result;
				#endif
			}

			template<typename Derived_Underlying, typename Base_Wrapper = void>
			class underlying
			{
				private:
					BOOST_STATIC_ASSERT((boost::is_base_and_derived<typename Base_Wrapper::underlying_type, Derived_Underlying>::value));
				protected:
					typedef underlying<Derived_Underlying, Base_Wrapper> underlying_template_type;
				private:
					inline underlying() {}
					friend class default_constructor;
					friend class no_base;
				public:
					typedef Derived_Underlying underlying_type;
					Derived_Underlying inline & operator()() { return down_cast<Derived_Underlying>(Base_Wrapper::operator()()); }
					Derived_Underlying inline const & operator()() const { return down_cast<Derived_Underlying const>(Base_Wrapper::operator()()); }
					inline operator Derived_Underlying & () { return (*this)(); }
					inline operator Derived_Underlying const & () const { return (*this)(); }
					inline operator Derived_Underlying * () { return &(*this)(); }
					inline operator Derived_Underlying const * () const { return &(*this)(); }
					class default_constructor
					:
						public Base_Wrapper,
						public underlying
					{
						protected:
							inline default_constructor(Derived_Underlying & underlying) : Base_Wrapper(underlying) {}
							typedef default_constructor underlying_template_type;
						public:
							typedef Derived_Underlying underlying_type;
							Derived_Underlying inline & operator()() { return underlying_template_type::operator()(); }
							Derived_Underlying inline const & operator()() const { return underlying_template_type::operator()(); }
					};
					class no_base
					:
						public underlying
					{
					};
			};

			template<typename Underlying>
			class underlying<Underlying, void>
			{
				private:
					Underlying & underlying_;
				protected:
					inline underlying(Underlying & underlying) : underlying_(underlying) {}
					typedef underlying underlying_template_type;
				public:
					typedef Underlying underlying_type;
					Underlying inline & operator()() { return underlying_; }
					Underlying inline const & operator()() const { return underlying_; }
					inline operator Underlying & () { return (*this)(); }
					inline operator Underlying const & () const { return (*this)(); }
					inline operator Underlying * () { return &(*this)(); }
					inline operator Underlying const * () const { return &(*this)(); }
			};
		}
	}
}

// arch-tag: b1cf00b6-68ad-46b8-9878-f3ae38f89298
