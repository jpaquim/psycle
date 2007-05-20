///\file
///\brief interface file for psycle::host::Global.
#pragma once

namespace std
{
	typedef boost::uint32_t uint32_t;
	typedef boost::uint64_t	uint64_t;
	typedef boost::int64_t	int64_t;
	typedef boost::int32_t	int32_t;
}
namespace psycle
{
	namespace host
	{
		class Song;
		class Player;
		class Configuration;
		class InputHandler;
		namespace dsp
		{
			class Resampler;
		}
		namespace vst
		{
			class host;
		}


		//\todo: move this source to a better place.
		namespace cpu
		{
			typedef std::uint64_t cycles_type;
			cycles_type inline cycles()
			{
				union result_type
				{
					struct split_type
					{
						std::uint32_t lo, hi;
					} split;
					std::uint64_t value;
				} result;

				QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&result));
				return result.value;
			}
		}

		class Global
		{
		private:
		public:
			Global();
			virtual ~Global() throw();

			static Song * _pSong;
			static Player * pPlayer;
			static Configuration * pConfig;
			static dsp::Resampler * pResampler;
			static cpu::cycles_type _cpuHz;
			static InputHandler* pInputHandler;
			static vst::host* pVstHost;

			static inline Song           & song() { return *_pSong; }
			static inline Player         & player(){ return *pPlayer; }
			static inline Configuration  & configuration(){ return *pConfig; }
			static inline dsp::Resampler & resampler(){ return *pResampler; }
			static inline vst::host		 & vsthost(){ return *pVstHost; }

			static inline cpu::cycles_type cpu_frequency(                              ) /*const*/ throw() { return _cpuHz; }
//			void             inline cpu_frequency(cpu::cycles_type const & value)           throw() { cpu_frequency_ = value; }
		};

		namespace loggers
		{
			/// very low level, debug, flooding output.
			///\see levels::trace
			void trace(const std::string & string) throw();
			/// normal, informative output.
			///\see levels::info
			void info(const std::string & string) throw();
			/// warnings.
			///\see levels::warning
			void warning(const std::string & string) throw();
			/// exceptions thrown from software, via "throw some_exception;".
			///\see levels::exception
			void exception(const std::string & string) throw();
			/// exceptions thrown from cpu/os.
			/// They are translated into c++ exceptions, see operating_system::exceptions::translated.
			///\see levels::crash
			void crash(const std::string & string) throw();
			/// levels of importance.
			namespace levels
			{
				enum level
				{
					trace, ///< very low level, debug, flooding output.
					info, ///< normal, informative output.
					warning, ///< warnings.
					exception, ///< exceptions thrown from software, via "throw some_exception;".
					crash ///< exceptions thrown from cpu/os. They are translated into c++ exceptions, see operating_system::exceptions::translated.
				};
			}
		}
		/// Safer version of delete that clears the pointer automatically. Don't use it for arrays!
		/// \param pointer pointer to single object to be deleted.
		/// \param new_value the new value pointer will be set to. By default it is null.
		template<typename single_object> inline single_object * zapObject(single_object *& pointer, single_object * const new_value = 0)
		{
			if(pointer) delete pointer;
			return pointer = new_value;
		}

		/// Safer version of delete[] that clears the pointer automatically. Only use it for arrays!
		/// \param pointer pointer to array to be deleted.
		/// \param new_value the new value pointer will be set to. By default it is null.
		template<typename object_array> inline object_array * zapArray(object_array *& pointer, object_array * const new_value = 0)
		{
			if(pointer) delete [] pointer;
			return pointer = new_value;
		}
	}
}
