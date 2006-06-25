#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/engine/psy3loader.hpp>
#include <psycle/host/engine/song.hpp>

namespace psycle {
	namespace host {

		std::string const Psy3Loader::FILE_FOURCC = "PSY3";
		/// Current version of the Song file and its chunks.
		/// format: 0xAABB
		/// A = Major version. It can't be loaded, skip the whole chunk.
		/// B = minor version. It can be loaded with the existing loader, but not all information will be avaiable.
		std::uint32_t const Psy3Loader::VERSION_INFO = 0x0000;
		std::uint32_t const Psy3Loader::VERSION_SNGI = 0x0000;
		std::uint32_t const Psy3Loader::VERSION_SEQD = 0x0000;
		std::uint32_t const Psy3Loader::VERSION_PATD = 0x0000;
		std::uint32_t const Psy3Loader::VERSION_MACD = 0x0000;
		std::uint32_t const Psy3Loader::VERSION_INSD = 0x0000;
		std::uint32_t const Psy3Loader::VERSION_WAVE = 0x0000;

		std::uint32_t const Psy3Loader::FILE_VERSION      = Psy3Loader::VERSION_INFO + Psy3Loader::VERSION_SNGI + Psy3Loader::VERSION_SEQD
			+ Psy3Loader::VERSION_PATD	+ Psy3Loader::VERSION_PATD + Psy3Loader::VERSION_MACD +	Psy3Loader::VERSION_INSD + Psy3Loader::VERSION_WAVE;




	}
}