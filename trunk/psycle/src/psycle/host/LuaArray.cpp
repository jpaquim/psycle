#include <psycle/host/detail/project.hpp>
#include "LuaArray.hpp"

#include <lua.hpp>

#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>


namespace psycle { namespace host {

const int PSArray::multiplier = 4;

PSArray::PSArray(int len, float v) : len_(len),
	                                 baselen_(len_),
									 cap_(len),
									 shared_(0),
                                     can_aligned_(!(len % multiplier))
									 {
  universalis::os::aligned_memory_alloc(16, ptr_, len);
  if (can_aligned_) {
     psycle::helpers::dsp::Clear(ptr_, len_);
  } else {		
 	 for (int i=0; i < len; ++i) ptr_[i] = v;
  }
  base_ = ptr_;
}

PSArray::PSArray(double start, double stop, double step) 
	:  shared_(0) {
		baselen_ = len_ = (stop-start+0.5)/step;
		cap_ = len_;
	    universalis::os::aligned_memory_alloc(16, ptr_, len_); // reserve		
		base_ = ptr_;
        int count = 0;
		for (double i=start; i < stop; i+=step, ++count)
			ptr_[count] = i;
		can_aligned_ = !(count % multiplier);
}   

PSArray::PSArray(PSArray& a1, PSArray& a2) : shared_(0) {
	baselen_ = len_ = a1.len() + a2.len();
	cap_ = len_;
	universalis::os::aligned_memory_alloc(16, ptr_, len_); // reserve
    base_ = ptr_;
	for (int i=0; i < a1.len_; ++i) ptr_[i] = a1.ptr_[i];
	for (int i=0; i < a2.len_; ++i) ptr_[i+a1.len_] = a2.ptr_[i];
	can_aligned_ = !(len_ % multiplier);
}

void PSArray::resize(int newsize) {
  if (!shared_ && cap_ < newsize) {	    
     //ptr_ = (float*) _aligned_realloc(ptr_, newsize, 16);
	 std::vector<float> buf(base_, base_+baselen_);
	 universalis::os::aligned_memory_dealloc(base_);
	 universalis::os::aligned_memory_alloc(16, ptr_, newsize);
	 for (int i = 0; i < baselen_; ++i) ptr_[i]=buf[i];
	 base_ = ptr_;
	 cap_ = newsize;	 
  }
  baselen_ = len_ = newsize;
  can_aligned_ = !(newsize % multiplier);
}

int PSArray::copyfrom(PSArray& src) {
    if (src.len() != len_) {
		return 0;
	}
	for (int i=0; i < len_; ++i) {
		  ptr_[i] = src.ptr_[i];
	}
	// TODO: sse optimizations don't work! probably because it requires len to be multiple of 4 too.
	//psycle::helpers::dsp::Mov(src.data(), ptr_, len_);
	return 1;
}


void PSArray::fillzero() {
  if (base_ == ptr_ && can_aligned_) {
      // try sse2
	 psycle::helpers::dsp::Clear(ptr_, len_);
  } else {
     fill(0, 0);
  }
}

void PSArray::fillzero(int pos) {
	//Cannot use dsp::Clear. pos is not guaranteed to be aligned
   //psycle::helpers::dsp::Clear(ptr_+pos, len_-pos);
   fill(0,pos);
}

void PSArray::fill(float val) {
   // todo sse2
   for (int i = 0; i < len_; ++i) {
	  ptr_[i] = val;
   }
}

void PSArray::fill(float val, int pos) {
   int num = len_ - pos;
   for (int i = pos; i < num; ++i) {
	  ptr_[i] = val;
   }
}

void PSArray::mul(float multi) {
  if (can_aligned_) {
	  psycle::helpers::dsp::Mul(ptr_, len_, multi);
  } else {
    for (int i = 0; i < len_; ++i) { ptr_[i] *= multi; }
  }
}

void PSArray::mix(PSArray& src, float multi) {
  float* srcf = src.data();
  if (can_aligned_) {
	 psycle::helpers::dsp::Add(srcf, ptr_, len_, multi);
  } else {
	for(int i = 0; i < len_; ++i) ptr_[i] += srcf[i] * multi;    
  }
}

void PSArray::rsum(double lv) {
  double sum = lv;
  for (int i = 0; i < len(); ++i) {
	  sum = sum + get_val(i);
	  set_val(i, sum);
   }
}

int PSArray::copyfrom(PSArray& src, int pos) {	
    if (pos < 0)
	  return 0;
	if (pos==0 && src.canaligned()) {		
		psycle::helpers::dsp::Mov(src.data(), ptr_, src.len());
	} else {
       for (int i = 0; i < src.len() && i+pos < len_; ++i) {
		  ptr_[i+pos] = src.get_val(i);
	  }
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

void LuaArrayBind::register_module(lua_State* L) {
  luaL_requiref(L, "psycle.array", open_array, 1);
  lua_pop(L, 1);
}

int LuaArrayBind::array_new(lua_State *L) {
	int n = lua_gettop(L);
	PSArray ** udata = (PSArray **)lua_newuserdata(L, sizeof(PSArray *));	
	luaL_setmetatable(L, "array_meta");
	int size;
	double val;
	switch (n) {
	  case 0 :
		  *udata = new PSArray();
		  break;
	  case 1 : {
		  if (lua_istable(L, 1)) {
			 std::vector<float> v;
	         for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
  	            v.push_back(luaL_checknumber(L, -1));
	         }
			 *udata = new PSArray(v.size(), 0);
			 std::memcpy((*udata)->data(), &v[0], v.size());
		  } else {
		    size = luaL_checknumber (L, 1);
		    *udata = new PSArray(size, 0);
		  }
		  }
		  break;
	  case 2:
		  size = luaL_checknumber (L, 1);
		  val = luaL_checknumber (L, 2);
		  *udata = new PSArray(size, val);
		  break;
      default:
		  luaL_error(L, "Got %d arguments expected max 2 (none, size, size+val)", n);
	  ;
    }
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
	lua_pushvalue(L, 1); // return array to enable chaining
	return 1;
}

int LuaArrayBind::array_arange(lua_State* L) {	
    float start = luaL_checknumber (L, 1);
	float stop = luaL_checknumber (L, 2);
	float step = luaL_checknumber (L, 3);
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
	assert(ptr);
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
	 lua_pushvalue(L, 1);
   } else {
	 struct {float* p; double c; float operator()(int y) {return p[y]+c;}} f;
	 f.c = luaL_checknumber (L, 2);
	 rv->do_op(f);
	 lua_pushvalue(L, 1);
   }
   return 1;
}

int LuaArrayBind::array_method_mix(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
   rv->mix(*v, luaL_checknumber (L, 3));
   lua_pushvalue(L, 1); // chaining
   return 1;
}


int LuaArrayBind::array_method_random(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {return (lua_Number)(rand()%RAND_MAX) / (lua_Number)RAND_MAX;;}} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_sin(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {return sin(p[y]);}} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_cos(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {return cos(p[y]);}} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_tan(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {return tan(p[y]);}} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_sqrt(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {return sqrt(p[y]);}} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_floor(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {return floor(p[y]);}} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_ceil(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {return ceil(p[y]);}} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_abs(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {return abs(p[y]);}} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_sgn(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {
	 return (p[y] > 0) ? 1 : ((p[y] < 0) ? -1 : 0);
   }} f;   
   rv->do_op(f);
   lua_pushvalue(L, 1);
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
	 //struct {float* p; double c; float operator()(int y) {return p[y]*c;}} f;
	 //f.c = luaL_checknumber (L, 2);
	 rv->mul(luaL_checknumber (L, 2)); //do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_min(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {return std::min(p[y],v[y]);}} f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; float c; float operator()(int y) {return std::min(p[y],c);}} f;
	 f.c = luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_max(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {return std::max(p[y],v[y]);}} f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; float c; float operator()(int y) {return std::max(p[y],c);}} f;
	 f.c = luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_div(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {return p[y]/v[y];}} f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; double c; float operator()(int y) {return p[y]/c;}} f;
	 f.c = luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_and(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {
		 return static_cast<int>(p[y]) & (int)v[y];}
	 } f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; int c; float operator()(int y) {
		 return static_cast<int>(p[y]) & c;}
	 } f;
	 f.c = (int) luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
   }


