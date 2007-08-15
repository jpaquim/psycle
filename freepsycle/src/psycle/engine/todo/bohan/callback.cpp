pa_call_callback(void (*callback) (void*), void * data) { callback(data); }
template<typename T, void (T::*Member_Function)()> void member_callback(void * t) { (static_cast<T*>(t)->*Member_Function)(); }
template<typename T, void (T::*Member_Function)()> void member_callback(T & t) { (t.*Member_Function)(); }

class C { public: void call_me(); };

int main(char**, int)
{
	C c;
	pa_call_callback( &member_callback<C, &C::call_me>, &c);
	// notice that member_callback<C, &C::call_me>(&c) is the same as c.call_me()
}
