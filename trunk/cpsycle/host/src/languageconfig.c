/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "languageconfig.h"
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"
#include "../../detail/os.h"

#ifdef PSYCLE_DEFAULT_LANG_USER
#ifdef DIVERSALIS__OS__MICROSOFT 
#include <Windows.h>
#endif
#endif

static void languageconfig_makelanguagechoice(LanguageConfig*);
static void languageconfig_makelanguagelist(LanguageConfig*);
static int languageconfig_enumlanguagedir(LanguageConfig*, const char* path,
	int flag);
static void languageconfig_setdefaultlanguage(LanguageConfig*);
static const char* languageconfig_defaultlanguagekey(LanguageConfig*);
static const char* languageconfig_chooselang(LanguageConfig* self, int id);

void languageconfig_init(LanguageConfig* self, psy_Property* parent,
	psy_Translator* translator)
{
	assert(self && parent && translator);

	self->parent = parent;
	self->translator = translator;
	psy_signal_init(&self->signal_changed);
	languageconfig_makelanguagechoice(self);
	languageconfig_makelanguagelist(self);
	languageconfig_setdefaultlanguage(self);
}

void languageconfig_dispose(LanguageConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void languageconfig_makelanguagechoice(LanguageConfig* self)
{
	self->languagechoice = psy_property_sethint(
		psy_property_append_choice(self->parent, "lang", 0),
		PSY_PROPERTY_HINT_LIST);
	psy_property_setid(self->languagechoice, PROPERTY_ID_LANG);
	psy_property_settext(self->languagechoice, "settingsview.global.language");
}

void languageconfig_makelanguagelist(LanguageConfig* self)
{
	char currdir[4096];
	
	assert(self);
	
	if (psy_workdir(currdir)) {
		psy_dir_enumerate(self, currdir, "*.ini", 0, (psy_fp_findfile)
			languageconfig_enumlanguagedir);
	}
}

int languageconfig_enumlanguagedir(LanguageConfig* self, const char* path,
	int flag)
{
	char lang[256];

	assert(self);

	if (psy_translator_test(self->translator, path, lang)) {
		psy_property_settext(psy_property_append_str(
			self->languagechoice, lang, path), lang);
	}
	return TRUE;
}

void languageconfig_setdefaultlanguage(LanguageConfig* self)
{
	psy_Property* defaultlang;

	if (defaultlang = psy_property_find(self->languagechoice,
			languageconfig_defaultlanguagekey(self),
			PSY_PROPERTY_TYPE_NONE)) {	
		uintptr_t index;

		index = psy_property_index(defaultlang);
		if (index != psy_INDEX_INVALID) {
			psy_property_setitem_int(self->languagechoice, index);
		}
	}
}

const char* languageconfig_defaultlanguagekey(LanguageConfig* self)
{
#ifdef DIVERSALIS__OS__MICROSOFT 
	#if defined PSYCLE_DEFAULT_LANG_USER
	LANGID langid;
	
	langid = GetUserDefaultUILanguage() & 0xFFFF;
	return languageconfig_chooselang(self, langid);
	#endif
#endif	
#if defined PSYCLE_DEFAULT_LANG_ES
		"es";
#elif defined PSYCLE_DEFAULT_LANG_EN
		"en";
#else
		"es";
#endif
}

const char* languageconfig_chooselang(LanguageConfig* self, int id)
{
	/*
	** map some countries to english and the rest to spanish
	** lang ids from https://www.codeproject.com/Articles/11040/Multiple-language-support-for-MFC-applications-wit
	** (Herbert Yu)
	*/
	switch (id) {
	case 0x043b:	/* SME	SME	Sami, Northern(Norway) */
	case 0x083b:	/* SMF	SME	Sami, Northern(Sweden) */
	case 0x0c3b:	/* SMG	SME	Sami, Northern(Finland) */
	case 0x103b:	/* SMJ	SME	Sami, Lule(Norway) */
	case 0x143b:	/* SMK	SME	Sami, Lule(Sweden) */
	case 0x183b:	/* SMA	SME	Sami, Southern(Norway) */
	case 0x1c3b:	/* SMB	SME	Sami, Southern(Sweden) */
	case 0x203b:	/* SMS	SME	Sami, Skolt(Finland) */
	case 0x243b:	/* SMN	SME	Sami, Inari(Finland) */
	case 0x0414:	/* NOR	NOR	Norwegian(Bokmal) */
	case 0x0814:	/* NON	NOR	Norwegian(Nynorsk) */
	case 0x0415: 	/* PLK	PLK	Polish */
	case 0x0406:	/* DAN	DAN	Danish */
	case 0x0425:	/* ETI	ETI	Estonian */
	case 0x0438:	/* FOS	FOS	Faeroese */
	case 0x040b:	/* FIN	FIN	Finnish */
	case 0x041d:	/* SVE	SVE	Swedish */
	case 0x081d:	/* SVF	SVE	Swedish(Finland) */
	case 0x0413:	/* NLD	NLD	Dutch(Netherlands) */
	case 0x0813:	/* NLB	NLD	Dutch(Belgium) */
	case 0x0407:	/* DEU	DEU	German(Standard) */
	case 0x0807:	/* DES	DEU	German(Switzerland) */
	case 0x0c07:	/* DEA	DEU	German(Austria) */
	case 0x1007:	/* DEL	DEU	German(Luxembourg) */
	case 0x1407:	/* DEC	DEU	German(Liechtenstein) */
	case 0x041f:	/* TRK	TRK	Turkish */
	case 0x044c:	/* MYM	MYM	Malayalam(India) */
	case 0x0481:	/* MRI	MRI	Maori(New Zealand) */
	case 0x043a:	/* MLT	MLT	Maltese(Malta) */
	case 0x0452:	/* CYM	CYM	Welsh(United Kingdom) */
	case 0x0409:	/* ENU	ENU	English(United States) */
	case 0x0809:	/* ENG	ENU	English(United Kingdom) */
	case 0x0c09:	/* ENA	ENU	English(Australian) */
	case 0x1009:	/* ENC	ENU	English(Canadian) */
	case 0x1409:	/* ENZ	ENU	English(New Zealand) */
	case 0x1809:	/* ENI	ENU	English(Ireland) */
	case 0x1c09:	/* ENS	ENU	English(South Africa) */
	case 0x2009:	/* ENJ	ENU	English(Jamaica) */
	case 0x2409:	/* ENB	ENU	English(Caribbean) */
	case 0x2809:	/* ENL	ENU	English(Belize) */
	case 0x2c09:	/* ENT	ENU	English(Trinidad) */
	case 0x3009:	/* ENW	ENU	Windows 98 / ME, Windows 2000 / XP: English(Zimbabwe) */
	case 0x3409:	/* ENP	ENU	Windows 98 / ME, Windows 2000 / XP : English(Philippines) */
		return "en";		
	default:
		break;
	}
	return "es";
}

void languageconfig_update_language(LanguageConfig* self)
{
	psy_Property* lang;

	assert(self);

	if (lang = psy_property_at_choice(self->languagechoice)) {
		/*
		** This updates also the ui components over the translator language
		** changed signal the ui is connected to.
		*/
		psy_translator_load(self->translator, psy_property_item_str(lang));
	}
}

uintptr_t languageconfig_on_changed(LanguageConfig* self,
	psy_Property* property)
{		
	assert(self);
	assert(property);
	
	if (psy_property_hasid(property, PROPERTY_ID_LANG)) {
		languageconfig_update_language(self);
		psy_signal_emit(&self->signal_changed, self, 1, property);		
	}
	return 0;
}

bool languageconfig_has_property(const LanguageConfig* self,
	psy_Property* property)
{
	assert(self && self->languagechoice);

	return (property == self->languagechoice) || psy_property_in_section(property, self->languagechoice);
}
