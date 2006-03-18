//
// Psycle FX Plugin API
//

typedef UINT (CALLBACK* LPFNDLLFUNC1)(DWORD,UINT);
typedef void (*PSXFREE)(int);
typedef void (*PSXCREATE)(void);
typedef int  (*PSXINITIALIZE)(void);
typedef int  (*PSXGETPARAMETERMINVALUE)(int);
typedef int  (*PSXGETPARAMETERMAXVALUE)(int);
typedef int  (*PSXGETPARAMETERCURVALUE)(int, int);
typedef int  (*PSXGETNUMPARAMETERS)(void);
typedef void (*PSXGETPARAMETERNAME)(int,char*);
typedef void (*PSXWORK)(int,float*,float*,int);
typedef bool (*PSXSETPARAMETER)(int,int,int);
typedef int	 (*PSXGETINSTANCECOUNT)(void);
