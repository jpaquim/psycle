///\file
///\brief interface file for psycle::host::Player.
#pragma once
#include "constants.hpp"
#include "Riff.hpp"
namespace psycle
{
	namespace host
	{
		class Machine;

		/// schedule the processing of machines, sends signal buffers and sequence events to them, ...
		class Player
		{
		public:
			struct CMD
			{
				enum{
				EXTENDED	= 0xFE,
				SET_TEMPO	= 0xFF,
				NOTE_DELAY	= 0xFD,
				RETRIGGER   = 0xFB,
				RETR_CONT	= 0xFA,
				SET_VOLUME	= 0x0FC,
				SET_PANNING = 0x0F8,
				BREAK_TO_LINE = 0xF2,
				JUMP_TO_ORDER = 0xF3,
				ARPEGGIO	  = 0xF0,
			
				// Extended Commands from 0xFE
				PATTERN_LOOP  = 0xB0, // Loops the current pattern x times. 0xFEB0 sets the loop start point.
				PATTERN_DELAY =	0xD0, // causes a "pause" of x rows ( i.e. the current row becomes x rows longer)
				FINE_PATTERN_DELAY=	0xF0 // causes a "pause" of x ticks ( i.e. the current row becomes x ticks longer)
				};
			};

			/// constructor.
			Player();
			/// destructor.
			virtual ~Player() throw();
			/// Moves the cursor one line forward, changing the pattern if needed.
			void AdvancePosition();
			/// Indicates to the playback engine that starts to process the current line in the pattern and send the events to machines.
			void ExecuteLine();
			/// Indicates if the playback has moved to a new line. Used for GUI updating.
			bool _lineChanged;
			/// the line currently being played in the current pattern
			int _lineCounter;
			/// the sequence position currently being played
			int _playPosition;
			/// the pattern currently being played.
			int _playPattern;
			/// elapsed time since playing started. Units is seconds and the float type allows for storing milliseconds.
			float _playTime;
			/// elapsed time since playing started in minutes.It just serves to complement the previous variable
			///\todo There is no need for two vars.
			int _playTimem;
			/// the current beats per minute at which to play the song.
			/// can be changed from the song itself using commands.
			int bpm;
			/// the current ticks per beat at which to play the song.
			/// can be changed from the song itself using commands.
			int tpb;
			/// Contains the number of *samples* until a line change comes in.
			int _ticksRemaining;
			/// starts to play.
			void Start(int pos,int line);
			/// wether this player has been started.
			bool _playing;
			/// wether this player should only play the selected block in the sequence.
			bool _playBlock;
			/// wheter this player should play the song/block in loop.
			bool _loopSong;
			/// stops playing.
			void Stop();
			/// work function. (Entrance for the callback function (audiodriver)
			static float * Work(void* context, int& nsamples);
			
			void SetBPM(int _bpm,int _tpb=0);

			void RecalcSPR() { SamplesPerRow((m_SampleRate*60)/(bpm*tpb)); }

			/// Returns the number of samples that it takes for each row of the pattern to be played
			const int SamplesPerRow(){ return m_SamplesPerRow;};
			/// Sets the number of samples that it takes for each row of the pattern to be played
			void SamplesPerRow(const int samplePerRow){m_SamplesPerRow = samplePerRow;};
			const int SampleRate() { return m_SampleRate; }
			void SampleRate(const int sampleRate);

			/// used by the plugins to indicate that they need redraw.
			bool Tweaker;
			///\name secondary output device, write to a file
			///\{
			/// starts the recording output device.
			void StartRecording(std::string psFilename,int bitdepth=-1,int samplerate =-1, int channelmode =-1);
			/// stops the recording output device.
			void StopRecording(bool bOk = true);
			/// wether the recording device has been started.
			bool _recording;
			///\}

		protected:			
			/// Stores which machine played last in each track. this allows you to not specify the machine number everytime in the pattern.
			int prevMachines[MAX_TRACKS];
			/// Stores the samplerate of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_rate;
			/// Stores the bitdepth of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_bits;
			/// Stores the channel mode (mono/stereo) of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_channelmode;
			/// Temporary buffer to get all the audio from Master (which work in small chunks), and send it to the soundcard after converting it to float.
			float _pBuffer[MAX_DELAY_BUFFER];
			/// file to which to output signal.
			WaveFile _outputWaveFile;

			/// samples per row. (Number of samples that are produced for each line(row) of pattern)
			/// This is computed from  BeatsPerMin(), LinesPerBeat() and SamplesPerSecond()
			int m_SamplesPerRow;
			int m_SampleRate;
			short _patternjump;
			short _linejump;
		};
	}
}