#include <psycle/host/detail/project.hpp>
#include "plugincatcher.hpp"
#include "Song.hpp"
#include "LuaInternals.hpp"
#include "LuaPlugin.hpp"
#include <universalis/os/terminal.hpp>
#include "PlotterDlg.hpp"
#include "LuaHost.hpp"
#include "Player.hpp"
#include "LuaHelper.hpp"

#include <boost/filesystem.hpp>

#include <lua.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "Song.hpp"
#include <psycle/helpers/resampler.hpp>


namespace psycle { namespace host {


LuaMachine::~LuaMachine() {  
	if (mac_ && !shared_) {
		delete mac_; 
	}
}

void LuaMachine::load(const char* name) {
   PluginCatcher* plug_catcher = static_cast<PluginCatcher*>(&Global::machineload());  
   PluginInfo* info = plug_catcher->info(name);
   if (info) {
     Song& song =  Global::song();
	 mac_ = song.CreateMachine(info->type, info->dllname.c_str(), 1024, 0);
	 mac_->Init();
	 build_buffer(mac_->samplesV, 256);
	 shared_ = false;
   } else
   throw std::runtime_error("plugin not found error");
}

void LuaMachine::work(int samples) {
	update_num_samples(samples);	
	mac_->GenerateAudio(samples, false);	
}

void LuaMachine::build_buffer(std::vector<float*>& buf, int num) {
	    sampleV_.clear();
		std::vector<float*>::iterator it = buf.begin();
		for ( ; it != buf.end(); ++it) {
  		  sampleV_.push_back(PSArray(*it, num));
		}
	}

void LuaMachine::set_buffer(std::vector<float*>& buf) {
	mac_->change_buffer(buf);
	build_buffer(mac_->samplesV, 256);
}

void LuaMachine::update_num_samples(int num) {
		psybuffer::iterator it = sampleV_.begin();
		for ( ; it != sampleV_.end(); ++it) {
			(*it).set_len(num);
		}
	}

///////////////////////////////////////////////////////////////////////////////
// PluginImportBind
///////////////////////////////////////////////////////////////////////////////
int LuaMachineBind::open(lua_State *L) {
  static const luaL_Reg plugin_methods[] = {
	  {"new", create},
  	  {"work", work},
	  {"tick", tick},
	  {"channel", channel},
	  {"setnumchannels", set_numchannels},
	  {"resize", resize},
	  {"setbuffer", setbuffer},
	  {"addparameters", add_parameters},
	  {"setparameters", set_parameters},
	  {"setnumcols", set_numcols },
//	  {"samplerate", samplerate },
	  {NULL, NULL}
  };
  luaL_newmetatable(L, "psypluginmeta");
  lua_pushcclosure(L, gc, 0);
  lua_setfield(L,-2, "__gc");
  luaL_newlib(L, plugin_methods);    
  return 1;
}

int LuaMachineBind::create(lua_State* L) {
  int n = lua_gettop(L);  // Number of arguments
  if (n==1) {
	 LuaHelper::new_userdata<LuaMachine>(L, "psypluginmeta", new LuaMachine());	 
     lua_newtable(L);
     lua_setfield(L, -2, "params");
	 return 1;
  }
  if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 1 (pluginname)", n); 
  }
  LuaMachine* udata = LuaHelper::new_userdata<LuaMachine>(L, "psypluginmeta", new LuaMachine()); 

  if (lua_isnumber(L, 2)) {
	 int idx = luaL_checknumber(L, 2);
	 if (Global::song()._pMachine[idx] == 0) {
	   luaL_error(L, "no machine at index %d", n);
	 }
	 Machine* mac = Global::song()._pMachine[idx];
	 udata->set_mac(mac);
  } else {
	 try {
	   size_t len;
	   const char* plug_name = luaL_checklstring(L, 2, &len); 
       udata->load(plug_name);   
     } catch (std::exception &e) {
	   luaL_error(L, "plugin not found error");
     } 
  }
  lua_newtable(L);
  for (int idx = 0; idx < udata->mac()->GetNumParams(); ++idx) {
    lua_getglobal(L, "require");
    lua_pushstring(L, "parameter");
    lua_pcall(L, 1, 1, 0);
    lua_getfield(L, -1, "new");
    lua_pushvalue(L, -2);    
    lua_pushstring(L, "");
    lua_pushstring(L, "");
    lua_pushnumber(L, 0);
    lua_pushnumber(L, 1);
    lua_pushnumber(L, 10);
    lua_pushnumber(L, 1);
	lua_pushnumber(L, 2); // mpf state
    lua_pcall(L, 8, 1, 0);  
    lua_pushcclosure(L, setnorm, 0);  
    lua_setfield(L, -2, "setnorm");
    lua_pushcclosure(L, name, 0);  
    lua_setfield(L, -2, "name");
    lua_pushcclosure(L, display, 0);  
    lua_setfield(L, -2, "display");
	lua_pushcclosure(L, getnorm, 0);  
    lua_setfield(L, -2, "norm");
	lua_pushcclosure(L, getrange, 0);  
    lua_setfield(L, -2, "range");
    lua_pushvalue(L,3);  
    lua_setfield(L, -2, "plugin");
    lua_pushnumber(L, idx);
    lua_setfield(L, -2, "idx");
    lua_rawseti(L, 4, idx+1);
  }
  lua_pushvalue(L, 4);  
  lua_setfield(L, 3, "params");  
  lua_pushvalue(L, 3);  
  return 1;
}

int LuaMachineBind::tick(lua_State* L) {
  int n = lua_gettop(L);
  if (n == 7) {
       LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta");        
	   int track = luaL_checknumber(L, 2);
	   int note = luaL_checknumber(L, 3);
	   int inst = luaL_checknumber(L, 4);
	   int mach = luaL_checknumber(L, 5);
	   int cmd = luaL_checknumber(L, 6);
	   int param = luaL_checknumber(L, 7); 
	   PatternEntry data(note, inst, mach, cmd, param);
	   plugin->mac()->Tick(track, &data);	   
  } else {
      luaL_error(L, "Got %d arguments expected 1 (self)", n); 
  }
  return 0;
}

int LuaMachineBind::setnorm(lua_State* L) {
	double newval = luaL_checknumber(L, 2);
	lua_pushvalue(L, 1);
	lua_getfield(L, -1, "idx");
	int idx = luaL_checknumber(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "plugin");
	lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, "psypluginmeta");
	int minval; int maxval;
	(*ud)->mac()->GetParamRange(idx, minval, maxval);
    int quantization = (maxval-minval);	
	(*ud)->mac()->SetParameter(idx, newval*quantization); 
	return 0;
}

int LuaMachineBind::name(lua_State* L) {
	lua_getfield(L, -1, "idx");
	int idx = luaL_checknumber(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "plugin");
	lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, "psypluginmeta");
	char buf[128];
    (*ud)->mac()->GetParamName(idx, buf);	
	lua_pushstring(L, buf);
	return 1;
}

int LuaMachineBind::display(lua_State* L) {
	lua_getfield(L, -1, "idx");
	int idx = luaL_checknumber(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "plugin");
	lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, "psypluginmeta");
	char buf[128];
    (*ud)->mac()->GetParamValue(idx, buf);
	lua_pushstring(L, buf);
	return 1;
}

