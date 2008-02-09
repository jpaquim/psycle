/* -*- mode:c++, indent-tabs-mode:t -*- */
// Constants used by psycle
#pragma once
#define PSYCLE__CONSTANTS
namespace psy
{
	namespace core
	{

		///\todo: Tweak slides (and midi slides that should be added aswell)
		/// number of samples per tweak slide update
		const int TWEAK_SLIDE_SAMPLES = 64;
		/// number of tws commands that can be active on one machine
		const int MAX_TWS = 16;


		///\todo: Question is if it is needed anymore to set these limits. we could have 256 machines.
		/// 0 be master (empty slot in pattern), 1 to 95 or 111 or 127 gens, and up to 255 fx.
		/// Slots avaiable to load machines of each class (gen and FX). Power of 2! Important!
		const int MAX_BUSES = 64;
		/// Max number of machines+1 (master)
		const int MAX_MACHINES = 129;
		/// Index of MasterMachine
		const int MASTER_INDEX = 128;

		/// Max number of instruments.
		const int MAX_INSTRUMENTS = 256;
		/// Instrument index of the wave preview. ( instrument.h(111): Instrument::id_type const PREV_WAV_INS(255); )
		//const int PREV_WAV_INS = 255; 


		///\todo: usability of this define. i.e. if we really want a fixed size array.
		/// Number of tracks of the sequence (psycle just support one sequence now). Modify this, CURRENT_FILE_VERSION_SEQD and add the appropiated load and save code.
		const int MAX_SEQUENCES = 1;


		///\todo: this will have a completely different meaning with multipattern
		/// maximum number of different patterns. PSY3 Fileformat supports up to 2^32. UI is limited to 256 for now.
		const int MAX_PATTERNS = 256;
		/// Max number of pattern tracks
		/*unsigned*/ int const MAX_TRACKS = 64;
		/// harcoded maximal number of lines per pattern
		const int MAX_LINES = 1024;
		/// maximum number of positions in the sequence. PSY3 Fileformat supports up to 2^32. UI is limited to 256 for now.
		const int MAX_SONG_POSITIONS = 256;

		
		///\todo: find a way out or removing this constant. Maybe have an unique pool of wires, with an index each
		/// Max input connections and output connections a machine can have.
		const int MAX_CONNECTIONS = 12;


		///\todo: All these won't have much use with the new data structures. Probably they'll be needed for loading compatibility
		/// Size in bytes of an event (note-aux-mac-effect). Increment if you add columns to a track. (like panning). Modify this, CURRENT_FILE_VERSION_PATD and add the apropiated load and save code.
		const int EVENT_SIZE = 5;
		/// Miscellaneous offset data.
		const int MULTIPLY = MAX_TRACKS * EVENT_SIZE;
		const int MULTIPLY2 = MULTIPLY * MAX_LINES;
		const int MAX_PATTERN_BUFFER_LEN = MULTIPLY2 * MAX_PATTERNS;

		///\todo: Mostly useless constants, or misplaced ones. Also, STREAM_SIZE should be dependant on audio driver parameters so we need some work.
		/// Lock latency acts like a semaphore (Sleep(LOCK_LATENCY)). Should we do a real semaphore instead?
		const int LOCK_LATENCY = 256;
		/// Temporary buffer to get all the audio from Master (which work in small chunks), and send it to the soundcard after converting it to float.
		const int MAX_DELAY_BUFFER = 65536;
		/// Sampler
		const int OVERLAPTIME = 128;
		/// Maximum size of the audio block to be passed to the Work() function.
		const int MAX_BUFFER_LENGTH = 256;
		const int STREAM_SIZE = MAX_BUFFER_LENGTH;

		/// Current version of the Song file chunks. 0xAABB  A= Major version (can't be loaded, skip the whole chunk), B=minor version. It can be loaded with the existing loader, but not all information will be avaiable.
		const int CURRENT_FILE_VERSION_INFO = 0x0000;
		const int CURRENT_FILE_VERSION_SNGI = 0x0000;
		const int CURRENT_FILE_VERSION_SEQD = 0x0000;
		const int CURRENT_FILE_VERSION_PATD = 0x0000;
		const int CURRENT_FILE_VERSION_MACD = 0x0000;
		const int CURRENT_FILE_VERSION_INSD = 0x0000;
		const int CURRENT_FILE_VERSION_WAVE = 0x0000;
		const int CURRENT_FILE_VERSION = CURRENT_FILE_VERSION_INFO+CURRENT_FILE_VERSION_SNGI+CURRENT_FILE_VERSION_SEQD+CURRENT_FILE_VERSION_PATD+CURRENT_FILE_VERSION_MACD+CURRENT_FILE_VERSION_INSD+CURRENT_FILE_VERSION_WAVE;

		const int CURRENT_CACHE_MAP_VERSION = 0x0002;

		///\todo add real detection of type size
		typedef unsigned char byte;
		typedef unsigned short word;
		typedef unsigned long dword;

	}
}
