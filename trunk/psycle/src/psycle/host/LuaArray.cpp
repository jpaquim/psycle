#include <psycle/host/detail/project.hpp>
#include "lua.hpp"
#include "LuaArray.hpp"

#include <lua.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace psycle { namespace host {

// for psycle samples

PSArray::PSArray(int len, float v) : len_(len), cap_(len), shared_(0) {  
  universalis::os::aligned_memory_alloc(16, ptr_, len);
  if (v == 0) {
     psycle::helpers::dsp::Clear(ptr_, len_);
  } else {		
	for (int i=0; i < len; ++i) ptr_[i] = v;
  }
}

PSArray::PSArray(double start, double stop, double step) 
	:  shared_(0) {
		len_ = (stop-start+0.5)/step;
		cap_ = len_;
	    universalis::os::aligned_memory_alloc(16, ptr_, len_); // reserve		
        int count = 0;
		for (double i=start; i < stop; i+=step, ++count)
			ptr_[count] = i;
}   

PSArray::PSArray(PSArray& a1, PSArray& a2) : shared_(0) {
	len_ = a1.len() + a2.len();
	cap_ = len_;
	universalis::os::aligned_memory_alloc(16, ptr_, len_); // reserve
	for (int i=0; i < a1.len_; ++i) ptr_[i] = a1.ptr_[i];
	for (int i=0; i < a2.len_; ++i) ptr_[i+a1.len_] = a2.ptr_[i];	
}

void PSArray::resize(int newsize) {
  if (!shared_ && cap_ < newsize) {	    
     //ptr_ = (float*) _aligned_realloc(ptr_, newsize, 16);
	 universalis::os::aligned_memory_dealloc(ptr_);
	 universalis::os::aligned_memory_alloc(16, ptr_, newsize);
	 cap_ = newsize;
  }
  len_ = newsize;
}

int PSArray::copyfrom(PSArray& src) {
    if (src.len() != len_) {
		return 0;
	}
	psycle::helpers::dsp::Mov(src.data(), ptr_, len_);
	return 1;
}

void PSArray::fillzero() {
   psycle::helpers::dsp::Clear(ptr_, len_);
}

void PSArray::fill(float val) {
   for (int i = 0; i < len_; ++i) {
	  ptr_[i] = val;
   }
}

int PSArray::copyfrom(PSArray& src, int pos) {
    //if ( src.len() + pos > len_)
	//	return 0;
    for (int i = 0; i < src.len() && i+pos < len_; ++i) {
		ptr_[i+pos] = src.get_val(i);
	}
	return 1;
}

std::string PSArray::tostring() const {
	std::stringstream res; 
	res << "[ ";
	for (int i=0; i < len_; ++i)
		res << ptr_[i] << " ";
	res << "]";
	return res.str();
}

template<class T>
void PSArray::do_op(T& func) {
	if (len_ > 0) {
	  func.p = ptr_;
	  for (int i=0; i < len_; ++i) ptr_[i] = func(i);
	}
}

// delay
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


std::map<lua_State*, LuaArrayBind*> LuaArrayBind::map_;

int LuaArrayBind::array_new(lua_State *L) {
	// PSArray* v = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
	int n = lua_gettop(L);
	PSArray ** udata = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));	
	luaL_setmetatable(L, "array_meta");
	int size;
	double val;
	switch (n) {
	  case 1 :
		  *udata = new PSArray();
		  break;
	  case 2 : {
		  if (lua_istable(L, 2)) {
			 std::vector<float> v;
	         for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
  	            v.push_back(luaL_checknumber(L, -1));
	         }
			 *udata = new PSArray(v.size(), 0);
			 std::memcpy((*udata)->data(), &v[0], v.size());
		  } else {
		    size = luaL_checknumber (L, 2);
		    *udata = new PSArray(size, 0);
		  }
		  }
		  break;
	  case 3:
		  size = luaL_checknumber (L, 2);
		  val = luaL_checknumber (L, 3);
		  *udata = new PSArray(size, val);
		  break;
      default:;
    }
	return 1;
}

