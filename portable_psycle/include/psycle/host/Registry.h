#pragma once
///\file
///\brief implementation file for psycle::host::Registry.
namespace psycle
{
	namespace host
	{
		/// the evil microsoft windows registry.
		class Registry
		{
		public:
			Registry();
			~Registry();
			LONG OpenRootKey(HKEY key, LPTSTR psName);
			LONG CloseRootKey();
			LONG OpenKey(LPTSTR psName);
			LONG CloseKey();
			LONG CreateKey(LPTSTR psName);
			LONG QueryValue(LPTSTR psName, LPDWORD pType, LPBYTE pData, LPDWORD pNumData);
			LONG SetValue(LPTSTR psName, DWORD type, LPBYTE pData, DWORD numData);
			LONG DeleteValue(LPTSTR psName);
		protected:
			HKEY _root;
			HKEY _key;
		};
	}
}
