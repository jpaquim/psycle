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
	PSArray() : ptr_(0), base_(0), len_(0), baselen_(0), cap_(0), shared_(0) {}
	PSArray(int len, float v);
	PSArray(double start, double stop, double step);
	PSArray(float* ptr, int len) : ptr_(ptr), base_(ptr), cap_(0), len_(len), baselen_(len), shared_(1) {}	
	PSArray(PSArray& a1, PSArray& a2);
	~PSArray() {
	  if (!shared_ && base_) {		 
		 universalis::os::aligned_memory_dealloc(base_);
	  }
	}
	void set_val(int i, float val) { ptr_[i] = val; }
	float get_val(int i) const { return ptr_[i]; }
	void set_len(int len) {len_ = len; }
	int len() const { return len_; }
	int copyfrom(PSArray& src);
	int copyfrom(PSArray& src, int pos);
	void resize(int newsize);
	std::string tostring() const;
	void fillzero();
	void fillzero(int pos);
	void fill(float val);
	void fill(float val, int pos);
	float* data() { return ptr_; }
	template<class T>
	void do_op(T&);	
	void rsum(double lv);
	void margin(int start, int end) {
		ptr_ = base_ + start;
		assert(end >= start && start >= 0 && end <=baselen_);
		len_ = end-start;
	}
	void offset(int offset) {
		base_ += offset;
		ptr_ += offset;
	}
	void clearmargin() { 
		ptr_ = base_;
		len_ = baselen_;
	}
private:	
	float* ptr_, *base_;
	int cap_;
	int len_;
    int baselen_;
	int shared_;
};

typedef std::vector<PSArray> psybuffer;

struct LuaArrayBind {
	static void register_module(lua_State* L);
	// helper
	static int open_array(lua_State* L);
	static PSArray* create_copy_array(lua_State* L, int idx=1);

	static int array_index(lua_State *L);
	static int array_new_index(lua_State *L);
	static int array_new(lua_State *L);
	static int array_new_from_table(lua_State *L);
	static int array_arange(lua_State *L);	
	static int array_copy(lua_State* L);
	static int array_tostring(lua_State *L);
	static int array_gc(lua_State* L);	
	
	// array methods
	static int array_size(lua_State* L);
	static int array_resize(lua_State* L);
	static int array_margin(lua_State* L);
	static int array_clearmargin(lua_State* L);
	static int array_concat(lua_State* L);
	static int array_fillzero(lua_State* L);
	static int array_method_fill(lua_State* L);
	static int array_method_add(lua_State* L);
	static int array_method_mul(lua_State* L);
	static int array_method_div(lua_State* L);
	static int array_method_rsum(lua_State* L); // x(n)=x(0)+..+x(n-1)
	static int array_method_and(lua_State* L); // binary and
	static int array_method_or(lua_State* L); // binary or
	static int array_method_xor(lua_State* L); // binary or
	static int array_method_sleft(lua_State* L); // binary shift left
	static int array_method_sright(lua_State* L); // binary shift right
	static int array_method_bnot(lua_State* L); // bitwise not
	// ops
	static int array_add(lua_State* L);
	static int array_sub(lua_State* L);
	static int array_mul(lua_State* L);
	static int array_sum(lua_State* L);
	static int array_rsum(lua_State* L); // x(n)=x(0)+..+x(n-1)
	// funcs
	static int array_sin(lua_State* L);
	static int array_cos(lua_State* L);
	static int array_tan(lua_State* L);
	static int array_sqrt(lua_State* L);
	static int array_random(lua_State* L);
	static int array_pow(lua_State* L);
};

}  // namespace
}  // namespace
