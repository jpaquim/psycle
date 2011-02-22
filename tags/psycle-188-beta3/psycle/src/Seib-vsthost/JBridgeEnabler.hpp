#pragma once
/// Tell the SDK that we want to support all the VST specs, not only VST2.4
#define VST_FORCE_DEPRECATED 0
#include <vst2.x/AEffectx.h>               /* VST header files                  */

// Typedef for BridgeMain proc
typedef AEffect * (*PFNBRIDGEMAIN)( audioMasterCallback audiomaster, char * pszPluginPath );

//*******************************
class JBridge {
public:
	static void getJBridgeLibrary(char szProxyPath[]);
	static PFNBRIDGEMAIN getBridgeMainEntry(HMODULE hModuleProxy);
};
