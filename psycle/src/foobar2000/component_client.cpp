/* -*- mode:c++, indent-tabs-mode:t -*- */
#include "component.h"
#pragma comment(linker,"/opt:nowin98")

static HINSTANCE g_hIns;

#ifndef FOOBAR2000_COMPONENT_CLIENT_LITE
static string_simple g_name,g_full_path;
#endif

static bool g_services_available = false;

extern "C"
{
	__declspec(dllexport) foobar2000_client * _cdecl foobar2000_get_interface(foobar2000_api * p_api,HINSTANCE hIns)
	{
		cfg_var::config_on_app_init();
		g_hIns = hIns;
		g_api = p_api;

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
#ifndef FOOBAR2000_COMPONENT_CLIENT_LITE
	const char * get_my_file_name()
	{
		return g_name;
	}

	const char * get_my_full_path()
	{
		return g_full_path;
	}

#endif
	bool are_services_available()
	{
		return g_services_available;
	}
	bool assert_main_thread()
	{
		return (g_services_available && g_api) ? g_api->assert_main_thread() : true;
	}

	bool is_main_thread()
	{
		return (g_services_available && g_api) ? g_api->is_main_thread() : true;
	}
}

void foobar2000_client::set_library_path(const char * path,const char * name)
{
#ifndef FOOBAR2000_COMPONENT_CLIENT_LITE
	g_full_path = path;
	g_name = name;
#endif
}

void foobar2000_client::services_init(bool val)
{
	g_services_available = val;
}
