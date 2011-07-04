#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */

#include "SongStructs.h"
#include "iConfigurable.h"
#include "iCompornentConfiguration.h"
namespace model {

class Machine;
class Instrument;

//namespace Song {


//}

namespace configuration {
	__interface iSong : public iCompornentConfiguration {
		const int DefaultPatternLines();
		void DefaultPatternLines(const int value);
	};
};

namespace SongConst {
	// �ÓI�ϐ���` Static Constants 
	const int MAX_NAME_LEN = 64;///< ���O�̍ő�l
	const int MAX_AUTHOR_LEN = 64;///< ��҂̍ő咷
	const int MAX_COMMENT_LEN = 256;///< �R�����g�̍ő咷
	const int PATTERN_NAME_LEN = 32;///< �p�^�[�����̍ő咷
};

__interface iSong : iConfigurable
{	

	// ���� Property
	const int MachineSoloed(); ///<�\�� Get
	void MachineSoloed(const int value);///<�\�� Set

	const string& FileName();///< �t�@�C����
	void FileName(const string& fileName);///< �t�@�C����

	const CPoint& ViewSize();///< �r���[�T�C�Y CPoint
	void ViewSize(const CPoint& point);///< �r���[�T�C�Y CPoint
	
	void ViewSizeX(const int xpos);///< �r���[�T�C�Y X
	const int ViewSizeX();///< �r���[�T�C�Y X

	void ViewSizeY(const int ypos);///< �r���[�T�C�Y Y
	const int ViewSizeY();///< �r���[�T�C�Y Y

	const bool IsSaved();///< �ۑ��������ǂ���
	void IsSaved(const bool bsaved);///<�ۑ��������ǂ���

	const int TrackSoloed();///< �g���b�N�\��
	void TrackSoloed(const int soloTrack);///< �g���b�N�\��

	const string& Name();///< �\���O��
	void  Name(const string& name);///< �\���O��

	const string& Author();///< ��Җ�
	void Author(const string& author);///< ��Җ�

	const string& Comment();///< �R�����g	
	void Comment(const string& comment);///< �R�����g	
	
	const bool IsTweaking();///< Tweak���邩
	void IsTweaking(const bool tweaking);///< Tweak���邩 

	const unsigned int CpuIdle();
	void CpuIdle(const unsigned int value);
	
	const unsigned int SampCount();
	void SampCount(const unsigned int value);

	const bool IsInvalided();
	void IsInvalided(const bool value);
	
	const int BeatsPerMin();

	void BeatsPerMin(const int value);
	
	const int TicksPerBeat();
	void TicksPerBeat(const int value);

	const int LineCounter();
	void LineCounter(const int value);
	
	const bool IsLineChanged();
	void IsLineChanged(const bool value);

	const char CurrentOctave();
	void CurrentOctave(const char value);

	const int SamplesPerTick();///< Tick���̃T���v����
	void SamplesPerTick(const int samplePerTick);///< Tick���̃T���v����
	
	/// �p�^�[���f�[�^�|�C���^�z��
	unsigned char * const pPatternData(const int index);
	/// �p�^�[���f�[�^�|�C���^�z��փ|�C���^���Z�b�g����
	void pPatternData(const int index,unsigned char * const pPattern);
	
	/// �v���C��
	const unsigned char PlayOrder(int index);
	void PlayOrder(const int index,const unsigned char value);
	
	const bool PlayOrderSel(const int index);
	void PlayOrderSel(const int index,const bool value);


	const int PatternLines(const int index);
	void PatternLines(const int index,const int value);
	
	TCHAR * PatternName(const int index);

	const int SongTracks();
	void SongTracks(const int value);

	const int MidiSelected();
	void MidiSelected(const int value);

	const int AuxcolSelected();
	void AuxcolSelected(const int value);

	const int TrackArmedCount();
	void TrackArmedCount(const int value);

	// InstrumentData
	const int InstSelected();
	void InstSelected(const int value);

	Instrument * const pInstrument(const int index);
	void pInstrument(const int index,Instrument * const value);
	
	const bool IsTrackMuted(const int index);
	void IsTrackMuted(const int index,const bool value);

	const bool IsTrackArmed(const int index);
	void IsTrackArmed(const int index,const bool value);

	// WaveData ------------------------------------------------------
	//
	const int WaveSelected();
	void WaveSelected(const int value);

	// Machines ------------------------------------------------------
	//
	const bool IsMachineLock();
	void IsMachineLock(const bool value);

	Machine* const pMachine(const int index);
	void pMachine(const int index,Machine* const value);

	const int SeqBus();
	void SeqBus(const int value);

	const int PlayLength();
	void PlayLength(const int value);


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
	const int AddBpm(const int addValue);
	void AddTpb(const int addValue);


	bool IsPatternUsed(int i);

	// Previews waving

	void PW_Work(float *psamplesL, float *pSamplesR, int numSamples);
	void PW_Play();
	
	const int PW_Phase();
	void PW_Phase(const int value);

	const int PW_Stage();
	void PW_Stage(const int value);

	const int PW_Length();
	void PW_Length(const int value);
	
	unsigned char * _ppattern(int ps);
	unsigned char * _ptrack(int ps, int track);
	unsigned char * _ptrackline(int ps, int track, int line);

	unsigned char * CreateNewPattern(int ps);
	void RemovePattern(int ps);

	void Load();
	void Load(const TCHAR* fName,const int fType);
	void Load(const TCHAR* fName);

	void Save(const string& fileName);

	const bool IsUnsaved();


};

};
