///\file
///\brief stringization
#pragma once
#if !defined STRINGIZED
	/// Interprets argument as a string litteral.
	/// The indirection in the call to # lets the macro expansion on the argument be done first.
	#define STRINGIZED(X) STRINGIZED__NO_EXPANSION(X)
	/// Don't call this macro directly ; call STRINGIZED, which calls this macro after macro expansion is done on the argument.
	///\relates STRINGIZED
	#define STRINGIZED__NO_EXPANSION(X) #X
#endif
