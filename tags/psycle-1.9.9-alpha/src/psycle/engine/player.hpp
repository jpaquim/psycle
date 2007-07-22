///\file
///\brief interface file for psycle::host::Player.
#pragma once
//#include "song.hpp"
#include <psycle/helpers/riff.hpp>
#include <psycle/helpers/dither.hpp>
#include <psycle/engine/machine.hpp>
namespace psycle
{
	namespace host
	{
		class Machine;
		class Song;

		/// schedule the processing of machines, sends signal buffers and sequence events to them, ...
		class Player
		{
		public:
			/// constructor.
			//Player();
			Player(Song &);
			/// destructor.
			virtual ~Player() throw();

		public:
			/// starts to play.
			void Start(int pos,int line);
			/// stops playing.
			void Stop();
			/// work function. (Entrance for the callback function (audiodriver)
			static float * Work(void* context, int& nsamples);
		private:
			float * Work(int & nsamples);

		public:
			/// Moves the playback cursor one line forward, changing the pattern if needed.
			void AdvancePosition();
			/// Initial Loop. Read new line and execute Global commands/tweaks.
			void ExecuteGlobalCommands();
			/// Notify all machines that a new Tick() comes.
			void NotifyNewLine();
			/// Final Loop. Read the line again for notes to send to the Machines
			void ExecuteNotes();
			/// Function to encapsulate all the three functions above.
			void ExecuteLine();

		public:
			Song inline & song() throw() { return *song_; }
			void song(Song & song) { this->song_ = &song; }
		private:
			Song * song_;

		PSYCLE__PRIVATE:
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
			float bpm;
			/// the current ticks per beat at which to play the song.
			/// can be changed from the song itself using commands.
			int tpb;
			/// wether this player has been started.
			bool _playing;
			/// wether this player should only play the selected block in the sequence.
			bool _playBlock;
			/// wheter this player should play the song/block in loop.
			bool _loopSong;

		public:
			/// ...
			void SetBPM(float _bpm,int _tpb=0);

			///\name sample rate
			///\{
				/// ...
				void RecalcSPR() { SamplesPerRow((m_SampleRate*60)/(bpm*tpb)); }
				/// Returns the number of samples that it takes for each row of the pattern to be played
				const int SamplesPerRow(){ return m_SamplesPerRow;};
				/// Sets the number of samples that it takes for each row of the pattern to be played
				void SamplesPerRow(const int samplePerRow){m_SamplesPerRow = samplePerRow;};
				const int SampleRate() { return m_SampleRate; }
				void SampleRate(const int sampleRate);
			///\}

			/// used by the plugins to indicate that they need redraw.
			bool Tweaker;

			///\name secondary output device, write to a file
			///\{
				/// starts the recording output device.
				void StartRecording(std::string psFilename,int bitdepth=-1,int samplerate =-1, int channelmode =-1, bool dodither=false, int ditherpdf=0, int noiseshape=0);
				/// stops the recording output device.
				void StopRecording(bool bOk = true);
				/// wether the recording device has been started.
				bool _recording;
				/// whether to apply dither to recording
				bool _dodither;
			///\}

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

			/// Used to indicate that the SamplesPerRow has been manually changed ( right now, in effects "pattern delay" and "fine delay" )
			bool _SPRChanged;
			/// Contains the number of samples until a line change comes in.
			int _samplesRemaining;
			/// samples per row. (Number of samples that are produced for each line(row) of pattern)
			/// This is computed from  BeatsPerMin(), LinesPerBeat() and SamplesPerSecond()
			int m_SamplesPerRow;
			int m_SampleRate;
			short _patternjump;
			short _linejump;
			short _loop_count;
			short _loop_line;
		};
	}
}
