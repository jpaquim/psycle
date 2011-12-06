// ----------------------------------------------------------------------------
// Generated by InterfaceFactory [Wed May 07 00:58:14 2003]
// 
// File        : api_syscb.h
// Class       : api_syscb
// class layer : Dispatchable Interface
// ----------------------------------------------------------------------------

#ifndef __API_SYSCB_H
#define __API_SYSCB_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

class SysCallback;


// ----------------------------------------------------------------------------

class NOVTABLE api_syscb: public Dispatchable
{
  protected:
    api_syscb() {}
    ~api_syscb() {}
  public:
    int syscb_registerCallback(SysCallback *cb, void *param = 0);
    int syscb_deregisterCallback(SysCallback *cb);
    int syscb_issueCallback(int eventtype, int msg, intptr_t param1 = 0, intptr_t param2 = 0);

		 /** pass eventtype == 0 to enumerate all syscallbacks 
		  ** call Release() on the returned SysCallback when you are done
			** although very few wasabi objects support this at this time (2 June 2008)
		  **/
		SysCallback *syscb_enum(int eventtype, size_t n);
  
  protected:
    enum {
      API_SYSCB_SYSCB_REGISTERCALLBACK = 20,
      API_SYSCB_SYSCB_DEREGISTERCALLBACK = 10,
      API_SYSCB_SYSCB_ISSUECALLBACK = 30,
			API_SYSCB_SYSCB_ENUM = 40,
    };
};

// ----------------------------------------------------------------------------

inline int api_syscb::syscb_registerCallback(SysCallback *cb, void *param) {
  int __retval = _call(API_SYSCB_SYSCB_REGISTERCALLBACK, (int)0, cb, param);
  return __retval;
}

inline int api_syscb::syscb_deregisterCallback(SysCallback *cb) {
  int __retval = _call(API_SYSCB_SYSCB_DEREGISTERCALLBACK, (int)0, cb);
  return __retval;
}

inline int api_syscb::syscb_issueCallback(int eventtype, int msg, intptr_t param1 , intptr_t param2) {
  int __retval = _call(API_SYSCB_SYSCB_ISSUECALLBACK, (int)0, eventtype, msg, param1, param2);
  return __retval;
}

inline SysCallback *api_syscb::syscb_enum(int eventtype, size_t n)
{
	return _call(API_SYSCB_SYSCB_ENUM, (SysCallback *)0, eventtype, n);
}
// ----------------------------------------------------------------------------


// -- generated code - edit in api_syscbi.h

// {57B7A1B6-700E-44ff-9CB0-70B92BAF3959}
static const GUID syscbApiServiceGuid = 
{ 0x57b7a1b6, 0x700e, 0x44ff, { 0x9c, 0xb0, 0x70, 0xb9, 0x2b, 0xaf, 0x39, 0x59 } };

extern api_syscb *sysCallbackApi;

#endif // __API_SYSCB_H
