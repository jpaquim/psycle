///\file
///\brief interface file for psycle::host::Player.
#pragma once
#include "constants.hpp"
#if !defined _WINAMP_PLUGIN_
	#include "Riff.hpp"
#endif
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
			/// Indicates if the playback has moved to a new line. Used for GUI updating.
			bool _lineChanged;
			/// the line currently being played in the pattern currently being played
			int _lineCounter;
			/// the position currently being played in the sequence of patterns
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
			#if defined _WINAMP_PLUGIN_
				/// work... why is that public?
				float * Work(void* context, int& nsamples);
			#else
				/// work... why is that public?
				static float * Work(void* context, int& nsamples);
			#endif
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
			/// Indicates to the playback engine that starts to process the current line in the pattern and send the events to machines.
			void ExecuteLine();
			/// Stores which machine played last in each track. this allows you to not specify the machine number in the pattern.
			int prevMachines[MAX_TRACKS];
			/// Stores the samplerate of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_rate;
			/// Stores the bitdepth of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_bits;
			/// Stores the channel mode (mono/stereo) of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_channelmode;
			/// Temporary buffer to get all the audio from Master (which work in small chunks), and send it to the soundcard after converting it to float.
			float _pBuffer[MAX_DELAY_BUFFER];
			#if !defined _WINAMP_PLUGIN_
				/// file to which to output signal.
				WaveFile _outputWaveFile;
			#endif
		};
	}
}