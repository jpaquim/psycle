///\file
///\brief interface file for psycle::host::Player.
#pragma once
#include "Global.hpp"

#include <universalis/stdlib/condition.hpp>
#include <universalis/stdlib/date_time.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/thread.hpp>
#include "Machine.hpp"
#if !defined WINAMP_PLUGIN
	#include <psycle/helpers/riff.hpp>
	#include <psycle/helpers/dither.hpp>
#endif //!defined WINAMP_PLUGIN
namespace psycle
{
	namespace host
	{
		class Machine;

		/// schedule the processing of machines, sends signal buffers and sequence events to them, ...
		class Player
		{
		public:
			/// constructor.
			Player();
			/// destructor.
			virtual ~Player() throw();
			/// Moves the cursor one line forward, changing the pattern if needed.
			void AdvancePosition();
			/// Initial Loop. Read new line and execute Global commands/tweaks.
			void ExecuteGlobalCommands(void);
			/// Notify all machines that a new Tick() comes.
			void NotifyNewLine(void);
			/// Final Loop. Read the line again for notes to send to the Machines
			void ExecuteNotes(void);
			/// Function to encapsulate all the three functions above.
			void ExecuteLine();
			/// Indicates if the playback has moved to a new line. Used for GUI updating.
			bool _lineChanged;
			/// Used to indicate that the SamplesPerRow has been manually changed ( right now, in effects "pattern delay" and "fine delay" )
			bool _SPRChanged;
			/// the line currently being played in the current pattern
			int _lineCounter;
			/// the line at which to stop playing (used by save block to wav)
			int _lineStop;
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
			/// Contains the number of samples until a line change comes in.
			int _samplesRemaining;
			/// starts to play, also specifying a line to stop playing at.
			void Start(int pos,int lineStart,int lineStop,bool initialize=true);
			/// starts to play.
			void Start(int pos,int line,bool initialize=true);
			/// wether this player has been started.
			bool _playing;
			/// wether this player should only play the selected block in the sequence.
			bool _playBlock;
			/// wheter this player should play the song/block in loop.
			bool _loopSong;
			/// Indicates if the player is processing audio right now (It is in work function)
			bool _isWorking;
			/// stops playing.
			void Stop();
			/// work function. (Entrance for the callback function (audiodriver)
			static float * Work(void* context, int nsamples);
			float * Work(int numSamples);
			
			void SetBPM(int _bpm,int _tpb=0);

			void RecalcSPR() { SamplesPerRow((m_SampleRate*60)/(bpm*tpb)); }

			/// Returns the number of samples that it takes for each row of the pattern to be played
			const int SamplesPerRow(){ return m_SamplesPerRow;}
			/// Sets the number of samples that it takes for each row of the pattern to be played
			void SamplesPerRow(const int samplePerRow){m_SamplesPerRow = samplePerRow;}
			const int SampleRate() { return m_SampleRate; }
			void SampleRate(const int sampleRate);

			/// used by the plugins to indicate that they need redraw.
			bool Tweaker;
			///\name secondary output device, write to a file
			///\{
			/// starts the recording output device.
			void StartRecording(std::string psFilename,int bitdepth=-1,int samplerate =-1, int channelmode =-1, bool isFloat = false, bool dodither=false, int ditherpdf=0, int noiseshape=0, std::vector<char*> *clipboardmem=0);
			/// stops the recording output device.
			void StopRecording(bool bOk = true);
			bool ClipboardWriteMono(float sample);
			bool ClipboardWriteStereo(float left, float right);
			/// wether the recording device has been started.
			bool _recording;
			/// wether the recording is done to memory.
			bool _clipboardrecording;
			std::vector<char*> *pClipboardmem;
			int clipbufferindex;
			/// whether to apply dither to recording
			bool _dodither;
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
#if !defined WINAMP_PLUGIN
			/// file to which to output signal.
			WaveFile _outputWaveFile;
			/// dither handler
			helpers::dsp::Dither dither;
#endif //!defined WINAMP_PLUGIN

			/// samples per row. (Number of samples that are produced for each line(row) of pattern)
			/// This is computed from  BeatsPerMin(), LinesPerBeat() and SamplesPerSecond()
			int m_SamplesPerRow;
			int m_SampleRate;
			short _patternjump;
			short _linejump;
			short _loop_count;
			short _loop_line;



				///\name multithreaded scheduler
	///\{
		private:
			void start_threads();
			void stop_threads();

			typedef std::list<universalis::stdlib::thread*> threads_type;
			threads_type threads_;
		public:
			unsigned long num_threads() { if(threads_.empty()){return 1;} return (unsigned long)threads_.size(); }
		private:
			void thread_function(std::size_t thread_number);

			typedef class universalis::stdlib::scoped_lock<universalis::stdlib::mutex> scoped_lock;
			universalis::stdlib::mutex mutable mutex_;
			universalis::stdlib::condition<scoped_lock> mutable condition_;
			universalis::stdlib::condition<scoped_lock> mutable main_condition_;

			bool stop_requested_;
			bool suspend_requested_;
			std::size_t suspended_;

			typedef std::list<Machine*> nodes_queue_type;
			/// nodes with no dependency.
			nodes_queue_type terminal_nodes_;
			/// nodes ready to be processed, just waiting for a free thread
			nodes_queue_type nodes_queue_;

			Machine::sched_deps input_nodes_, output_nodes_;

			std::size_t graph_size_, processed_node_count_;

			void suspend_and_compute_plan();
			void compute_plan();
			void clear_plan();
			void process_loop() throw(std::exception);
			int samples_to_process_;
	///\}
		};
	}
}