int LuaArrayBind::delay_new(lua_State *L) {
   int k = luaL_checknumber (L, 1);
   luaL_argcheck(L, k >= 0, 2, "index is negativ");
   PSDelay ** ud = (PSDelay **)lua_newuserdata(L, sizeof(PSDelay *));
   *ud = new PSDelay(k);
   luaL_getmetatable(L, "delay_meta");
   lua_setmetatable(L, -2);
   return 1;
}

int LuaArrayBind::delay_work(lua_State* L) {
   PSDelay* ud = *(PSDelay **)luaL_checkudata(L, 1, "delay_meta");
   PSArray* x = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
   PSArray** y = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));
   *y = new PSArray(x->len(), 0);
   luaL_setmetatable(L, "array_meta");
   ud->work(*x, **y);
   return 1;
}

int LuaArrayBind::array_new_from_sampleV(lua_State* L) {
	int idx = luaL_checknumber (L, 1);
	PSArray ** udata = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));
	std::map<lua_State*, LuaArrayBind*>::iterator it;
	LuaArrayBind* binder = map_.find(L)->second;
	*udata = new PSArray(binder->sampleV_[idx].data(), binder->sampleV_[idx].len());
	luaL_getmetatable(L, "array_meta");
	lua_setmetatable(L, -2);
	return 1;
}

int LuaArrayBind::array_copy(lua_State* L) {  
	int n = lua_gettop(L);
	PSArray* dest = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
	if (n==3) {
	   int pos = luaL_checknumber (L, 2);
	   PSArray* src = *(PSArray **)luaL_checkudata(L, 3, "array_meta");	
	   dest->copyfrom(*src, pos);	   
	} else {
	   PSArray* src = *(PSArray **)luaL_checkudata(L, 2, "array_meta");		 
	   std::ostringstream o;
	   o << "size src:" << src->len() << ", dst:" << dest->len() << "not compatible";
	   luaL_argcheck(L, dest->copyfrom(*src), 2, o.str().c_str());	
	}
	return 1;
}

int LuaArrayBind::array_arange(lua_State* L) {	
    float start = luaL_checknumber (L, 2);
	float stop = luaL_checknumber (L, 3);
	float step = luaL_checknumber (L, 4);
	PSArray ** udata = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));	
	*udata = new PSArray(start, stop, step);
	luaL_setmetatable(L, "array_meta");	
	return 1;
}

int LuaArrayBind::array_random(lua_State* L) {
	int size = luaL_checknumber (L, 1);
    PSArray ** ud = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));	
	*ud = new PSArray(size, 1);
	struct {float* p; float operator()(int y) {
		return p[y] = (lua_Number)(rand()%RAND_MAX) / (lua_Number)RAND_MAX; }
	} f;
	(*ud)->do_op(f);
	luaL_setmetatable(L, "array_meta");	
	return 1;
}

int LuaArrayBind::array_gc (lua_State *L) {
	PSArray* ptr = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
	delete ptr;	
	return 0;
}

int LuaArrayBind::delay_gc (lua_State *L) {
	PSDelay* ptr = *(PSDelay **)luaL_checkudata(L, 1, "delay_meta");
	delete ptr;	
	return 0;
}

PSArray* LuaArrayBind::create_copy_array(lua_State* L, int idx) {
	PSArray* udata = *(PSArray **)luaL_checkudata(L, idx, "array_meta");
	PSArray ** ret_datum = (PSArray **)lua_newuserdata(L, sizeof(PSArray*));
    *ret_datum = new PSArray(udata->len(), 0);
	(*ret_datum)->copyfrom(*udata);
	luaL_setmetatable(L, "array_meta");
	return (*ret_datum);
}

int LuaArrayBind::array_method_add(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {return p[y]+v[y];}} f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; double c; float operator()(int y) {return p[y]+c;}} f;
	 f.c = luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   return 1;
}

int LuaArrayBind::array_method_mul(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {return p[y]*v[y];}} f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; double c; float operator()(int y) {return p[y]*c;}} f;
	 f.c = luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   return 1;
}