int LuaMachineBind::getnorm(lua_State* L) {
	lua_getfield(L, -1, "idx");
	int idx = luaL_checknumber(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "plugin");
	lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, "psypluginmeta");	
	int minval; int maxval;
	(*ud)->mac()->GetParamRange(idx, minval, maxval);
    int quantization = (maxval-minval);	
    double val = (*ud)->mac()->GetParamValue(idx)-minval;
	val = val  / quantization;
	lua_pushnumber(L, val);
	return 1;
}

int LuaMachineBind::getrange(lua_State* L) {
	lua_getfield(L, -1, "idx");
	int idx = luaL_checknumber(L, -1);
	lua_pop(L,1);
	lua_getfield(L, -1, "plugin");
	lua_getfield(L, -1, "__self");
    LuaMachine** ud = (LuaMachine**) luaL_checkudata(L, -1, "psypluginmeta");	
	int minval; int maxval;
	(*ud)->mac()->GetParamRange(idx, minval, maxval);
	lua_pushnumber(L, 0);
	lua_pushnumber(L, 1);
	int steps = maxval - minval;
	lua_pushnumber(L, steps);
	return 3;
}

int LuaMachineBind::work(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
       LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta"); 
       assert(plugin != 0);
       int num = luaL_checknumber (L, 2);
	   plugin->work(num);
	}  else {
       luaL_error(L, "Got %d arguments expected 2 (self, num)", n); 
	}
	return 0;
}

int LuaMachineBind::resize(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
       LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta"); 
       assert(plugin != 0);
	   int size = luaL_checknumber (L, 2);	
	   plugin->update_num_samples(size);
	}  else {
       luaL_error(L, "Got %d arguments expected 2 (self, size)", n); 
	}
	return 0;
}

int LuaMachineBind::channel(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
       LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta"); 
	   int idx = luaL_checknumber (L, 2);	
	   PSArray ** udata = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));	
	   PSArray* a = plugin->channel(idx);
	   *udata = new PSArray(a->data(), a->len());
       luaL_setmetatable(L, "array_meta");
	}  else {
       luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
	}
	return 1;
}

int LuaMachineBind::set_numchannels(lua_State* L) {	
	LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta"); 
	int num = luaL_checknumber(L, 2);
	plugin->mac()->InitializeSamplesVector(num);
	plugin->build_buffer(plugin->mac()->samplesV, 256);
	return 0;
}


int LuaMachineBind::set_numcols(lua_State* L) {	
    LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta"); 
	plugin->set_numcols(luaL_checknumber(L, 2));
	return 0;
}

int LuaMachineBind::numchannels(lua_State* L) {	
	LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta");    
	lua_pushnumber(L, plugin->samples().size());
	return 1;
}

int LuaMachineBind::gc(lua_State* L) {
	LuaMachine* ud = *(LuaMachine**) luaL_checkudata(L, 1, "psypluginmeta");
	delete ud;
	return 0;
}

int LuaMachineBind::setbuffer(lua_State* L) {
  int n = lua_gettop(L);
  if (n == 2) {
    LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta"); 
    assert(plugin != 0);
	luaL_checktype(L, 2, LUA_TTABLE);	
	lua_pushvalue(L, 2);
	std::vector<float*> sampleV;
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
  	     PSArray* v = *(PSArray **)luaL_checkudata(L, -1, "array_meta");
		 sampleV.push_back(v->data());		 
	}
	plugin->set_buffer(sampleV);    
  } else {
	 luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
  }
  return 0;
}

int luaL_orderednext(lua_State *L)
{
  luaL_checkany(L, -1);                 // previous key
  luaL_checktype(L, -2, LUA_TTABLE);    // self
  luaL_checktype(L, -3, LUA_TFUNCTION); // iterator
  lua_pop(L, 1);                        // pop the key since 
                                        // opair doesn't use it

  // iter(self)
  lua_pushvalue(L, -2);
  lua_pushvalue(L, -2);
  lua_call(L, 1, 2);

  if(lua_isnil(L, -2))
  {
    lua_pop(L, 2);
    return 0;
  }
  return 2;
}

int LuaMachineBind::add_parameters(lua_State* L) {	
    LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta");    
	lua_getfield(L, 1, "params");
	lua_pushvalue(L, 2);
	luaL_checktype(L, 2, LUA_TTABLE);
	// t:opairs()
	lua_getfield(L, 2, "opairs");
	lua_pushvalue(L, -2);
	lua_call(L, 1, 2);
	size_t len;
	// iter, self (t), nil
	for(lua_pushnil(L); luaL_orderednext(L);)
	{
		const char* key = luaL_checklstring(L, -2, &len);
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "id_");
		lua_rawseti(L, 3, lua_rawlen(L, 3)+1); // params[#params+1] = newparam
	}
	plugin->set_numparams(lua_rawlen(L, 3));
	return 0;
}

int LuaMachineBind::set_parameters(lua_State* L) {	
	LuaMachine* plugin = LuaHelper::check<LuaMachine>(L, 1, "psypluginmeta"); 
    plugin->set_numparams(lua_rawlen(L, 2));
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_setfield(L, -2, "params");
	return 0;
}

/////////////////////////////////////
// PlayerBind
//////////////////////////////////////
int LuaPlayerBind::open(lua_State *L) {
  static const luaL_Reg plugin_methods[] = {
	  {"new", create},
	  {"samplerate", samplerate},
	  {NULL, NULL}
  };
  luaL_newmetatable(L, "psyplayermeta");
  luaL_newlib(L, plugin_methods);    
  return 1;
}

int LuaPlayerBind::create(lua_State* L) {
  int n = lua_gettop(L);  // Number of arguments
  if (n==1) {
	 LuaHelper::new_userdata<Player>(L, "psyplayermeta", &Global::player());	 
	 return 1;
  }
  return 1;
}

int LuaPlayerBind::samplerate(lua_State* L) {	
	Player* player = LuaHelper::check<Player>(L, 1, "psyplayermeta");    	
	lua_pushnumber(L, player->SampleRate());
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// PlotterBind
///////////////////////////////////////////////////////////////////////////////
int LuaPlotterBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
	 {"new", create},
	 {"stem", stem },
	 { NULL, NULL }
  };
  luaL_newmetatable(L, "psyplottermeta");
  lua_pushcclosure(L, gc, 0);
  lua_setfield(L,-2, "__gc");
  luaL_newlib(L, methods);  
  return 1;
}

int LuaPlotterBind::create(lua_State* L) {	
  luaL_checktype(L, 1, LUA_TTABLE);
  int n = lua_gettop(L);  // Number of arguments
  if (n != 1) {
     return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
  }  
  LuaHelper::new_userdata<CPlotterDlg>(L, "psyplottermeta", new CPlotterDlg(AfxGetMainWnd()));
  return 1;
}

int LuaPlotterBind::gc(lua_State* L) {
	CPlotterDlg* ud = *(CPlotterDlg**) luaL_checkudata(L, 1, "psyplottermeta");			
	AfxGetMainWnd()->SendMessage(0x0501, (WPARAM)ud, 0);
    /*if (ud) {
		// ud->DestroyWindow();
    }*/
	return 0;
}

