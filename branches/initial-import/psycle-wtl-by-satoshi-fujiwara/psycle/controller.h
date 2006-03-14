#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */
class Song;

#include "Player.h"
#include "dsp.h"
#include "iController.h"
#include "iMainFrame.h"
#include "IConfiguration.h"
#include "iSong.h"

class InputHandler;

class Controller : public iController
{
public:
	~Controller(void);
	static iController & Instance(){
		if(m_pController.get() == NULL){
			boost::mutex::scoped_lock _lock(m_mutex);
			if(m_pController.get() == NULL){
				m_pController = std::auto_ptr<Controller>(new Controller());
			}
		}
		return (*m_pController);
	}

	static iController * InstancePtr(){
		if(m_pController.get() == NULL){
			boost::mutex::scoped_lock _lock(m_mutex);
			if(m_pController.get() == NULL){
				m_pController = std::auto_ptr<Controller>(new Controller());
			}
		}
		return m_pController.get();
	}

	::configuration::IConfiguration* const configuration(){return m_pConfig;};
	
	iSong* const Song(){return m_pSong;};

	::iPlayer* const Player(){return m_pPlayer;};
	::iResampler* const Resampler(){return m_pResampler;};
	::iMainFrame* const MainFrame(){return m_pMainFrame;};

	void UpdatePlayOrder(const bool mode);
	void UpdateSequencer();

	void SelectFile(string& inputFileName);
	void InitializeBeforeLoading();
	void ProcessAfterLoading();

	void AddBpm(const int addValue);
	void AddTpb(const int addValue);

	void NewFile();
	void SaveFile(const bool bFileSaveAs);

	const bool IsOutputActive() const {return  m_bOutputActive;};
	void IsOutputActive(const bool bActive){m_bOutputActive = bActive;};

	const ULONG CpuHz() const {return m_CpuHz;};

	const int SamplesPerSec() const
	{
		return m_pOutputDriver->SamplesPerSec();
	};

	::iAudioDriver* const OutputDriver() const {return m_pOutputDriver;};
	::IMidiInput* const MidiInput() const {return m_pMidiInput;};	// MIDI IMPLEMENTATION

	HRESULT Run(ULONG hInstance,BSTR lpstrCmdLine,LONG nCmdShow);

	const string& CurrentDirectory() const {return m_CurrentDirectory;}; 

private:
	Controller(void);
	ULONG m_CpuHz;
	::configuration::IConfiguration *m_pConfig;
	::iSong* m_pSong;
	::iPlayer* m_pPlayer;
	::iResampler* m_pResampler;
	::iMainFrame *m_pMainFrame;

	//InputHandler m_InputHandler;
	string m_CurrentDirectory;
	bool m_bOutputActive;
	int _numOutputDrivers;

	iAudioDriver** _ppOutputDrivers;
	iAudioDriver* m_pOutputDriver;
	IMidiInput* m_pMidiInput;	// MIDI IMPLEMENTATION

	int _samplesPerSec;

	static std::auto_ptr<Controller> m_pController;
	static boost::mutex m_mutex;
};

