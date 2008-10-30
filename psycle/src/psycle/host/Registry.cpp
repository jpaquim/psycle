///\file
///\implementation psycle::host::Registry.
#include <psycle/project.private.hpp>
#include "Registry.hpp"
namespace psycle
{
	namespace host
	{
		Registry::Registry()
		:
			root(),
			current()
		{
		}

		Registry::~Registry()
		{
			if(current) ::RegCloseKey(current);
			if(root) ::RegCloseKey(root);
		}

		Registry::result
		Registry::OpenRootKey(Registry::key const & key, Registry::name const & name)
		{
			result result(::RegOpenKeyEx(key, name.c_str(), 0, KEY_ALL_ACCESS, &root));
			if(result == ERROR_FILE_NOT_FOUND) result = ::RegCreateKey(key, name.c_str(), &root);
			return result;
		}

		Registry::result
		Registry::OpenKey(Registry::name const & name)
		{
			return ::RegOpenKeyEx(root, name.c_str(), 0, KEY_ALL_ACCESS, &current);
		}

		Registry::result
		Registry::CloseKey()
		{
			result result(::RegCloseKey(current));
			if(result == ERROR_SUCCESS) current = key();
			return result;
		}

		Registry::result
		Registry::CloseRootKey()
		{
			result result(::RegCloseKey(root));
			if(result == ERROR_SUCCESS) root = key();
			return result;
		}

		Registry::result
		Registry::CreateKey(Registry::name const & name)
		{
			return ::RegCreateKey(root, name.c_str(), &current);
		}

		Registry::result
		Registry::DeleteValue(Registry::name const & name)
		{
			return ::RegDeleteValue(current, name.c_str());
		}

		Registry::result
		Registry::QueryTypeAndSize(Registry::name const & name, type & type, std::size_t & size)
		{
			BOOST_STATIC_ASSERT((sizeof(long int) == sizeof(int))); // microsoft likes using unsigned long int, aka their DWORD typedef, instead of std::size_t
			type = REG_NONE;
			size = 0;
			return ::RegQueryValueEx(current, name.c_str(), 0, &type, 0, reinterpret_cast<unsigned long int*>(&size));
		}

		Registry::result
		Registry::QueryValue(Registry::name const & name,              bool & b)
		{
			return QueryValue(name, b, REG_BINARY);
		}
		Registry::result
		Registry::QueryValue(Registry::name const & name, unsigned      int & i)
		{
			return QueryValue(name, i, REG_DWORD);
		}
		Registry::result
		Registry::QueryValue(Registry::name const & name,   signed      int & i)
		{
			return QueryValue(name, i, REG_DWORD);
		}
		Registry::result
		Registry::QueryValue(Registry::name const & name, unsigned long int & i)
		{
			return QueryValue(name, i, REG_DWORD);
		}
		Registry::result
		Registry::QueryValue(Registry::name const & name,   signed long int & i)
		{
			return QueryValue(name, i, REG_DWORD);
		}
		Registry::result
		Registry::QueryValue(Registry::name const & name,       std::string & s)
		{
			std::size_t size;
			type type;
			// get length of string
			result error(QueryTypeAndSize(name, type, size));
			if(error != ERROR_SUCCESS) return error;
			if(type != REG_SZ) return ERROR_FILE_CORRUPT;
			// allocate a buffer
			char * buffer(new char[size]);
			error = ::RegQueryValueEx(current, name.c_str(), 0, &type, reinterpret_cast<unsigned char *>(buffer), reinterpret_cast<unsigned long int*>(&size));
			if(error == ERROR_SUCCESS) s = buffer;
			delete[] buffer;
			return error;
		}

		Registry::result
		Registry::SetValue(Registry::name const & name,              bool const & b)
		{
			return SetValue(name, b, REG_BINARY);
		}
		Registry::result
		Registry::SetValue(Registry::name const & name, unsigned      int const & i)
		{
			return SetValue(name, i, REG_DWORD);
		}
		Registry::result
		Registry::SetValue(Registry::name const & name,   signed      int const & i)
		{
			return SetValue(name, i, REG_DWORD);
		}
		Registry::result
		Registry::SetValue(Registry::name const & name, unsigned long int const & i)
		{
			return SetValue(name, i, REG_DWORD);
		}
		Registry::result
		Registry::SetValue(Registry::name const & name,   signed long int const & i)
		{
			return SetValue(name, i, REG_DWORD);
		}
		Registry::result
		Registry::SetValue(Registry::name const & name,       std::string const & s)
		{
			return ::RegSetValueEx(current, name.c_str(), 0, REG_SZ, reinterpret_cast<unsigned char const *>(s.c_str()), s.length() + 1);
		}
	}
}