int LuaPlotterBind::stem(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
	   CPlotterDlg* plotter = LuaHelper::check<CPlotterDlg>(L, 1, "psyplottermeta");
	   plotter->ShowWindow(SW_SHOW);
	   PSArray* x = *(PSArray **)luaL_checkudata(L, -1, "array_meta");
	   plotter->set_data(x->data(), x->len());
	   plotter->UpdateWindow();
	 } else {
	   luaL_error(L, "Got %d arguments expected 2 (self, array)", n); 
     }
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Delay
///////////////////////////////////////////////////////////////////////////////
void PSDelay::work(PSArray& x, PSArray& y) {
	int n = x.len();
	int k = mem.len();
    float* mp = mem.data();
	float* xp = x.data();
	float* yp = y.data();
	if (k <= n) {
	  for (int i = 0; i < k; i++, ++yp) *yp = (*mp++);
	  for (int i = 0; i < n-k; i++, ++yp) *yp = (*xp++);
	  mp = mem.data();
	  for (int i = 0; i < k; i++, ++mp) *mp = (*xp++);
	} else {
	  for (int i = 0; i < n; i++, ++yp) *yp = (*mp++);
	  mp = mem.data();
	  for (int i = n; i < k; i++, ++mp) *mp = mp[i];
	  for (int i = 0; i < n; i++, ++mp) *mp = (*xp++);
	}
}

///////////////////////////////////////////////////////////////////////////////
// DelayBind
///////////////////////////////////////////////////////////////////////////////
int  LuaDelayBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
	  {"new", create},
  	  {"work", work },
   	  { "__tostring", tostring },
	  { NULL, NULL }
  };
  luaL_newmetatable(L, "psydelaymeta");
  lua_pushcclosure(L, gc, 0);
  lua_setfield(L,-2, "__gc");
  luaL_newlib(L, methods);  
  return 1;
}

int LuaDelayBind::create(lua_State *L) {
  int n = lua_gettop(L);  // Number of arguments
  if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 2 (self, k)", n); 
  }
  int k = luaL_checknumber (L, 2);
  luaL_argcheck(L, k >= 0, 2, "negative index not allowed");
  LuaHelper::new_userdata<PSDelay>(L, "psydelaymeta", new PSDelay(k));
  return 1;
}

int LuaDelayBind::work(lua_State* L) {
   int n = lua_gettop(L);
	if (n == 2) {
       PSDelay* delay = LuaHelper::check<PSDelay>(L, 1, "psydelaymeta");
       assert(delay != 0);
	   PSArray* x = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
       PSArray** y = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));
       *y = new PSArray(x->len(), 0);
       luaL_setmetatable(L, "array_meta");
       delay->work(*x, **y);
	}  else {
       luaL_error(L, "Got %d arguments expected 2 (self, arrayinput)", n); 
	}
   return 1;
}

int LuaDelayBind::gc (lua_State *L) {
	PSDelay* ptr = *(PSDelay **)luaL_checkudata(L, 1, "psydelaymeta");
	delete ptr;	
	return 0;
}

int LuaDelayBind::tostring(lua_State *L) {
	PSDelay** ud = (PSDelay**) luaL_checkudata(L, 1, "psydelaymeta");   
	lua_pushfstring(L, "test");
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// WaveOsc
///////////////////////////////////////////////////////////////////////////////
void WaveOscTables::saw(float* data, int num, int maxharmonic)  {
  double gain = 0.5 / 0.777;
  for  (int h = 1; h <= maxharmonic; ++h) {
    double amplitude = gain / h;
	double to_angle = 2* psycle::helpers::math::pi / num * h;
	for (int i = 0; i < num; ++i) {
	  data[i] += ::sin(pow(-1.0,h+1)*i*to_angle)*amplitude*0.7;
    }  
  }
}

void WaveOscTables::sqr(float* data, int num, int maxharmonic)  {
  double gain = 0.5 / 0.777;
  for  (int h = 1; h <= maxharmonic; h=h+2) {
    double amplitude = gain / h;
	double to_angle = 2* psycle::helpers::math::pi / num * h;
	for (int i = 0; i < num; ++i) {
	  data[i] += ::sin(i*to_angle)*amplitude*0.7;
    }  
  }
}

void WaveOscTables::tri(float* data, int num, int maxharmonic)  {
  double gain = 0.5 / 0.777;
  for (int h = 1; h <= maxharmonic; h=h+2) {
    //double amplitude = gain / h;
	double to_angle = 2*psycle::helpers::math::pi/num*h;
	for (int i = 0; i < num; ++i) {
		 // 
	  data[i] += pow(-1.0,(h-1)/2.0)/(h*h)*::sin(i*to_angle); // *amplitude*0.7;
    }  
  }  
}

void WaveOscTables::sin(float* data, int num, int maxharmonic)  {
  double to_angle = 2*psycle::helpers::math::pi/num;
  for (int i = 0; i < num; ++i) {
	  data[i] = ::sin(i*to_angle);
  }
}

void WaveOscTables::cwave(double fh,  XMInstrument::WaveData<float>& wave, void (*func)(float*, int, int), int sr) {
  double f = 261.6255653005986346778499935233; // C4
  int num = (int) (sr/f + 0.5);
  int hmax = (int) (sr/2/fh);
  wave.AllocWaveData(num, false);
  wave.WaveSampleRate(sr);
  wave.WaveLoopStart(0);
  wave.WaveLoopEnd(num);
  wave.WaveLoopType(XMInstrument::WaveData<float>::LoopType::NORMAL);
  //Warning!! do not use dsp::Clear. Memory is not aligned in wavedata.
  for (int i = 0; i < num; ++i) wave.pWaveDataL()[i]=0;
  func(wave.pWaveDataL(), num, hmax);
}

void WaveOscTables::set_samplerate(int sr) {
	if (sin_tbl.size() != 0) {
	  cleartbl(sin_tbl);
	  cleartbl(tri_tbl);
	  cleartbl(sqr_tbl);
	  cleartbl(saw_tbl);
	}
	double f_lo = 440*std::pow(2.0, (0-notecommands::middleA)/12.0);
	for (int i = 0; i < 10; ++i) {
		double f_hi = 2 * f_lo;
		if (i==0) {
			f_lo = 0;
		}
		XMInstrument::WaveData<float>* w;
		w =  new XMInstrument::WaveData<float>();
		cwave(f_hi, *w, &sqr, sr);
		sqr_tbl[range<double>(f_lo, f_hi)] = w;
		w =  new XMInstrument::WaveData<float>();
		cwave(f_hi, *w, &saw, sr);
		saw_tbl[range<double>(f_lo, f_hi)] = w;
		w =  new XMInstrument::WaveData<float>();
		cwave(f_hi, *w, &sin, sr);
		sin_tbl[range<double>(f_lo, f_hi)] = w;
		w =  new XMInstrument::WaveData<float>();
		cwave(f_hi, *w, &tri, sr);
		tri_tbl[range<double>(f_lo, f_hi)] = w;
		f_lo = f_hi;
	}
}

void WaveOscTables::cleartbl(WaveList<float>::Type& tbl) {
	WaveList<float>::Type::iterator it;
	it = tbl.begin();
	for ( ; it != tbl.end(); ++it) {
		delete it->second;
	}
	tbl.clear();
}


WaveOsc::WaveOsc(WaveOscTables::Shape shape) {   
   WaveList<float>::Type tbl = WaveOscTables::getInstance()->tbl(shape);
   resampler = new ResamplerWrap<float, 1>(tbl);
   resampler->set_frequency(263);
}

void WaveOsc::set_shape(WaveOscTables::Shape shape) {
   WaveList<float>::Type tbl = WaveOscTables::getInstance()->tbl(shape);
   resampler->SetData(tbl);
}

///////////////////////////////////////////////////////////////////////////////
// WaveOscBind
///////////////////////////////////////////////////////////////////////////////
int LuaWaveOscBind::open(lua_State *L) {
  static const luaL_Reg delay_methods[] = {
	  {"new", create},
	  {"frequency", get_base_frequency},
	  {"setfrequency", set_base_frequency},
	  {"setgain", set_gain},
	  {"gain", gain},
  	  {"work", work },
	  {"stop", stop },
	  {"start", start },
	  {"isplaying", isplaying},
   	  { "__tostring", tostring },
	  { "setshape", set_shape },
   	  {"setquality", set_quality},
	  {"quality", set_quality},
	  { NULL, NULL }
  };
  luaL_newmetatable(L, "psyoscmeta");
  lua_pushcclosure(L, gc, 0);
  lua_setfield(L,-2, "__gc");
  luaL_newlib(L, delay_methods);
  lua_pushnumber(L, 1);
  lua_setfield(L, -2, "SIN");
  lua_pushnumber(L, 2);
  lua_setfield(L, -2, "SAW");
  lua_pushnumber(L, 3);
  lua_setfield(L, -2, "SQR");
  lua_pushnumber(L, 4);
  lua_setfield(L, -2, "TRI");
  // Quality
  lua_pushnumber(L, 1);
  lua_setfield(L, -2, "ZEROHOLD");
  lua_pushnumber(L, 2);
  lua_setfield(L, -2, "LINEAR");
  lua_pushnumber(L, 3);
  lua_setfield(L, -2, "SINC");
  return 1;
}

int LuaWaveOscBind::create(lua_State *L) {
  int n = lua_gettop(L);  // Number of arguments
  if (n != 2 and n!=3) {
     return luaL_error(L, "Got %d arguments expected 2[,3] (self, shape [, f])", n); 
  }
  int type = luaL_checknumber (L, 2);
  // luaL_argcheck(L, f >= 0, 2, "negative frequency is not allowed");
  WaveOsc* osc = LuaHelper::new_userdata<WaveOsc>(L, "psyoscmeta", new WaveOsc((WaveOscTables::Shape)type));
  if (n==3) {
	double f = luaL_checknumber (L, 3);
    osc->set_frequency(f);
  }
  return 1;  
}

int LuaWaveOscBind::set_gain(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==2) {
       WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
       assert(osc != 0);	   
	   osc->set_gain(luaL_checknumber(L, -1));
	}  else {
       luaL_error(L, "Got %d arguments expected 2 (self, gain)", n); 
	}
	return 1;
}

