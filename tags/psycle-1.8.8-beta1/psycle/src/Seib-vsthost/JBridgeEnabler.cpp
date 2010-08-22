#pragma once
#include "JBridgeEnabler.hpp"

// Name of the proxy DLL to load
#define JBRIDGE_PROXY_REGKEY        "Software\\JBridge"

#ifdef _WIN64
#define JBRIDGE_PROXY_REGVAL        "Proxy64"  //use this for x64 builds
#else
#define JBRIDGE_PROXY_REGVAL        "Proxy32"  //use this for x86 builds
#endif

void JBridge::getJBridgeLibrary(char szProxyPath[]) {
	// Get path to JBridge proxy
	szProxyPath[0]='\0' ;
	HKEY hKey;
	if ( RegOpenKey(HKEY_LOCAL_MACHINE, JBRIDGE_PROXY_REGKEY, &hKey) == ERROR_SUCCESS )
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
