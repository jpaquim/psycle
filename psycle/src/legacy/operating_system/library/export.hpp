///\file
///\brief handling of shared dynamic-link libraries for microsoft's dll format
#undef LIBRARY
#if defined OPERATING_SYSTEM__MICROSOFT && !defined COMPILER__GNU // gcc handles dll transparently :-)
	// the microsoft dll horror show system begins...
#	define LIBRARY __declspec(dllexport)
#else
	// everything is nice and simple
#	define LIBRARY
#endif