int LuaWaveOscBind::gain(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==1) {
       WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
       assert(osc != 0);	   
	   lua_pushnumber(L, osc->gain());
	}  else {
       luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}
	return 1;
}

int LuaWaveOscBind::set_shape(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==2) {
       WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
       assert(osc != 0);	   
	   osc->set_shape((WaveOscTables::Shape)(int)luaL_checknumber(L, -1));
	}  else {
       luaL_error(L, "Got %d arguments expected 2 (self, shape)", n); 
	}
	return 1;
}

int LuaWaveOscBind::stop(lua_State* L) {
  int n = lua_gettop(L);  // Number of arguments
  if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 2 (self, phase)", n); 
  }
  double phase = luaL_checknumber (L, 2);
  WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
  osc->Stop(phase);
  return 0;
}

int LuaWaveOscBind::start(lua_State* L) {
  int n = lua_gettop(L);  // Number of arguments
  double phase = 0;
  if (n!=1 and n != 2) {
     return luaL_error(L, "Got %d arguments expected 1 or 2 (self [, phase])", n); 
  }
  if (n==2) {
    phase = luaL_checknumber (L, 2);
  }
  WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
  osc->Start(phase);
  return 0;
}

int LuaWaveOscBind::isplaying(lua_State* L) {
  int n = lua_gettop(L); 
  if (n != 1) {
     return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
  }
  WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
  lua_pushboolean(L, osc->IsPlaying());
  return 1;
}

int LuaWaveOscBind::get_base_frequency(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==1) {
       WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
	   lua_pushnumber(L, osc->base_frequency());
	}  else {
       luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}
	return 1;
}

int LuaWaveOscBind::set_base_frequency(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==2) {
       WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
	   osc->set_frequency(luaL_checknumber(L, -1));
	}  else {
       luaL_error(L, "Got %d arguments expected 2 (self, frequency)", n); 
	}
	return 1;
}

int LuaWaveOscBind::work(lua_State* L) {
   int n = lua_gettop(L);
	if (n == 2 or n==3 or n==4) {
       WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
	   PSArray* data = *(PSArray **)luaL_checkudata(L, 2, "array_meta");   
	   float* fm = 0;
	   float* env = 0;
		  if (n>2 && (!lua_isnil(L,3))) {
	      PSArray*arr = *(PSArray **)luaL_checkudata(L, 3, "array_meta");   
		  fm = arr->data();
	   }
       if (n==4) {
		  PSArray* arr = *(PSArray **)luaL_checkudata(L, 4, "array_meta");
		  env = arr->data();
	   }
	   osc->work(data->len(), data->data(), fm, env);
	   lua_pushvalue(L, 2); // return data
	}  else {
       luaL_error(L, "Got %d arguments expected 2 or 3 (self, num, fm)", n); 
	}
   return 1;
}

int LuaWaveOscBind::gc(lua_State *L) {
	WaveOsc* ptr = *(WaveOsc **)luaL_checkudata(L, 1, "psyoscmeta");
	delete ptr;	
	return 0;
}

int LuaWaveOscBind::tostring(lua_State *L) {
	WaveOsc** ud = (WaveOsc**) luaL_checkudata(L, 1, "psyoscmeta");   
	lua_pushfstring(L, "test");
	return 1;
}

int LuaWaveOscBind::set_quality(lua_State* L) {
   int n = lua_gettop(L); 
   if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 2 (self, frequency)", n); 
   }   
   WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
   int quality = luaL_checknumber(L, 2)-1;
   osc->set_quality((psycle::helpers::dsp::resampler::quality::type)quality);
   return 0;
}

int LuaWaveOscBind::quality(lua_State* L) {
  int n = lua_gettop(L); 
  if (n != 1) {
     return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
  }
  WaveOsc* osc = LuaHelper::check<WaveOsc>(L, 1, "psyoscmeta");
  lua_pushnumber(L, (int)(osc->quality()+1));
  return 1;
}


///////////////////////////////////////////////////////////////////////////////
// DspMathHelperBind
///////////////////////////////////////////////////////////////////////////////
int LuaDspMathHelper::open(lua_State *L) {
	static const luaL_Reg funcs[] = {
	  {"notetofreq", notetofreq},
	  {"freqtonote", freqtonote},
	  {NULL, NULL}
  };
  luaL_newlib(L, funcs);
  return 1;
}

int LuaDspMathHelper::notetofreq(lua_State* L) {
   double note = luaL_checknumber(L, 1);
   int n = lua_gettop(L);
   int base = notecommands::middleA;
   if (n==2) {
	  base = luaL_checknumber(L, 2);
   }
   lua_pushnumber(L, 440*std::pow(2.0, (note-base)/12.0));   
   return 1;
}

int LuaDspMathHelper::freqtonote(lua_State* L) {
   double f = luaL_checknumber(L, 1);
   int n = lua_gettop(L);
   int base = notecommands::middleA;
   if (n==2) {
	  base = luaL_checknumber(L, 2);
   }
   double note = 12*std::log10(f/440.0)/std::log10(2.0)+base;
   lua_pushnumber(L, note);   
   return 1;
}