int LuaArrayBind::array_method_or(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {
		 return static_cast<int>(p[y]) | (int)v[y];}
	 } f;
     f.v = v->data();
     rv->do_op(f);	
   } else {
	 struct {float* p; int c; float operator()(int y) {
		 return static_cast<int>(p[y]) | c;}
	 } f;
	 f.c = (int) luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_xor(lua_State* L) {
    PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {
		 return static_cast<int>(p[y]) ^ (int)v[y];}
	 } f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; int c; float operator()(int y) {
		 return static_cast<int>(p[y]) ^ c;}
	 } f;
	 f.c = (int) luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_sleft(lua_State* L) {
    PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {
		 return static_cast<int>(p[y]) << (int)v[y];}
	 } f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; int c; float operator()(int y) {
		 return static_cast<int>(p[y]) << c;}
	 } f;
	 f.c = (int) luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_sright(lua_State* L) {
    PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   if (lua_isuserdata(L, 2)) {
     PSArray* v = *(PSArray **)luaL_checkudata(L, 2, "array_meta");
     luaL_argcheck(L, rv->len() == v->len(), 2, "size not compatible");
     struct {float* p; float* v; float operator()(int y) {
		 return static_cast<int>(p[y]) >> (int)v[y];}
	 } f;
     f.v = v->data();
     rv->do_op(f);
   } else {
	 struct {float* p; int c; float operator()(int y) {
		 return static_cast<int>(p[y]) >> c;}
	 } f;
	 f.c = (int) luaL_checknumber (L, 2);
	 rv->do_op(f);
   }
   lua_pushvalue(L, 1);
   return 1;
}

