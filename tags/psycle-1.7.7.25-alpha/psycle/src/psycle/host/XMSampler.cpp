#include "stdafx.h"
/** @file 
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
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
#include "XMInstrument.h"
#include "XMSampler.h"
#include "Song.h"
#include "FileIO.h"
//#include "IPsySongLoader.h"
#include "Configuration.h"
#pragma unmanaged

//namespace SF {
namespace psycle
{
	namespace host
	{
		TCHAR* XMSampler::_psName = _T("XMSampler");
		
		const int XMSampler::Channel::m_SinTable[64] = 
		{
			0 ,12 ,25 ,37 ,49 ,60 ,71 ,81 ,90 ,98 ,106 ,112 ,117 ,122 ,125 ,126 ,
			127 ,126 ,125 ,122 ,117 ,112 ,106 ,98 ,90 ,81 ,71 ,60 ,49 ,37 ,25 ,12 ,
			0 ,-12 ,-25 ,-37 ,-49 ,-60 ,-71 ,-81 ,-90 ,-98 ,-106 ,-112 ,-117 ,-122 ,-125 ,-126 ,
			-127 ,-126 ,-125 ,-122 ,-117 ,-112 ,-106 ,-98 ,-90 ,-81 ,-71 ,-60 ,-49 ,-37 ,-25 ,-12 
		};

		// Triangle wave table (ramp down)
		const int XMSampler::Channel::m_RampDownTable[64] =
		{
			0 ,-4 ,-8 ,-12 ,-16 ,-20 ,-24 ,-28 ,-32 ,-36 ,-40 ,-44 ,-48 ,-52 ,-56 ,-60 ,
			-64 ,-68 ,-72 ,-76 ,-80 ,-84 ,-88 ,-92 ,-96 ,-100 ,-104 ,-108 ,-112 ,-116 ,-120 ,-124 ,
			127 ,123 ,119 ,115 ,111 ,107 ,103 ,99 ,95 ,91 ,87 ,83 ,79 ,75 ,71 ,67 ,
			63 ,59 ,55 ,51 ,47 ,43 ,39 ,35 ,31 ,27 ,23 ,19 ,15 ,11 ,7 ,3 
		};

		// Square wave table
		const int XMSampler::Channel::m_SquareTable[64] =
		{
			127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,
			127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,127 ,
			-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,
			-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 ,-127 
		};

		// Random wave table
		const int XMSampler::Channel::m_RandomTable[64] =
		{
			98 ,-127 ,-43 ,88 ,102 ,41 ,-65 ,-94 ,125 ,20 ,-71 ,-86 ,-70 ,-32 ,-16 ,-96 ,
			17 ,72 ,107 ,-5 ,116 ,-69 ,-62 ,-40 ,10 ,-61 ,65 ,109 ,-18 ,-38 ,-13 ,-76 ,
			-23 ,88 ,21 ,-94 ,8 ,106 ,21 ,-112 ,6 ,109 ,20 ,-88 ,-30 ,9 ,-127 ,118 ,
			42 ,-34 ,89 ,-4 ,-51 ,-72 ,21 ,-29 ,112 ,123 ,84 ,-101 ,-92 ,98 ,-54 ,-95 
		};

		const int XMSampler::Voice::m_ft2VibratoTable[256] = 
		{
			0,-2,-3,-5,-6,-8,-9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
			-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,
			-43,-44,-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,
			-56,-57,-58,-59,-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,
			-63,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-63,-63,
			-63,-62,-62,-62,-61,-61,-60,-60,-59,-59,-58,-57,-56,-56,
			-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,
			-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,-24,-23,
			-22,-20,-19,-17,-16,-14,-12,-11,-9,-8,-6,-5,-3,-2,0,
			2,3,5,6,8,9,11,12,14,16,17,19,20,22,23,24,26,27,29,30,
			32,33,34,36,37,38,39,41,42,43,44,45,46,47,48,49,50,51,
			52,53,54,55,56,56,57,58,59,59,60,60,61,61,62,62,62,63,
			63,63,64,64,64,64,64,64,64,64,64,64,64,63,63,63,62,62,
			62,61,61,60,60,59,59,58,57,56,56,55,54,53,52,51,50,49,
			48,47,46,45,44,43,42,41,39,38,37,36,34,33,32,30,29,27,
			26,24,23,22,20,19,17,16,14,12,11,9,8,6,5,3,2 
		};

		// Random wave table
		const int XMSampler::Voice::m_RandomTable[64] =
		{
			98 ,-127 ,-43 ,88 ,102 ,41 ,-65 ,-94 ,125 ,20 ,-71 ,-86 ,-70 ,-32 ,-16 ,-96 ,
			17 ,72 ,107 ,-5 ,116 ,-69 ,-62 ,-40 ,10 ,-61 ,65 ,109 ,-18 ,-38 ,-13 ,-76 ,
			-23 ,88 ,21 ,-94 ,8 ,106 ,21 ,-112 ,6 ,109 ,20 ,-88 ,-30 ,9 ,-127 ,118 ,
			42 ,-34 ,89 ,-4 ,-51 ,-72 ,21 ,-29 ,112 ,123 ,84 ,-101 ,-92 ,98 ,-54 ,-95 
		};
		
		//XMInstrument XMSampler::m_Instruments[MAX_INSTRUMENT];

		XMSampler::XMSampler(int index)
		{
			
			_macIndex = index;
			_numPars = 0;
			_type = MACH_XMSAMPLER;
			_mode = MACHMODE_GENERATOR;
			_stprintf(_editName, _T("XMSampler"));

			_resampler.SetQuality(RESAMPLE_LINEAR);
			
			m_TicksPerRow = 6;// 
			m_BPM = Global::_pSong->BeatsPerMin();

			int i;
			for (i = 0; i < XMSampler::MAX_POLYPHONY; i++)
			{
				_voices[i].Init();
				_voices[i].pSampler(this);
			}

			for (i = 0; i < MAX_TRACKS; i++)
			{
				lastInstrument[i] = 255;
				m_Channel[i].pSampler(this);
				m_Channel[i].Index(i);
			}

			//for(i = 0; i < MAX_INSTRUMENT;i++)
			//{
		//		m_Instruments[i].Init();
	//		}
		}


		void XMSampler::Init(void)
		{
			Machine::Init();

			_numVoices = XMSampler::MAX_POLYPHONY;
			int i;
			for (i=0; i < _numVoices; i++)
			{
				_voices[i].Init();
			
				
			}

			for(i = 0; i < MAX_TRACKS;i++)
			{
				m_Channel[i].Init();
			}

		}

		void XMSampler::Work(int numSamples)
		{

			boost::recursive_mutex::scoped_lock _lock(m_Mutex);

			CPUCOST_INIT(cost);
			int i;

			if (!_mute)
			{
				int ns = numSamples;
				int _songtracks = Global::_pSong->SongTracks();
				int nextevent;

				while (ns)
				{
					nextevent = ns + 1;
					
					for (i = 0; i < _songtracks; i++)
					{
						if (TriggerDelay[i]._cmd)
						{
							if (TriggerDelayCounter[i] < nextevent)
							{
								nextevent = TriggerDelayCounter[i];
							}
						}
					}

					if (nextevent > ns)
					{
						for (i = 0; i < _songtracks; i++)
						{
							// come back to this
							if (TriggerDelay[i]._cmd)
							{
								TriggerDelayCounter[i] -= ns;
							}
						}

						for (int voice = 0; voice < _numVoices; voice++)
						{
							//VoiceWork(ns, voice);
							if(_voices[voice].IsPlaying()){
								_voices[voice].Work(ns,_pSamplesL,_pSamplesR,_resampler);
							}
						}
						ns = 0;
					}
					else
					{
						if (nextevent)
						{
							ns -= nextevent;
							for (int voice = 0; voice < _numVoices; voice++)
							{
								if(_voices[voice].IsPlaying()){
									_voices[voice].Work(nextevent,_pSamplesL,_pSamplesR,_resampler);
								}
								//VoiceWork(nextevent, voice);
							}
						}

						for (i = 0; i < _songtracks; i++)
						{
							// come back to this
							if (TriggerDelay[i]._cmd == PatternCmd::DELAY)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i,&TriggerDelay[i]);
									TriggerDelay[i]._cmd = 0;
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
							else if (TriggerDelay[i]._cmd == PatternCmd::RETRIG)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i,&TriggerDelay[i]);
									TriggerDelayCounter[i] 
									= (RetriggerRate[i] * Global::_pSong->SamplesPerTick()) / 256;
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
							else if (TriggerDelay[i]._cmd == 0xfa)
							{
								if (TriggerDelayCounter[i] == nextevent)
								{
									// do event
									Tick(i,&TriggerDelay[i]);
									TriggerDelayCounter[i] = (RetriggerRate[i]*Global::_pSong->SamplesPerTick())/256;
									int parameter = TriggerDelay[i]._parameter&0x0f;
									if (parameter < 9)
									{
										RetriggerRate[i]+= 4*parameter;
									}
									else
									{
										RetriggerRate[i]-= 2 * (16 - parameter);
										if (RetriggerRate[i] < 16)
										{
											RetriggerRate[i] = 16;
										}
									}
								}
								else
								{
									TriggerDelayCounter[i] -= nextevent;
								}
							}
						}
					}
				}

				Machine::SetVolumeCounter(numSamples);
				
				if ( Global::pConfig->autoStopMachines )
				{
					if (_volumeCounter < 8.0f)	{
						_volumeCounter = 0.0f;
						_volumeDisplay = 0;
						_stopped = true;
					}
					else _stopped=false;
				}
			}

			CPUCOST_CALC(cost, numSamples);
			_cpuCost += cost;
			_worked = true;
		}// XMSampler::Work()

		void XMSampler::Stop(void)
		{
			for (int i = 0; i < _numVoices; i++)
			{
				_voices[i].NoteOffFast();
			}
		}// XMSampler::Stop(void)

		bool XMSampler::Load(RiffFile& riffFile)
		{
			int i;
			char junk[256];
			memset(&junk, 0, sizeof(junk));

			riffFile.Read(&_editName,16);
			_editName[15] = 0;

			riffFile.Read(&_inputMachines[0], sizeof(_inputMachines));
			riffFile.Read(&_outputMachines[0], sizeof(_outputMachines));
			riffFile.Read(&_inputConVol[0], sizeof(_inputConVol));
			riffFile.Read(&_connection[0], sizeof(_connection));
			riffFile.Read(&_inputCon[0], sizeof(_inputCon));
		#if defined (_WINAMP_PLUGIN_)
			riffFile.Skip(96) ; // sizeof(CPoint) = 8.
		#else
			riffFile.Read(&_connectionPoint[0], sizeof(_connectionPoint));
		#endif
			riffFile.Read(&_numInputs, sizeof(_numInputs));
			riffFile.Read(&_numOutputs, sizeof(_numOutputs));

			riffFile.Read(&_panning, sizeof(_panning));
			Machine::SetPan(_panning);
			riffFile.Read(&junk[0], 8 * sizeof(int)); // SubTrack[]
			riffFile.Read(&_numVoices, sizeof(_numVoices)); // numSubtracks

		/*	if (_numVoices < 4)  // No more need for this code.
			{
				// Most likely an old polyphony
				_numVoices = 8;
			}
		*/
			riffFile.Read(&i, sizeof(int)); // interpol
			switch (i)
			{
			case 2:
				_resampler.SetQuality(RESAMPLE_SPLINE);
				break;
			case 0:
				_resampler.SetQuality(RESAMPLE_NONE);
				break;
			default:
			case 1:
				_resampler.SetQuality(RESAMPLE_LINEAR);
				break;
			}

			riffFile.Read(&junk[0], sizeof(int)); // outwet
			riffFile.Read(&junk[0], sizeof(int)); // outdry

			riffFile.Read(&junk[0], sizeof(int)); // distPosThreshold
			riffFile.Read(&junk[0], sizeof(int)); // distPosClamp
			riffFile.Read(&junk[0], sizeof(int)); // distNegThreshold
			riffFile.Read(&junk[0], sizeof(int)); // distNegClamp

			riffFile.Read(&junk[0], sizeof(char)); // sinespeed
			riffFile.Read(&junk[0], sizeof(char)); // sineglide
			riffFile.Read(&junk[0], sizeof(char)); // sinevolume
			riffFile.Read(&junk[0], sizeof(char)); // sinelfospeed
			riffFile.Read(&junk[0], sizeof(char)); // sinelfoamp

			riffFile.Read(&junk[0], sizeof(int)); // delayTimeL
			riffFile.Read(&junk[0], sizeof(int)); // delayTimeR
			riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackL
			riffFile.Read(&junk[0], sizeof(int)); // delayFeedbackR

			riffFile.Read(&junk[0], sizeof(int)); // filterCutoff
			riffFile.Read(&junk[0], sizeof(int)); // filterResonance
			riffFile.Read(&junk[0], sizeof(int)); // filterLfospeed
			riffFile.Read(&junk[0], sizeof(int)); // filterLfoamp
			riffFile.Read(&junk[0], sizeof(int)); // filterLfophase
			riffFile.Read(&junk[0], sizeof(int)); // filterMode

			return true;
		}


		void XMSampler::SaveSpecificChunk(RiffFile& riffFile)
		{
			int temp;
	//		UINT size = 2 * sizeof(temp);
	//		riffFile.Write(&size,sizeof(size));
			riffFile.Write(VERSION);
			riffFile.Write(_numVoices); // numSubtracks
			switch (_resampler._quality)
			{
			case RESAMPLE_NONE:
				temp = 0;
				break;
			case RESAMPLE_LINEAR:
				temp = 1;
				break;
			case RESAMPLE_SPLINE:
				temp = 2;
				break;
			}
			riffFile.Write(temp); // quality
			//

			riffFile.Write(m_bLinearFreq);
			riffFile.Write(m_BPM);
			riffFile.Write(m_TicksPerRow);
			
			// Instrument Data Save
			int numInstruments = 0;		
			for(int i = 0;i < MAX_INSTRUMENT;i++){
				if(m_Instruments[i].IsEnabled()){
					numInstruments++;
				}
			}

			riffFile.Write(numInstruments);

			for(int i = 0;i < MAX_INSTRUMENT;i++){
				if(m_Instruments[i].IsEnabled()){
					riffFile.Write(i);
					m_Instruments[i].Save(riffFile,XMSampler::VERSION);
				}
			}

		};//void SaveSpecificChunk(RiffFile& riffFile) 

		bool XMSampler::LoadSpecificFileChunk(RiffFile& riffFile, int version)
		{

			int temp;
			UINT _version;
			riffFile.Read(_version); 

			riffFile.Read(_numVoices); // numSubtracks
			riffFile.Read(temp); // quality

			switch (temp)
			{
			case 2:
				_resampler.SetQuality(RESAMPLE_SPLINE);
				break;
			case 0:
				_resampler.SetQuality(RESAMPLE_NONE);
				break;
			default:
			case 1:
				_resampler.SetQuality(RESAMPLE_LINEAR);
				break;
			}

			riffFile.Read(m_bLinearFreq);
			riffFile.Read(m_BPM);
			riffFile.Read(m_TicksPerRow);
			
			// Instrument Data Load
			int numInstruments;
			riffFile.Read(numInstruments);
			int idx;
			for(int i = 0;i < numInstruments;i++){
				riffFile.Read(idx);
				m_Instruments[idx].Load(riffFile,_version);
				m_Instruments[idx].IsEnabled(true);
			}

			return TRUE;
		};

		//void XMSampler::VoiceWork(int numsamples, int voice)
		//{
		//	_voices[voice].Work(numsamples,_pSamplesL,_pSamplesR,_resampler);
		//}// XMSampler::VoiceWork()

		void XMSampler::Tick()
		{
			
			boost::recursive_mutex::scoped_lock _lock(m_Mutex);

			for (int voice = 0; voice < _numVoices ; voice++)
			{
				_voices[voice].Tick();
			}
		}// XMSampler::Tick()


		void XMSampler::Tick(
			int channel,
			PatternEntry* pData)
		{
			boost::recursive_mutex::scoped_lock _lock(m_Mutex);

			rChannel(channel).EffectFlags(0);
		
			if ( _mute ) return; // Avoid new note entering when muted.

			if ( pData->_note > 120 ) // don't process twk , twf of Mcm Commands
			{
				if ( (pData->_cmd == 0 && pData->_volcmd == 0) || pData->_note != 255) return; // Return in everything but commands!
		
			}

			
			if (Global::_pSong->IsInvalided()) 
				return;
			
			// �{�C�X���������� //

			
			PatternEntry& data = *pData;///< �p�^�[���G���g���ւ̎Q��
			bool _bInstrumentSet = data._inst < 255;///< �C���X�g�������g���Z�b�g����Ă��邩�H
			bool _bvoiceFound = false;///<���݂��̃`�����l���Ŕ������̃{�C�X�����邩�H
			bool _bNoteOn = (data._note < 120) & (data._cmd != CMD::NOTE_OFF) & (data._volcmd != CMD::NOTE_OFF);///<�m�[�g�I�����H
			bool _bNotPorta2Note = !((data._cmd == CMD::PORTA2NOTE) || (data._volcmd == CMD::PORTA2NOTE));
			
			///< ���݂��̃`�����l���Ŕ������Ă���{�C�X�̃C���f�b�N�X
			int _current;///< ���݂��̃`�����l���Ŕ������Ă���{�C�X�̃C���f�b�N�X
			
			XMSampler::Channel& _channel = rChannel(channel);///< �`�����l���I�u�W�F�N�g�ւ̎Q��
			_channel.IsPeriodChange(false);
			_channel.IsVolumeChange(false);
			
			// ���O�����G�t�F�N�g
			switch(data._cmd)
			{
							// Set Speed 
				case CMD::SETSPEED:
					if(data._parameter > 0){
						
						if(data._parameter < 0x20){
							// < 20 is speed.
							TicksPerRow(data._parameter);
						} else {
							// > 20 is BPM.
							BPM(data._parameter);// XMSampler inner BPM
						}

						CalcBPMAndTick();

					}
					break;

			}
			
			Voice* _currentVoice;///< ���݂��̃`�����l���Ŕ������Ă���{�C�X�ւ̃|�C���^
			Voice* _newVoice;///< �����Ώۂ̃{�C�X�ւ̃|�C���^

			// ���݂��̃`�����l���Ŕ������̃{�C�X����������
			for(_current = 0;_current < XMSampler::MAX_POLYPHONY;_current++)
			{
				if ( (_voices[_current].Channel() == channel) & (_voices[_current].IsPlaying())) // NoteOff previous Notes in this channel.
				{
					_bvoiceFound = true;
					_currentVoice = &(_voices[_current]);
					break;
				}
			}

			// �{�C�X������������ANNA�̒l�ɂ��������ăm�[�g�I�t����
			if(_bvoiceFound)
			{
				if ((_bNoteOn) & (_bNotPorta2Note))
				{	
					// New Note Action
					switch (m_Instruments[_currentVoice->Instrument()].NNA())
					{
						case 0:
							// �Ԃ�������
							_currentVoice->NoteOffFast();
							break;
						case 1:
							// NNA
							_currentVoice->NoteOff();
							break;
					}
				} else if(data._note == 120 ){
					_currentVoice->NoteOff();
				}
			}
			
			// �V�����󂫃{�C�X�̌���
			if(((_bNoteOn) & (_bNotPorta2Note)) | (!_bvoiceFound))
			{
				int _newVoiceIndex = -1 ;///< �󂫃{�C�X�ւ̃C���f�b�N�X
				bool _bFound = false;///< �󂫃{�C�X�������������H

				for (int _voice = 0; _voice < XMSampler::MAX_POLYPHONY; _voice++)	// Find a voice to apply the new note
				{
					if(!_voices[_voice].IsPlaying()){
						_newVoiceIndex = _voice;
						break;
					} else if(_voices[_voice].AmplitudeEnvelope().Stage() == EnvelopeController::EnvelopeStage::END){
						_newVoiceIndex = _voice;
					}
				}

				if(_newVoiceIndex == -1)
					return;
				
				_newVoice = &(_voices[_newVoiceIndex]);

				if(_bvoiceFound){
					_currentVoice->Channel(-1);
				}
			} else {
				_newVoice = _currentVoice;
			}

			// Tick�J�E���^�̏�����
			_newVoice->TickCounter(0);
			
			// XMTick�G�~�����[�g�p
			m_DeltaTick = 
				Global::_pSong->SamplesPerTick() 
				* Global::_pSong->TicksPerBeat() / (4 * TicksPerRow());
	//			* Global::_pSong->TicksPerBeat() / (4 * (TicksPerRow()));

			m_TickCount = 0;
			_newVoice->NextTick(0);
			_newVoice->ClearEffectFlags();
			_newVoice->TickRemain(TicksPerRow() - 1);
			// �C���X�g�������g�̃Z�b�g
			

			if(_bInstrumentSet){// 
				_newVoice->Instrument(data._inst);
				_newVoice->pInstrument(&(m_Instruments[data._inst]));
				_channel.InstrumentNo(data._inst);
			} else {
				if(_channel.InstrumentNo() >= 255)
				{
					return;
				}
				_newVoice->Instrument(_channel.InstrumentNo());
				_newVoice->pInstrument(&(m_Instruments[_channel.InstrumentNo()]));

			}


			// �`�����l���̐ݒ�
			_newVoice->Channel(channel);
			_newVoice->pChannel(&(m_Channel[channel]));

			XMInstrument & _inst = m_Instruments[_newVoice->Instrument()];
			
			// Total Wave Length
			int _layer = 0; // Change this when adding working Layering code.
			
			// �m�[�g�I���̏���

			if(_bNoteOn)
			{
				int _note = data._note;

				_channel.Porta2NoteDestNote(_note);


				if(_bNotPorta2Note ){
					_layer = _inst.NoteToSample(data._note);
					_channel.Note(_note,_layer);
				}
				
				
				_layer = _inst.NoteToSample(_channel.Note());
				
				int twlength = _inst.rWaveData(_layer).WaveLength();

				if(twlength > 0){

					if((!_bNotPorta2Note) & (_bvoiceFound))
					{
						_bNoteOn = false; // ���݂�Voice����������
						if((_newVoice->AmplitudeEnvelope().Stage() == EnvelopeController::RELEASE) | (_newVoice->AmplitudeEnvelope().Stage() == EnvelopeController::END))
						{
							_channel.Note(data._note,_inst.NoteToSample(data._note));
							_layer = _inst.NoteToSample(_channel.Note());
							_newVoice->Wave().Layer(_layer);
							_newVoice->NoteOn();
						} else {
							_newVoice->Wave().Layer(_layer);
						}
					} else 	{
						_newVoice->VoiceInit(_channel.Note()); // ������
						_channel.EffectInit();

						if (( data._cmd == CMD::EXTENDED) & ((data._parameter & 0xf0) == CMD::EXT_NOTEDELAY))
						{
							_newVoice->TriggerNoteDelay( 
								(Global::_pSong->SamplesPerTick() / 6)
								* (data._parameter & 0x0f));
						} else {
							// ���̏ꍇ�̓m�[�g�I��
							_newVoice->NoteOn();

							if ((data._cmd == CMD::RETRIG) & ((data._parameter & 0x0f) > 0))
							{
								_newVoice->Retrigger(data._parameter & 0x0f
									,(data._parameter & 0xf0) >> 4);
							}
							else
							{
								_newVoice->TriggerNoteDelay(0);
							}
						}
						
						// �E�F�[�u�I�t�Z�b�g�R�}���h
						__int64 w_offset;

						if (data._cmd == CMD::OFFSET)
						{
							w_offset = data._parameter * _newVoice->Wave().Length();
							_newVoice->Wave().Position(w_offset << 24);
						}
						else
						{
							_newVoice->Wave().Position(0);
						}

					}
					

				}
			} 

			// �{�����[���R�}���h /////////////////////////////////////////////////////
			
			if((data._cmd == CMD::VOLUME) | (data._volcmd == CMD::VOLUME))
			{
				// �{�����[���R�}���h
				int _volume = (data._cmd == CMD::VOLUME)?data._parameter:data._volume;
				_channel.Volume(_volume);
				_channel.IsVolumeChange(true);
			} 
			else if (_bNoteOn)
			{
				if(!_bInstrumentSet)
				{
					// �{�����[���͌��݂̐ݒ�������p��
					_channel.IsVolumeChange(true);
				} else {

					rChannel(channel).Volume(_inst.rWaveData(_layer).WaveVolume() * 255 / 100);
					_channel.IsVolumeChange(true);
				}
			} else if(_bInstrumentSet)
			{
				// �C���X�g�������g�݂̂̐ݒ�
				rChannel(channel).Volume(_inst.rWaveData(_layer).WaveVolume() * 255 / 100);
				_channel.IsVolumeChange(true);
			}
			
			// �p�� ////////////////////////////////////////////////////////////////

			if (_inst.IsRandomPanning())
			{
				// Instrument�̃����_���p���j���O
				_channel.PanFactor((float)rand() * 0.000030517578125f);
				_channel.IsVolumeChange(true);
			}
			else if ( data._cmd == CMD::PANNING) 
			{
				// Panninng �R�}���h
				_channel.PanFactor(CValueMapper::Map_255_1(data._parameter));
				_channel.IsVolumeChange(true);
				_channel.IsSurround(false);
			}
			else if ( data._volcmd == CMD::PANNING) 
			{
				// Panninng �R�}���h
				_channel.PanFactor(CValueMapper::Map_255_1(data._volume));
				_channel.IsVolumeChange(true);
				_channel.IsSurround(false);
			}
			else {
				// Instrument�̃f�t�H���g Panning
				if((_channel.PanFactor() < 0 || _bInstrumentSet) & (!_channel.IsSurround()))
				{
					_channel.PanFactor(_inst.Pan());
					_channel.IsVolumeChange(true);
				}
			}

			
			// �{�����[���G�t�F�N�g�R�}���h
			_channel.PerformEffect(*_newVoice,data._volcmd,data._volume);

			// Effect Command
			_channel.PerformEffect(*_newVoice,data._cmd,data._parameter);


			// �{�����[���l�̌v�Z //
			if(_channel.IsVolumeChange() & _newVoice->IsPlaying()){
				_newVoice->SetWaveVolume(_channel.Volume());
			}


			if(_channel.IsPeriodChange() & _newVoice->IsPlaying()){
				_newVoice->PeriodToSpeed();
			}
		}



	// ----------------------------------------------------------------------------
	// Voice Class Implementation 
	// ----------------------------------------------------------------------------
		
		void XMSampler::Voice::VoiceInit(const UCHAR note)
		{

			// Init filter synthesizer
			//

			XMInstrument & _inst = m_pSampler->Instrument(_instrument);

			// �s�b�`��������G�t�F�N�g�l�̏�����
			m_pChannel->EffectInit();

			// Envelope ������
			m_FilterEnvelope.Init(_inst.FilterEnvelope());
			m_AmplitudeEnvelope.Init(_inst.AmpEnvelope());
			m_PitchEnvelope.Init(_inst.PitchEnvelope());
			m_PanEnvelope.Init(_inst.PanEnvelope());

			_filter.Init();

			if (_inst.IsRandomCutoff())
			{
				_cutoff = alteRand(_inst.FilterCutoff());
			}
			else
			{
				_cutoff = _inst.FilterCutoff();
			}
			
			if (_inst.IsRandomResonance())
			{
				_filter._q = alteRand(_inst.FilterResonance());
			}
			else
			{
				_filter._q = _inst.FilterResonance();
			}

			_filter._type = (FilterType)_inst.FilterType();
			_coModify = (float)_inst.FilterEnvAmount();
			
	//		_filterEnv.Sustain(
	//			(float)Global::_pSong->pInstrument(_instrument)->ENV_F_SL * 0.0078125f
	//		);

	//		FilterEnvelope().Stage(Envelope::ATTACK);
	//
	//		_filterEnv.Step( 
	//			(1.0f / Global::_pSong->pInstrument(_instrument)->ENV_F_AT)
	//			* (44100.0f / Global::pConfig->_pOutputDriver->_samplesPerSec)
	//		);
	//
	//		_filterEnv.Value(0);
			
			// Init Wave
			//
			
			m_WaveDataController.Init(_inst,rChannel(),note);
			PeriodToSpeed();

		}// XMSampler::Voice::VoiceInit) 

		void XMSampler::Voice::Work(int numSamples,float * pSamplesL,float * pSamplesR,Cubic& _resampler)
		{
			PRESAMPLERFN pResamplerWork;
			pResamplerWork = _resampler._pWorkFn;

			float left_output = 0.0f;
			float right_output = 0.0f;

			//TickCounter( TickCounter() + numSamples);

			if (Global::_pSong->IsInvalided())
			{
				IsPlaying(false);
				return;
			}

			bool _bTrigger = false;
			
			while (numSamples)
			{
				_tickCounter++;
				
				if (!_bTrigger  & (_triggerNoteDelay) & (_tickCounter >= _triggerNoteDelay))
				{
					_bTrigger = true;

					if ( effCmd == CMD::RETRIG && effretTicks)
					{
						_triggerNoteDelay = _tickCounter + effVal;
						
						NoteOn();
						
						effretTicks--;
						m_WaveDataController.Position(0);

						if ( effretMode == 1 )
						{
							m_WaveDataController.LVolumeDest(m_WaveDataController.LVolumeDest() + effretVol);
							m_WaveDataController.RVolumeDest(m_WaveDataController.RVolumeDest() + effretVol);
						}
						else if (effretMode == 2 )
						{
							m_WaveDataController.LVolumeDest(m_WaveDataController.LVolumeDest() * effretVol);
							m_WaveDataController.RVolumeDest(m_WaveDataController.RVolumeDest() * effretVol);
						}

					}
					else 
					{
						_triggerNoteDelay = 0;
					}
				}
				/*
				else if (!IsPlaying())
				{
					m_WaveDataController.LVolumeCurr(0);
					m_WaveDataController.RVolumeCurr(0);
					return;
				}*/
				else if (!_bTrigger & (_triggerNoteOff) & (_tickCounter >= _triggerNoteOff))
				{
					_bTrigger = true;
					_triggerNoteOff = 0;
					NoteOff();
				}

				// MOD�ł���Tick��������G�t�F�N�g���Ăяo���B
				// ?������SampleTick�P�ʂɕύX����
				if(_tickCounter > NextTick())
				{
					//::ATLTRACE2(_T("remain:%x tick:%x delta:%x next:%x "),m_TickRemain,_tickCounter,m_pSampler->DeltaTick(),m_NextTick);
					m_NextTick += m_pSampler->DeltaTick();

					PerformFx();
					if(m_TickRemain > 0){
						m_TickRemain--;
						if(m_TickRemain == 0){
							rChannel().EffectFlags(0);
						}
					}
				}
				
				// ���T���v������
				_LARGE_INTEGER _temp;
				_temp.QuadPart = m_WaveDataController.Position();
				left_output = pResamplerWork(
					*(m_WaveDataController.pLeft() + _temp.HighPart - 1),
					*(m_WaveDataController.pLeft() + _temp.HighPart),
					*(m_WaveDataController.pLeft() + _temp.HighPart + 1),
					*(m_WaveDataController.pLeft() + _temp.HighPart + 2), // Attention, this can (and does)go out of range 
					_temp.LowPart, _temp.HighPart, m_WaveDataController.Length());// It is not a problem since
				if (m_WaveDataController.IsStereo())								 // the resample function already takes care of it
				{
					right_output = pResamplerWork(
						*(m_WaveDataController.pRight() + _temp.HighPart - 1),
						*(m_WaveDataController.pRight() + _temp.HighPart),
						*(m_WaveDataController.pRight() + _temp.HighPart + 1),
						*(m_WaveDataController.pRight() + _temp.HighPart + 2), // Attention, this can (and does)go out of range 
						_temp.LowPart, _temp.HighPart, m_WaveDataController.Length());// It is not a problem since
				}															// the resample function already takes care of it

				// Filter section
				//
				if (_filter._type <= FILTER_BR)
				{
					m_FilterEnvelope.Work();

	//					TickFilterEnvelope();
					_filter._cutoff = _cutoff + Dsp::F2I(m_FilterEnvelope.ModulationAmount() * _coModify);

					if (_filter._cutoff < 0)
					{
						_filter._cutoff = 0;
					}

					if (_filter._cutoff > 127)
					{
						_filter._cutoff = 127;
					}

					_filter.Update();

					if (m_WaveDataController.IsStereo())
					{
						_filter.WorkStereo(left_output, right_output);
					}
					else
					{
						left_output = _filter.Work(left_output);
					}
				}

				// Calculate Volume
				float _volume = Wave().Volume();
			
				// Amplitude Envelope 
				if(m_AmplitudeEnvelope.Envelope().IsEnabled()){
					m_AmplitudeEnvelope.Work();
					_volume *= m_AmplitudeEnvelope.ModulationAmount();
				}

				if(m_pChannel->EffectFlags() & Channel::EffectFlag::TREMOLO)
				{
					_volume += m_pChannel->TremoloDelta();
				}

				// Volume Fade Out
				if(m_VolumeFadeSpeed > 0.0f)
				{
					m_VolumeFadeAmount -= m_VolumeFadeSpeed;
					if( m_VolumeFadeAmount < 0){
						_volume = 0.0f;
						IsPlaying(false);
						return;
					}
					_volume *= m_VolumeFadeAmount;
				}

				//_volume *= m_pChannel->ChannelVolume();

				if(_volume > 1.0f ){
					_volume = 1.0f;
				} else 	if(_volume  <  0.0f){
					_volume = 0.0f;
				}

				// Picth Envelope

				double _pitchmod = 1;
	//			if(Channel() != -1){
	//				_pitchmod = m_pChannel->PortamentoAmount()	*  m_pChannel->VibratoAmount() * AutoVibratoAmount();
	//			}
		
				if(m_PitchEnvelope.Envelope().IsEnabled()){
					m_PitchEnvelope.Work();
				}

				
				float _rvol = m_pChannel->PanFactor();
				float _lvol = 0;
				
				if(m_PanEnvelope.Envelope().IsEnabled()){
					m_PanEnvelope.Work();

		//			if(m_pChannel->PanFactor() > 0.5f){
		//				_rvol += (m_PanEnvelope.ModulationAmount() - 0.5f)*(1.0f - m_pChannel->PanFactor())*0.5f;
		//			} else {
		//				_rvol += (m_PanEnvelope.ModulationAmount() - 0.5f)*m_pChannel->PanFactor()*0.5f;
		//			}
					_rvol = (m_pChannel->PanFactor() - 0.5f + m_PanEnvelope.ModulationAmount());
					if(m_pChannel->EffectFlags() & XMSampler::Channel::EffectFlag::PANBRELLO){
						_rvol = _rvol - 0.5f + m_pChannel->PanbrelloDelta();
					}
					
					if(_rvol > 1.0f){
						_rvol = 1.0f;
					}

					if(_rvol < 0.0f){
						_rvol = 0.0f;
					}
					
					//	Wave().RVolumeDest(_rvol * 
				}

				_lvol = 1.0f - _rvol;
				if(_lvol > 1.0f) { _lvol = 1.0f;};
				if(_rvol > 1.0f) { _rvol = 1.0f;};

				// calculate volume
	/*				
				if(_lvol < 0.0f)
					_lvol = m_WaveDataController.LVolumeDest();

				if(m_WaveDataController.RVolumeCurr() < 0)
					m_WaveDataController.RVolumeCurr(m_WaveDataController.RVolumeDest());

				if(m_WaveDataController.LVolumeCurr() > m_WaveDataController.LVolumeDest())
					m_WaveDataController.LVolumeCurr(m_WaveDataController.LVolumeCurr() - 0.005f);

				if(m_WaveDataController.LVolumeCurr() < m_WaveDataController.LVolumeDest())
					m_WaveDataController.LVolumeCurr(m_WaveDataController.LVolumeCurr() + 0.005f);

				if(m_WaveDataController.RVolumeCurr() > m_WaveDataController.RVolumeDest())
					m_WaveDataController.RVolumeCurr(m_WaveDataController.RVolumeCurr() - 0.005f);

				if(m_WaveDataController.RVolumeCurr() < m_WaveDataController.RVolumeDest())
					m_WaveDataController.RVolumeCurr(m_WaveDataController.RVolumeCurr() + 0.005f);

				if(!m_WaveDataController.IsStereo())
					right_output = left_output;

				right_output *= (m_WaveDataController.RVolumeCurr()) * _volume;
				left_output *= (m_WaveDataController.LVolumeCurr()) * _volume;
	*/
				
				// Monoral�̏ꍇ�Aleft output��right output�̒l�𓯂��ɂ���
				if(!m_WaveDataController.IsStereo()){
					right_output = left_output;
				}

				
				right_output *= _rvol * _volume;
				left_output *= _lvol * _volume;
				
				if(m_WaveDataController.CurrentLoopDirection() == XMSampler::WaveDataController::LoopDirection::FORWARD){
					m_WaveDataController.Position(m_WaveDataController.Position() + (__int64)(m_WaveDataController.Speed() * _pitchmod * 4294967296.0));
				} else {
					m_WaveDataController.Position(m_WaveDataController.Position() - (__int64)(m_WaveDataController.Speed() * _pitchmod * 4294967296.0));
				}


				// Loop handler
				//

				
				if (m_WaveDataController.LoopType() != XMInstrument::WaveData::LoopType::DO_NOT){
					if(m_WaveDataController.LoopType() == XMInstrument::WaveData::LoopType::NORMAL){
					// Normal
						if((int)(m_WaveDataController.Position() >> 32) >= m_WaveDataController.LoopEnd())
						{
							m_WaveDataController.Position(((__int64)m_WaveDataController.LoopStart()) << 32);
						} 
					} else {
					// BiDi
						if(m_WaveDataController.CurrentLoopDirection() == XMSampler::WaveDataController::LoopDirection::FORWARD)
						{
							if((int)(m_WaveDataController.Position() >> 32)  >= m_WaveDataController.LoopEnd())
							{
								m_WaveDataController.Position(((__int64)m_WaveDataController.LoopEnd()) << 32);
								m_WaveDataController.CurrentLoopDirection(XMSampler::WaveDataController::LoopDirection::BACKWARD);
							} 
						} else {
							if((int)(m_WaveDataController.Position() >> 32)  <= m_WaveDataController.LoopStart())
							{
								m_WaveDataController.Position(((__int64)m_WaveDataController.LoopStart()) << 32);
								m_WaveDataController.CurrentLoopDirection(XMSampler::WaveDataController::LoopDirection::FORWARD);
							} 
						}
					}
				}
				
				if ((int)(m_WaveDataController.Position() >> 32) > m_WaveDataController.Length())
				{
					IsPlaying(false);
					return;
				}
					
				if(m_pChannel->IsSurround()){
					*pSamplesL++ = -(*pSamplesL + left_output);
				} else {
					*pSamplesL++ = (*pSamplesL + left_output);
				}
				*pSamplesR++ = *pSamplesR + right_output;
				numSamples--;
			}
		
		}

		// �m�[�g�I�t
		void XMSampler::Voice::NoteOff()
		{
			
			if(!IsPlaying()){
				return;
			}

			if(m_AmplitudeEnvelope.Envelope().IsEnabled())
			{
				m_AmplitudeEnvelope.NoteOff();
				m_VolumeFadeSpeed = m_pInstrument->VolumeFadeSpeed();
				m_VolumeFadeAmount = 1.0f;
			} else {
				IsPlaying(false);
				return;
			}
			
			if(m_FilterEnvelope.Envelope().IsEnabled()){
				m_FilterEnvelope.NoteOff();
			}
			
			if(m_PitchEnvelope.Envelope().IsEnabled()){
				m_PitchEnvelope.NoteOff();
			}

			if(m_PanEnvelope.Envelope().IsEnabled()){
				m_PanEnvelope.NoteOff();
			}
			

		}// XMSampler::Voice::NoteOff()

		// �m�[�g�I�t�t�@�[�X�g
		void XMSampler::Voice::NoteOffFast()
		{
		
			if(!IsPlaying()){
				return;
			}

			if(m_AmplitudeEnvelope.Envelope().IsEnabled()){
				m_AmplitudeEnvelope.NoteOff();
				// Fade Out Volume
				m_VolumeFadeSpeed = 1.0f / 64.0f;
				m_VolumeFadeAmount = 1.0f;
			} else {
				IsPlaying(false);
			}
			
			if(m_FilterEnvelope.Envelope().IsEnabled()){
				m_FilterEnvelope.NoteOff();
			}
			
			if(m_PitchEnvelope.Envelope().IsEnabled()){
				m_PitchEnvelope.NoteOff();
			}

			if(m_PanEnvelope.Envelope().IsEnabled()){
				m_PanEnvelope.NoteOff();
			}


		}// XMSampler::Voice::NoteOffFast()

		// ���g���K�[�R�}���h
		void XMSampler::Voice::Retrigger(const int ticks,const int volumeModifier)
		{
			effretTicks = ticks; // number of Ticks.
			effVal	= (Global::_pSong->SamplesPerTick() / (effretTicks + 1));
			
			switch (volumeModifier) 
			{
				case 0:
				case 8:	effretVol = 0; effretMode=0; break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5: effretVol = (float)(pow(2,volumeModifier-1)/64); effretMode=1; break;
				case 6: effretVol = 0.66666666f;	 effretMode=2; break;
				case 7: effretVol = 0.5f;			 effretMode=2; break;
				case 9:
				case 10:
				case 11:
				case 12:
				case 13: effretVol = (float)(pow(2,volumeModifier - 9) * ( -1 )) / 64; effretMode=1; break;
				case 14: effretVol = 1.5f;effretMode = 2; break;
				case 15: effretVol = 2.0f;effretMode = 2; break;
			}

			TriggerNoteDelay(effVal);
		}// XMSampler::Voice::Retrigger(const int ticks,const int volumeModifier)

		void XMSampler::Voice::Tick()
		{
			if ( effCmd != CMD::EXTENDED )
			{
				effOld = effCmd;
				effCmd = CMD::NONE;
				m_EffectFlags = 0;
			}
		}// XMSampler::Voice::Tick() -------------------------------------------

		void XMSampler::Voice::PerformFx()
		{
			m_pChannel->IsPeriodChange(false);

			if(m_pInstrument->IsAutoVibrato())
			{
				AutoVibrato();
			}

			if(this->Channel() != -1){
			
				if(m_pChannel->EffectFlags() & Channel::EffectFlag::PORTAUP)
				{
					m_pChannel->PortamentoUp();
				}

				if(m_pChannel->EffectFlags() & Channel::EffectFlag::PORTADOWN)
				{
					m_pChannel->PortamentoDown();
				}
				
				if(m_pChannel->EffectFlags() & Channel::EffectFlag::PORTA2NOTE)
				{
					m_pChannel->Porta2Note(*this);
				}
				
				if(m_pChannel->EffectFlags() & Channel::EffectFlag::VIBRATO)
				{
					m_pChannel->Vibrato();
				}

				if(m_pChannel->EffectFlags() & Channel::EffectFlag::VOLUMESLIDE)
				{
					m_pChannel->VolumeSlide();
				}

				if(m_pChannel->EffectFlags()  & Channel::EffectFlag::PANSLIDE)
				{
					m_pChannel->PanningSlide();
				}

			
				if(m_pChannel->EffectFlags() & Channel::EffectFlag::TREMOLO)
				{
					m_pChannel->Tremolo();
				}

				if(m_pChannel->EffectFlags() & Channel::EffectFlag::PANBRELLO)
				{
					m_pChannel->Panbrello();
				}

				if(m_pChannel->EffectFlags() & Channel::EffectFlag::NOTECUT)
				{
					m_pChannel->NoteCut();
				}
				
				if(m_pChannel->EffectFlags() & Channel::EffectFlag::GLOBALVOLSLIDE)
				{
					m_pChannel->GLobalVolumeSlide();
				}

				if(m_pChannel->EffectFlags() & Channel::EffectFlag::TREMOR)
				{
					m_pChannel->Tremor();
				} else {
					m_pChannel->IsTremorMute(false);
				}

				if(m_pChannel->IsVolumeChange())
				{
					SetWaveVolume(m_pChannel->Volume());
				}

				if(m_pChannel->IsArpeggio())
				{
					m_pChannel->Arpeggio();
				}
			}
			
			if(m_pChannel->IsPeriodChange()){
				PeriodToSpeed();
			}

				
			{
	//			int _test = 48;
	//			double _p = m_pChannel->NoteToPeriod(_test);
	//			int _n = m_pChannel->PeriodToNote(_p);
	//			ATLASSERT(_test == _n);
			}

		}// PerformFX() -------------------------------------------

		// �g�[���|���^�����g�����ݒ� //
		void XMSampler::Channel::Porta2Note(const UCHAR note,const int parameter,const int layer)
		{
			m_EffectFlags |= EffectFlag::PORTA2NOTE;
			
			if(parameter != 0) {
				// == 0 Continue
				m_PortamentoSpeed = parameter;
			}
			

			//int layer = 0; // Change this when adding working Layering code.
			//float const finetune = 
			//	CValueMapper::Map_255_1(m_Instruments[_instrument].rWaveData(layer).WaveFineTune());

			Porta2NoteDestPeriod(note,layer);
			//m_effDestPortaNote = 
			//	(pow(2.0f, ((note
			//		+ m_Instruments[_instrument].rWaveData(layer).WaveTune()) - 48.0f
			//		+ finetune) / 12.0f) 
			//		* (44100.0	/ Global::pConfig->_pOutputDriver->_samplesPerSec));
		}// Porta2Note()  -------------------------------------------


		void XMSampler::Voice::SetWaveVolume(const int volume)
		{

			if(!m_pChannel->IsTremorMute()){
				Wave().Volume(
					(float)m_pSampler->Instrument(_instrument).rWaveData(Wave().Layer()).WaveVolume()
					* 0.01f * CValueMapper::Map_255_1(volume)
				);
			} else {
				Wave().Volume( 0.0f);
			}
			
			m_pChannel->IsVolumeChange(false);
			
		} // SetWaveVolume -------------------------------------------


		void XMSampler::Voice::NoteOn()
		{
			IsPlaying(true);

			m_VolumeFadeSpeed = 0.0f;
			m_VolumeFadeAmount = 0.0f;

			if(m_AmplitudeEnvelope.Envelope().IsEnabled()){
				m_AmplitudeEnvelope.NoteOn();
			}

			if(m_PanEnvelope.Envelope().IsEnabled()){
				m_PanEnvelope.NoteOn();
			}

			if(m_FilterEnvelope.Envelope().IsEnabled()){
				m_FilterEnvelope.NoteOn();
			}

			if(m_PitchEnvelope.Envelope().IsEnabled()){
				m_PitchEnvelope.NoteOn();
			}

		};// NoteOn() --------------------------------------------------------

		void XMSampler::EnvelopeController::CalcStep(const int start,const int  end)
		{
			m_Step = (m_pEnvelope->Value(end) - m_pEnvelope->Value(start))
				/ (ValueType)(m_pEnvelope->Point(end) - m_pEnvelope->Point(start));
	//			* ( (ValueType)(44100.0) / (ValueType)(Global::pConfig->_pOutputDriver->_samplesPerSec));
		};// XMSampler::EnvelopeController::CalcStep() ----------------------------------

		void XMSampler::WaveDataController::Init(XMInstrument & instrument,Channel& channel,const int note){
			
			m_Layer = instrument.NoteToSample(note);
			int _layer = m_Layer;

			m_pL = const_cast<short *>(instrument.rWaveData(_layer).pWaveDataL());
			m_pR = const_cast<short *>(instrument.rWaveData(_layer).pWaveDataR());
			

			IsStereo(instrument.rWaveData(_layer).IsWaveStereo());
			Length(instrument.rWaveData(_layer).WaveLength());
			
			if (instrument.rWaveData(_layer).WaveLoopType() != XMInstrument::WaveData::LoopType::DO_NOT)
			{
				LoopType(instrument.rWaveData(_layer).WaveLoopType());
				LoopStart(instrument.rWaveData(_layer).WaveLoopStart());
				LoopEnd(instrument.rWaveData(_layer).WaveLoopEnd());
				CurrentLoopDirection(LoopDirection::FORWARD);
			}
			else
			{
				LoopType(XMInstrument::WaveData::LoopType::DO_NOT);
				CurrentLoopDirection(LoopDirection::FORWARD);
			}
			
			// Init Resampler
		
		//	if (instrument.rWaveData(_layer).IsWaveLooped())
		//	{
		//		double const totalsamples = 
		//			(double)(Global::_pSong->SamplesPerTick() 
		//			* instrument.Lines());
		//		Speed((__int64)((Length() / totalsamples) * 4294967296.0f));
		//	}	
		//	else
			//{
			//	const double finetune =
			//		CValueMapper::Map_255_1(instrument.rWaveData(_layer).WaveFineTune());
			//	const double _period = ((double)(note + instrument.rWaveData(_layer).WaveTune()) - 48.0 + finetune) / 12.0;

			//	Speed(
			//		(
			//			pow(2.0,_period)	* (44100.0 / (double)Global::pConfig->_pOutputDriver->_samplesPerSec)
			//		)
			//	);
			//}
		} // WaveDataController::Init() ---------------------------------------------------
		
		void XMSampler::Channel::Porta2NoteDestPeriod(const int note,const int layer)
		{
			//int layer = 0; // Change this when adding working Layering code.
			//
			//const double finetune = 
			//	(double)CValueMapper::Map_255_1(m_Instruments[m_InstrumentNo].rWaveData(layer).WaveFineTune());
			//m_Porta2NoteDestSpeed = 
			//pow(2.0, ((double)(note
			//		+ m_Instruments[m_InstrumentNo].rWaveData(layer).WaveTune()) - 48.0
			//		+ finetune) / 12.0) 
			//		* (44100.0	/ (double)Global::pConfig->_pOutputDriver->_samplesPerSec);

			m_Porta2NoteDestPeriod = NoteToPeriod(note,layer);

		}// Porta2NoteDestSpeed ----------------------------------


		void XMSampler::Channel::Porta2Note(XMSampler::Voice & voice)
		{
			

			if(m_Porta2NoteDestPeriod > m_Period)
			{
				m_Period += m_PortamentoSpeed;
				
			
				if(m_Period > m_Porta2NoteDestPeriod ){
					m_Period = m_Porta2NoteDestPeriod;
				}
				m_bPeriodChange = true;
			} else if(m_Porta2NoteDestPeriod < m_Period)
			{
				m_Period -= m_PortamentoSpeed;
				if(m_Period < m_Porta2NoteDestPeriod){
					m_Period = m_Porta2NoteDestPeriod;
				}
				m_bPeriodChange = true;
			}
			
	//		::ATLTRACE2(_T("porta2note: value:%x dest:%f curr:%f\n"),m_PortamentoValue,m_Porta2NoteDestSpeed,voice.Wave().Speed() * m_PortamentoAmount);
		}// Porta2Note() -------------------------------------------------------

		void XMSampler::Channel::Vibrato(const int depth,const int speed)
		{
			if(depth != 0){
				m_VibratoDepth = depth; 
			}

			if(speed != 0){
				m_VibratoSpeed = speed;
			}
			
			m_EffectFlags |= EffectFlag::VIBRATO;

		}// XMSampler::Voice::Vibrato(const int depth,const int speed) ------------------------
		
		void XMSampler::Channel::Vibrato()
		{
			
			int vdelta;
			
			switch (m_VibratoType & 0x03)
			{
			case 1:
				vdelta = m_RampDownTable[m_VibratoPos & 0x3f ];
				break;
			case 2:
				vdelta = m_SquareTable[m_VibratoPos & 0x3f];
				break;
			case 3:
				vdelta = m_RandomTable[m_VibratoPos & 0x3f];
				break;
			default:
				vdelta = m_SinTable[m_VibratoPos & 0x3f];
			}

			vdelta = ((vdelta * m_VibratoDepth) >> 6);
			VibratoAmount((double)vdelta);	
			m_VibratoPos = (m_VibratoPos + m_VibratoSpeed) & 0x3F;
			m_bPeriodChange = true;

		}//XMSampler::Channel::Vibrato() -------------------------------------

		void XMSampler::Voice::AutoVibrato()
		{
			
			int vdelta;
			XMInstrument& _inst = *m_pInstrument;
			
			switch (_inst.AutoVibratoType())
			{
			case 4:	// Random
				vdelta = m_RandomTable[m_AutoVibratoPos & 0x3F];
				m_AutoVibratoPos++;
				break;
			case 3:	// Ramp Down
				vdelta = ((0x40 - (m_AutoVibratoPos >> 1)) & 0x7F) - 0x40;
				break;
			case 2:	// Ramp Up
				vdelta = ((0x40 + (m_AutoVibratoPos >> 1)) & 0x7f) - 0x40;
				break;
			case 1:	// Square
				vdelta = (m_AutoVibratoPos & 128) ? +64 : -64;
				break;
			default:	// Sine
				vdelta = m_ft2VibratoTable[m_AutoVibratoPos & 255];
			}

			if(_inst.AutoVibratoSweep())
			{
				m_AutoVibratoDepth += (_inst.AutoVibratoDepth() << 8) / _inst.AutoVibratoSweep();
				if((m_AutoVibratoDepth >> 8) > _inst.AutoVibratoDepth()){
					m_AutoVibratoDepth = _inst.AutoVibratoDepth() << 8;
				}
			} else {
				m_AutoVibratoDepth = _inst.AutoVibratoDepth() << 8;
			}

			vdelta = ((vdelta * m_AutoVibratoDepth) >> 14);
			
			rChannel().AutoVibratoAmount((double)vdelta);
			rChannel().IsPeriodChange(true);

			m_AutoVibratoPos = (m_AutoVibratoPos + _inst.AutoVibratoRate());


		}//XMSampler::Voice::AutoVibrato() -------------------------------------

		void XMSampler::Channel::Tremolo()
		{
				
			int vdelta;
			
			switch (m_TremoloType & 0x03)
			{
			case 1:
				vdelta = m_RampDownTable[m_TremoloPos];
				break;
			case 2:
				vdelta = m_SquareTable[m_TremoloPos];
				break;
			case 3:
				vdelta = m_RandomTable[m_TremoloPos];
				break;
			default:
				vdelta = m_SinTable[m_TremoloPos];
			}

			vdelta = ((vdelta * m_TremoloDepth) >> 5);
			m_TremoloDelta = vdelta / 127.0f;
			m_TremoloPos = (m_TremoloPos + m_TremoloSpeed) & 0x3F;


		};// Tremolo() -------------------------------------------

		void XMSampler::Channel::Panbrello()
		{
				
			int vdelta;
			
			switch (m_PanbrelloType & 0x03)
			{
			case 1:
				vdelta = m_RampDownTable[m_PanbrelloPos];
				break;
			case 2:
				vdelta = m_SquareTable[m_PanbrelloPos];
				break;
			case 3:
				vdelta = m_RandomTable[m_PanbrelloPos];
				break;
			default:
				vdelta = m_SinTable[m_PanbrelloPos];
			}

			vdelta = ((vdelta * m_PanbrelloDepth) >> 5);
			m_PanbrelloDelta = vdelta / 127.0f;
			m_PanbrelloPos = (m_PanbrelloPos + m_PanbrelloSpeed) & 0x3F;


		};// Panbrello() -------------------------------------------

		/// �m�[�g�l����s���I�h�����߂�
		const double XMSampler::Channel::NoteToPeriod(const int note,const int layer)
		{
			XMInstrument::WaveData& _wave 
				= m_pSampler->Instrument(this->InstrumentNo()).rWaveData(layer);

			if(m_pSampler->IsLinearFreq())
			{
				// ���`���g��Ӱ��
				return ((10.0 /* octave */ * 12.0 /* note/octave */ * 64.0  - (double)(note + _wave.WaveTune()) * 64.0)
					- ((double)(_wave.WaveFineTune()) * 0.25));
			} else {
				// Amiga Period�̋ߎ���
				return pow(2.0,(116.898 - ((double)(note + _wave.WaveTune())
					+ ((double)_wave.WaveFineTune()) / 128.0))/12.0) * 32.0;
			}
		};

		/// �s���I�h����X�s�[�h�����߂�
		void XMSampler::Voice::PeriodToSpeed()
		{
			double _period = rChannel().Period();

			if(rChannel().IsArpeggio()){
				_period = rChannel().ArpeggioPeriod();
			}

			if(rChannel().IsGrissando())
			{
				int _note = rChannel().PeriodToNote(_period,Wave().Layer());
				_period = rChannel().NoteToPeriod(_note,Wave().Layer());
				// ATLTRACE2("%d %f",_note,_period);
			}

			if(m_pSampler->IsLinearFreq()){
				// Linear Frequency
				Wave().Speed(pow(2.0,(6.0 * 12.0 * 64.0 - (_period + rChannel().AutoVibratoAmount() + rChannel().VibratoAmount() + 30.0 * 64.0))/(12.0 * 64.0)) 
					* (44100.0 / (double)Global::pConfig->_pOutputDriver->_samplesPerSec));
			} else {
				// amiga period mode
				Wave().Speed(((7159090.5 /* Amiga Freq */) / ((_period + rChannel().AutoVibratoAmount() + rChannel().VibratoAmount() ) * 0.5 /* 1/2.0 */))/(8363.0)
					* (44100.0 / (double)Global::pConfig->_pOutputDriver->_samplesPerSec) * pow(2.0,-30.0/12.0));

			}
		};

		/// �s���I�h����m�[�g�����߂�
		const int XMSampler::Channel::PeriodToNote(const double period,const int layer)
		{
			XMInstrument::WaveData& _wave = m_pSampler->Instrument(this->InstrumentNo()).rWaveData(layer);

			if(m_pSampler->IsLinearFreq()){
				// period = ((10.0 * 12.0 * 64.0 - ((double)note + (double)_wave.WaveTune()) * 64.0)
				//	- (_wave.WaveFineTune() / 256.0) * 64.0);
				// period / 64.0 = 10.0 * 12.0  - ((double)note + (double)_wave.WaveTune()) - _wave.WaveFineTune() / 256.0;
				// note = (int)(10.0 * 12.0  - (double)_wave.WaveTune() - _wave.WaveFineTune() / 256.0 - period / 64.0 + 0.5/* �l�̌ܓ�*/);
				
				return (int)(10.0*12.0 - (double)_wave.WaveTune() - (double)_wave.WaveFineTune() / 256.0  - period / 64.0 + 0.5);
			} else {
				//period = pow(2.0,(116.898 - ((double)(note + _wave.WaveTune()) + (double)_wave.WaveFineTune() / 128.0))/12.0) * 32;
				//log2(period/32) = (116.898 - ((double)(note + _wave.WaveTune()) + (double)_wave.WaveFineTune() / 128.0))/12.0;
				//note/12 = (116.898 - ((double)(_wave.WaveTune()) + (double)_wave.WaveFineTune() / 128.0))/12.0 - log2(period/32); 
				//note = (116.898 - ((double)(_wave.WaveTune()) + (double)_wave.WaveFineTune() / 128.0)) - log2(period/32) * 12.0;
				int _note = (int)(116.898 - ((double)_wave.WaveTune() + (double)_wave.WaveFineTune() / 128.0) 
					-12.0 * log((double)(period / 32.0))/(0.69314718055994529 /*log(2)*/ ));
				return _note;
					
			}
		}

		/// �G�t�F�N�g����������
		void XMSampler::Channel::PerformEffect(Voice& voice,const int cmd,const int parameter)
		{
					// �G�t�F�N�g�R�}���h
			switch(cmd) // DO NOT ADD here those commands that REQUIRE a note.
			{
				case CMD::VOLUMESLIDE:
					VolumeSlide(0,true);
					break;
				case CMD::VOLSLIDEDOWN:
					VolumeSlide(parameter,false);
					break;
				case CMD::VOLSLIDEUP:
					VolumeSlide(parameter,true);
					break;
				case CMD::EXTENDED:
					if ((parameter & 0xf0) == CMD::EXT_NOTEOFF)
					{
						voice.TriggerNoteOff
							((Global::_pSong->SamplesPerTick() / 6)
							* (parameter & 0x0f));
					}
					break;
				case CMD::PORTAUP:
					PortamentoUp(parameter);
					break;
				case CMD::PORTADOWN:
					PortamentoDown(parameter);
					break;
				case CMD::VIBRATO:
					Vibrato((parameter & 0x0F)*4,((parameter >> 4) & 0x0F));
					break;

				// �g�[���|���^�����g
				case CMD::PORTA2NOTE:
					Porta2Note(Porta2NoteDestNote(),parameter * 4,voice.Wave().Layer());
					break;
				// �g������
				case CMD::TREMOLO:
					Tremolo((parameter & 0x0F) << 2,(parameter>> 4) & 0x0F);
					break;
				// Panbrello
				case CMD::PANBRELLO:
					Panbrello((parameter & 0x0F) << 2,(parameter>> 4) & 0x0F);
					break;
				case CMD::FINE_PORTAMENTO_DOWN:
					FinePortamentoDown(parameter);
					break;
				case CMD::FINE_PORTAMENTO_UP:
					FinePortamentoUp(parameter);
					break;
				case CMD::GRISSANDO:
					IsGrissando(parameter != 0);
					break;
				case CMD::VIBRATO_TYPE:
					VibratoType(parameter);
					break;
				case CMD::TONEPORTAVOL:
					if(parameter & 0xf0){
						VolumeSlide((parameter & 0xf0) >> 2,true);
					} else 
					{
						VolumeSlide((parameter & 0xf) << 2,false);
					}
					Porta2Note(Porta2NoteDestNote(),0,voice.Wave().Layer());
					break;
				case CMD::VIBRATOVOL:
					if(parameter & 0xf0){
						VolumeSlide((parameter & 0xf0) >> 2,true);
					} else {
						VolumeSlide((parameter & 0xf) << 2,false);
					}
					Vibrato(0);
					break;
				case CMD::PANNINGSLIDE:
					PanningSlide(0,true);
					break;
				case CMD::PANSLIDELEFT:
					PanningSlide(parameter,true);
					break;
				case CMD::PANSLIDERIGHT:
					PanningSlide(parameter,false);
					break;
				case CMD::TREMOLO_TYPE:
					TremoloType(parameter);
					break;
				case CMD::FINEVOLDOWN:
					// Fine Volume Down
					VolumeDown(parameter);
					IsVolumeChange(true);
					break;
				case CMD::FINEVOLUP:
					// Fine Volume Up
					VolumeUp(parameter);
					IsVolumeChange(true);
					break;
				case CMD::VIBRATO_SPEED:
					Vibrato(parameter);
					break;
				case CMD::VIBRATO_DEPTH:
					VibratoDepth(parameter);
					Vibrato(0);
					break;
				case CMD::ARPEGGIO:
					Arpeggio(parameter,voice.Wave().Layer());
					break;
				case CMD::NOTE_CUT:
					NoteCut(parameter);
					break;
				case CMD::SET_GLOBAL_VOLUME:
					m_pSampler->SetWireVolume(0,(float)parameter / 256.0f);
					break;

				case CMD::GLOBAL_VOLUME_SLIDE:
					GlobalVolumeSlide(parameter);
					break;

				case XMSampler::CMD::EXTRA_FINE_PORTAMENTO_DOWN:
					if(parameter != 0){
						m_ExtraFinePortamentoSpeed  = parameter;
					}
					m_Period -= m_ExtraFinePortamentoSpeed;
					IsPeriodChange(true);
					break;

				case XMSampler::CMD::EXTRA_FINE_PORTAMENTO_UP:
					if(parameter != 0){
						m_ExtraFinePortamentoSpeed  = parameter;
					}
					m_Period += m_ExtraFinePortamentoSpeed;
					IsPeriodChange(true);
					break;

				case XMSampler::CMD::POSITION_JUMP:
					break;
				case XMSampler::CMD::PLAY_FORWARD:
					break;
				case XMSampler::CMD::PLAY_BACKWARD:
					break;
				case XMSampler::CMD::PATTERN_DELAY:
					break;
				case XMSampler::CMD::PATTERN_LOOP:
					break;
				case XMSampler::CMD::PANBRELLO_WAVE_FORM:
					PanbrelloType(parameter);
					break;
				case XMSampler::CMD::FINE_PATTERN_DELAY:
					break;
				case XMSampler::CMD::SURROUND_OFF:
					IsSurround(false);
					break;
				case XMSampler::CMD::SURROUND_ON:
					SurroundOn();
					break;
				case XMSampler::CMD::REVERB_OFF:
					break;
				case XMSampler::CMD::REVERB_ON:
					break;
				case XMSampler::CMD::CENTER_SURROUND:
					break;
				case XMSampler::CMD::QUOAD_SURROUND:
					break;
				case XMSampler::CMD::GROBAL_FILTERS:
					break;
				case XMSampler::CMD::LOCAL_FILTERS:
					break;
				case XMSampler::CMD::SET_HIGH_OFFSET:
					break;
				case XMSampler::CMD::TREMOR:
					Tremor(parameter);
					break;
				case XMSampler::CMD::SET_PANNING:
					break;
				case XMSampler::CMD::RETRIGGER_NOTE:
					break;
				case XMSampler::CMD::FINE_VOLSLIDE_UP:
					break;
				case XMSampler::CMD::FINE_VOLSLIDE_DOWN:
					break;
				case XMSampler::CMD::NOTE_DELAY:
					break;
				case XMSampler::CMD::SET_ACTIVE_MACRO:
					break;
			}

		}
#if defined WTL
		void XMSampler::ResamplerQuality(const ::ResamplerQuality value){
			_resampler._quality = value;
		}

		const ::ResamplerQuality XMSampler::ResamplerQuality(){
			return _resampler._quality;
		}
#else
		void XMSampler::ResamplerQuality(const psycle::host::ResamplerQuality value){
			_resampler._quality = value;
		}

		const psycle::host::ResamplerQuality XMSampler::ResamplerQuality(){
			return _resampler._quality;
		}
#endif

		/// XM��Tempo��Speed������ۂ�Tempo�����delta tick���v�Z����
		void XMSampler::CalcBPMAndTick()
		{
			Global::_pSong->BeatsPerMin(6 * BPM() / TicksPerRow());
			Global::_pSong->SamplesPerTick(
				(Global::pConfig->_pOutputDriver->_samplesPerSec * 15 * 4)
				/ (Global::_pSong->BeatsPerMin() * Global::_pSong->TicksPerBeat())
			);
			m_DeltaTick = 
				Global::_pSong->SamplesPerTick() 
				* Global::_pSong->TicksPerBeat() / (4 * TicksPerRow());
		}
	}
}