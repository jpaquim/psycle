/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper <natti@linux>                 *
 *   Copyright (C) 2006 by Johan Boule <bohan@jabber.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef NPROPERTY_H
#define NPROPERTY_H

#include <map>
#include <string>
#include <typeinfo>
#include <stdexcept>

///\internal
/// the user doesn't have to use the NProperty class directly,
/// it's only used via NPropertyMap.
namespace detail
{
	/// One property is composed of
	/// a reference to an object and pointers to getter and setter member functions
	template<typename Class, typename Value>
	class NProperty
	{
		///\name instance
		/// the instance we call the getter and/or setter member functions with
		///\{
			protected:
				Class       & instance()       { return *instance_; }
				Class const & instance() const { return *instance_; }
			private:
				Class       * instance_;

			public:
				std::type_info type() const { return typeid(instance()); }
		///\}

		///\name member function pointers
		///\{
			public:
				/// the signature for the getter member function that Class should have.
				/// It takes no argument and returns a const reference to a Value.
				/// This is a const function of Class
				typedef Value const & (Class::*GetterMemberFunction) () const;
				/// the signature for the setter member function that Class should have.
				/// It takes one const reference to a Value as argument and returns nothing.
				/// This is a non-const function of Class
				typedef void (Class::*SetterMemberFunction) (Value const &);

			private:
				/// pointer to the getter member function in Class
				GetterMemberFunction getterMemberFunction;
				/// pointer to the getter member function in Class
				SetterMemberFunction setterMemberFunction;
		///\}

		public:
			NProperty
			(
				Class & instance,
				GetterMemberFunction getterMemberFunction,
				SetterMemberFunction setterMemberFunction
			)
			:
				instance_(&instance),
				getterMemberFunction(getterMemberFunction),
				setterMemberFunction(setterMemberFunction)
			{}

			/// calls the getter member function on the instance
			Value const & get() const throw(std::exception)
			{
				if(!getterMemberFunction) throw("not a readable property");
				return (this->instance().*getterMemberFunction)();
			}

			/// calls the setter member function on the instance
			void set(Value const & value) throw(std::exception)
			{
				if(!setterMemberFunction) throw("not a writable property");
				(this->instance().*setterMemberFunction)(value);
			}

		public: // friend class std::map<xxx,xxx>
			NProperty() {}
	};
}

class NPropertyMap
{
	private:
		/// the type of key used for the mapping
		typedef std::string Key;

		/// associative container that maps a Key to a NProperty
		template<typename Class, typename Value> class Map : public std::map< Key, detail::NProperty<Class, Value> > {};

		///\name the ugly work around for the lack of covariance on template classes in C++
		///\{
			class AnyClass {};
			typedef void const * AnyValue;
			typedef Map<AnyClass, AnyValue> AnyMap;
			AnyMap anyMap;
		///\}

	///\name property registration functions
	///\{
		public: // it's declared public but will actually probably only used by the object that owns the property map

			template<typename Value, typename Class>
			void registrate
			(
				std::string const & name,
				Class & instance,
				typename detail::NProperty<Class, Value>::GetterMemberFunction getterMemberFunction,
				typename detail::NProperty<Class, Value>::SetterMemberFunction setterMemberFunction = 0
			)
			{
				(reinterpret_cast<Map<Class, Value>&>(anyMap))[name] =
					detail::NProperty<Class, Value>(instance, getterMemberFunction, setterMemberFunction);
			}

			template<typename Value, typename Class>
			void registrate
			(
				std::string const & name,
				Class & instance,
				typename detail::NProperty<Class, Value>::SetterMemberFunction setterMemberFunction
			)
			{
				(reinterpret_cast<Map<Class, Value>&>(anyMap))[name] =
					detail::NProperty<Class, Value>(instance, 0, setterMemberFunction);
			}
	///\}

	///\name getter and setter functions
	///\{
		public:
			template<typename Value>
			void get(std::string const & name, Value & result) const throw(std::exception)
			{
				result = get<Value>(name);
			}

			template<typename Value>
			Value const & get(std::string const & name) const throw(std::exception)
			{
				typename AnyMap::const_iterator i(anyMap.find(name));
				if(i == anyMap.end()) throw std::runtime_error(name + " was not found in the property map");
				return reinterpret_cast<Value const &>(i->second.get());
			}

			template<typename Value>
			void set(std::string const & name, Value const & value) throw(std::exception)
			{
				typename AnyMap::const_iterator i(anyMap.find(name));
				if(i == anyMap.end()) throw std::runtime_error(name + " was not found in the property map");
				i->second.set(reinterpret_cast<AnyValue const &>(value));
			}
	///\}
};

#endif
