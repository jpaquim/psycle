//////////////////////////////////////////////////////////////////////
//
// Import/Export routines
//

#include "stdafx.h"
#include "Psycle2.h"
#include "Song.h"
#include "SaveDlg.h"
#include "FileIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveDlg dialog


CSaveDlg::CSaveDlg(CWnd* pParent /*=NULL*/)
: CDialog(CSaveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveDlg, CDialog)
//{{AFX_MSG_MAP(CSaveDlg)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveDlg message handlers

BOOL CSaveDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return TRUE;
}

void CSaveDlg::SaveSong(bool silent) 
{
	OldPsyFile file;
	if (!file.Create(szFile, true))
	{
		if (!silent) MessageBox("Error creating file", "Error", MB_OK);
		return;
	}
	if (!_pSong->Save(&file))
	{
		if (!silent) MessageBox("Error saving file", "Error", MB_OK);
	}
	else _pSong->_saved=true;
	file.Close();
//	Sleep(1000);

/*	The following code is the original 1.0 Code.
    Mats did a really Good job Recoding it to the current one in version 1.1b1.


	FILE* hFile=fopen(szFile,"wb");

	// Write PSY HEADER ----------------------------------------------
	
	fwrite("PSY2SONG",sizeof(char),8,hFile);
	
	// Write general properties --------------------------------------
	
	fwrite(&_pSong->Name[0],sizeof(char),32,hFile);
	fwrite(&_pSong->Author[0],sizeof(char),32,hFile);
	fwrite(&_pSong->Comment[0],sizeof(char),128,hFile);
	
	fwrite(&_pSong->BeatsPerMin,sizeof(int),1,hFile);
	fwrite(&_pSong->SamplesPerTick,sizeof(int),1,hFile);
	fwrite(&_pSong->currentOctave,sizeof(unsigned char),1,hFile);
	
	// Write Buses data ----------------------------------------------
	
	fwrite(&_pSong->busMachine[0],1,MAX_BUSES,hFile);
	
	// Write pos data
	fwrite(&_pSong->playOrder,sizeof(char),MAX_SONG_POSITIONS,hFile);
	fwrite(&_pSong->playLength,sizeof(int),1,hFile);
	fwrite(&_pSong->SONGTRACKS,sizeof(int),1,hFile);
	
	// Write Pattern data --------------------------------------------
	
	int npats=_pSong->GetNumPatternsUsed();
	fwrite(&npats,sizeof(int),1,hFile);
	
	for(int c=0;c<npats;c++)
	{
		fwrite(&_pSong->patternLines[c],sizeof(int),1,hFile);
		fwrite(&_pSong->patternName[c][0],sizeof(char),32,hFile);
		
		int sizep=_pSong->patternLines[c]*MAX_TRACKS*5;
		
		unsigned char *pOffset=_pSong->pPatternData+MULTIPLY2*c;
		fwrite(pOffset,sizeof(char),sizep,hFile);
	}
	
	// Write Instrument data -----------------------------------------
	
	fwrite(&_pSong->instSelected,sizeof(int),1,hFile);
	fwrite(&_pSong->instName[0][0],MAX_INSTRUMENTS*32,1,hFile);
	fwrite(&_pSong->instNNA[0],sizeof(unsigned char),MAX_INSTRUMENTS,hFile);
	
	fwrite(&_pSong->instENV_AT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_DT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_SL[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_RT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	
	fwrite(&_pSong->instENV_F_AT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_F_DT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_F_SL[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_F_RT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	
	fwrite(&_pSong->instENV_F_CO[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_F_RQ[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_F_EA[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instENV_F_TP[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	
	fwrite(&_pSong->instPAN[0],sizeof(int),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instRPAN[0],sizeof(bool),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instRCUT[0],sizeof(bool),MAX_INSTRUMENTS,hFile);
	fwrite(&_pSong->instRRES[0],sizeof(bool),MAX_INSTRUMENTS,hFile);
	
	// Writing waves -------------------------------------------------
	
	fwrite(&_pSong->waveSelected,sizeof(int),1,hFile);
	
	for(int i=0;i<MAX_INSTRUMENTS;i++)
	{
		for(int w=0;w<MAX_WAVES;w++)
		{
			unsigned int wLen=_pSong->waveLength[i][w];
			fwrite(&wLen,sizeof(unsigned int),1,hFile);
			
			if(wLen>0) // Wavedata allocated
			{
				fwrite(&_pSong->waveName[i][w][0],sizeof(char),32,hFile);
				fwrite(&_pSong->waveVolume[i][w],sizeof(unsigned short),1,hFile);
				fwrite(&_pSong->waveFinetune[i][w],sizeof(unsigned short),1,hFile); 			
				fwrite(&_pSong->waveLoopStart[i][w],sizeof(unsigned int),1,hFile);
				fwrite(&_pSong->waveLoopEnd[i][w],sizeof(unsigned int),1,hFile);
				fwrite(&_pSong->waveLoopType[i][w],sizeof(bool),1,hFile);
				fwrite(&_pSong->waveStereo[i][w],sizeof(bool),1,hFile);
				
				fwrite(_pSong->waveDataL[i][w],sizeof(signed short),wLen,hFile);
				
				if(_pSong->waveStereo[i][w])
					fwrite(_pSong->waveDataR[i][w],sizeof(signed short),wLen,hFile);
			}
		} // Wave save bucle
	} // Instrument save bucle
	
	
	// VST PARAMETER DATA WRITE ---------------------------------------
/*	
	for(int np=0; np<MAX_VST_INSTANCES; np++)
	{
		bool pi=_pSong->m_Vst.Instanced[np];
		
		fwrite(&pi,sizeof(bool),1,hFile);
		
		if(pi)
		{
			fwrite(_pSong->m_Vst.GetPlugName(np),sizeof(char),128,hFile);
			
			int npars=_pSong->m_Vst.effect[np]->numParams;
			fwrite(&npars,sizeof(int),1,hFile);
			
			for(int i=0;i<_pSong->m_Vst.effect[np]->numParams;i++)
			{
				float value=_pSong->m_Vst.effect[np]->getParameter(_pSong->m_Vst.effect[np],i);
				fwrite(&value,sizeof(float),1,hFile);
			}
			/*
			//float oee(AEffect *effect, long opcode, long index,long value, void *ptr, float opt));
			
			  _pSong->m_Vst.effect[np]->dispatcher(_pSong->m_Vst.effect[np],effSetChunk
			  
				void *
				effGetChunk,		// host requests pointer to chunk into (void**)ptr, byteSize returned
				effSetChunk,		// plug-in receives saved chunk, byteSize passed
			*//*
		}
	}
*/	
	// Writing the fucked machines -----------------------------------
/*	
	fwrite(&_pSong->_machineActive[0],sizeof(bool),MAX_MACHINES,hFile);
	
	for (int m=0; m<MAX_MACHINES; m++)
	{
		if (_pSong->_machineActive[m])
		{
			Machines* wm=_pSong->_pMachines[m];
			
			// general
			fwrite(&wm->x,sizeof(int),1,hFile);
			fwrite(&wm->y,sizeof(int),1,hFile);
			fwrite(&wm->machineMode,sizeof(int),1,hFile);
			
			if(wm->machineMode==8)
				fwrite(_pSong->BfxName[wm->buzzdll],sizeof(char),256,hFile);
			
			fwrite(&wm->editName[0],sizeof(char),16,hFile);
			
			// machine data save
			if(wm->machineMode==8)
			{
				GETINFO bfxGetInfo =(GETINFO)GetProcAddress(_pSong->hBfxDll[wm->buzzdll],"GetInfo");
				int np=bfxGetInfo()->numParameters;
				fwrite(&np,sizeof(int),1,hFile); // <-- write paralength
				fwrite(&wm->mi->Vals[0],sizeof(int),np,hFile);
				wm->mi->SaveData(hFile);
			}
			
			// links
			fwrite(&wm->inputSource[0],sizeof(int),MAX_CONNECTIONS,hFile);
			fwrite(&wm->outputDest[0],sizeof(int),MAX_CONNECTIONS,hFile);
			fwrite(&wm->connectionVol[0],sizeof(float),MAX_CONNECTIONS,hFile);				
			fwrite(&wm->conection[0],sizeof(bool),MAX_CONNECTIONS,hFile);
			fwrite(&wm->inCon[0],sizeof(bool),MAX_CONNECTIONS,hFile);
			fwrite(&wm->connectionPoint[0],sizeof(CPoint),MAX_CONNECTIONS,hFile);
			fwrite(&wm->numInputs,sizeof(int),1,hFile);
			fwrite(&wm->numOutputs,sizeof(int),1,hFile);
			
			// misc
			fwrite(&wm->panning,sizeof(int),1,hFile);
			fwrite(&wm->SubTrack[0],sizeof(int),MAX_SUBTRACKS,hFile);
			fwrite(&wm->numSubTracks,sizeof(int),1,hFile);
			fwrite(&wm->interpol,sizeof(int),1,hFile);
			
			// parameters
			fwrite(&wm->outDry,sizeof(int),1,hFile);
			fwrite(&wm->outWet,sizeof(int),1,hFile);
			
			// distortion
			fwrite(&wm->distPosThreshold,sizeof(int),1,hFile);
			fwrite(&wm->distPosClamp,sizeof(int),1,hFile);
			fwrite(&wm->distNegThreshold,sizeof(int),1,hFile);
			fwrite(&wm->distNegClamp,sizeof(int),1,hFile);
			
			// am
			fwrite(&wm->sinespeed,sizeof(unsigned char),1,hFile);
			fwrite(&wm->sineglide,sizeof(unsigned char),1,hFile);
			fwrite(&wm->sinevolume,sizeof(unsigned char),1,hFile);
			fwrite(&wm->sinelfospeed,sizeof(unsigned char),1,hFile);
			fwrite(&wm->sinelfoamp,sizeof(unsigned char),1,hFile);
			
			// delay
			fwrite(&wm->delayTimeL,sizeof(int),1,hFile);
			fwrite(&wm->delayTimeR,sizeof(int),1,hFile);
			fwrite(&wm->delayFeedbackL,sizeof(int),1,hFile);
			fwrite(&wm->delayFeedbackR,sizeof(int),1,hFile);
			
			// filter
			fwrite(&wm->filterCutoff,sizeof(int),1,hFile);
			fwrite(&wm->filterResonance,sizeof(int),1,hFile);
			fwrite(&wm->filterLfospeed,sizeof(int),1,hFile);
			fwrite(&wm->filterLfoamp,sizeof(int),1,hFile);
			fwrite(&wm->filterLfophase,sizeof(int),1,hFile);
			fwrite(&wm->filterMode,sizeof(int),1,hFile);
			
			// VST
			if(wm->machineMode==9 || wm->machineMode==10)
			{
				fwrite(&wm->ovst.ANY,sizeof(bool),1,hFile);
				fwrite(&wm->ovst.instance,sizeof(int),1,hFile);
				fwrite(&wm->ovst.midichannel,sizeof(char),1,hFile);
			}
		}
	}
	// Loop synchro data
	fwrite(_pSong->instLoop,sizeof(bool),MAX_INSTRUMENTS,hFile);
	fwrite(_pSong->instLines,sizeof(int),MAX_INSTRUMENTS,hFile);
*/
	
}

