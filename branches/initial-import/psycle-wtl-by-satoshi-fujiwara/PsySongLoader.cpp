/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.5 $
 */
#include "stdafx.h"
#include "NewMachine.h"
#include "MainFrm.h"
#include "PsycleWTLView.h"
#include "ProgressDialog.h"

//	extern CPsycleApp theApp;
#include "Song.h"
#include "IPsySongLoader.h"
#include "Instrument.h"
#include "Machine.h" // It wouldn't be needed, since it is already included in "song.h"
#include "Sampler.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "DataCompression.h"
#include "Psy3SongLoader.h"
#include "Psy2SongLoader.h"
#include "PsyFSongLoader.h"
#include ".\psysongloader.h"

//#include <sstream>

#ifdef CONVERT_INTERNAL_MACHINES
	#include "convert_internal_machines.h" // conversion
#endif

#include "Riff.h"	 // For Wave file loading.

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


namespace SF {
	PsySongLoader::PsySongLoader(void)
	{
		m_Loader["PSY2SONG"] = new SF::Psy2SongLoader();
		m_Loader["PSY3SONG"] = new SF::Psy3SongLoader();
		m_Loader["PSYFSONG"] = new SF::PsyFSongLoader();
	
	}

	PsySongLoader::~PsySongLoader(void)
	{
		for(PsySongLoaderMap::iterator it = m_Loader.begin();it != m_Loader.end();it++){
			delete it->second;
		}
	}
	
	void PsySongLoader::Load(string& fileName,Song& song,const bool fullopen)
	{
		
		OldPsyFile file;

		//TODO: ‚±‚±‚ðSong‚ÉŽ‚Á‚Ä‚¢‚­
		if (!file.Open(fileName.c_str()))
		{
			throw ISongLoader::Exception(string(SF::CResourceString(IDS_ERR_MSG0106)));
		}
	
		char Header[9];
	//	char _buffer[256];
		file.Read(&Header, 8);
		Header[8] = 0;

		PsySongLoaderMap::iterator _it = m_Loader.find(Header);

		if(_it != m_Loader.end()){
			_it->second->Load(file,song,fullopen);
			song.IsSaved(true);
			return;
		} 
		// load did not work
		throw Exception(string(SF::CResourceString(IDS_ERR_MSG0107)));
	}

}