///////////////////////////////////////////////////////////////////////////////
// DspFilterBind
///////////////////////////////////////////////////////////////////////////////
int LuaDspFilterBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
	  {"new", create},
  	  {"work", work },
	  {"setcutoff", setcutoff },
	  {"setresonance", setresonance},
	  {"settype", setfiltertype },
   	  { "__tostring", tostring },
	  { NULL, NULL }
  };
  luaL_newmetatable(L, "psydspfiltermeta");
  lua_pushcclosure(L, gc, 0);
  lua_setfield(L,-2, "__gc");
  luaL_newlib(L, methods);  
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "LOWPASS");
  lua_pushnumber(L, 1);
  lua_setfield(L, -2, "HIGHPASS");
  lua_pushnumber(L, 2);
  lua_setfield(L, -2, "BANDPASS");
  lua_pushnumber(L, 3);
  lua_setfield(L, -2, "BANDREJECT");
  lua_pushnumber(L, 4);
  lua_setfield(L, -2, "NONE");
  lua_pushnumber(L, 5);
  lua_setfield(L, -2, "ITLOWPASS");
  return 1;
}

int LuaDspFilterBind::create(lua_State *L) {  
  int n = lua_gettop(L);  // Number of arguments
  if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 2 (self, f)", n); 
  }
  int type = luaL_checknumber (L, 2);
  psycle::helpers::dsp::Filter* ud = new psycle::helpers::dsp::Filter();
  ud->SampleRate(44100);
  ud->Type(static_cast<psycle::helpers::dsp::FilterType>(type));
  LuaHelper::new_userdata<psycle::helpers::dsp::Filter>(L, "psydspfiltermeta", ud);  
  return 1;
}

int LuaDspFilterBind::work(lua_State* L) {
   int n = lua_gettop(L);
	if (n ==2 or n==3 or n==4) {
	   float* vcfc = 0;
	   float* vcfr = 0;
	   psycle::helpers::dsp::Filter* filter = LuaHelper::check<psycle::helpers::dsp::Filter>(L, 1, "psydspfiltermeta");
       assert(filter != 0);	   
	   PSArray* x_input = *(PSArray **)luaL_checkudata(L, 2, "array_meta");   
	   if (n>2) {
	      PSArray* arr = *(PSArray **)luaL_checkudata(L, 3, "array_meta");   
		  vcfc = arr->data();
	   }
	   if (n>3) {
	      PSArray* arr = *(PSArray **)luaL_checkudata(L, 3, "array_meta");   
		  vcfr = arr->data();
	   }
	   int num = x_input->len();
	   float* data = x_input->data();
	   for (int i=0; i < num; ++i) {
		 if (vcfc) {
		   filter->Cutoff(vcfc[i]);
		 }
		 if (vcfr) {
           filter->Ressonance(vcfr[i]);
		 }
		 data[i] = filter->Work(data[i]);
	   }
       lua_pushvalue(L, 2);
	}  else {
       luaL_error(L, "Got %d arguments expected 2 or 3(self, array filter input [, voltage control])", n); 
	}
   return 1;
}

int LuaDspFilterBind::setcutoff(lua_State* L) {
   int n = lua_gettop(L);
	if (n ==2) {
	   psycle::helpers::dsp::Filter* filter = LuaHelper::check<psycle::helpers::dsp::Filter>(L, 1, "psydspfiltermeta");
       assert(filter != 0);	   
	   int cutoff = luaL_checknumber(L, -1);
	   filter->Cutoff(cutoff);
	}  else {
       luaL_error(L, "Got %d arguments expected 2(self, cutoff)", n); 
	}
   return 0;
}


int LuaDspFilterBind::getcutoff(lua_State* L) {
	// todo no getter in filter class
	/*int n = lua_gettop(L);
	if (n ==1) {
       psycle::helpers::dsp::Filter* filter = check<psycle::helpers::dsp::Filter>(L, 1, "psydspfiltermeta");
       assert(filter != 0);	   	   
	   lua_pushnumber(L, filter->cutoff());
	}  else {
       luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}*/
	return 1;
}

int LuaDspFilterBind::setresonance(lua_State* L) {
   int n = lua_gettop(L);
	if (n ==2) {
	   psycle::helpers::dsp::Filter* filter = LuaHelper::check<psycle::helpers::dsp::Filter>(L, 1, "psydspfiltermeta");
       assert(filter != 0);	   
	   int resonance = luaL_checknumber(L, -1);
	   filter->Ressonance(resonance);
	}  else {
       luaL_error(L, "Got %d arguments expected 2(self, resonance)", n); 
	}
   return 0;
}

int LuaDspFilterBind::getresonance(lua_State* L) {
	// todo no getter in filter class
	/*int n = lua_gettop(L);
	if (n ==1) {
       psycle::helpers::dsp::Filter* filter = check<psycle::helpers::dsp::Filter>(L, 1, "psydspfiltermeta");
       assert(filter != 0);	   
	   lua_pushnumber(L,filter->resonance());
	}  else {
       luaL_error(L, "Got %d arguments expected 1 (self)", n); 
	}*/
	return 1;
}

int LuaDspFilterBind::setfiltertype(lua_State* L) {
   int n = lua_gettop(L);
	if (n ==2) {
	   psycle::helpers::dsp::Filter* filter = LuaHelper::check<psycle::helpers::dsp::Filter>(L, 1, "psydspfiltermeta");
       assert(filter != 0);	   
	   int filtertype = luaL_checknumber(L, -1);
	   filter->Type(static_cast<psycle::helpers::dsp::FilterType>(filtertype));
	}  else {
       luaL_error(L, "Got %d arguments expected 2(self, filtertype)", n); 
	}
   return 0;
}

int LuaDspFilterBind::gc (lua_State *L) {
	psycle::helpers::dsp::Filter* ptr = *(psycle::helpers::dsp::Filter **)luaL_checkudata(L, 1, "psydspfiltermeta");
	delete ptr;	
	return 0;
}

