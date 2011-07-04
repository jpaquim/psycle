#ifndef _SONG_H
#define _SONG_H

/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.5 $
 */

#include "UndoRedo.h"
#include "Constants.h"
#include "FileIO.h"
#include "SongStructs.h"
#include "ISongLoader.h"
#include "iController.h"
#include "iSong.h"

//class CComCriticalSection;
namespace SF {
	struct IPsySongLoader;

}

struct VSTLoader
{
	bool		valid;
	TCHAR		dllName[128];
	int			numpars;
	float*		pars;

};

class Instrument;
class Machine;
/** Songクラス */
class Song : public iSong
{
public:
	// インナークラス Inner Class
	
	/// Song Exception
	class Exception : public std::exception
	{
	public:
		Exception(const string& reason)
		{
			m_Reason = reason;
		};
		const TCHAR * what() {return m_Reason.c_str();};
		const string& what_str() { return m_Reason;};
	private:
		string m_Reason;
	};
	
	/// マシンが見つからない例外 
	class MachineNotFoundException : public Exception 
	{
	public:
		MachineNotFoundException(const string& reason) : Exception(reason) {;};
	};


	/// コンストラクタ 
	explicit Song(iController * pController);
	/// デストラクタ
	~Song();

	// 属性 Property
	const int MachineSoloed(){return m_MachineSoloed;}; ///<ソロ Get
	void MachineSoloed(const int value){m_MachineSoloed = value;};///<ソロ Set
	
	const string& FileName(){return m_FileName;};///< ファイル名
	void FileName(const string& fileName){m_FileName = fileName;};///< ファイル名

	const CPoint& ViewSize() { return m_ViewSize;};///< ビューサイズ CPoint
	void ViewSize(const CPoint& point){m_ViewSize = point;};///< ビューサイズ CPoint
	
	void ViewSizeX(const int xpos){m_ViewSize.x = xpos;};///< ビューサイズ X
	const int ViewSizeX(){return m_ViewSize.x;};///< ビューサイズ X

	void ViewSizeY(const int ypos){m_ViewSize.y = ypos;};///< ビューサイズ Y
	const int ViewSizeY(){ return m_ViewSize.y;};///< ビューサイズ Y

	const bool IsSaved() { return m_bSaved;};///< 保存したかどうか
	void IsSaved(const bool bsaved){ m_bSaved = bsaved;};///<保存したかどうか

	const int TrackSoloed() { return m_TrackSoloed;};///< トラックソロ
	void TrackSoloed(const int soloTrack){m_TrackSoloed = soloTrack;};///< トラックソロ

	CComAutoCriticalSection& Door(){return m_Door;};///< ロックオブジェクト
	
	const string& Name(){return m_Name;};///< ソング名
	void  Name(const string& name){m_Name = name;};///< ソング名

	const string& Author(){return m_Author;};///< 作者名
	void Author(const string& author){m_Author = author;};///< 作者名

	const string& Comment(){return m_Comment;};///< コメント	
	void Comment(const string& comment){m_Comment = comment;};///< コメント	
	
	const bool IsTweaking(){ return m_bTweaking;};///< Tweakするか
	void IsTweaking(const bool tweaking){m_bTweaking = true;};///< Tweakするか 

	const unsigned int CpuIdle(){return m_CpuIdle;};
	void CpuIdle(const unsigned int value){m_CpuIdle = value;};
	
	const unsigned int SampCount(){return m_SampCount;};
	void SampCount(const unsigned int value){m_SampCount = value;};

	const bool IsInvalided(){return m_bInvalided;};
	void IsInvalided(const bool value){m_bInvalided = value;};
	
	const int BeatsPerMin(){return m_BeatsPerMin;};

	const int DefaultPatternLines(){return m_DefaultPatternLines;};
	void DefaultPatternLines(const int value){m_DefaultPatternLines = value;};

	void BeatsPerMin(const int value){
		m_BeatsPerMin = value;
		if (m_BeatsPerMin < 33)
		{
			m_BeatsPerMin = 33;
		}
		if (m_BeatsPerMin > 999)
		{
			m_BeatsPerMin = 999;
		}
	};

	const int AddBpm(const int addValue)
	{
		// TODO Undoコードの組み込み
		//AddMacViewUndo();
		BeatsPerMin(BeatsPerMin() + addValue);
		SetBPM(BeatsPerMin(),TicksPerBeat(),m_pController->SamplesPerSec());
		return BeatsPerMin();
	};

	void AddTpb(const int addValue)
	{
		if ( addValue != 0)
		{
			m_TicksPerBeat += addValue;
			SetBPM(
				BeatsPerMin(),
				TicksPerBeat(),
				m_pController->SamplesPerSec()
				);
		}
	};

	const int TicksPerBeat(){return m_TicksPerBeat;};
	void TicksPerBeat(const int value)
	{
		m_TicksPerBeat = value;
		if (m_TicksPerBeat < 1) {
			m_TicksPerBeat = 1;
		} else {
			if (m_TicksPerBeat > 32) 
				m_TicksPerBeat = 32;
		}
	};

