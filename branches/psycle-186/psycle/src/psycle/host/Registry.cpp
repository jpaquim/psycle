///\file
///\implementation psycle::host::Registry.


#include "Registry.hpp"
namespace psycle
{
	namespace host
	{
		Registry::Registry(root_t tree_root, std::string version)
			: ConfigStorage(version), config_root(), current_group()
		{
			if (tree_root == HKCU) {
				hk_root = HKEY_CURRENT_USER;
			} else if (tree_root == HKLM) {
				hk_root = HKEY_LOCAL_MACHINE;
			}
			version_config_ = version;
		}

		Registry::~Registry()
		{
			CloseLocation();
		}

		bool Registry::CreateLocation(std::string const & location, bool overwrite)
		{
			//Creates the specified registry key. If the key already exists, the function opens it.
			result result = ::RegCreateKeyEx(hk_root, location.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &config_root, 0);
			if (result == ERROR_SUCCESS)
			{
				if(overwrite)
				{
					///\todo.
				}
				current_group = config_root;
				Write("ConfigVersion",version_config_);
				current_group = NULL;
				return true;
			}
			else return false;
		}
		bool Registry::OpenLocation(std::string const & location, bool create_if_missing)
		{
			result result = ::RegOpenKeyEx(hk_root, location.c_str(), 0, KEY_READ, &config_root);
			if(result == ERROR_FILE_NOT_FOUND && create_if_missing) 
			{
				result = ::RegCreateKeyEx(hk_root, location.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &config_root, 0);
				::RegCloseKey(config_root);
				config_root = NULL;
				version_config_ = "";
				result = ::RegOpenKeyEx(hk_root, location.c_str(), 0, KEY_READ, &config_root);
				if(result == ERROR_SUCCESS)
				{
				}
			}
			if(result == ERROR_SUCCESS) 
			{
				current_group = config_root;
				Read("ConfigVersion",version_config_);
				current_group = NULL;
			}
			return (result == ERROR_SUCCESS);
		}
		void Registry::CloseLocation()
		{
			::RegCloseKey(current_group);
			current_group = NULL;
			::RegCloseKey(config_root);
			config_root = NULL;
		}
		bool Registry::CreateGroup(std::string const & group, bool overwrite)
		{
			::RegCloseKey(current_group);
			current_group = NULL;
			result result = ::RegCreateKeyEx(config_root, group.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &current_group, 0);
			if (result == ERROR_SUCCESS)
			{
				if(overwrite)
				{
					///\todo.
				}
				return true;
			}
			else return false;
		}
		bool Registry::OpenGroup(std::string const & group, bool create_if_missing)
		{
			::RegCloseKey(current_group);
			current_group = NULL;
			result result = ::RegOpenKeyEx(config_root, group.c_str(), 0, KEY_READ, &current_group);
			if(result == ERROR_FILE_NOT_FOUND && create_if_missing)
			{
				result = ::RegCreateKeyEx(config_root, group.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &current_group, 0);
				::RegCloseKey(current_group);
				current_group = NULL;
				result = ::RegOpenKeyEx(config_root, group.c_str(), 0, KEY_READ, &current_group);
			}
			return (result == ERROR_SUCCESS);
		}
		bool Registry::CloseGroup()
		{
			result result = ::RegCloseKey(current_group);
			current_group = NULL;
			return (result == ERROR_SUCCESS);
		}

		bool Registry::DeleteGroup(std::string const & key, bool fail_if_not_empty)
		{
			return (::RegDeleteKey(config_root, key.c_str()) == ERROR_SUCCESS);
		}

		bool Registry::DeleteKey(std::string const & key)
		{
			return (::RegDeleteValue(current_group, key.c_str()) == ERROR_SUCCESS);
		}
		///\todo: finish
		 std::list<std::string> Registry::GetGroups(){std::list<std::string> list; return list;}
		 std::list<std::string> Registry::GetKeys(){std::list<std::string> list; return list;}
		 bool Registry::Read(std::string const & key, bool & x){ return QueryValue(key, x, REG_BINARY); }
		 bool Registry::Write(std::string const & key, bool x){return SetValue(key, x, REG_BINARY);}

