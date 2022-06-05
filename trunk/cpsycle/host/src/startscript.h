/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(STARTSCRIPT_H)
#define STARTSCRIPT_H

/* host */
#include "workspace.h"
/* audio */
#include <psyclescript.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct Link {
  /*public:
   Link() : viewport_(CHILDVIEWPORT), user_interface_(MDI) {}
   Link(const std::string& dll_name, const std::string& label, int viewport, int user_interface) : 
      dll_name_(dll_name),
      label_(label),
      viewport_(viewport),
      user_interface_(user_interface) {
   }

   const std::string& dll_name() const { return dll_name_; }
   const std::string& label() const { return label_; }
   const int viewport() const { return viewport_; }
   const int user_interface() const { return user_interface_; }

   boost::weak_ptr<LuaPlugin> plugin;*/
   
 /* internal */
   char* label_;
   char* dllname_;
   int viewport_;
   int userinterface_;
} Link;

void link_init(Link*);
void link_init_all(Link*, const char* dllname, const char* label,
	int viewport, int user_interface);
void link_dispose(Link*);

Link* link_clone(const Link*);

struct MainFrame;

typedef struct StartScript {	
	psy_PsycleScript script;
	/* references */
	struct MainFrame* mainframe;
} StartScript;

void startscript_init(StartScript*, struct MainFrame* mainframe);
void startscript_dispose(StartScript*);

void startscript_run(StartScript*);

#ifdef __cplusplus
}
#endif

#endif /* STARTSCRIPT_H */