int LuaDspFilterBind::tostring(lua_State *L) {
	psycle::helpers::dsp::Filter** ud = (psycle::helpers::dsp::Filter**) luaL_checkudata(L, 1, "psydspfiltermeta");   
	lua_pushfstring(L, "test");
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// WaveDataBind
///////////////////////////////////////////////////////////////////////////////
int LuaWaveDataBind::open(lua_State *L) {
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
  luaL_newmetatable(L, "psywavedatameta");
  lua_pushcclosure(L, gc, 0);
  lua_setfield(L,-2, "__gc");
  luaL_newlib(L, methods);  
  lua_pushnumber(L, 0);
  lua_setfield(L, -2, "DO_NOT");
  lua_pushnumber(L, 1);
  lua_setfield(L, -2, "NORMAL");
  lua_pushnumber(L, 2);
  lua_setfield(L, -2, "BIDI");
  return 1;
}

int LuaWaveDataBind::create(lua_State *L) {  
  int n = lua_gettop(L);  // Number of arguments
  if (n != 1) {
     return luaL_error(L, "Got %d arguments expected 2 (self)", n); 
  }    
  LuaHelper::new_userdata<>(L, "psywavedatameta", new XMInstrument::WaveData<float>);
  return 1;
}

int LuaWaveDataBind::copy(lua_State *L) {
  int n = lua_gettop(L); 
  if (n!=2 and n!=3) {
    return luaL_error(L, "Got %d arguments expected 2 (self, array, array)", n); 
  }   
  XMInstrument::WaveData<float>* wave = LuaHelper::check<XMInstrument::WaveData<float>>(L, 1, "psywavedatameta");  
  PSArray* la = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
  PSArray* ra = 0;
  bool is_stereo = (n==3 );
  if (is_stereo) {
     ra = *(PSArray **)luaL_checkudata(L, 3, "array_meta");
  }
  wave->AllocWaveData(la->len(), is_stereo);    
  float* l = wave->pWaveDataL();
  float* r = wave->pWaveDataR();
  for (int i = 0; i < la->len(); ++i) {
	l[i] = la->get_val(i);
	if (is_stereo) {
	  r[i] = ra->get_val(i);
	}
  }  
  return 0;
}

int LuaWaveDataBind::set_bank(lua_State *L) {
  int n = lua_gettop(L); 
  if (n!=2) {
    return luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
  }   
  int index = luaL_checknumber(L, 2);
  XMInstrument::WaveData<float>* wave = LuaHelper::check<XMInstrument::WaveData<float>>(L, 1, "psywavedatameta");  
  XMInstrument::WaveData<std::int16_t> wave16;  
  int len = wave->WaveLength();
  wave16.AllocWaveData(wave->WaveLength(), wave->IsWaveStereo());  
  wave16.WaveName("test");
  std::int16_t* ldest = wave16.pWaveDataL();
  std::int16_t* rdest = wave16.pWaveDataR();
  for (int i = 0; i < len; ++i) {
	ldest[i] = wave->pWaveDataL()[i]*32767;
	if (wave->IsWaveStereo()) {
	  rdest[i] = wave->pWaveDataL()[i]*32767;
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
  list.SetSample(wave16, index);
  return 0;
}


int LuaWaveDataBind::set_wave_sample_rate(lua_State *L) {
  int n = lua_gettop(L); 
  if (n!=2) {
    return luaL_error(L, "Got %d arguments expected 2 (self, samplerate)", n); 
  }   
  XMInstrument::WaveData<float>* wave = LuaHelper::check<XMInstrument::WaveData<float> >(L, 1, "psywavedatameta");    
  int rate = luaL_checknumber(L, 2);
  wave->WaveSampleRate(rate);
  return 0;
}

int LuaWaveDataBind::set_wave_tune(lua_State *L) {
  int n = lua_gettop(L); 
  if (n!=2) {
    return luaL_error(L, "Got %d arguments expected 2 (self, tune)", n); 
  }   
  XMInstrument::WaveData<float>* wave = LuaHelper::check<XMInstrument::WaveData<float> >(L, 1, "psywavedatameta");    
  int tune = luaL_checknumber(L, 2);
  wave->WaveTune(tune);
  return 0;
}

int LuaWaveDataBind::set_wave_fine_tune(lua_State *L) {
  int n = lua_gettop(L); 
  if (n!=2) {
    return luaL_error(L, "Got %d arguments expected 2 (self, finetune)", n); 
  }   
  XMInstrument::WaveData<float>* wave = LuaHelper::check<XMInstrument::WaveData<float> >(L, 1, "psywavedatameta");    
  int tune = luaL_checknumber(L, 2);
  wave->WaveFineTune(tune);
  return 0;
}

int LuaWaveDataBind::set_loop(lua_State *L) {
  int n = lua_gettop(L); 
  if (n!=3 and n!=4) {
    return luaL_error(L, "Got %d arguments expected 3 (self, start, end [, looptype])", n); 
  }   
  XMInstrument::WaveData<float>* wave = LuaHelper::check<XMInstrument::WaveData<float> >(L, 1, "psywavedatameta");    
  int loop_start = luaL_checknumber(L, 2);
  int loop_end = luaL_checknumber(L, 3);
  if (n==4) {
    int loop_type = luaL_checknumber(L, 4);
	wave->WaveLoopType(static_cast<const  XMInstrument::WaveData<float>::LoopType::Type>(loop_type));
  } else {
	wave->WaveLoopType(XMInstrument::WaveData<>::LoopType::NORMAL);
  }
  wave->WaveLoopStart(loop_start);
  wave->WaveLoopEnd(loop_end);
  return 0;
}

int LuaWaveDataBind::gc (lua_State *L) {
	XMInstrument::WaveData<float>* ptr = *(XMInstrument::WaveData<float> **)luaL_checkudata(L, 1, "psywavedatameta");
	delete ptr;	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Resampler Wrap
///////////////////////////////////////////////////////////////////////////////
template <class T, int VOL>
void ResamplerWrap<T, VOL>::Start(double phase) {  
  wave_it = waves_.find(range<double>(f_));
  last_wave = wave_it != waves_.end() ? wave_it->second : 0;
  if (wave_it != waves_.end()) {
    wavectrl.Init(last_wave, 0, resampler);
    wavectrl.Position(phase*last_wave->WaveLength());
	basef = 440 *std::pow(2.0, (notecommands::middleC-last_wave->WaveTune()-last_wave->WaveFineTune()/100.0-notecommands::middleA)/12.0) * last_wave->WaveSampleRate() / (double) Global::player().SampleRate();
    speed_ = f_/basef;   
    wavectrl.Speed(resampler, speed_);
    wavectrl.Playing(true);
  }
  dostop_ = false;
}

template <class T, int VOL>
void ResamplerWrap<T, VOL>::set_frequency(double f) {
	f_ = f;
	speed_ = f_/basef;
    wavectrl.Speed(resampler, speed_);
}

template <class T, int VOL>
int ResamplerWrap<T, VOL>::work(int numSamples, float* pSamplesL, float* pSamplesR, float* fm, float* env) {
  if (!wavectrl.Playing())
	  return 0;
  int num = numSamples;
  float left_output = 0.0f;
  float right_output = 0.0f;
  double f = f_;
  //float voldelta=500.f/ 44100; //delta samples for two milliseconds.
  while(numSamples) {
     XMSampler::WaveDataController<T>::WorkFunction pWork;
	 int pos = wavectrl.Position();
	 if (wave_it != waves_.end() && speed_ > 0) {
		 int nextsamples = std::min(wavectrl.PreWork(numSamples, &pWork), numSamples);
		 numSamples-=nextsamples;
		 while (nextsamples) {
			 ++pos;
			 if (dostop_) {
				double phase = pos / (double) last_wave->WaveLength();
				while (phase >= 1) {
					phase -= 1;
				}
				if (abs(stopphase_ - phase) <= speed_/ (double) last_wave->WaveLength()) {
					wavectrl.NoteOff();
					wavectrl.Playing(false);
					dostop_ = false;
				}
			 }
			 if (fm != 0) { 
				f = (f_+*(fm++));
				speed_ = f/basef;
				wavectrl.Speed(resampler, speed_);			
			 }	
			 pWork(wavectrl, &left_output, &right_output);		 
			 if (!wavectrl.IsStereo()) {
				right_output = left_output;
			 }
			 if (env != 0) {
				 float v = (*env++);
				 left_output *= v;
				 right_output *= v;
			 }
			 if (pSamplesL) *pSamplesL++ += left_output*adjust_vol*gain_;
			 if (pSamplesR) *pSamplesR++ += right_output*adjust_vol*gain_;		 
			 nextsamples--;
		 }	 
		 wavectrl.PostWork();
	 } else {
		 numSamples--;
		 if (wave_it == waves_.end()) {
			check_wave(f);
		 } else
		 if (fm != 0) { 
			f = (f_+*(fm++));
			speed_ = f/basef;
			wavectrl.Speed(resampler, speed_);			
		 }		 
	 }   
     if (!wavectrl.Playing()) {		
	   return num - numSamples; 
     }
	 check_wave(f);
   }			
   return num;
}

template <class T, int VOL>
void ResamplerWrap<T, VOL>::check_wave(double f) {
	if (wave_it == waves_.end()) {
		wave_it = waves_.find(range<double>(f_));
	} else {
	  while (wave_it != waves_.begin() && wave_it->first.min() > f) {
		 wave_it--;
	  } 
      while (wave_it != waves_.end() && wave_it->first.max() < f) {
	     wave_it++;	     
	  }
	}
	if (wave_it != waves_.end()) {
      const XMInstrument::WaveData<T>* wave = wave_it->second;
      if (last_wave!=wave) {
	    int oldpos = wavectrl.Position();
	    wavectrl.Init(wave, 0, resampler);
		basef = 440 *std::pow(2.0, (notecommands::middleC-wave->WaveTune()-wave->WaveFineTune()/100-notecommands::middleA)/12.0) * wave->WaveSampleRate() / Global::player().SampleRate();
	    wavectrl.Speed(resampler, f/basef);
	    wavectrl.Playing(true);
	    double phase = oldpos / (double) last_wave->WaveLength();
	    int newpos = phase * wave->WaveLength();
	    wavectrl.Position(newpos);
	    last_wave = wave;
	  }
	} else {
	  last_wave = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Resampler Bind
///////////////////////////////////////////////////////////////////////////////
int LuaResamplerBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
	  {"new", create},
	  {"newwavetable", createwavetable},
	  {"work", work},
	  {"noteoff", noteoff},
	  {"setfrequency", set_frequency},
	  {"start", start},
	  {"isplaying", isplaying},
	  {"setwavedata", set_wave_data},
	  {"setquality", set_quality},
	  {"quality", set_quality},
	  { NULL, NULL }
  };
  luaL_newmetatable(L, "psyresamplermeta");
  lua_pushcclosure(L, gc, 0);
  lua_setfield(L,-2, "__gc");
  luaL_newlib(L, methods);  
  lua_pushnumber(L, 1);
  lua_setfield(L, -2, "ZEROHOLD");
  lua_pushnumber(L, 2);
  lua_setfield(L, -2, "LINEAR");
  lua_pushnumber(L, 3);
  lua_setfield(L, -2, "SINC");
  return 1;
}

int LuaResamplerBind::createwavetable(lua_State *L) {  
  int n = lua_gettop(L);  // Number of arguments
  if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 2 (self, wavetable)", n); 
  }
  RWInterface* rwrap = 0;
  WaveList<float>::Type waves = check_wavelist(L);
  rwrap = new ResamplerWrap<float, 1>(waves);
  assert(rwrap);
  LuaHelper::new_userdata<>(L, "psyresamplermeta", rwrap);
  return 1;
}

int LuaResamplerBind::create(lua_State *L) {  
  int n = lua_gettop(L);  // Number of arguments
  if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 2 (self, wave)", n); 
  }
  RWInterface* rwrap = 0;
  XMInstrument::WaveData<float>* wave = LuaHelper::check<XMInstrument::WaveData<float> >(L, 4, "psywavedatameta");
  WaveList<float>::Type waves;
  waves[range<double>(0, 96000)] = wave;
  rwrap = new ResamplerWrap<float, 1>(waves);
  assert(rwrap);
  LuaHelper::new_userdata<>(L, "psyresamplermeta", rwrap);
  return 1;
}

WaveList<float>::Type LuaResamplerBind::check_wavelist(lua_State* L) {
  WaveList<float>::Type waves;
  if (lua_istable(L, 2)) {
	size_t len = lua_rawlen(L, 2);
	for (size_t i = 1; i <= len; ++i) {
	   lua_rawgeti(L, 2, i); // get triple {Wave, flo, fhi}
	   lua_rawgeti(L, 3, 1); // GetWaveData
	   XMInstrument::WaveData<float>* wave = LuaHelper::check<XMInstrument::WaveData<float> >(L, 4, "psywavedatameta");
	   lua_pop(L,1);
	   lua_rawgeti(L, 3, 2); // GetFreqLo
	   double lo = luaL_checknumber(L, 4);
	   lua_pop(L,1);
	   lua_rawgeti(L, 3, 3); // GetFreqHi
	   double hi = luaL_checknumber(L, 4);
	   lua_pop(L,1);
	   lua_pop(L,1);
	   waves[range<double>( lo, hi )] = wave;
	}	
  }  
  return waves;
}

int LuaResamplerBind::set_wave_data(lua_State* L) {
  int n = lua_gettop(L);
  if (n != 2) {
	 return luaL_error(L, "Got %d arguments expected 2 (self, wavedata)", n); 
  }
  RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, "psyresamplermeta");
  WaveList<float>::Type waves = check_wavelist(L);
  rwrap->SetData(waves);
  return 0;
}

int LuaResamplerBind::work(lua_State* L) {
   int n = lua_gettop(L); 
   if (n!=3 and n != 4 and n!=5) {
     return luaL_error(L, "Got %d arguments expected 2 (self, left, right [,fm])", n); 
   }   
   RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, "psyresamplermeta");
   if (!rwrap->Playing()) {
	   lua_pushnumber(L, 0);
	   return 1;
   }
   PSArray* l = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
   PSArray* r = *(PSArray **)luaL_checkudata(L, 3, "array_meta");
   float* fm = 0;
   float* env = 0;
   if (n>3 && (!lua_isnil(L,3))) {
	    PSArray*arr = *(PSArray **)luaL_checkudata(L, 4, "array_meta");   
	    fm = arr->data();
   }
   if (n==5) {
	  PSArray* arr = *(PSArray **)luaL_checkudata(L, 5, "array_meta");
	  env = arr->data();
   }   
   int processed = rwrap->work(l->len(), l->data(), r->data(), fm, env);
   lua_pushnumber(L, processed); // The total number of samples resampled is returned
   return 1;
}

