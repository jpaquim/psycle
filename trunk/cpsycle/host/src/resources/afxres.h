// Dummy afxres.h file so that it can be compiled even if MFC is not installed
#define DUMMY_AFXRES_H 1

#ifdef RC_INVOKED
#ifndef _INC_WINDOWS
#define _INC_WINDOWS
   #include "winres.h"           // extract from windows header
#endif
#endif
