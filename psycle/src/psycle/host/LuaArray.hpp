#pragma once
#include <psycle/host/detail/project.hpp>
#include <vector>
#include <map>
#include <string>
#include <universalis/os/aligned_alloc.hpp>
#include <psycle/helpers/value_mapper.hpp>
#include <psycle/helpers/dsp.hpp>

struct lua_State;

namespace psycle { namespace host {

// array wrapper (shared : float*, created: vector<float>}
class PSArray {
public:
	PSArray() : ptr_(0), len_(0), shared_(0) {}
	PSArray(int len, float v);
	PSArray(double start, double stop, double step);
	PSArray(float* ptr, int len) : ptr_(ptr), len_(len), shared_(1) {}	
	PSArray(PSArray& a1, PSArray& a2);
	~PSArray() {
	  if (!shared_) {
		 universalis::os::aligned_memory_dealloc(ptr_);
	  }
	}

	void set_val(int i, float val) { ptr_[i] = val; }
	float get_val(int i) const { return ptr_[i]; }
	void set_len(int len) {len_ = len; }
	int len() const { return len_; }
	int copyfrom(PSArray& src);
	void resize(int newsize);
	std::string tostring() const;
	
	float* data() { return ptr_; }

	template<class T>
	void do_op(T&);
	
private:	
	float* ptr_;
	int len_;
	int shared_;
};

class PSDelay {
public:
  PSDelay(int k) : mem(k,0) {}
  PSArray mem;  
  void work(PSArray& x, PSArray& y);
};

typedef std::vector<PSArray> psybuffer;

class LuaArrayBind {
public:    
	LuaArrayBind(lua_State* state) {	  
		set_state(state);
	}
	~LuaArrayBind() {
		map_.erase(map_.find(L));
	}

	void build_buffer(std::vector<float*>& buf, int num) {
		std::vector<float*>::iterator it = buf.begin();
		for ( ; it != buf.end(); ++it) {
  		  sampleV_.push_back(PSArray(*it, num));
		}
	}

	void update_num_samples(int num) {
		psybuffer::iterator it = sampleV_.begin();
		for ( ; it != sampleV_.end(); ++it) {
			(*it).set_len(num);
		}
	}

	void set_state(lua_State* state) { 		
		L = state;		
		map_[L] = this;
		export_c_funcs(L);
	}

private:
	// helper
	static PSArray* create_copy_array(lua_State* L, int idx=1);

	static int array_index(lua_State *L);
	static int array_new_index(lua_State *L);
	static int array_new(lua_State *L);
	static int array_new_from_sampleV(lua_State *L);
	static int array_arange(lua_State *L);	
	static int array_copyto(lua_State* L);
	static int array_tostring(lua_State *L);
	static int array_gc(lua_State* L);	
	static int delay_new(lua_State *L);
	// delay methods
	static int delay_work(lua_State* L);
	static int delay_tostring(lua_State* L);
	static int delay_gc(lua_State* L);
	// array methods
	static int array_size(lua_State* L);
	static int array_resize(lua_State* L);
	static int array_concat(lua_State* L);
	// ops
	static int array_add(lua_State* L);
	static int array_sub(lua_State* L);
	static int array_mul(lua_State* L);
	static int array_sum(lua_State* L);
	static int array_rsum(lua_State* L); // x(n)=E(n-p), p=0..N-1
	// funcs
	static int array_sin(lua_State* L);
	static int array_cos(lua_State* L);
	static int array_tan(lua_State* L);
	static int array_sqrt(lua_State* L);
	static int array_random(lua_State* L);
	static int array_pow(lua_State* L);

	void export_c_funcs(lua_State* L);

	lua_State* L;
	psybuffer sampleV_;
    // bind array to luascript,needed only for array creation to get the Lua State
    // else we get a userdata psarray pointer
	static std::map<lua_State*, LuaArrayBind*> map_;
};

}  // namespace
}  // namespace
