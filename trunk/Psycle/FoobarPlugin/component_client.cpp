#include "stdafx.h"
#include "../SDK/component.h"

static HINSTANCE g_hIns;
static string8 g_name;
static string8 g_full_path;

extern "C"
{
	__declspec(dllexport) foobar2000_client * _cdecl foobar2000_get_interface(foobar2000_api * p_api,HINSTANCE hIns)
	{
		cfg_var::config_on_app_init();
		g_hIns = hIns;
		g_api = p_api;

		TCHAR temp[MAX_PATH];
		GetModuleFileName(hIns,temp,MAX_PATH);
		g_full_path = "file://";
		g_full_path += string_utf8_from_os(temp);
		g_name = string_filename(g_full_path);

		return &g_client;
	}
}


namespace core_api
{

	HINSTANCE get_my_instance()
	{
		return g_hIns;
	}

	HWND get_main_window()
	{
		return g_api->get_main_window();
	}

	const char * get_my_file_name()
	{
		return g_name;
	}

	const char * get_my_full_path()
	{
		return g_full_path;
	}
}

#pragma comment(linker,"/opt:nowin98")