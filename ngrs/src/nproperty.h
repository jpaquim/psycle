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
	template<typename Class, typename GetValue, typename SetValue = GetValue>
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
				typedef GetValue (Class::*GetterMemberFunction) () const;
				/// the signature for the setter member function that Class should have.
				/// It takes one const reference to a Value as argument and returns nothing.
				/// This is a non-const function of Class
				typedef void (Class::*SetterMemberFunction) (SetValue);

			private:
				/// pointer to the getter member function in Class
				GetterMemberFunction getterMemberFunction;
				/// pointer to the getter member function in Class
				SetterMemberFunction setterMemberFunction;
		///\}

		///\name runtime type information of Value
		///\{
			public:
				std::type_info const & getType() const { return *getType_; }
			private:
				std::type_info const * getType_;

			public:
				std::type_info const & setType() const { return *setType_; }
			private:
				std::type_info const * setType_;
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
				setterMemberFunction(setterMemberFunction),
				getType_(&typeid(GetValue)),
				setType_(&typeid(SetValue))
			{}

		public:
			/// calls the getter member function on the instance
			GetValue get() const throw(std::exception)
			{
				if(!getterMemberFunction) throw("not a readable property");
				return (this->instance().*getterMemberFunction)();
			}

			/// calls the setter member function on the instance
			void set(SetValue value) throw(std::exception)
			{
				if(!setterMemberFunction) throw("not a writable property");
				(this->instance().*setterMemberFunction)(value);
			}

		///\name signal emited automatically when the set(Value const &) function is called
		///\{
			public:
				///\todo [bohan] not sure NProperty<Class, GetValue, SetValue> is what's needed in the signal
				typedef sigslot::signal1<NProperty<Class, GetValue, SetValue>&> OnChangeSignal;
				OnChangeSignal const & onChangeSignal() const { return onChangeSignal_; }
				OnChangeSignal       & onChangeSignal()       { return onChangeSignal_; }
			private:
				OnChangeSignal         onChangeSignal_;
		///\}

		public: // friend class NPropertyMap::Map<AnyClass, AnyValue, AnyValue>
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
		template<typename Class, typename GetValue, typename SetValue> class Map : public std::map< Key, detail::NProperty<Class, GetValue, SetValue> > {};

		///\name the ugly work around for the lack of covariance on template classes in C++
		///\{
			class AnyClass {};
			typedef void const * AnyValue;
			typedef Map<AnyClass, AnyValue, AnyValue> AnyMap;
			AnyMap anyMap;

			template<typename Class, typename GetValue, typename SetValue>
			Map<Class, GetValue, SetValue> & map()
			{
				return reinterpret_cast<Map<Class, GetValue, SetValue>&>(anyMap);
			}
		///\}

	///\name property registration functions
	///\{
		public: // it's declared public but will actually probably only used by the object that owns the property map

			/// read and optionally write with different types for getter and setter
			template<typename GetValue, typename SetValue, typename Class>
			void registrate
			(
				Key const & key,
				Class & instance,
				typename detail::NProperty<Class, GetValue, SetValue>::GetterMemberFunction getterMemberFunction,
				typename detail::NProperty<Class, GetValue, SetValue>::SetterMemberFunction setterMemberFunction = 0
			)
			{
				map<Class, GetValue, SetValue>()[key] = detail::NProperty<Class, GetValue, SetValue>(instance, getterMemberFunction, setterMemberFunction);
			}

			/// write-only with different types for getter and setter
			template<typename GetValue, typename SetValue, typename Class>
			void registrate
			(
				Key const & key,
				Class & instance,
				typename detail::NProperty<Class, GetValue, SetValue>::SetterMemberFunction setterMemberFunction
			)
			{
				map<Class, GetValue, SetValue>()[key] = detail::NProperty<Class, GetValue, SetValue>(instance, 0, setterMemberFunction);
			}

			/// read and optionally write with same type for getter and setter (Value const &)
			template<typename Value, typename Class>
			void registrate
			(
				Key const & key,
				Class & instance,
				typename detail::NProperty<Class, Value const &>::GetterMemberFunction getterMemberFunction,
				typename detail::NProperty<Class, Value const &>::SetterMemberFunction setterMemberFunction = 0
			)
			{
				registrate<Value const &, Value const &>(key, instance, getterMemberFunction, setterMemberFunction);
			}

			/// write-only with same type for getter and setter (Value const &)
			template<typename Value, typename Class>
			void registrate
			(
				Key const & key,
				Class & instance,
				typename detail::NProperty<Class, Value const &>::SetterMemberFunction setterMemberFunction
			)
			{
				registrate<Value const &>(key, instance, 0, setterMemberFunction);
			}
	///\}

	///\name getter and setter functions
	///\{
		public:
			std::type_info const & get_type(Key const & key) const throw(std::exception)
			{
				return find<AnyClass, AnyValue, AnyValue>(key).getType();
			}

			std::type_info const & set_type(Key const & key) const throw(std::exception)
			{
				return find<AnyClass, AnyValue, AnyValue>(key).setType();
			}

			template<typename Value>
			void get(Key const & key, Value & result) const throw(std::exception)
			{
				result = get<Value>(key);
			}

			template<typename Value>
			Value const & get(Key const & key) const throw(std::exception)
			{
				return find<AnyClass, Value, AnyValue>(key).get();
			}

			template<typename Value>
			void set(Key const & key, Value const & value) throw(std::exception)
			{
				find<AnyClass, AnyValue, Value>(key).set(value);
			}

			template<typename Class, typename GetValue, typename SetValue>
			typename detail::NProperty<Class, GetValue, SetValue>::OnChangeSignal const & onChangeSignal(Key const & key) const throw(std::exception)
			{
				return find<Class, GetValue, SetValue>(key).onChangeSignal();
			}

			template<typename Class, typename GetValue, typename SetValue>
			typename detail::NProperty<Class, GetValue, SetValue>::OnChangeSignal & onChangeSignal(Key const & key) throw(std::exception)
			{
				return find<Class, GetValue, SetValue>(key).onChangeSignal();
			}
                        
			std::vector<Key> methodNames() const
			{
				std::vector<Key> listing;
				for (AnyMap::const_iterator i(anyMap.begin()) ; i != anyMap.end() ; ++i) listing.push_back(i->first);
				return listing;
			}
 
		private:
			template<typename Class, typename GetValue, typename SetValue>
			detail::NProperty<Class, GetValue, SetValue> const & find(Key const & key) const throw(std::exception)
			{
				return const_cast<NPropertyMap&>(*this).find<Class, GetValue, SetValue>(key);
			}

			template<typename Class, typename GetValue, typename SetValue>
			detail::NProperty<Class, GetValue, SetValue> & find(Key const & key) throw(std::exception)
			{
				typedef Map<Class, GetValue, SetValue> Map;
				Map & map(this->map<Class, GetValue, SetValue>());
				typename Map::iterator i(map.find(key));
				if(i == map.end()) throw std::runtime_error(key + " was not found in the property map");
				return i->second;
			}
	///\}
};

#endif
