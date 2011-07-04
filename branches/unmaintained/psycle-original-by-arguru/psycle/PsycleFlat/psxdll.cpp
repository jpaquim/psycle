//////////////////////////////////////////////////////////////////////
// PsyDLL API10 implentation for gear


bool psyGear::InstanceDll(char *file)
{
	if(dllInstanced)
	{
		psxfree(dllInstance);
		FreeLibrary(hDLL);
		dllInstanced=false;
	}
	
	hDLL=LoadLibrary(file);

	if (hDLL != NULL)
	{
		psxfree					=(PSXFREE)GetProcAddress(hDLL,"psxFree");
		psxinitialize			=(PSXINITIALIZE)GetProcAddress(hDLL,"psxInitialize");
		psxgetparameterminvalue	=(PSXGETPARAMETERMINVALUE)GetProcAddress(hDLL,"psxGetParameterMinValue");
		psxgetparametermaxvalue	=(PSXGETPARAMETERMAXVALUE)GetProcAddress(hDLL,"psxGetParameterMaxValue");
		psxgetparametercurvalue	=(PSXGETPARAMETERCURVALUE)GetProcAddress(hDLL,"psxGetParameterCurValue");
		psxgetnumparameters		=(PSXGETNUMPARAMETERS)GetProcAddress(hDLL,"psxGetNumParameters");
		psxgetparametername		=(PSXGETPARAMETERNAME)GetProcAddress(hDLL,"psxGetParameterName");
		psxwork					=(PSXWORK)GetProcAddress(hDLL,"psxWork");
		psxsetparameter			=(PSXSETPARAMETER)GetProcAddress(hDLL,"psxSetParameter");
		psxgetinstancecount		=(PSXGETINSTANCECOUNT)GetProcAddress(hDLL,"psxGetInstanceCount");
		psxcreate				=(PSXCREATE)GetProcAddress(hDLL,"psxCreate");
		
		if(
			!psxgetinstancecount ||
			!psxfree ||
			!psxinitialize ||
			!psxgetnumparameters ||
			!psxgetparameterminvalue ||
			!psxgetparametermaxvalue ||
			!psxgetparametercurvalue ||
			!psxgetparametername ||
			!psxwork ||
			!psxsetparameter ||
			!psxcreate
			)
		return false;

		// Initialize DLL;
		dllInstance=psxgetinstancecount();
		
		if(dllInstance=0)
			psxcreate();

		psxinitialize();
		
		dllInstanced=true;

		sprintf(DLLPLUGINFILE,file);

		return true;
	}
	else
	{
		dllInstanced=false;
		return false;
	}	
	
	return false;
}