	const int LineCounter(){return m_LineCounter;};
	void LineCounter(const int value){m_LineCounter = value;};
	
	const bool IsLineChanged(){return m_bLineChanged;};
	void IsLineChanged(const bool value){m_bLineChanged = value;};

	const char CurrentOctave(){return m_CurrentOctave;};
	void CurrentOctave(const char value)
	{	m_CurrentOctave = value;
		if ( m_CurrentOctave < 0 )
		{ m_CurrentOctave = 0; } 
		else if ( m_CurrentOctave > 8 )
		{ m_CurrentOctave = 8; }
	};

	const int SamplesPerTick(){ return m_SamplesPerTick;};///< Tick毎のサンプル数
	void SamplesPerTick(const int samplePerTick){m_SamplesPerTick = samplePerTick;};///< Tick毎のサンプル数
	
	/// パターンデータポインタ配列
	unsigned char * const pPatternData(const int index){return m_ppPatternData[index];};
	/// パターンデータポインタ配列へポインタをセットする
	void pPatternData(const int index,unsigned char * const pPattern){ m_ppPatternData[index] = pPattern ;};
	
	/// プレイ順
	const unsigned char PlayOrder(int index){return m_PlayOrder[index];};
	void PlayOrder(const int index,const unsigned char value){m_PlayOrder[index] = value;};
	
	const bool PlayOrderSel(const int index){ return m_PlayOrderSel[index];};
	void PlayOrderSel(const int index,const bool value){ m_PlayOrderSel[index] = value;};


	const int PatternLines(const int index){return m_PatternLines[index];};
	void PatternLines(const int index,const int value){m_PatternLines[index] = value;};
	
	TCHAR * PatternName(const int index){return &(m_PatternName[index][0]);};

	const int SongTracks(){return m_SongTracks;};
	void SongTracks(const int value){m_SongTracks = value;};

	const int MidiSelected(){return m_MidiSelected;};
	void MidiSelected(const int value){m_MidiSelected = value;};

	const int AuxcolSelected(){return m_AuxcolSelected;};
	void AuxcolSelected(const int value){m_AuxcolSelected = value;};

	const int TrackArmedCount(){return m_TrackArmedCount;};
	void TrackArmedCount(const int value){m_TrackArmedCount = value;};

	// InstrumentData
	const int InstSelected(){return m_InstSelected;};
	void InstSelected(const int value){m_InstSelected = value;};

	Instrument * const pInstrument(const int index){return m_pInstrument[index];};
	void pInstrument(const int index,Instrument * const value){m_pInstrument[index] = value;};
	
	const bool IsTrackMuted(const int index){ return m_TrackMuted[index];};
	void IsTrackMuted(const int index,const bool value){ m_TrackMuted[index] = value;};

	const bool IsTrackArmed(const int index){return m_TrackArmed[index];};
	void IsTrackArmed(const int index,const bool value){m_TrackArmed[index] = value;};

	// WaveData ------------------------------------------------------
	//
	const int WaveSelected(){return m_WaveSelected;};
	void WaveSelected(const int value){ m_WaveSelected = value;};

	// Machines ------------------------------------------------------
	//
	const bool IsMachineLock(){return m_MachineLock;};
	void IsMachineLock(const bool value){m_MachineLock = value;};

	Machine* const pMachine(const int index){return m_pMachine[index];};
	void pMachine(const int index,Machine* const value){m_pMachine[index] = value;};

	const int SeqBus(){return m_SeqBus;};
	void SeqBus(const int value){m_SeqBus = value;};

	const int PlayLength(){return m_PlayLength;};
	void PlayLength(const int value){m_PlayLength = value;};


	int WavAlloc(int iInstr,int iLayer,const TCHAR * str);
	int WavAlloc(int iInstr,int iLayer,bool bStereo,long iSamplesPerChan,const TCHAR * sName);
	int IffAlloc(int instrument,int layer,const TCHAR * str);

	void New(void);
	void Reset(void);

	int GetFreeMachine(void);
	bool CreateMachine(MachineType type, int x, int y, TCHAR* psPluginDll, int index);
	void DestroyMachine(int mac);
	void DestroyAllMachines();
	int GetNumPatternsUsed();
	bool InsertConnection(int src,int dst,float value = 1.0f);
	int GetFreeBus();
	int GetFreeFxBus();
	int FindBusFromIndex(int smac);
	int GetBlankPatternUnused(int rval = 0);
	bool AllocNewPattern(int pattern,TCHAR *name,int lines,bool adaptsize);
	const bool CloneMac(const int src,const int dst);
	const bool CloneIns(const int src,const int dst);

	void DeleteAllPatterns(void);
	void DeleteInstrument(int i);
	void DeleteInstruments();
	void DeleteLayer(int i,int c);
	void DestroyAllInstruments();

	void SetBPM(const int bpm, const int tpb, const int srate);

	bool IsPatternUsed(int i);