int LuaArrayBind::array_method_bnot(lua_State* L) {
   PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");
   struct {float* p; float operator()(int y) {
	 return ~static_cast<unsigned int>(p[y]);}
   } f;
   rv->do_op(f);
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
   int n = lua_gettop(L);   
   double sum = 0;
   if (n==2) {
	   sum = luaL_checknumber(L, 2);
   }
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
  PSArray* rv = *(PSArray **)luaL_checkudata(L, 1, "array_meta");   
  rv->resize(luaL_checknumber (L, 2));
  lua_pushvalue(L, 1); // return array to enable chaining
  return 1;
}

int LuaArrayBind::array_fillzero(lua_State* L) {  
  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
  (*ud)->fillzero();
  lua_pushvalue(L, 1); // return array to enable chaining
  return 1;
}

int LuaArrayBind::array_method_fill(lua_State* L) {
  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
  (*ud)->fill(luaL_checknumber(L, 2));
  lua_pushvalue(L, 1); // return array to enable chaining
  return 1;
}

int LuaArrayBind::array_margin(lua_State* L) {
  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
  // todo range checks
  int start = luaL_checknumber(L, 2);
  int stop = luaL_checknumber(L, 3);
  (*ud)->margin(start, stop);
  lua_pushvalue(L, 1); // return array to enable chaining
  return 1;
}

int LuaArrayBind::array_clearmargin(lua_State* L) {
  PSArray** ud = (PSArray**) luaL_checkudata(L, 1, "array_meta");
  (*ud)->clearmargin();
  lua_pushvalue(L, 1); // return array to enable chaining
  return 1;
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
	  lua_getmetatable(L, 1);
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
	  lua_error(L);
	  return 0;
	}
}

int LuaArrayBind::open_array(lua_State* L) {
	static const luaL_Reg pm_lib[] = {
		{ "new", array_new },
		{ "arange", array_arange },
		{ "random", array_random },
		{ "sin", array_sin },
		{ "cos", array_cos },
		{ "tan", array_tan },
		{ "sqrt", array_sqrt },
		{ "sum", array_sum },
		{ "rsum", array_rsum },
		{ "pow", array_pow },
		{ NULL, NULL }
	};
	static const luaL_Reg pm_meta[] = {
		{ "random", array_method_random},
		{ "sin", array_method_sin},
		{ "cos", array_method_cos },
		{ "tan", array_method_tan },
		{ "sqrt", array_method_sqrt},
		{ "add", array_method_add},
		{ "mix", array_method_mix},
		{ "mul", array_method_mul},
		{ "div", array_method_div},
		{ "rsum", array_method_rsum},
		{ "floor", array_method_floor},
		{ "abs", array_method_abs},
		{ "sgn", array_method_sgn},
		{ "ceil", array_method_ceil},
		{ "max", array_method_max},
		{ "min", array_method_min},
		{ "band", array_method_and},
		{ "bor", array_method_or},
		{ "bxor", array_method_xor},
		{ "bleft", array_method_sleft},
		{ "bright", array_method_sright},
		{ "bnot", array_method_bnot},
		{ "size", array_size},
		{ "resize", array_resize},
		{ "copy", array_copy},
		{ "fillzero", array_fillzero},
		{ "fill", array_method_fill},
		{ "tostring", array_tostring },
		{ "margin", array_margin},
		{ "clearmargin", array_clearmargin },
		{ "__index", array_index },
		{ "__newindex", array_new_index },
		{ "__index", array_index },
		{ "__gc", array_gc },
		{ "__tostring", array_tostring },
		{ "__add", array_add },
		{ "__sub", array_sub },
		{ "__mul", array_mul },
		{ "__concat", array_concat },		
		{ NULL, NULL }
	};
	luaL_newmetatable(L, "array_meta");
	luaL_setfuncs(L, pm_meta, 0);    
	lua_pop(L,1);
    luaL_newlib(L, pm_lib);	
	return 1;
}


}  // namespace
}  // namespace
