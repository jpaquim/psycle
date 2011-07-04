// Main.cpp : DLL エクスポートの実装です。

#include "stdafx.h"
#include "resourceobject.h"


// このモジュール属性は、DllMain、DllRegisterServer および DllUnregisterServer を自動的に実装します。
[ module(dll, uuid = "{79CF997C-9828-4C28-AEE8-68AAA4442513}", 
		 name = "Main", 
		 helpstring = "Main 1.0 タイプ ライブラリ",
		 resource_name = "IDR_MAIN") ]
class CMainModule
{
public:
// CAtlDllModuleT メンバのオーバーライド
};	 