	// Previews waving

	void PW_Work(float *psamplesL, float *pSamplesR, int numSamples);
	void PW_Play();
	
	const int PW_Phase(){ return m_PW_Phase;};
	void PW_Phase(const int value){ m_PW_Phase = value;};

	const int PW_Stage(){return m_PW_Stage;};
	void PW_Stage(const int value){m_PW_Stage = value;};

	const int PW_Length() { return m_PW_Length;};
	void PW_Length(const int value){m_PW_Length = value;};

	
	inline unsigned char * _ppattern(int ps);
	inline unsigned char * _ptrack(int ps, int track);
	inline unsigned char * _ptrackline(int ps, int track, int line);

	unsigned char * CreateNewPattern(int ps);
	void RemovePattern(int ps);

	void Load();
	void Load(const TCHAR* fName,const int fType);
	void Load(const TCHAR* fName);

	void Save(const string& fileName);

	typedef std::map<string,SF::ISongLoader *> SongLoaderMap;
	
	const bool IsUnsaved(){return true;};

private:

	void SetSongDirAndFileName(const TCHAR *  pFilePath,const TCHAR * extention = _T(".psy"))
	{	
		string str = pFilePath;
		int index = str.find_last_of(_T('\\'));
		if (index != std::string::npos)
		{
			m_pController->configuration()->SongDir(str.substr(0,index + 1).c_str());
			str = str.substr(index + 1,str.length() - index - 1) ;
			str += extention;
			m_pController->Song()->FileName(str);
		}
		else
		{
			m_pController->Song()->FileName(str + extention);
		}
	}

	SongLoaderMap m_SongLoaderMap; 
	int m_UndoCounter;
	int m_UndoSaved;

	iController* m_pController;
	SF::UndoController m_UndoController;
	int m_MachineSoloed;///< マシンソロ
	string m_FileName;///< ファイルネーム
	CPoint m_ViewSize;///< ビューサイズ
	bool m_bSaved;///< 保存したか？ 
	int m_TrackSoloed;///< トラックソロ

	bool autosaveSong;///< 自動保存するか
	int autosaveSongTime;///< 自動保存間隔

	CComAutoCriticalSection m_Door; ///< クリティカルセクション

	//TCHAR Name[MAX_NAME_LEN];								///< Song Name
	//TCHAR Author[MAX_AUTHOR_LEN];							///< Song Author
	//TCHAR Comment[MAX_COMMENT_LEN];							///< Song Comment
	
	string m_Name;	///< Song Name
	string m_Author;///< Song Author
	string m_Comment;///< Song Comment

	bool m_bTweaking;

	unsigned m_CpuIdle;
	unsigned m_SampCount;

	bool m_bInvalided;
	
	int m_BeatsPerMin;
	int m_TicksPerBeat;
	int m_SamplesPerTick;
	int m_LineCounter;
	bool m_bLineChanged;
	
	char m_CurrentOctave;

	// Buses data
//	unsigned char busEffect[MAX_BUSES];
//	unsigned char busMachine[MAX_BUSES];

	// Pattern data
	unsigned char * m_ppPatternData[MAX_PATTERNS];

	int m_PlayLength;
	
	unsigned char m_PlayOrder[MAX_SONG_POSITIONS];

	bool m_PlayOrderSel[MAX_SONG_POSITIONS];

	int m_PatternLines[MAX_PATTERNS];
	TCHAR m_PatternName[MAX_PATTERNS][SongConst::PATTERN_NAME_LEN];
	int m_SongTracks;

	int m_MidiSelected;
	int m_AuxcolSelected;
	int m_TrackArmedCount;
	// InstrumentData
	int m_InstSelected;
	Instrument * m_pInstrument[MAX_INSTRUMENTS];

	bool m_TrackMuted[MAX_TRACKS];
	bool m_TrackArmed[MAX_TRACKS];

	// WaveData ------------------------------------------------------
	//
	int m_WaveSelected;
	// Machines ------------------------------------------------------
	//
	bool m_MachineLock;
	Machine* m_pMachine[MAX_MACHINES];

	int m_SeqBus;

	int m_PW_Phase;
	int m_PW_Stage;
	int m_PW_Length;

	int m_DefaultPatternLines;
};


inline unsigned char * ::Song::_ppattern(int ps)
{
	if (!m_ppPatternData[ps])
	{
		return CreateNewPattern(ps);
	}
	return m_ppPatternData[ps];
}

inline unsigned char * ::Song::_ptrack(int ps, int track)
{
	if (!m_ppPatternData[ps])
	{
		return CreateNewPattern(ps);
	}
	return m_ppPatternData[ps] + (track * EVENT_SIZE);
}	

inline unsigned char * ::Song::_ptrackline(int ps, int track, int line)
{
	if (!m_ppPatternData[ps])
	{
		return CreateNewPattern(ps);
	}
	return m_ppPatternData[ps] + (track * EVENT_SIZE) + (line * MULTIPLY);
}



#endif
