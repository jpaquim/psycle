#pragma once
#include "JBridgeEnabler.hpp"

// Name of the proxy DLL to load
#define JBRIDGE_PROXY_REGKEY        TEXT("Software\\JBridge")

#ifdef _WIN64
#define JBRIDGE_PROXY_REGVAL        TEXT("Proxy64")  //use this for x64 builds
#else
#define JBRIDGE_PROXY_REGVAL        TEXT("Proxy32")  //use this for x86 builds
#endif

void JBridge::getJBridgeLibrary(char szProxyPath[]) {
	// Get path to JBridge proxy
	szProxyPath[0]='\0' ;
	HKEY hKey;
	if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, JBRIDGE_PROXY_REGKEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
	{
		DWORD dw=sizeof(szProxyPath);
		RegQueryValueEx(hKey, JBRIDGE_PROXY_REGVAL, NULL, NULL, (LPBYTE)szProxyPath, &dw);
		RegCloseKey(hKey);
	}
}

PFNBRIDGEMAIN JBridge::getBridgeMainEntry(HMODULE hModuleProxy)
{
	return (PFNBRIDGEMAIN)GetProcAddress(hModuleProxy, "BridgeMain");
}