int LuaResamplerBind::isplaying(lua_State* L) {
  int n = lua_gettop(L); 
  if (n != 1) {
     return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
  }
  RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, "psyresamplermeta");
  lua_pushboolean(L, rwrap->Playing());
  return 1;
}

int LuaResamplerBind::noteoff(lua_State* L) {
   int n = lua_gettop(L); 
   if (n != 1) {
     return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
   }   
   RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, "psyresamplermeta");
   rwrap->NoteOff();
   return 0;
}

int LuaResamplerBind::start(lua_State* L) {   
  int n = lua_gettop(L);  // Number of arguments
  double phase = 0;
  if (n!=1 and n != 2) {
     return luaL_error(L, "Got %d arguments expected 1 or 2 (self [, phase])", n); 
  }
  if (n==2) {
    phase = luaL_checknumber (L, 2);
  } 
  RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, "psyresamplermeta");
  rwrap->Start(phase);
  return 0;
}

int LuaResamplerBind::set_frequency(lua_State* L) {
   int n = lua_gettop(L); 
   if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 2 (self, frequency)", n); 
   }   
   RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, "psyresamplermeta");
   double f = luaL_checknumber(L, 2);
   rwrap->set_frequency(f);
   return 0;
}

int LuaResamplerBind::set_quality(lua_State* L) {
   int n = lua_gettop(L); 
   if (n != 2) {
     return luaL_error(L, "Got %d arguments expected 2 (self, frequency)", n); 
   }   
   RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, "psyresamplermeta");
   int quality = luaL_checknumber(L, 2);
   rwrap->set_quality((psycle::helpers::dsp::resampler::quality::type)(quality-1));
   return 0;
}

