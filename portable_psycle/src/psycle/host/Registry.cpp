#include "stdafx.h"
#include "Registry.h"
///\file
///\brief implementation file for psycle::host::Registry.
namespace psycle
{
	namespace host
	{
		Registry::Registry()
		{
			_root = (HKEY)0;
			_key = (HKEY)0;
		}

		Registry::~Registry()
		{
			if (_key != (HKEY)0)
			{
				RegCloseKey(_key);
			}
			if (_root != (HKEY)0)
			{
				RegCloseKey(_root);
			}
		}

		LONG
		Registry::OpenRootKey(HKEY key, LPTSTR psName)
		{
			LONG result = RegOpenKeyEx(key, psName, 0, KEY_ALL_ACCESS, &_root);
			if (result != ERROR_SUCCESS)
			{
				if (result == ERROR_FILE_NOT_FOUND)
				{
					result = RegCreateKey(key, psName, &_root);
				}
			}
			return result;
		}

		LONG
		Registry::CloseRootKey()
		{
			LONG result = RegCloseKey(_root);
			if (result == ERROR_SUCCESS)
			{
				_root = (HKEY)0;
			}
			return result;
		}

		LONG
		Registry::OpenKey(LPTSTR psName)
		{
			return RegOpenKeyEx(_root, psName, 0, KEY_ALL_ACCESS, &_key);
		}

		LONG
		Registry::CloseKey()
		{
			LONG result = RegCloseKey(_key);
			if (result == ERROR_SUCCESS)
			{
				_key = (HKEY)0;
			}
			return result;
		}

		LONG
		Registry::CreateKey(LPTSTR psName)
		{
			return RegCreateKey(_root, psName, &_key);
		}

		LONG
		Registry::QueryValue(LPTSTR psName, LPDWORD pType, LPBYTE pData, LPDWORD pNumData)
		{
			return RegQueryValueEx(_key, psName, 0, pType, pData, pNumData);
		}

		LONG
		Registry::SetValue(LPTSTR psName, DWORD type, LPBYTE pData, DWORD numData)
		{
			return RegSetValueEx(_key, psName, 0, type, pData, numData);
		}

		LONG
		Registry::DeleteValue(LPTSTR psName)
		{
			return RegDeleteValue(_key, psName);
		}
	}
}
