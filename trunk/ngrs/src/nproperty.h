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
#include <vector>
#include <typeinfo>
#include <stdexcept>
#include "sigslot.h"

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

			/// runtime type information
			std::type_info const & type() const { return typeid(Value); }

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

		///\name signal emited automatically when the set(Value const &) function is called
		///\{
			public:
				typedef sigslot::signal1<NProperty<Class, Value>&> OnChangeSignal;
				OnChangeSignal const & onChangeSignal() const { return onChangeSignal_; }
				OnChangeSignal       & onChangeSignal()       { return onChangeSignal_; }
			private:
				OnChangeSignal         onChangeSignal_;
		///\}

		public: // friend class NPropertyMap::Map<AnyClass, AnyValue>
			NProperty() {}
	};
}

class NPropertyMap
{
	public:
		/// the type of key used for the mapping
		typedef std::string Key;

	private:
		/// associative container that maps a Key to a NProperty
		template<typename Class, typename Value> class Map : public std::map< Key, detail::NProperty<Class, Value> > {};

		///\name the ugly work around for the lack of covariance on template classes in C++
		///\{
			class AnyClass {};
			typedef void const * AnyValue;
			typedef Map<AnyClass, AnyValue> AnyMap;
			AnyMap anyMap;

			template<typename Class, typename Value>
			Map<Class, Value> & map()
			{
				return reinterpret_cast<Map<Class, Value>&>(anyMap);
			}
		///\}

	///\name property registration functions
	///\{
		public: // it's declared public but will actually probably only used by the object that owns the property map

			template<typename Value, typename Class>
			void registrate
			(
				Key const & key,
				Class & instance,
				typename detail::NProperty<Class, Value>::GetterMemberFunction getterMemberFunction,
				typename detail::NProperty<Class, Value>::SetterMemberFunction setterMemberFunction = 0
			)
			{
				map<Class, Value>()[key] = detail::NProperty<Class, Value>(instance, getterMemberFunction, setterMemberFunction);
			}

			template<typename Value, typename Class>
			void registrate
			(
				Key const & key,
				Class & instance,
				typename detail::NProperty<Class, Value>::SetterMemberFunction setterMemberFunction
			)
			{
				map<Class, Value>()[key] = detail::NProperty<Class, Value>(instance, 0, setterMemberFunction);
			}
	///\}

	///\name getter and setter functions
	///\{
		public:
			std::type_info const & type(Key const & key) const throw(std::exception)
			{
				return find<AnyClass, AnyValue>(key).type();
			}

			template<typename Value>
			void get(Key const & key, Value & result) const throw(std::exception)
			{
				result = get<Value>(key);
			}

			template<typename Value>
			Value const & get(Key const & key) const throw(std::exception)
			{
				return find<AnyClass, Value>(key).get();
			}

			template<typename Value>
			void set(Key const & key, Value const & value) throw(std::exception)
			{
				find<AnyClass, Value>(key).set(value);
			}

			template<typename Class, typename Value>
			typename detail::NProperty<Class, Value>::OnChangeSignal const & onChangeSignal(Key const & key) const throw(std::exception)
			{
				return find<Class, Value>(key).onChangeSignal();
			}

			template<typename Class, typename Value>
			typename detail::NProperty<Class, Value>::OnChangeSignal & onChangeSignal(Key const & key) throw(std::exception)
			{
				return find<Class, Value>(key).onChangeSignal();
			}
                        
                        std::vector<Key> methodNames() const  {
                           std::vector<Key> listing;
			   for (AnyMap::const_iterator i(anyMap.begin()) ; i != anyMap.end() ; ++i) listing.push_back(i->first);
                           return listing;
                        }
 
		private:
			template<typename Class, typename Value>
			detail::NProperty<Class, Value> const & find(Key const & key) const throw(std::exception)
			{
				return const_cast<NPropertyMap&>(*this).find<Class, Value>(key);
			}

			template<typename Class, typename Value>
			detail::NProperty<Class, Value> & find(Key const & key) throw(std::exception)
			{
				typedef Map<Class, Value> Map;
				Map & map(this->map<Class, Value>());
				typename Map::iterator i(map.find(key));
				if(i == map.end()) throw std::runtime_error(key + " was not found in the property map");
				return i->second;
			}
	///\}
};

#endif
