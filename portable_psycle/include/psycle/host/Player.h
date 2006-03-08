#pragma once
#include "constants.h"
#if !defined _WINAMP_PLUGIN_
	#include "Riff.h"
#endif
#include "Constants.h"
///\file
///\brief interface file for psycle::host::Player.
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
			/// ???
			void AdvancePosition();
			/// ???
			bool _lineChanged;
			/// the line currently being played in the pattern currently being played
			int _lineCounter;
			/// the position currently being played in the sequence of patterns
			int _playPosition;
			/// the pattern currently being played.
			int _playPattern;
			/// elapsed time since playing started in what unit?
			///\todo what unit?
			float _playTime;
			/// elapsed time since playing started in miliseconds?
			///\todo rename to a more explicit name.
			int _playTimem;
			/// the current beats per minute at which to play the song.
			/// can be changed from the song itself using commands.
			int bpm;
			/// the current ticks per beat at which to play the song.
			/// can be changed from the song itself using commands.
			int tpb;
			/// ???
			int _ticksRemaining;
			/// starts to play.
			void Start(int pos,int line);
			/// wether this player has been started.
			bool _playing;
			/// wether this player should only play the selected block
			bool _playBlock;
			/// wheter this player should play the song in loop.
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
			///\name secondary output device, write the a file
			///\{
			/// starts the recording output device.
			void StartRecording(std::string psFilename,int bitdepth=-1,int samplerate =-1, int channelmode =-1);
			/// stops the recording output device.
			void StopRecording(bool bOk = true);
			/// wether the recording device has been started.
			bool _recording;
			///\}
		protected:
			/// ???
			void ExecuteLine();
			/// the previous machine seen in each track
			int prevMachines[MAX_TRACKS];
			/// the previous what?
			///\todo what is that?
			int backup_rate;
			/// the previous what?
			///\todo what is that?
			int backup_bits;
			/// ???
			///\todo what is that?
			int backup_channelmode;
			/// ???
			float _pBuffer[MAX_DELAY_BUFFER];
			#if !defined _WINAMP_PLUGIN_
				/// file to which to output signal.
				WaveFile _outputWaveFile;
			#endif
		};
	}
}