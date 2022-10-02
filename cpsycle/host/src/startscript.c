/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "startscript.h"
/* host */
#include "mainframe.h"
/* std */
#include <assert.h>
/* platform */
#include "../../detail/portable.h"

static const int CHILDVIEWPORT = 1;
static const int FRAMEVIEWPORT = 2;
static const int TOOLBARVIEWPORT = 3;

static const int MDI = 3;
static const int SDI = 4;

static const int CREATELAZY = 5;
static const int CREATEATSTART = 6;

/* Link */
/* implementation */
void link_init(Link* self)
{
	assert(self);

	self->label_ = NULL;
	self->dllname_ = NULL;
	self->viewport_ = CHILDVIEWPORT;
	self->userinterface_ = MDI;
}

void link_init_all(Link* self, const char* dllname,
	const char* label, int viewport, int userinterface)
{
	assert(self);

	self->dllname_ = psy_strdup(dllname);
	self->label_ = psy_strdup(label);
	self->viewport_ = viewport;
	self->userinterface_ = userinterface;
}

void link_dispose(Link* self)
{
	assert(self);

	free(self->label_);
	self->label_ = NULL;
	free(self->dllname_);
	self->dllname_ = NULL;
}

Link* link_clone(const Link* self)
{
	Link* rv;

	rv = (Link*)malloc(sizeof(Link));
	if (rv) {
		rv->dllname_ = psy_strdup(self->dllname_);
		rv->label_ = psy_strdup(self->label_);
		rv->viewport_ = self->viewport_;
		rv->userinterface_ = self->userinterface_;
	}
	return rv;
}

void links_init(Links* self)
{
	psy_table_init(&self->container);
}

void links_dispose(Links* self)
{
	psy_table_dispose_all(&self->container, (psy_fp_disposefunc)link_dispose);
}

void links_add(Links* self, const Link* link)
{
	psy_table_insert(&self->container, psy_table_size(&self->container),
		link_clone(link));
}

const Link* links_at(const Links* self, uintptr_t index)
{
	return (const Link*)psy_table_at_const(&self->container, index);
}


/* StartScript */
/* prototypes */
int startscript_addmenu(lua_State*);
int startscript_replacemenu(lua_State*);
int startscript_addextension(lua_State*);
int startscript_terminal_output(lua_State*);
int startscript_cursor_test(lua_State*);
int startscript_alert(lua_State*);
int startscript_confirm(lua_State*);
/* implementation */
void startscript_init(StartScript* self, MainFrame* mainframe)
{
	assert(self);
	
	psyclescript_init(&self->script);
	self->mainframe = mainframe;
}

void startscript_dispose(StartScript* self)
{
	assert(self);

	psyclescript_dispose(&self->script);
}

void startscript_prepare(StartScript* self)
{
  static const luaL_Reg methods[] = {    
    {"addmenu", startscript_addmenu},
    {"replacemenu", startscript_replacemenu},
    {"addextension", startscript_addextension},
    {"output", startscript_terminal_output},
	{"cursortest", startscript_cursor_test},
    {"alert", startscript_alert},
    {"confirm", startscript_confirm},	
    {NULL, NULL}
  };
  psyclescript_preparestate(&self->script, methods, self);
  lua_getglobal(self->script.L, "psycle"); 
  lua_pushinteger(self->script.L, CHILDVIEWPORT);
  lua_setfield(self->script.L, -2, "CHILDVIEWPORT");
  lua_pushinteger(self->script.L, FRAMEVIEWPORT);
  lua_setfield(self->script.L, -2, "FRAMEVIEWPORT");
  lua_pushinteger(self->script.L, TOOLBARVIEWPORT);
  lua_setfield(self->script.L, -2, "TOOLBARVIEWPORT");
  lua_pushinteger(self->script.L, MDI);
  lua_setfield(self->script.L, -2, "MDI");
  lua_pushinteger(self->script.L, SDI);
  lua_setfield(self->script.L, -2, "SDI");
  lua_pushinteger(self->script.L, CREATELAZY);
  lua_setfield(self->script.L, -2, "CREATELAZY");
  lua_pushinteger(self->script.L, CREATEATSTART);
  lua_setfield(self->script.L, -2, "CREATEATSTART");
  lua_pop(self->script.L, 1);
}

void startscript_run(StartScript* self)
{
	char path[4096];
	int err;

	psy_snprintf(path, 4096, "%s/%s", dirconfig_lua_scripts(
		&self->mainframe->workspace.config.directories), "start.lua");
	if (err = psyclescript_load(&self->script, path)) {
		return;	
	}
	startscript_prepare(self);
	if (err = psyclescript_run(&self->script)) {
		return;	
	}	
}

int startscript_addmenu(lua_State* L)
{  
	StartScript* self;
	Link link;
	const char* name;

	self = (StartScript*)psyclescript_host(L);
	name = luaL_checkstring(L, 1);
	/* parse link table */
	lua_getfield(L, 2, "plugin");
	lua_getfield(L, 2, "label");    
	lua_getfield(L, 2, "viewport");
	lua_getfield(L, 2, "userinterface");  
		
	link_init_all(&link,
		luaL_checkstring(L, -4),
		luaL_checkstring(L, -3),
		(int)luaL_checkinteger(L, -2),
		(int)luaL_checkinteger(L, -1));
	mainviews_add_link(&self->mainframe->mainviews, &link);	
	link_dispose(&link);
	return 0;
}

int startscript_replacemenu(lua_State* L)
{
  return 0;
}

int startscript_addextension(lua_State* L)
{
  return 0;
}

int startscript_terminal_output(lua_State* L)
{
	return 0;
}

int startscript_cursor_test(lua_State* L)
{
	return 0;
}

int startscript_alert(lua_State* L)
{
	return 0;
}

int startscript_confirm(lua_State* L)
{
	return 0;
}
