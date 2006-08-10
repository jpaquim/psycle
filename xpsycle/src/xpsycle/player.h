///\file
///\brief interface file for psycle::host::Player.
/// schedule the processing of machines, sends signal buffers and sequence events to them, ...

#pragma once
#include "song.h"
#include "dither.h"
#include "machine.h"
#include "riff.h"

namespace psycle
{
	namespace host
	{

		class Player
		{
		public:

			Player();

			virtual ~Player();

			Song inline & song() throw() {
				return *song_;
			}

			void inline song(Song & song) {
				song_ = &song;
			}

			void setFileName( const std::string & fileName);
			const std::string fileName() const;

			sigslot::signal0<> recordStopped;

			void setAutoRecording( bool on );
			void startRecording( );
			void stopRecording( );


		private:

			Song * song_;
			std::string fileName_;

			bool autoRecord_;
			bool recording_;

			/// Final Loop. Read new line for notes to send to the Machines
			void ExecuteNotes( double beatOffset , PatternLine & line );
			void ProcessGlobalEvent(const GlobalEvent & event);
			void Process(int nsamples);


//		PSYCLE__PRIVATE:
		public:
			/// Indicates if the playback has moved to a new line. Used for GUI updating.
			bool _lineChanged;
			/// elapsed time since playing started. Units is seconds and the float type allows for storing milliseconds.
			float _playTime;
			/// elapsed time since playing started in minutes.It just serves to complement the previous variable
			///\todo There is no need for two vars.
			int _playTimem;
			/// the current beats per minute at which to play the song.
			/// can be changed from the song itself using commands.
			float bpm;
			/// starts to play.
			void Start(double pos);
			/// wether this player has been started.
			bool _playing;
			/// wether this player should only play the selected block in the sequence.
			bool _playBlock;
			/// wheter this player should play the song/block in loop.
			bool _loopSong;

		public:
			/// stops playing.
			void Stop();
			/// work function. (Entrance for the callback function (audiodriver)
			float * Work(int nsamples);
		//private:
			static float * Work(void* context, int& nsamples);


		public:
			/// ...
			void SetBPM(float _bpm,int _tpb=0);

			///\name sample rate
			///\{
				void RecalcSPB() { SamplesPerBeat((m_SampleRate*60)/bpm); }
				const float SamplesPerBeat(){ return m_SamplesPerBeat;};
				void SamplesPerBeat(const float samplePerBeat){m_SamplesPerBeat = samplePerBeat;};

				const int SampleRate() { return m_SampleRate; }
				void SampleRate(const int sampleRate);
			///\}

		public:
			double PlayPos() const {return playPos;}
			double SetPlayPos(int newpos) {playPos=newpos;}
		private:
			/// current master playback position
			double playPos;

		public:

			/// used by the plugins to indicate that they need redraw.
			bool Tweaker;

		private:
			/// Stores which machine played last in each track. this allows you to not specify the machine number everytime in the pattern.
			Machine::id_type prevMachines[MAX_TRACKS];
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
			/// dither handler
			dsp::Dither dither;

			float m_SamplesPerBeat;
			int m_SampleRate;

			void writeSamplesToFile( int amount );
			bool _doDither;

		///\name deprecated by multiseq
		///\{
		public:
				/// ...
				void RecalcSPR() { SamplesPerRow((m_SampleRate*60)/(bpm*tpb)); }
				/// Returns the number of samples that it takes for each row of the pattern to be played
				const int SamplesPerRow(){ return m_SamplesPerRow;};
				/// Sets the number of samples that it takes for each row of the pattern to be played
				void SamplesPerRow(const int samplePerRow){m_SamplesPerRow = samplePerRow;};
			/// Used to indicate that the SamplesPerRow has been manually changed ( right now, in effects "pattern delay" and "fine delay" )
			bool _SPRChanged;
			/// the line currently being played in the current pattern
			int _lineCounter;
			/// the sequence position currently being played
			int _playPosition;
			/// the pattern currently being played.
			/// Moves the cursor one line forward, changing the pattern if needed.
			//void ExecuteGlobalCommands( std::list<PatternLine*> & tempPlayLines );
			int tpb;
			/// Contains the number of samples until a line change comes in.
			int _samplesRemaining;


		private:
			/// samples per row. (Number of samples that are produced for each line(row) of pattern)
			/// This is computed from  BeatsPerMin(), LinesPerBeat() and SamplesPerSecond()
			int m_SamplesPerRow;
			short _patternjump;
			short _linejump;
			short _loop_count;
			short _loop_line;
		///\}
		};
	}
}