// ops
int LuaArrayBind::array_add(lua_State* L) {
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		PSArray* rv = create_copy_array(L);
	    PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
		luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
		struct {float* p; float* v; float operator()(int y) {return p[y]+v[y];}} f;
		f.v = v->data();
		rv->do_op(f);
	} else {
	   double param1 = 0;
	   PSArray* rv = 0;
	   if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
	     param1 = luaL_checknumber (L, 2);
	     rv = create_copy_array(L);
	   } else {
		 param1 = luaL_checknumber (L, 1);
		 rv = create_copy_array(L, 2);
	   }	 
	   struct {float* p; double c; float operator()(int y) {return p[y]+c;}} f;
	   f.c = param1;
	   rv->do_op(f);
	}
    return 1;
}

int LuaArrayBind::array_sub(lua_State* L) {
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		PSArray* rv = create_copy_array(L);
	    PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
		luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
		struct {float* p; float* v; float operator()(int y) {return p[y]-v[y];}} f;
		f.v = v->data();
		rv->do_op(f);
	} else {
	   float param1 = 0;
	   PSArray* rv = 0;
	   if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
	     param1 = luaL_checknumber (L, 2);
	     rv = create_copy_array(L);
	   } else {
		 param1 = luaL_checknumber (L, 1);
		 rv = create_copy_array(L, 2);
	   }	 
	   struct {float* p; float c; float operator()(int y) {return p[y]-c;}} f;
	   f.c = param1;
	   rv->do_op(f);
	}
    return 1;
}

int LuaArrayBind::array_sum(lua_State* L) {
	 luaL_checktype(L, 1, LUA_TTABLE);
	 struct {float* p; float* v; float operator()(int y) {return p[y]+v[y];}} f;
	 PSArray* dest = 0;
	 size_t len;
	 for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
  	     PSArray* v = *(PSArray **)luaL_checkudata(L, -1, "array_meta");
		 float* data = v->data();
		 if (!dest) {
			dest = new PSArray(v->len(), 0);
		 }
		 luaL_argcheck(L, dest->len() == v->len(), 2, "size not compatible");
		 f.v = v->data();
		 dest->do_op(f);
	}
    if (!dest)
		luaL_error(L, "no input arrays");
	PSArray ** ret_datum = (PSArray **)lua_newuserdata(L, sizeof(PSArray*));
	luaL_setmetatable(L, "array_meta");
	*ret_datum = dest;
	return 1;
}

int LuaArrayBind::array_rsum(lua_State* L) {
   PSArray* v = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   PSArray ** rv = (PSArray **)lua_newuserdata(L, sizeof(PSArray*));
   *rv = new PSArray(v->len(), 0);
   luaL_setmetatable(L, "array_meta");
   double sum = 0;
   for (int i = 0; i < v->len(); ++i) {
	  sum = sum + v->get_val(i);
	  (*rv)->set_val(i, sum);
   }
   return 1;
}

int LuaArrayBind::array_method_rsum(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");   
   double sum = 0;
   for (int i = 0; i < rv->len(); ++i) {
	  sum = sum + rv->get_val(i);
	  rv->set_val(i, sum);
   }
   return 1;
}

int LuaArrayBind::array_mul(lua_State* L) {
	if ((lua_isuserdata(L, 1)) && (lua_isuserdata(L, 2))) {
		PSArray* rv = create_copy_array(L);
	    PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
		luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
		struct {float* p; float* v; float operator()(int y) {return p[y]*v[y];}} f;
		f.v = v->data();
		rv->do_op(f);
	} else {
	   float param1 = 0;
	   PSArray* rv = 0;
	   if ((lua_isuserdata(L, 1)) && (lua_isnumber(L, 2))) {
	     param1 = luaL_checknumber (L, 2);
	     rv = create_copy_array(L);
	   } else {
		 param1 = luaL_checknumber (L, 1);
		 rv = create_copy_array(L, 2);
	   }	 
	   struct {float* p; float c; float operator()(int y) {return p[y]*c;}} f;
	   f.c = param1;
	   rv->do_op(f);
	}
    return 1;
}

