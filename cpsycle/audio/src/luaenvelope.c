/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2007-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "luaenvelope.h"

#include "array.h"
#include "luaarray.h"

#include "psyclescript.h"

#include <hashtbl.h>

#include <stdlib.h>
#include <math.h>


typedef struct LEnvelope {
	psy_audio_Array out_;
    int fs_, sc_, sus_, stage_, nexttime_;
    double m_, lv_, startpeak_;
    bool susdone_ ,up_;
    psy_Table times_;
    psy_Table peaks_;
    psy_Table types_;    
} LEnvelope;

static void lenvelope_calcstage(LEnvelope*, double peakStart, double peakEnd,
	double time, int type);

void lenvelope_init(LEnvelope* self,
	psy_Table* times,
    psy_Table* peaks,
    psy_Table* types,    
	int suspos,
	double startpeak,
	int samplerate)
{
	psy_TableIterator it;	
	assert(self);

	psy_audio_array_init_len(&self->out_, 256, 0.0);
	psy_table_init(&self->times_);
    psy_table_init(&self->peaks_);
    psy_table_init(&self->types_);
	for (it = psy_table_begin(times);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		psy_table_insert(&self->times_, psy_tableiterator_key(&it),
			psy_tableiterator_value(&it));
	}
	for (it = psy_table_begin(peaks);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_table_insert(&self->peaks_, psy_tableiterator_key(&it),
			psy_tableiterator_value(&it));
	}
	for (it = psy_table_begin(types);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_table_insert(&self->types_, psy_tableiterator_key(&it),
			psy_tableiterator_value(&it));
	}	
	self->sus_ = suspos;
	self->startpeak_ = startpeak;
	self->lv_ = startpeak;
	self->fs_ = samplerate;
	self->up_ = FALSE;
	self->stage_ = psy_table_size(&self->peaks_);
}

void lenvelope_dispose(LEnvelope* self)
{
	psy_audio_array_dispose(&self->out_);
	psy_table_dispose_all(&self->times_, NULL);
    psy_table_dispose_all(&self->peaks_,NULL);
    psy_table_dispose_all(&self->types_, NULL);
}

void lenvelope_setstartvalue(LEnvelope* self, float val)
{
	self->startpeak_ = val;
}

float lenvelope_lastvalue(const LEnvelope* self)
{
	return self->lv_;
}

void lenvelope_start(LEnvelope* self)
{
	self->lv_ = self->startpeak_;
	self->stage_ = 0;
	self->up_ = false;	
	lenvelope_calcstage(self, self->startpeak_,
		*((double*)psy_table_at(&self->peaks_, 0)),
		*((double*)psy_table_at(&self->times_, 0)),
		*((lua_Integer*)psy_table_at(&self->types_, 0)));
	self->susdone_ = false;
}

bool lenvelope_is_playing(const LEnvelope* self)
{
	return !(self->stage_ > psy_table_size(&self->peaks_) - 1);
}
    
psy_audio_Array* lenvelope_out(LEnvelope* self)
{	
	return &self->out_;
}

double lenvelope_peak(const LEnvelope* self, int stage)
{
	const double* val;
		
	val = (const double*)psy_table_at_const(&self->peaks_, stage);
	assert(val);
	return (*val);
}

double lenvelope_time(const LEnvelope* self, int stage)
{
	const double* val;
		
	val = (const double*)psy_table_at_const(&self->times_, stage);
	assert(val);
	return (*val);
}


int lenvelope_type(const LEnvelope* self, int stage)
{
	const int* val;
		
	val = (const int*)psy_table_at_const(&self->types_, stage);
	assert(val);
	return (*val);
}

int lenvelope_stage(const LEnvelope* self)
{
	return self->stage_;
}