int LuaResamplerBind::quality(lua_State* L) {
  int n = lua_gettop(L); 
  if (n != 1) {
     return luaL_error(L, "Got %d arguments expected 1 (self)", n); 
  }
  RWInterface* rwrap = LuaHelper::check<RWInterface>(L, 1, "psyresamplermeta");
  lua_pushnumber(L, (int)(rwrap->quality()+1));
  return 1;
}

int LuaResamplerBind::gc (lua_State *L) {
	RWInterface* ptr = *(RWInterface **)luaL_checkudata(L, 1, "psyresamplermeta");
	delete ptr;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Envelope
///////////////////////////////////////////////////////////////////////////////
void LEnvelope::work(int num) {
	out_.resize(num);
	out_.fill(m_);	
	if (is_playing() && (stage_ != sus_ || susdone_)) {
		int i = nexttime_ - sc_;
		while (i < num) {
			++stage_;
			if (stage_ > peaks_.size()-1 or (!susdone_ && stage_ == sus_)) {			   
			   out_.fillzero(i);
			   m_ = 0;
			   break;
			}
			calcstage(stage_, peaks_[stage_-1]);			
			out_.fill(m_, i);
			i = nexttime_;
		}
		sc_ += num;
	}
	out_.rsum(lv_);	
	lv_ = out_.get_val(num-1);		
}

void LEnvelope::release() {
   susdone_ = true;
   if (stage_ != sus_) {
     stage_ = peaks_.size()-1;
   } 
   calcstage(stage_, lv_);   
}

///////////////////////////////////////////////////////////////////////////////
// LuaEnvelopeBind
///////////////////////////////////////////////////////////////////////////////
int LuaEnvelopeBind::open(lua_State *L) {
  static const luaL_Reg methods[] = {
	  {"new", create},
	  {"newahdsr", createahdsr},
  	  {"work", work },
	  {"release", release },
	  {"isplaying", isplaying },
	  {"start", start },
	  {"setpeak", setpeak},
	  {"peak", peak},
	  {"settime", setstagetime},
	  {"time", time},
	  { "__tostring", tostring },
	  { NULL, NULL }
  };
  luaL_newmetatable(L, "psyenvelopemeta");
  lua_pushcclosure(L, gc, 0);
  lua_setfield(L,-2, "__gc");
  luaL_newlib(L, methods);    
  return 1;
}

int LuaEnvelopeBind::create(lua_State *L) {  
  int n = lua_gettop(L);  // Number of arguments
  if (n != 3) {
     return luaL_error(L, "Got %d arguments expected 3 (self, points, sustainpos", n); 
  }
  std::vector<double> times;
  std::vector<double> peaks;
  if (lua_istable(L, 2)) {
	size_t len = lua_rawlen(L, 2);
	for (size_t i = 1; i <= len; ++i) {
	   lua_rawgeti(L, 2, i); // get point {time, peak}
	   lua_rawgeti(L, 4, 1); // get time
	   double t = luaL_checknumber(L, 5);
	   times.push_back(t);
	   lua_pop(L,1);
	   lua_rawgeti(L, 4, 2); // get peak
	   peaks.push_back(luaL_checknumber(L, 5));
	   lua_pop(L,1);
	   lua_pop(L,1);
	}	
  }  
  int suspos = luaL_checknumber(L, 3)-1;
  double startpeak = 0;
  if (n==4) {
	 startpeak = luaL_checknumber(L, 4);
  }
  LEnvelope* ud = new LEnvelope(times, peaks, suspos, startpeak, Global::player().SampleRate());
  LuaHelper::new_userdata<LEnvelope>(L, "psyenvelopemeta", ud);
  return 1;
}

int LuaEnvelopeBind::createahdsr(lua_State *L) {  
  int n = lua_gettop(L);  // Number of arguments
  if (n != 6) {
     return luaL_error(L, "Got %d arguments expected 6 (self, a, h, d, s, r", n); 
  }
  std::vector<double> times;
  std::vector<double> peaks;
  times.push_back(luaL_checknumber(L, 2)); // a
  times.push_back(luaL_checknumber(L, 3)); // h
  times.push_back(luaL_checknumber(L, 4)); // d
  times.push_back(luaL_checknumber(L, 6)); // r
  peaks.push_back(1);
  peaks.push_back(1);
  peaks.push_back(luaL_checknumber(L, 5)); // s
  peaks.push_back(0);  
  LEnvelope* ud = new LEnvelope(times, peaks, 3, 0, Global::player().SampleRate());
  LuaHelper::new_userdata<LEnvelope>(L, "psyenvelopemeta", ud);
  return 1;
}

int LuaEnvelopeBind::work(lua_State* L) {
    int n = lua_gettop(L);
	if (n ==2) {
	   LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta");
       assert(env != 0);	   
	   int num = luaL_checknumber(L, 2);
	   env->work(num);
	   PSArray ** rv = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));
	   luaL_setmetatable(L, "array_meta");
	   *rv = new PSArray(env->out().data(), num);
	}  else {
       luaL_error(L, "Got %d arguments expected 2 (self, num)", n); 
	}
   return 1;
}

int LuaEnvelopeBind::isplaying(lua_State *L) {
	LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta");
	lua_pushboolean(L, env->is_playing());
	return 1;
}

int LuaEnvelopeBind::release(lua_State *L) {
	LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta");
	env->release();
	return 0;
}

int LuaEnvelopeBind::start(lua_State *L) {
	LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta");
	env->start();
	return 0;
}

int LuaEnvelopeBind::setpeak(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==3) {
	   LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta");
       assert(env != 0);	   
	   int idx = luaL_checknumber(L, 2);
	   double peak = luaL_checknumber(L, 3);
	   env->setstagepeak(idx-1, peak);
	} else {
       luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
	}
	return 0;
}

int LuaEnvelopeBind::peak(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==2) {
	   LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta");
       assert(env != 0);	   
	   int idx = luaL_checknumber(L, 2)-1;
	   lua_pushnumber(L, env->peak(idx));
	} else {
       luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
	}
	return 1;
}

int LuaEnvelopeBind::time(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==2) {
	   LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta");
       assert(env != 0);	   
	   int idx = luaL_checknumber(L, 2)-1;
	   lua_pushnumber(L, env->time(idx));
	} else {
       luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
	}
	return 1;
}

int LuaEnvelopeBind::setstagetime(lua_State* L) {
	int n = lua_gettop(L);
	if (n ==3) {
	   LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta");
       assert(env != 0);	   
	   int idx = luaL_checknumber(L, 2);
	   double time = luaL_checknumber(L, 3);
	   env->setstagetime(idx-1, time);
	} else {
       luaL_error(L, "Got %d arguments expected 2 (self, index)", n); 
	}
	return 0;
}

int LuaEnvelopeBind::gc (lua_State *L) {
	LEnvelope* ptr = *(LEnvelope **)luaL_checkudata(L, 1, "psyenvelopemeta");
	delete ptr;	
	return 0;
}

int LuaEnvelopeBind::tostring(lua_State *L) {
	LEnvelope* env = LuaHelper::check<LEnvelope>(L, 1, "psyenvelopemeta"); 
	lua_pushfstring(L, "test");
	return 1;
}


} // namespace
} // namespace