void CSaveDlg::LoadSong(bool silent) 
{
	
	
	OldPsyFile file;
	if (!file.Open(szFile))
	{
		if (!silent) MessageBox("Could not Open file. Check that the location is correct.", "Loading Error", MB_OK);
		return;
	}
	_pSong->Load(&file);
	file.Close();

	_pSong->_saved=true;
	
/*	
	char fbuffer[32][512];

	int FailedMac=0;
  
// Intermediate import variable
	
	char HeadBuf[16];

    //////////////////////////////////////////////////////////////////////
	//	Read Function
	
	// Read PSY HEADER -------------------------------------------
	
	fread(&HeadBuf[0],sizeof(char),8,hFile);
	
	if(strncmp(HeadBuf,"PSY2SONG",8)==0)
	{ 
		_pSong->New();
		
		fread(&_pSong->Name[0],sizeof(char),32,hFile);
		fread(&_pSong->Author[0],sizeof(char),32,hFile);
		fread(&_pSong->Comment[0],sizeof(char),128,hFile);
		
		fread(&_pSong->BeatsPerMin,sizeof(int),1,hFile);
		fread(&_pSong->SamplesPerTick,sizeof(int),1,hFile);
		fread(&_pSong->currentOctave,sizeof(unsigned char),1,hFile);
		
		// Read Buses data ----------------------------------------------
		
		fread(&_pSong->busMachine[0],1,MAX_BUSES,hFile);
		
		// Write pos data
		fread(&_pSong->playOrder,sizeof(char),MAX_SONG_POSITIONS,hFile);
		fread(&_pSong->playLength,sizeof(int),1,hFile);
		fread(&_pSong->SONGTRACKS,sizeof(int),1,hFile);
		
		// Read Pattern data --------------------------------------------
		
		int nPatterns;
		fread(&nPatterns,sizeof(int),1,hFile);
		
		for(int c=0;c<nPatterns;c++)
		{
			fread(&_pSong->patternLines[c],sizeof(int),1,hFile);
			fread(&_pSong->patternName[c][0],sizeof(char),32,hFile);			
			unsigned char *pOffset=_pSong->pPatternData+MULTIPLY2*c;
			fread(pOffset,sizeof(char),MULTIPLY*_pSong->patternLines[c],hFile);
		}
		
		// Read Instrument data -----------------------------------------
		
		fread(&_pSong->instSelected,sizeof(int),1,hFile);
		fread(&_pSong->instName[0][0],MAX_INSTRUMENTS*32,1,hFile);
		fread(&_pSong->instNNA[0],sizeof(unsigned char),MAX_INSTRUMENTS,hFile);
		
		fread(&_pSong->instENV_AT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_DT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_SL[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_RT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		
		fread(&_pSong->instENV_F_AT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_F_DT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_F_SL[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_F_RT[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		
		fread(&_pSong->instENV_F_CO[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_F_RQ[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_F_EA[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instENV_F_TP[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		
		fread(&_pSong->instPAN[0],sizeof(int),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instRPAN[0],sizeof(bool),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instRCUT[0],sizeof(bool),MAX_INSTRUMENTS,hFile);
		fread(&_pSong->instRRES[0],sizeof(bool),MAX_INSTRUMENTS,hFile);
		
		// Read waves -------------------------------------------------
		
		fread(&_pSong->waveSelected,sizeof(int),1,hFile);
		
		for(int i=0;i<MAX_INSTRUMENTS;i++)
		{
			for(int w=0;w<MAX_WAVES;w++)
			{
				fread(&_pSong->waveLength[i][w],sizeof(unsigned int),1,hFile);
				
				if(_pSong->waveLength[i][w]>0) // Wavedata allocated
				{
					fread(&_pSong->waveName[i][w][0],sizeof(char),32,hFile);
					fread(&_pSong->waveVolume[i][w],sizeof(unsigned short),1,hFile);
					fread(&_pSong->waveFinetune[i][w],sizeof(unsigned short),1,hFile);
					fread(&_pSong->waveLoopStart[i][w],sizeof(unsigned int),1,hFile);
					fread(&_pSong->waveLoopEnd[i][w],sizeof(unsigned int),1,hFile);
					fread(&_pSong->waveLoopType[i][w],sizeof(bool),1,hFile);
					fread(&_pSong->waveStereo[i][w],sizeof(bool),1,hFile);
					
					_pSong->waveDataL[i][w]=new signed short[_pSong->waveLength[i][w]];
					fread(_pSong->waveDataL[i][w],sizeof(signed short),_pSong->waveLength[i][w],hFile);
					
					if(_pSong->waveStereo[i][w])
					{
						_pSong->waveDataR[i][w]=new signed short[_pSong->waveLength[i][w]];
						fread(_pSong->waveDataR[i][w],sizeof(signed short),_pSong->waveLength[i][w],hFile);
					}
				}
			} // Wave save bucle
		} // Instrument save bucle
		
		
		// VST PARAMETER DATA READ ---------------------------------------
/*		
		for(int np=0;np<MAX_VST_INSTANCES;np++)
		{
			bool ins;
			fread(&ins,sizeof(bool),1,hFile);
			
			if(ins)
			{
				char dlln[256];
				
				fread(dlln,sizeof(char),128,hFile);
				_pSong->m_Vst.InstanciateDll(dlln,np);
				
				fread(&_pSong->m_Vst.effect[np]->numParams,sizeof(int),1,hFile);
				
				for(int i=0;i<_pSong->m_Vst.effect[np]->numParams;i++)
				{
					float value;
					fread(&value,sizeof(float),1,hFile);
					_pSong->m_Vst.effect[np]->setParameter(_pSong->m_Vst.effect[np],i,value);
				}
			}
		}
		
		// Read the fucked machines -----------------------------------
		
		fread(&_pSong->Activemachine[0],sizeof(bool),MAX_MACHINES,hFile);
		
		for (int m=0;m<MAX_MACHINES;m++)
		{
			if(_pSong->Activemachine[m])
			{
				int mlx;
				int mly;
				int mlm;
				int mdl=-1;
				
				bool fcm=false;
				
				// general
				fread(&mlx,sizeof(int),1,hFile);
				fread(&mly,sizeof(int),1,hFile);
				fread(&mlm,sizeof(int),1,hFile);
				
				char tmpdn[256];
				
				if(mlm==8)
				{
					fread(tmpdn,sizeof(char),256,hFile);
					mdl=_pSong->GetIntDLL(tmpdn);
					
					if(mdl==-1)
					{
						sprintf(fbuffer[FailedMac],"Machine '%s' is not installed in your plugins directory",tmpdn);
						++FailedMac;
						mlm=255;
						fcm=true;
					}
				}
				
				if(m>0)_pSong->CreateMachine(mlm,mlx,mly,m,mdl);
				
				psyGear* wm=_pSong->machine[m];
				
				fread(&wm->editName[0],sizeof(char),16,hFile);
				
				// machine data load
				
				int np;
				
				if(wm->machineMode==8)
				{
					GETINFO bfxGetInfo =(GETINFO)GetProcAddress(_pSong->hBfxDll[mdl],"GetInfo");
					fread(&np,sizeof(int),1,hFile);
					fread(&wm->mi->Vals[0],sizeof(int),bfxGetInfo()->numParameters,hFile);
					wm->mi->LoadData(hFile);
					wm->dllTweakAll(bfxGetInfo()->numParameters);
				}
				
				if(fcm)
				{
					fread(&np,sizeof(int),1,hFile);
					fseek(hFile,np*sizeof(int),SEEK_CUR);
				}
				
				// links
				fread(&wm->inputSource[0],sizeof(int),MAX_CONNECTIONS,hFile);
				fread(&wm->outputDest[0],sizeof(int),MAX_CONNECTIONS,hFile);
				fread(&wm->connectionVol[0],sizeof(float),MAX_CONNECTIONS,hFile);				
				fread(&wm->conection[0],sizeof(bool),MAX_CONNECTIONS,hFile);
				fread(&wm->inCon[0],sizeof(bool),MAX_CONNECTIONS,hFile);
				fread(&wm->connectionPoint[0],sizeof(CPoint),MAX_CONNECTIONS,hFile);
				fread(&wm->numInputs,sizeof(int),1,hFile);
				fread(&wm->numOutputs,sizeof(int),1,hFile);
				
				// misc
				fread(&wm->panning,sizeof(int),1,hFile);wm->changePan(wm->panning);
				fread(&wm->SubTrack[0],sizeof(int),MAX_SUBTRACKS,hFile);
				fread(&wm->numSubTracks,sizeof(int),1,hFile);
				fread(&wm->interpol,sizeof(int),1,hFile);
				
				// parameters
				fread(&wm->outDry,sizeof(int),1,hFile);
				fread(&wm->outWet,sizeof(int),1,hFile);
				
				// distortion
				fread(&wm->distPosThreshold,sizeof(int),1,hFile);
				fread(&wm->distPosClamp,sizeof(int),1,hFile);
				fread(&wm->distNegThreshold,sizeof(int),1,hFile);
				fread(&wm->distNegClamp,sizeof(int),1,hFile);
				
				// am
				fread(&wm->sinespeed,sizeof(unsigned char),1,hFile);
				fread(&wm->sineglide,sizeof(unsigned char),1,hFile);
				fread(&wm->sinevolume,sizeof(unsigned char),1,hFile);
				fread(&wm->sinelfospeed,sizeof(unsigned char),1,hFile);
				fread(&wm->sinelfoamp,sizeof(unsigned char),1,hFile);
				
				// delay
				int d1,d2,d3,d4;
				wm->delayTimeL=0;
				wm->delayTimeR=0;
				wm->delayFeedbackL=0;
				wm->delayFeedbackR=0;
				
				fread(&d1,sizeof(int),1,hFile);
				fread(&d2,sizeof(int),1,hFile);
				fread(&d3,sizeof(int),1,hFile);
				fread(&d4,sizeof(int),1,hFile);
				wm->UpdateDelay(d1,d2,d3,d4);
				
				// filter
				fread(&wm->filterCutoff,sizeof(int),1,hFile);
				fread(&wm->filterResonance,sizeof(int),1,hFile);
				fread(&wm->filterLfospeed,sizeof(int),1,hFile);
				fread(&wm->filterLfoamp,sizeof(int),1,hFile);
				fread(&wm->filterLfophase,sizeof(int),1,hFile);
				fread(&wm->filterMode,sizeof(int),1,hFile);
				
				// VST
				if(wm->machineMode==9 || wm->machineMode==10)
				{
					wm->ovst.pVST=&_pSong->m_Vst;
					fread(&wm->ovst.ANY,sizeof(bool),1,hFile);
					fread(&wm->ovst.instance,sizeof(int),1,hFile);
					fread(&wm->ovst.midichannel,sizeof(char),1,hFile);
					wm->ovst.SetVstInstance(wm->ovst.instance);
				}
				
				switch(wm->machineMode)
				{
				case 5:
					wm->UpdateFilter();
					break;
					
				case 7:
					wm->UpdateFlanger();
					break;
				}
			}
	}
*//*
	
	// Loop synchro data
	fread(_pSong->instLoop,sizeof(bool),MAX_INSTRUMENTS,hFile);
	fread(_pSong->instLines,sizeof(int),MAX_INSTRUMENTS,hFile);

	if(FailedMac>0)
	{
		//for(int n=0;n<FailedMac;n++)
		AfxMessageBox("Failed loading some machines");
	}
	
	}
	else
	{
	}
*/	
}