void lenvelope_set_samplerate(LEnvelope* self, int sr)
{
	int newsc;
	
	newsc = self->sc_ * (int)(sr/(double)self->fs_);
	lenvelope_calcstage(self,
		(self->stage_ == 0)
		? self->startpeak_
		: lenvelope_peak(self, self->stage_ - 1),
		lenvelope_peak(self, self->stage_),
		lenvelope_time(self, self->stage_),
		0);
	self->sc_ = newsc;
	self->fs_ = sr;
}

void lenvelope_calcstage(LEnvelope* self, double peak_start, double peak_end,
	double time, int type)
{
	self->nexttime_ = time * self->fs_;
	self->lv_ = peak_start;
	self->sc_ = 0;      
	if (type == 1) {
		if (peak_start == 0) {
			peak_start = 0.001;
		}
		if (peak_end == 0) {
			peak_end = 0.001;
		}
		self->lv_ = peak_start;
		//up_ = peakEnd > peakStart;
		//if (up_) {
		// m_ = 1.0 + (log(peakStart) - log(peakEnd)) / (nexttime_);
		// lv_ = peakEnd;
		//} else {
		// m_ = 1.0 + (log(peakEnd) - log(peakStart)) / (nexttime_);
		//   lv_ = peakStart;
		//}
		//lv_ = peakStart;
		//if (peakStart < peakEnd) {
		self->m_ = 1.0 + (log(peak_end) - log(peak_start)) / (self->nexttime_);
		//} else {
		//m_ = 1.0 + (log(peakEnd) - log(peakStart)) / (nexttime_);
		//}
	} else {
		self->m_ = (peak_end - peak_start) / self->nexttime_;
	}
}

void lenvelope_work(LEnvelope* self, int num)
{
	float* data;
	int k;
	
	psy_audio_array_resize(&self->out_, num);
	if (!lenvelope_is_playing(self) || (self->stage_ == self->sus_ &&
			!self->susdone_)) {
		psy_audio_array_fill(&self->out_, self->lv_);		
		return;
	}
	data = psy_audio_array_data(&self->out_);
	for (k = 0; k < num; ++k) {
		if (self->nexttime_ == self->sc_) {
			uintptr_t peak_size;
			
			++self->stage_;
			peak_size = psy_table_size(&self->peaks_);
			if ((self->stage_ > peak_size - 1) || (!self->susdone_ &&
					self->stage_ == self->sus_)) {
				int j;
				
				self->lv_ = lenvelope_peak(self, self->stage_ - 1);
				self->m_ = 0;
				for (j = k; j < num; ++j) {
					psy_audio_array_set(&self->out_, j, self->lv_);
				}
				break;
			}
			lenvelope_calcstage(self,		
				lenvelope_peak(self, self->stage_ - 1),
				lenvelope_peak(self, self->stage_),
				lenvelope_time(self, self->stage_),
				lenvelope_type(self, self->stage_));
		}
		if (lenvelope_type(self, self->stage_) == 0) {
			*(data++) = self->lv_;			
			self->lv_ += self->m_;			
		} else {
			// *(data++) = (up_) ? peaks_[stage_]-lv_ : lv_;
			*(data++) = self->lv_;			
			self->lv_ *= self->m_;			
		}
		++self->sc_;
	}	
}

void lenvelope_release(LEnvelope* self)
{
  if (lenvelope_is_playing(self)) {
    // int old = stage_;
    self->susdone_ = true;
    if (self->stage_ != self->sus_) {
		uintptr_t peak_size;
						
		peak_size = psy_table_size(&self->peaks_);
		self->stage_ = peak_size - 1;
    }
    //calcstage((up_) ?  peaks_[old]-lv_
    //	            : lv_, peaks_[stage_], times_[stage_], types_[stage_]);
    
    lenvelope_calcstage(self, self->lv_,	
		*(double*)psy_table_at(&self->peaks_, self->stage_),				
		*(double*)psy_table_at(&self->times_, self->stage_),
		*(lua_Integer*)(psy_table_at(&self->types_, self->stage_)));        
  }
}

