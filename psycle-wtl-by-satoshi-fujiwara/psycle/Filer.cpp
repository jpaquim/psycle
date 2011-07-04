#include "stdafx.h"
#include "filer.h"

Filer::Filer(void)
{
}

Filer::~Filer(void)
{
}

Filer::LoadSong()
{
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0] = _T('\0');
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetParent().m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	SF::CResourceString _filter(IDS_MSG0022);
	boost::scoped_array<TCHAR> _pfilter(new TCHAR[ _filter.Length() + 2 ]);
	memcpy(_pfilter.get(),_filter,_filter.Length() * sizeof(TCHAR));
	
	_pfilter[_filter.Length() - 1] = 0;
	_pfilter[_filter.Length()] = 0;

	ofn.lpstrFilter = _pfilter.get();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Global::pConfig->GetSongDir();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn)==TRUE)
	{
		OnFileLoadsongNamed(szFile, ofn.nFilterIndex);
	}

}

