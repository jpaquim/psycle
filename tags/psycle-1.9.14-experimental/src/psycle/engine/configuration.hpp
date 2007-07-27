///\interface psycle::host::Configuration
#pragma once
#include <psycle/audiodrivers/AudioDriver.hpp>
#include <cstddef>
namespace psycle
{
	namespace host
	{

		class CMidiInput;

		/// configuration.
		class Configuration
		{
		public:
			Configuration();
			~Configuration() throw();
			
			bool autoStopMachines;


		public:
			class midi_type
			{
				public:
					midi_type() : groups_(16), velocity_(0x0c), pitch_(1), raw_()
					{
						for(unsigned int i(0) ; i < groups().size() ; ++i) group(i).message() = group(i).command() = i + 1;
					}

				public:
					class group_with_message;
					class group_type
					{
						public:
							group_type(int const & command = 0) : record_(), type_(), command_(command), from_(), to_(0xff) {}

						public:
							bool const inline & record() const throw() { return record_; }
							bool       inline & record()       throw() { return record_; }
						private:
							bool                record_;

						public:
							int const inline & type() const throw() { return type_; }
							int       inline & type()       throw() { return type_; }
						private:
							int                type_;

						public:
							int const inline & command() const throw() { return command_; }
							int       inline & command()       throw() { return command_; }
						private:
							int                command_;

						public:
							int const inline & from() const throw() { return from_; }
							int       inline & from()       throw() { return from_; }
						private:
							int                from_;

						public:
							int const inline & to() const throw() { return to_; }
							int       inline & to()       throw() { return to_; }
						private:
							int                to_;

						public:
							typedef group_with_message with_message;
					};
					class group_with_message : public group_type
					{
						public:
							group_with_message() : message_() {}

						public:
							int const inline & message() const throw() { return message_; }
							int       inline & message()       throw() { return message_; }
						private:
							int                message_;
					};

				public:
					typedef std::vector<group_type::with_message> groups_type;
					groups_type              const inline & groups()                          const throw() { return groups_        ; }
					groups_type                    inline & groups()                                throw() { return groups_        ; }
					groups_type::value_type  const inline & group (std::size_t const & index) const throw() { return groups()[index]; }
					groups_type::value_type        inline & group (std::size_t const & index)       throw() { return groups()[index]; }
				private:
					groups_type                             groups_;

				public:
					group_type const inline & velocity() const throw() { return velocity_; }
					group_type       inline & velocity()       throw() { return velocity_; }
				private:
					group_type                velocity_;

				public:
					group_type const inline & pitch() const throw() { return pitch_; }
					group_type       inline & pitch()       throw() { return pitch_; }
				private:
					group_type                pitch_;

				public:
					bool const inline & raw() const throw() { return raw_; }
					bool       inline & raw()       throw() { return raw_; }
				private:
					bool                raw_;
			};

		public:
			midi_type const inline & midi() const throw() { return midi_; }
			midi_type       inline & midi()       throw() { return midi_; }
		private:
			midi_type                midi_;

		public:
			AudioDriver* _pOutputDriver;
			CMidiInput* _pMidiInput;
			int _midiHeadroom;

			bool Initialized() { return _initialized; }
			bool Read();
			void Write();

			inline int GetSamplesPerSec() const throw()
			{
				return _pOutputDriver->_samplesPerSec;
			}

			bool _initialized;


		public:
			std::string const & appPath                () const throw() { return program_executable_dir_; }
		private:
			std::string         program_executable_dir_;

		public:
			std::string const & GetInstrumentDir       () const throw() { return instrument_dir_; }
			               void SetInstrumentDir       (std::string const &);
			std::string const & GetCurrentInstrumentDir() const throw() { return current_instrument_dir_; }
			               void SetCurrentInstrumentDir(std::string const &);
		private:
			std::string instrument_dir_;
			std::string current_instrument_dir_;

		public:
			std::string const & GetSongDir             () const throw() { return song_dir_; }
			               void SetSongDir             (std::string const &);
			std::string const & GetCurrentSongDir      () const throw() { return current_song_dir_; }
			               void SetCurrentSongDir      (std::string const &);
		private:
			std::string song_dir_;
			std::string current_song_dir_;

		public:
			std::string const & GetSkinDir             () const throw() { return skin_dir_; }
			               void SetSkinDir             (std::string const &);
		private:
			std::string skin_dir_;

		public:
			std::string const & GetPluginDir           () const throw() { return plugin_dir_; }
			               void SetPluginDir           (std::string const &);
		private:
			std::string plugin_dir_;

		public:
			std::string const & GetVstDir              () const throw() { return vst_dir_; }
			               void SetVstDir              (std::string const &);
		private:
			std::string vst_dir_;

		};
	}
}
