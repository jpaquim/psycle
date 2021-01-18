// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef psy_ui_win_IMPFACTORY_H
#define psy_ui_win_IMPFACTORY_H

#include "../../uiimpfactory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_win_ImpFactory {
	psy_ui_ImpFactory imp;
} psy_ui_win_ImpFactory;

void psy_ui_win_impfactory_init(psy_ui_win_ImpFactory*);
psy_ui_win_ImpFactory* psy_ui_win_impfactory_alloc(void);
psy_ui_win_ImpFactory* psy_ui_win_impfactory_allocinit(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_IMPFACTORY_H */
#endif /* PSYCLE_TK_WIN32 */