		 bool Registry::Read(std::string const & key, uint8_t & x){uint32_t x2; bool result = QueryValue(key, x2, REG_DWORD); x = x2; return result; }
		 bool Registry::Read(std::string const & key, int8_t & x){uint32_t x2;  bool result =  QueryValue(key, x2, REG_DWORD); x = x2; return result;  }
		 bool Registry::Read(std::string const & key, char & x){uint32_t x2;  bool result =  QueryValue(key, x2, REG_DWORD); x = x2; return result;  } // 'char' is equivalent to either 'signed char' or 'unsigned char', but considered as a different type
		 bool Registry::Write(std::string const & key, uint8_t x){uint32_t x2 = x; return SetValue(key, x2, REG_DWORD);}
		 bool Registry::Write(std::string const & key, int8_t x){uint32_t x2 = x; return SetValue(key, x2, REG_DWORD);}
		 bool Registry::Write(std::string const & key, char x){uint32_t x2 = x; return SetValue(key, x2, REG_DWORD);} // 'char' is equivalent to either 'signed char' or 'unsigned char', but considered as a different type

		 bool Registry::Read(std::string const & key, uint16_t & x){ uint32_t x2;  bool result =  QueryValue(key, x2, REG_DWORD); x = x2; return result;  }
		 bool Registry::Read(std::string const & key, int16_t & x){uint32_t x2;  bool result =  QueryValue(key, x2, REG_DWORD); x = x2; return result;  }
		 bool Registry::Write(std::string const & key, uint16_t x){uint32_t x2 = x; return SetValue(key, x2, REG_DWORD);}
		 bool Registry::Write(std::string const & key, int16_t x){uint32_t x2 = x; return SetValue(key, x2, REG_DWORD);}

		 bool Registry::Read(std::string const & key, uint32_t & x){return QueryValue(key, x, REG_DWORD);}
		 bool Registry::Read(std::string const & key, int32_t & x){return QueryValue(key, x, REG_DWORD);}
		 bool Registry::Read(std::string const & key, COLORREF & x){return QueryValue(key, x, REG_DWORD);}
		 bool Registry::Write(std::string const & key, uint32_t x){return SetValue(key, x, REG_DWORD);}
		 bool Registry::Write(std::string const & key, int32_t x){return SetValue(key, x, REG_DWORD);}
		 bool Registry::Write(std::string const & key, COLORREF x){return SetValue(key, x, REG_DWORD);}

		 bool Registry::Read(std::string const & key, uint64_t & x){return QueryValue(key, x, REG_QWORD);}
		 bool Registry::Read(std::string const & key, int64_t & x){return QueryValue(key, x, REG_QWORD);}
		 bool Registry::Write(std::string const & key, uint64_t x){return SetValue(key, x, REG_QWORD);}
		 bool Registry::Write(std::string const & key, int64_t x){return SetValue(key, x, REG_QWORD);}

		 bool Registry::Read(std::string const & key, float & x){return QueryValue(key, x, REG_DWORD);}
		 bool Registry::Write(std::string const & key, float x){return SetValue(key, x, REG_DWORD);}

		 bool Registry::Read(std::string const & key, double & x){return QueryValue(key, x, REG_QWORD);}
		 bool Registry::Write(std::string const & key, double x){return SetValue(key, x, REG_QWORD);}
		 bool Registry::Read(std::string const & key, std::string &s)
		 {
			std::size_t size;
			type type;
			// get length of string
			result error(QueryTypeAndSize(key, type, size));
			if(error != ERROR_SUCCESS) return false;
			if(type != REG_SZ) return false;
			// allocate a buffer
			char * buffer(new char[size]);
			error = ::RegQueryValueEx(current_group, key.c_str(), 0, &type, reinterpret_cast<unsigned char *>(buffer), reinterpret_cast<unsigned long int*>(&size));
			if(error == ERROR_SUCCESS) s = buffer;
			delete[] buffer;
			return (error == ERROR_SUCCESS);
		 }
		 bool Registry::Read(std::string const & key, char *buffer, std::size_t max_length)
		 {
			std::size_t size;
			type type;
			// get length of string
			result error(QueryTypeAndSize(key, type, size));
			if(error != ERROR_SUCCESS) return false;
			if(type != REG_SZ) return false;
			if(size > max_length) return false;
			error = ::RegQueryValueEx(current_group, key.c_str(), 0, &type, reinterpret_cast<unsigned char *>(buffer), reinterpret_cast<unsigned long int*>(&size));
			return (error == ERROR_SUCCESS);
		 }
		 bool Registry::Write(std::string const & key, std::string const &s)
		 {
			return ::RegSetValueEx(current_group, key.c_str(), 0, REG_SZ, reinterpret_cast<unsigned char const *>(s.c_str()), (DWORD)(s.length() + 1));
		 }

		bool Registry::ReadRaw(std::string const & key, void *data, std::size_t max_length)
		{
			type type_read = REG_BINARY;
			result const error(::RegQueryValueEx(current_group, key.c_str(), 0, &type_read, reinterpret_cast<unsigned char *>(data), reinterpret_cast<unsigned long int*>(&max_length)));
			return (error == ERROR_SUCCESS);
		}
		bool Registry::WriteRaw(std::string const & key, void *data, std::size_t max_length)
		{
			result const error(::RegSetValueEx(current_group, key.c_str(), 0, REG_BINARY , reinterpret_cast<unsigned char const *>(data), max_length));
			return (error == ERROR_SUCCESS);
		}