int LuaArrayBind::array_sin(lua_State* L) {		
	PSArray* rv = create_copy_array(L);	
	if (rv) {
	  struct {float* p; float operator()(int y) {return ::sin(p[y]);}} f;	
	  rv->do_op(f);
	}
	return 1;
}

int LuaArrayBind::array_cos(lua_State* L) {		
	PSArray* rv = create_copy_array(L);	
	if (rv) {
	  struct {float* p; float operator()(int y) {return ::cos(p[y]);}} f;	
	  rv->do_op(f);
	}
	return 1;
}

int LuaArrayBind::array_tan(lua_State* L) {		
	PSArray* rv = create_copy_array(L);	
	if (rv) {
	  struct {float* p; float operator()(int y) {return ::tan(p[y]);}} f;	
	  rv->do_op(f);
	}
	return 1;
}

int LuaArrayBind::array_pow(lua_State* L) {		
	double exp = luaL_checknumber (L, 2);
	PSArray* rv = create_copy_array(L);	
	if (rv) {
	  struct {float* p; double exp; float operator()(int y) {return ::pow((double)p[y], exp);}} f;	
	  rv->do_op(f);
	}
	return 1;
}

int LuaArrayBind::array_sqrt(lua_State* L) {		
	PSArray* rv = create_copy_array(L);	
	if (rv) {
	  struct {float* p; float operator()(int y) {return ::sqrt(p[y]);}} f;	
	  rv->do_op(f);
	}
	return 1;
}

int LuaArrayBind::array_size(lua_State* L) {
  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");   
  lua_pushnumber(L, (*ud)->len());
  return 1;
}

int LuaArrayBind::array_resize(lua_State* L) {
  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");   
  (*ud)->resize(luaL_checknumber (L, 2));
  return 0;
}

int LuaArrayBind::array_fillzero(lua_State* L) {
  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
  (*ud)->fillzero();
  return 0;
}

int LuaArrayBind::array_method_fill(lua_State* L) {
  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
  (*ud)->fill(luaL_checknumber(L, 2));
  return 0;
}

int LuaArrayBind::array_concat(lua_State* L) {
  PSArray* v1 = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
  PSArray* v2 = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
  PSArray ** rv = (PSArray **)lua_newuserdata(L, sizeof(PSArray*));
  *rv = new PSArray(*v1, *v2);
  luaL_setmetatable(L, "array_meta");
  return 1;
}

int LuaArrayBind::array_tostring(lua_State *L) {
	PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");   
	lua_pushfstring(L, (*ud)->tostring().c_str());
	return 1;
}

int LuaArrayBind::delay_tostring(lua_State *L) {
	PSDelay** ud = (PSDelay**) luaL_checkudata(L, 1, "delay_meta");   
	lua_pushfstring(L, "test");
	return 1;
}

int LuaArrayBind::array_index(lua_State *L) {
	int index = 0;
	if (lua_isnumber(L, 2)) {
	  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
	  int index = luaL_checknumber(L, 2);
	  std::ostringstream o;
	  o << "index" << index << " out of range (max)" << (*ud)->len();
	  luaL_argcheck(L, 0 <= index && index < (*ud)->len(), 2,
                       o.str().c_str());
	  lua_pushnumber(L, (*ud)->get_val(index));
	  return 1;
	} else 
	if (lua_istable(L, 2)) {
	  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
	  std::vector<int> p;
	  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
  	     p.push_back(luaL_checknumber(L, -1));
	  }
	  PSArray ** rv = (PSArray **)lua_newuserdata(L, sizeof(PSArray*));
	  if (p[1]-p[0] == 0) {
		  *rv = new PSArray();
	  } else {
		std::ostringstream o;
	    o << "index" << p[0] << "," << p[1] << "," << " out of range (max)" << (*ud)->len();
	    luaL_argcheck(L, (0 <= p[0] && p[0] <= (*ud)->len()) &&
					     (0 <= p[1] && p[1] <= (*ud)->len()) &&
					     p[0] < p[1], 2, o.str().c_str());	    
	    *rv = new PSArray(p[1]-p[0], 0);        
	    struct {float* p; float* v; int s; float operator()(int y) {return v[y+s];}} f;
	    f.s = p[0];
	    f.v = (*ud)->data();
  	    (*rv)->do_op(f);
	  }
	  luaL_setmetatable(L, "array_meta");
	  return 1;
	} else {
	  size_t len;
	  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
	  const char* key = luaL_checklstring(L, 2, &len);	
	  lua_getglobal(L, "array_methods");
	  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
	     const char* method = luaL_checklstring(L, -2, &len);
	     int result = strcmp(key, method);
	     if (result == 0) {
		   return 1;
	     }
	  }	
	}
	// no method found	
	return 0;
}

