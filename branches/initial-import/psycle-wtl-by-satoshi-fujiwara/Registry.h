#ifndef _REGISTRY_H
#define _REGISTRY_H

/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
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

#endif