/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2007-2022 members of the psycle project http://psycle.sourceforge.net
*/
#include "../../detail/prefix.h"


#include "luawavedata.h"

#include "waveosc.h"
#include <stdlib.h>

#include <lauxlib.h>
#include <lualib.h>

#include "psyclescript.h"
#include "luaarray.h"

#include "../../detail/stdint.h"
#include "../../detail/portable.h"

static const char* luawavedata_meta = "psywavedatameta";

static int create(lua_State*);
static int copy(lua_State*);
static int set_wave_sample_rate(lua_State*);
static int set_wave_tune(lua_State*);
static int set_wave_fine_tune(lua_State*);
static int set_loop(lua_State*);
static int set_bank(lua_State*);
static int gc(lua_State*);

int psy_audio_luabind_wavedata_open(lua_State *L)
{
    static const char* const e[] = { "DO_NOT", "NORMAL", "BIDI" };	
    static const luaL_Reg methods[] = {
    {"new", create},
    {"copy", copy},
    {"setwavesamplerate", set_wave_sample_rate},
    {"setwavetune", set_wave_tune},
    {"setwavefinetune", set_wave_fine_tune},
    {"setloop", set_loop},
    {"copytobank", set_bank},
    { NULL, NULL }
    };

    psyclescript_open(L, luawavedata_meta, methods, gc, 0);
    psyclescript_buildenum(L, e, sizeof(e) / sizeof(e[0]), 1);
    return 1;  
}

int create(lua_State* L)
{
    int n;
    psy_audio_Sample* sample;
    
    n = lua_gettop(L);  /* Number of arguments */
    if (n != 1) {
        return luaL_error(L, "Got %d arguments expected 2 (self)", n);
    }
    sample = psy_audio_sample_allocinit(2);    
    psyclescript_createuserdata(L, 1, luawavedata_meta, sample);    
    return 1;
}

int gc(lua_State* L)
{
    psy_audio_Sample** ud = (psy_audio_Sample**)
        luaL_checkudata(L, 1, luawavedata_meta);
    psy_audio_sample_deallocate(*ud);    
    return 0;    
}

int copy(lua_State* L) {
    int n;
    psy_audio_Sample* wave;
    psy_audio_Array* la;
    psy_audio_Array* ra;
    float* l;
    float* r;
    bool is_stereo;
    int i;
    
    n = lua_gettop(L);
    if (n != 2 && n != 3) {
        return luaL_error(L,
            "Got %d arguments expected 2 (self, array, array)", n);
    }
    wave = psyclescript_checkself(L, 1, luawavedata_meta);
    la = *(psy_audio_Array**)luaL_checkudata(L, 2, luaarraybind_meta);
    ra = 0;    
    is_stereo = (n == 3);
    if (is_stereo) {
        ra = *(psy_audio_Array**)luaL_checkudata(L, 3, luaarraybind_meta);
    }
    psy_audio_sample_dispose(wave);
    psy_audio_sample_init(wave, is_stereo ? 2 : 1);
    wave->numframes = psy_audio_array_len(la);
    psy_audio_sample_allocwavedata(wave);    
        
    l = psy_audio_buffer_at(&wave->channels, 0);
    if (is_stereo) {
        r = psy_audio_buffer_at(&wave->channels, 1);
    } else {
        r = NULL;
    }
    r = psy_audio_buffer_at(&wave->channels, 1);    
    for (i = 0; i < psy_audio_array_len(la); ++i) {
        l[i] = psy_audio_array_at(la, i);
        if (is_stereo) {
            r[i] = psy_audio_array_at(ra, i);            
        }
    }
    return 0;
}

int set_bank(lua_State* L) {
    int n;
    
    n = lua_gettop(L);
    /* if (n != 2) {
        return luaL_error(L, "Got %d arguments expected 2 (self, index)", n);
    }
    int index = luaL_checknumber(L, 2);
    typedef XMInstrument::WaveData<float> T;
    boost::shared_ptr<T> wave = LuaHelper::check_sptr<T>(L, 1, meta);
    XMInstrument::WaveData<std::int16_t> wave16;
    int len = wave->WaveLength();
    wave16.AllocWaveData(wave->WaveLength(), wave->IsWaveStereo());
    wave16.WaveName("test");
    std::int16_t* ldest = wave16.pWaveDataL();
    std::int16_t* rdest = wave16.pWaveDataR();
    for (int i = 0; i < len; ++i) {
        ldest[i] = wave->pWaveDataL()[i] * 32767;
        if (wave->IsWaveStereo()) {
            rdest[i] = wave->pWaveDataL()[i] * 32767;
        }
    }
    wave16.WaveLoopStart(wave->WaveLoopStart());
    wave16.WaveLoopEnd(wave->WaveLoopEnd());
    wave16.WaveLoopType(wave->WaveLoopType());
    wave16.WaveTune(wave->WaveTune());
    wave16.WaveFineTune(wave->WaveFineTune());
    wave16.WaveSampleRate(wave->WaveSampleRate());
    wave16.WaveName("test");
    SampleList& list = Global::song().samples;
    list.SetSample(wave16, index);*/
    return 0;
}

int set_wave_sample_rate(lua_State* L) {
    int n;    
    psy_audio_Sample* wave;
    lua_Number rate;
    
    n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Got %d arguments expected 2 (self, samplerate)", n);
    }
    wave = psyclescript_checkself(L, 1, luawavedata_meta);
    rate = luaL_checknumber(L, 2);
    psy_audio_sample_set_samplerate(wave, rate);
    return 0;
}

int set_wave_tune(lua_State* L) {
    int n;    
    psy_audio_Sample* wave;
    lua_Number tune;
    
    n = lua_gettop(L);    
    if (n != 2) {
        return luaL_error(L, "Got %d arguments expected 2 (self, tune)", n);
    }
    wave = psyclescript_checkself(L, 1, luawavedata_meta);
    tune = luaL_checknumber(L, 2);
    psy_audio_sample_set_tune(wave, (int16_t)tune);
    return 0;
}

int set_wave_fine_tune(lua_State* L) {
    int n;
    psy_audio_Sample* wave;
    lua_Number tune;

    n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Got %d arguments expected 2 (self, tune)", n);
    }
    wave = psyclescript_checkself(L, 1, luawavedata_meta);
    tune = luaL_checknumber(L, 2);
    psy_audio_sample_set_fine_tune(wave, (int16_t)tune);
    return 0;
}

int set_loop(lua_State* L) {
    int n;
    psy_audio_Sample* wave;
    uintptr_t loop_start;
    uintptr_t loop_end;

    n = lua_gettop(L);
    if (n != 3 && n != 4) {
        return luaL_error(L,
            "Got %d arguments expected 3 (self, start, end [, looptype])", n);
    }
    wave = psyclescript_checkself(L, 1, luawavedata_meta);
    loop_start = (uintptr_t)luaL_checknumber(L, 2);
    loop_end = (uintptr_t)luaL_checknumber(L, 3);
    if (n == 4) {
        int loop_type = (int)luaL_checknumber(L, 4);
        psy_audio_sample_setcontloop(wave,
            psy_audio_sampleloop_make((psy_audio_SampleLoopType)loop_type,
                loop_start, loop_end));
    } else {
        psy_audio_sample_setcontloop(wave,
            psy_audio_sampleloop_make(psy_audio_SAMPLE_LOOP_NORMAL,
                loop_start, loop_end));        
    }    
    return 0;
}