static const char* luaenvelope_meta = "psyenvelopemeta";
static int luaenvelope_create(lua_State* L);
static int luaenvelope_gc(lua_State* L);
static int work(lua_State* L);
static int release(lua_State* L);
static int start(lua_State* L);
static int isplaying(lua_State* L);
static int setpeak(lua_State* L);
static int peak(lua_State* L);
static int tostring(lua_State* L);
static int setstagetime(lua_State* L);
static int time(lua_State* L);
static int setstartvalue(lua_State* L);
static int lastvalue(lua_State* L);

// lua machine bind
int psy_audio_luabind_envelope_open(lua_State *L)
{
	static const luaL_Reg methods[] = {
		{"new", luaenvelope_create},
		{"work", work },
		{"release", release},
		{"isplaying", isplaying},
		{"start", start },
		{"setpeak", setpeak},
		{"setstartvalue", setstartvalue},
		{"lastvalue", lastvalue},
		{"peak", peak},
		{"settime", setstagetime},
		{"time", time},
		{"tostring", tostring},
		{NULL, NULL}
	};
	psyclescript_open(L, luaenvelope_meta, methods, 
		luaenvelope_gc, 0);  
	return 1;
}

int luaenvelope_create(lua_State* L)
{
	int n;
	int self = 1;
	LEnvelope* env;
	psy_Table times;
	psy_Table peaks;
	psy_Table types; 
		
	lua_Integer suspos;
	lua_Number startpeak;		
	psy_table_init(&times);
	psy_table_init(&peaks);
	psy_table_init(&types);

	n = lua_gettop(L);  // Number of arguments
	if (n != 3) {
		return luaL_error(L,
			"Got %d arguments expected 3 (self, points, sustainpos)", n);
	}
	env = (LEnvelope*)malloc(sizeof(LEnvelope));
	if (!env) {
		return luaL_error(L, "Memory Error");
	}	
	if (lua_istable(L, 2)) {
		
		lua_Unsigned len;
		size_t i;
		
		len = lua_rawlen(L, 2);
		for (i = 1; i <= len; ++i) {
			double* t;
			double* peak;			
			lua_Unsigned argnum;

			lua_rawgeti(L, 2, i); // get point {time, peak}
			argnum = lua_rawlen(L, 4);
			lua_rawgeti(L, 4, 1); // get time
			t = malloc(sizeof(double));
			*t = luaL_checknumber(L, 5);			
			psy_table_insert(&times, i - 1, (void*)t);
			lua_pop(L, 1);
			lua_rawgeti(L, 4, 2); // get peak
			peak = malloc(sizeof(double));
			*peak = luaL_checknumber(L, 5);				
			psy_table_insert(&peaks, i - 1, (void*)peak);
			lua_pop(L, 1);
			if (argnum == 3) {
				lua_rawgeti(L, 4, 3); // get type
				if (!lua_isnil(L, 5)) {
					lua_Integer* type;
					
					type = malloc(sizeof(lua_Integer));
					*type = luaL_checkinteger(L, 5);
					psy_table_insert(&types, i - 1, (void*)type);
				} else {
					lua_Integer* type;
					
					type = malloc(sizeof(lua_Integer));
					*type = 0;
					psy_table_insert(&types, i - 1, (void*)type);
				}
				lua_pop(L, 1);
			} else {
				lua_Integer* type;
					
				type = malloc(sizeof(lua_Integer));
				*type = 0;
				psy_table_insert(&types, i - 1, (void*)type);
			}			
			lua_pop(L, 1);
		}
		
	}
	suspos = luaL_checkinteger(L, 3) - 1;
	startpeak = 0;
	if (n == 4) {
		startpeak = luaL_checknumber(L, 4);
	}
	lenvelope_init(env, &times, &peaks, &types, suspos, startpeak, 44100);
	psy_table_dispose(&times);
	psy_table_dispose(&peaks);
	psy_table_dispose(&types);
	psyclescript_createuserdata(L, 1, luaenvelope_meta, env);	
	return 1;
}

