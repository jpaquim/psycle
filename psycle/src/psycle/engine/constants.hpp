#pragma once
#include <psycle/engine/version.hpp>
#include <psycle/plugin_interface.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		using plugin_interface::MAX_TRACKS;
		using plugin_interface::MAX_BUFFER_LENGTH;

		/// Temporary buffer to get all the audio from Master (which work in small chunks), and send it to the soundcard after converting it to float.
		int const MAX_DELAY_BUFFER = 65536;

		/// Sampler
		int const OVERLAPTIME = 128;

		/// number of samples per tweak slide update
		int const TWEAK_SLIDE_SAMPLES = 64;
		/// number of tws commands that can be active on one machine
		int const MAX_TWS = 16;
		/// Slots avaiable to load machines of each class (gen and FX). Power of 2! Important!
		int const MAX_BUSES = 64;
		/// Max number of machines+1 (master)
		int const MAX_MACHINES = 129;
		/// Max number of instruments.
		int const MAX_INSTRUMENTS = 256;
		/// Number of tracks of the sequence (psycle just support one sequence now). Modify this, CURRENT_FILE_VERSION_SEQD and add the appropiated load and save code.
		int const MAX_SEQUENCES = 1;
		/// harcoded maximal number different patterns.
		int const MAX_PATTERNS = 256;
		/// harcoded maximal number of lines per pattern
		int const MAX_LINES = 256;

		int const MAX_VST_EVENTS = 128;
		/// Size in bytes of an event (note-aux-mac-effect). Increment if you add columns to a track. (like panning). Modify this, CURRENT_FILE_VERSION_PATD and add the apropiated load and save code.
		//PSYCLE__DEPRECATED("This sux.")
		#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
			#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
		#else
			#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
				int const EVENT_SIZE = 6;
			#else
				int const EVENT_SIZE = 5;
			#endif
		#endif
		/// PSY2-fileformat Constants
		int const OLD_MAX_TRACKS = 32;
		int const OLD_MAX_WAVES = 16;
		int const OLD_MAX_INSTRUMENTS = 255;
		int const OLD_MAX_PLUGINS = 256;
		/// \todo Lock latency acts like a semaphore (Sleep(LOCK_LATENCY)). Should we do a real semaphore instead?
		int const LOCK_LATENCY = 256;
		/// \todo changing this breaks file format.
		int const MAX_SONG_POSITIONS = 128;
		/// Max input connections and output connections a machine can have. (\todo: should be replaced by a dynamic array)
		/// [bohan] note: this is a preprocessor macro because it's used in a preprocessor loop in the master machine mixer gui (thanks to fucking mfc resource system)
		#define PSYCLE__MAX_CONNECTIONS 12
		/// Max input connections and output connections a machine can have. (\todo: should be replaced by a dynamic array)
		int const MAX_CONNECTIONS = PSYCLE__MAX_CONNECTIONS;

		/// Miscellaneous offset data.
		//PSYCLE__DEPRECATED("sux")
		int const MULTIPLY = MAX_TRACKS * EVENT_SIZE;
		//PSYCLE__DEPRECATED("sux")
		int const MULTIPLY2 = MULTIPLY * MAX_LINES;
		//PSYCLE__DEPRECATED("sux")
		int const MAX_PATTERN_BUFFER_LEN = MULTIPLY2 * MAX_PATTERNS;

		/// Current version of the Song file and its chunks.
		/// format: 0xAABB
		/// A = Major version. It can't be loaded, skip the whole chunk.
		/// B = minor version. It can be loaded with the existing loader, but not all information will be avaiable.
		std::uint32_t const CURRENT_FILE_VERSION      = 0x0000;
		std::uint32_t const CURRENT_FILE_VERSION_INFO = 0x0000;
		std::uint32_t const CURRENT_FILE_VERSION_SNGI = 0x0000;
		std::uint32_t const CURRENT_FILE_VERSION_SEQD = 0x0000;
		std::uint32_t const CURRENT_FILE_VERSION_PATD = 0x0000;
		std::uint32_t const CURRENT_FILE_VERSION_MACD = 0x0000;
		std::uint32_t const CURRENT_FILE_VERSION_INSD = 0x0000;
		std::uint32_t const CURRENT_FILE_VERSION_WAVE = 0x0000;

		std::uint32_t const CURRENT_CACHE_MAP_VERSION = 1;

	}
}
