///\file
///\interface psycle::host::Registry.
#pragma once
#include <string>
#include <boost/static_assert.hpp>
#include "Loggers.hpp"
namespace psycle
{
	namespace host
	{
		/// the evil microsoft windows registry.
		class Registry
		{
			public:
				Registry();
				virtual ~Registry();
				typedef ::HKEY key;
				typedef std::string name;
				typedef unsigned long int type;
				typedef long int result;
				result OpenRootKey(key const &, name const &);
				result OpenKey(name const &);
				result CloseKey();
				result CloseRootKey();
				result CreateKey(name const &);
				result DeleteValue(name const &);

				result QueryTypeAndSize(name const &, type & type, std::size_t & size);

				template<typename x>
				result QueryValue(name const & name, x & data, type const & type_wanted = REG_BINARY)
				{
					BOOST_STATIC_ASSERT((sizeof(long int) == sizeof(int))); // microsoft likes using unsigned long int, aka their DWORD typedef, instead of std::size_t
					std::size_t size(sizeof data);
					type type_read;
					result const error(::RegQueryValueEx(current, name.c_str(), 0, &type_read, reinterpret_cast<unsigned char *>(&data), reinterpret_cast<unsigned long int*>(&size)));
					// automatically upgrade old versions of registry settings
					if
					(
						error == ERROR_FILE_NOT_FOUND || // create new entry
						error == ERROR_MORE_DATA /* size > sizeof data */ || // reduce the size
						error == ERROR_SUCCESS && type_read != type_wanted /* && size == sizeof data */ // change the type

					)
					{
						assert(size >= sizeof data);
						return SetValue(name, data, type_wanted);
					}
					// everything else should be all right, assert:
					assert(type_read == type_wanted);
					assert(size == sizeof data);
					return error;
				}
				result QueryValue(name const &,              bool & b);
				result QueryValue(name const &, unsigned      int & i);
				result QueryValue(name const &,   signed      int & i);
				result QueryValue(name const &, unsigned long int & i);
				result QueryValue(name const &,   signed long int & i);
				result QueryValue(name const &,       std::string & s);

				template<typename x>
				result SetValue(name const & name, x const & data, type const & type = REG_BINARY)
				{
					result const error(::RegSetValueEx(current, name.c_str(), 0, type , reinterpret_cast<unsigned char const *>(&data), sizeof data));
					if(error != ERROR_SUCCESS) loggers::warning("could not write " + name + "to registry.");
					return error;
				}
				result SetValue(name const &,              bool const & b);
				result SetValue(name const &, unsigned      int const & i);
				result SetValue(name const &,   signed      int const & i);
				result SetValue(name const &, unsigned long int const & i);
				result SetValue(name const &,   signed long int const & i);
				result SetValue(name const &,       std::string const & s);

			private:
				key root;
				key current;
		};
	}
}
