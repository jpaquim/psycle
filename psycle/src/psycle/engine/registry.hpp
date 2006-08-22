///\file
///\interface psycle::host::Registry.
#pragma once
#include <psycle/host/uiglobal.hpp> // use of "loggers"
#include <string>
#include <boost/static_assert.hpp>
namespace psycle
{
	namespace host
	{
		/// facility to load/save persistent configuration settings.
		class Registry
		{
			public:
				Registry();
				~Registry() throw();

				typedef
					#if defined PSYCLE__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT // make use of windows registry database
						::HKEY
					#else
						#error todo
					#endif
					key;

				typedef std::string name;

				typedef
					#if defined PSYCLE__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT // make use of windows registry database
						unsigned long int
					#else
						#error todo
					#endif
					type;

				typedef
					#if defined PSYCLE__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT // make use of windows registry database
						long int
					#else
						#error todo
					#endif
					result;

				///\todo what happens to the notion of root key in a broader sense, i.e., when using some other implementation that windows' registry db?
				result OpenRootKey(key const &, name const &);
				result CloseRootKey();

				result OpenKey(name const &);
				result CloseKey();

				result CreateKey(name const &);
				result DeleteValue(name const &);

				result QueryTypeAndSize(name const &, type & type, std::size_t & size);

				template<typename x>
				result QueryValue
				(
					name const & name, x & data, type const & type_wanted =
					#if defined PSYCLE__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT // make use of windows registry database
						REG_BINARY
					#else
						#error todo
					#endif
				)
				{
					#if defined PSYCLE__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT // make use of windows registry database
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
					#else
						#error todo
					#endif
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
					#if defined PSYCLE__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT // make use of windows registry database
						result const error(::RegSetValueEx(current, name.c_str(), 0, type , reinterpret_cast<unsigned char const *>(&data), sizeof data));
						if(error != ERROR_SUCCESS) loggers::warning("could not write " + name + " to registry.");
						return error;
					#else
						#error todo
					#endif
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
