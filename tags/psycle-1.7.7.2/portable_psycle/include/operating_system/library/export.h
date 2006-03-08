#undef LIBRARY
#if defined OPERATING_SYSTEM__MICROSOFT
	// the microsoft dll horror show system begins...
#	define LIBRARY EXPORT
#else
	// everything is nice and simple
#	define LIBRARY
#endif