int LuaArrayBind::array_new_index(lua_State *L) {
	int index = 0;
	if (lua_isnumber(L, 2)) {
	  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
	  int index = luaL_checknumber(L, 2);
	  float value = luaL_checknumber(L, 3);
	  luaL_argcheck(L, 0 <= index && index < (*ud)->len(), 2,
                       "index out of range");
	  (*ud)->set_val(index, value);
	  return 0;
	} else {
	  //error
	}
}

void LuaArrayBind::export_c_funcs(lua_State* L) {
	static const luaL_Reg pm_lib[] = {
		{ "delay", &LuaArrayBind::delay_new },
		{ "new", &LuaArrayBind::array_new },
		{ "channel", &array_new_from_sampleV},
		{ "arange", &LuaArrayBind::array_arange },
		{ "random", &LuaArrayBind::array_random },
		{ "sin", &LuaArrayBind::array_sin },
		{ "cos", &LuaArrayBind::array_cos },
		{ "tan", &LuaArrayBind::array_tan },
		{ "sqrt", &LuaArrayBind::array_sqrt },
		{ "sum", &LuaArrayBind::array_sum },
		{ "rsum", &LuaArrayBind::array_rsum },
		{ "pow", &LuaArrayBind::array_pow },
		{ NULL, NULL }
	};
	static const luaL_Reg pm_methods[] = {
		{ "add", &LuaArrayBind::array_method_add},
		{ "mul", &LuaArrayBind::array_method_mul},
		{ "rsum", &LuaArrayBind::array_method_rsum},
		{ "size", &LuaArrayBind::array_size},
		{ "resize", &LuaArrayBind::array_resize},
		{ "copy", &LuaArrayBind::array_copy},
		{ "fillzero", &LuaArrayBind::array_fillzero},
		{ "fill", &LuaArrayBind::array_method_fill},
		{ "tostring", &LuaArrayBind::array_tostring },
		{ NULL, NULL }
	}; 
	static const luaL_Reg delay_meta[] = {
		{ "work", &LuaArrayBind::delay_work},
		{ "__tostring", &LuaArrayBind::delay_tostring },
		{ "__gc", &LuaArrayBind::delay_gc },
		{ NULL, NULL }
	}; 
	static const luaL_Reg pm_meta[] = {
		{ "__index", &LuaArrayBind::array_index },
		{ "__newindex", &LuaArrayBind::array_new_index },
		{ "__index", &LuaArrayBind::array_index },
		{ "__gc", &LuaArrayBind::array_gc },
		{ "__tostring", &LuaArrayBind::array_tostring },
		{ "__add", &LuaArrayBind::array_add },
		{ "__sub", &LuaArrayBind::array_sub },
		{ "__mul", &LuaArrayBind::array_mul },
		{ "__concat", &LuaArrayBind::array_concat },		
		{ NULL, NULL }
	};
	luaL_newlib(L, pm_lib);
	lua_setglobal(L, "psynum");
	luaL_newlib(L, pm_methods);
	lua_setglobal(L, "array_methods");
	luaL_newmetatable(L, "array_meta");
	luaL_setfuncs(L, pm_meta, 0);    
	lua_pop(L,1);
	luaL_newmetatable(L, "delay_meta");
	lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3); // metatable.__index = metatable
    luaL_setfuncs(L, delay_meta, 0);
	lua_pop(L,1);
}


}
}