int luaenvelope_gc(lua_State* L)
{
	LEnvelope** ud = (LEnvelope**)
		luaL_checkudata(L, 1, luaenvelope_meta);	
	lenvelope_dispose(*ud);	
	free(*ud);
	return 0;
}

int work(lua_State* L)
{
	int n = lua_gettop(L);
	if (n == 2) {		
		LEnvelope* self;
		uintptr_t i;
		uintptr_t num;
		psy_audio_Array** rv;

		self = psyclescript_checkself(L, 1, luaenvelope_meta);
		num = (uintptr_t)luaL_checkinteger(L, 2);
		lenvelope_work(self, num);
		rv = (psy_audio_Array**)lua_newuserdata(L, sizeof(psy_audio_Array*));
		*rv = malloc(sizeof(psy_audio_Array));
		luaL_setmetatable(L, luaarraybind_meta);
		psy_audio_array_init_shared(*rv, psy_audio_array_data(
			lenvelope_out(self)), num);
	} else {
		return luaL_error(L, "Got %d arguments expected 2 (self, num)", n);
	}
	return 1;
}

int release(lua_State* L)
{
	LEnvelope* self;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	lenvelope_release(self);
	return psyclescript_chaining(L);
}

int start(lua_State* L)
{
	LEnvelope* self;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	lenvelope_start(self);
	return psyclescript_chaining(L);
}

int isplaying(lua_State* L)
{
	LEnvelope* self;
	bool playing;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	playing = lenvelope_is_playing(self);
	lua_pushboolean(L, playing);
	return 1;
}

int setpeak(lua_State* L)
{
	LEnvelope* self;	
	lua_Integer stage;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	stage = luaL_checkinteger(L, 2);
	if (stage > 0) {
		lua_Number* peak;
		
		peak = (lua_Number*)psy_table_at(&self->peaks_, stage - 1);
		if (!peak) {
			peak = malloc(sizeof(double));
		}
		*peak = luaL_checknumber(L, 3);		
		psy_table_insert(&self->peaks_, stage - 1, peak);		
	}
	return psyclescript_chaining(L);
}

int peak(lua_State* L)
{	
	LEnvelope* self;
	lua_Number idx;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	idx = luaL_checknumber(L, 2) - 1;
	lua_pushnumber(L, lenvelope_peak(self, idx - 1));
	return 1;
}

int tostring(lua_State* L)
{
	lua_pushfstring(L, "env");
	return 1;
}

int setstagetime(lua_State* L)
{
	LEnvelope* self;	
	lua_Integer stage;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	stage = luaL_checkinteger(L, 2);
	if (stage > 0) {
		lua_Number* t;
		
		t = (lua_Number*)psy_table_at(&self->times_, stage - 1);
		if (!t) {
			t = malloc(sizeof(lua_Number));
		}
		*t = luaL_checknumber(L, 3);
		psy_table_insert(&self->times_, stage - 1, t);		
	}	
	return psyclescript_chaining(L);
}

int time(lua_State* L)
{
	LEnvelope* self;
	lua_Number idx;

	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	idx = luaL_checknumber(L, 2) - 1;
	lua_pushnumber(L, lenvelope_time(self, idx - 1));
	return 1;
}

int setstartvalue(lua_State* L)
{
	LEnvelope* self;		
	lua_Number value;
	
	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	value = luaL_checknumber(L, 2);
	lenvelope_setstartvalue(self, value);
	return psyclescript_chaining(L);
}

int lastvalue(lua_State* L)
{
	LEnvelope* self;	
	
	self = psyclescript_checkself(L, 1, luaenvelope_meta);
	lua_pushnumber(L, lenvelope_lastvalue(self));
	return 1;
}