		long int
		Registry::QueryTypeAndSize(std::string const & name, type & type, std::size_t & size)
		{
			BOOST_STATIC_ASSERT((sizeof(long int) == sizeof(int))); // microsoft likes using unsigned long int, aka their DWORD typedef, instead of std::size_t
			type = REG_NONE;
			size = 0;
			return ::RegQueryValueEx(current_group, name.c_str(), 0, &type, 0, reinterpret_cast<unsigned long int*>(&size));
		}


		//*************************************************************
		//
		//  RegDelnodeRecurse()
		//
		//  Purpose:    Deletes a registry key and all its subkeys / values.
		//
		//  Parameters: hKeyRoot    -   Root key
		//              lpSubKey    -   SubKey to delete
		//
		//  Return:     TRUE if successful.
		//              FALSE if an error occurs.
		//
		//*************************************************************
#if 0
		BOOL RegDelnodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
		{
			LPTSTR lpEnd;
			LONG lResult;
			DWORD dwSize;
			TCHAR szName[MAX_PATH];
			HKEY hKey;
			FILETIME ftWrite;

			// First, see if we can delete the key without having
			// to recurse.

			lResult = RegDeleteKey(hKeyRoot, lpSubKey);

			if (lResult == ERROR_SUCCESS) 
				return TRUE;

			lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

			if (lResult != ERROR_SUCCESS) 
			{
				if (lResult == ERROR_FILE_NOT_FOUND) {
					printf("Key not found.\n");
					return TRUE;
				} 
				else {
					printf("Error opening key.\n");
					return FALSE;
				}
			}

			// Check for an ending slash and add one if it is missing.

			lpEnd = lpSubKey + lstrlen(lpSubKey);

			if (*(lpEnd - 1) != TEXT('\\')) 
			{
				*lpEnd =  TEXT('\\');
				lpEnd++;
				*lpEnd =  TEXT('\0');
			}

			// Enumerate the keys

			dwSize = MAX_PATH;
			lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
								   NULL, NULL, &ftWrite);

			if (lResult == ERROR_SUCCESS) 
			{
				do {

					StringCchCopy (lpEnd, MAX_PATH*2, szName);

					if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
						break;
					}

					dwSize = MAX_PATH;

					lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
										   NULL, NULL, &ftWrite);

				} while (lResult == ERROR_SUCCESS);
			}

			lpEnd--;
			*lpEnd = TEXT('\0');

			RegCloseKey (hKey);

			// Try again to delete the key.

			lResult = RegDeleteKey(hKeyRoot, lpSubKey);

			if (lResult == ERROR_SUCCESS) 
				return TRUE;

			return FALSE;
		}
		std::list<std::string> GetKeys()
		{
			DWORD nValues = 0;
			::RegQueryInfoKey(RegKey, 0, 0, 0, 0, 0, 0, &nValues, 0, 0, 0, 0);
			if(nValues)
			{
				::MENUITEMINFO hNewItemInfo;
				int iCount = 0;
				char cntBuff[3];
				DWORD cntSize = sizeof cntBuff;
				char nameBuff[1 << 10];
				DWORD nameSize = sizeof nameBuff;
				while
					(
					::RegEnumValue
					(
					RegKey,
					iCount,
					cntBuff,
					&cntSize,
					0,
					0,
					reinterpret_cast<unsigned char*>(nameBuff),
					&nameSize
					) == ERROR_SUCCESS
					)
				{
				}
			}
		}
#endif

		template<typename x>
		long int Registry::QueryValue(std::string const & name, x & data, type const & type_wanted)
		{
			BOOST_STATIC_ASSERT((sizeof(long int) == sizeof(int))); // microsoft likes using unsigned long int, aka their DWORD typedef, instead of std::size_t
			std::size_t size(sizeof data);
			type type_read;
			result const error(::RegQueryValueEx(current_group, name.c_str(), 0, &type_read, reinterpret_cast<unsigned char *>(&data), reinterpret_cast<unsigned long int*>(&size)));
			return error;
		}

		template<typename x>
		long int Registry::SetValue(std::string const & name, x const & data, type const & type)
		{
			result const error(::RegSetValueEx(current_group, name.c_str(), 0, type , reinterpret_cast<unsigned char const *>(&data), sizeof data));
			if(error != ERROR_SUCCESS) loggers::warning()("could not write " + name + "to registry.");
			return error;
		}

	}
}
