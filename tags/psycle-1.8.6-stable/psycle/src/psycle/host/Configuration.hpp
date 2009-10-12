///\file
///\interface psycle::host::Configuration.
#pragma once
#include "AudioDriver.hpp"
#include "Version.hpp"
#include <universalis/compiler/stringized.hpp> // to convert a token into a string literal (UNIVERSALIS__COMPILER__STRINGIZED)
#include <cstddef>
namespace psycle
{
	namespace host
	{
		#define PSYCLE__PATH__REGISTRY__ROOT "Software\\" PSYCLE__TAR_NAME "\\" PSYCLE__BRANCH
		#define PSYCLE__PATH__REGISTRY__CONFIGKEY "Configuration--" UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__VERSION__MAJOR) "." UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__VERSION__MINOR)
		#define PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN "Psycle Default (internal)"
		#define PSYCLE__PATH__DEFAULT_MACHINE_SKIN "Psycle Default (internal)"

		class CMidiInput; // MIDI IMPLEMENTATION 

		/// configuration.
		class Configuration
		{
		public:
			Configuration();
			virtual ~Configuration() throw();

#if !defined WINAMP_PLUGIN
			void CreateFonts();
			bool CreatePsyFont(CFont & f, std::string const & sFontFace, int const & HeightPx, bool const & bBold, bool const & bItalic);
			
			bool _allowMultipleInstances;
			bool _bShowPatternNames;
			bool _toolbarOnVsts;
			COLORREF mv_colour;
			COLORREF mv_wirecolour;
			COLORREF mv_wireaacolour;
			COLORREF mv_wireaacolour2;
			COLORREF mv_polycolour;
			COLORREF mv_generator_fontcolour;
			COLORREF mv_effect_fontcolour;

			COLORREF pvc_separator;
			COLORREF pvc_separator2;
			COLORREF pvc_background;
			COLORREF pvc_background2;
			COLORREF pvc_row4beat;
			COLORREF pvc_row4beat2;
			COLORREF pvc_rowbeat;
			COLORREF pvc_rowbeat2;
			COLORREF pvc_row;
			COLORREF pvc_row2;
			COLORREF pvc_font;
			COLORREF pvc_font2;
			COLORREF pvc_fontPlay;
			COLORREF pvc_fontPlay2;
			COLORREF pvc_fontCur;
			COLORREF pvc_fontCur2;
			COLORREF pvc_fontSel;
			COLORREF pvc_fontSel2;
			COLORREF pvc_selection;
			COLORREF pvc_selection2;
			COLORREF pvc_playbar;
			COLORREF pvc_playbar2;
			COLORREF pvc_cursor;
			COLORREF pvc_cursor2;

			COLORREF vu1;
			COLORREF vu2;
			COLORREF vu3;

			COLORREF machineGUITopColor;
			COLORREF machineGUIFontTopColor;
			COLORREF machineGUIBottomColor;
			COLORREF machineGUIFontBottomColor;

			COLORREF machineGUIHTopColor;
			COLORREF machineGUIHFontTopColor;
			COLORREF machineGUIHBottomColor;
			COLORREF machineGUIHFontBottomColor;

			COLORREF machineGUITitleColor;
			COLORREF machineGUITitleFontColor;

			bool mv_wireaa;
			int  mv_wirewidth;
			bool _wrapAround;
			bool _centerCursor;
			bool _cursorAlwaysDown;
			bool _midiMachineViewSeqMode;
			bool _windowsBlocks;
			bool _RecordNoteoff;
			bool _RecordTweaks;
			bool _notesToEffects;
			bool _RecordUnarmed;
			bool _NavigationIgnoresStep;
			bool _RecordMouseTweaksSmooth;
			bool useDoubleBuffer;
			bool _showAboutAtStart;
			int _pageUpSteps;		// 0 -> one beat, 1 -> one bar , > 1 -> steps

			int mv_triangle_size;
			int pv_timesig;

			bool bBmpBkg;
			bool bBmpDial;
			std::string szBmpBkgFilename;
			std::string szBmpDialFilename;

			std::string pattern_fontface;
			std::string pattern_header_skin;
			int pattern_font_point;
			int pattern_font_x;
			int pattern_font_y;

			UINT pattern_font_flags;
			UINT generator_font_flags;
			UINT effect_font_flags;

			bool pattern_draw_empty_data;
			bool draw_mac_index;
			bool draw_vus;

			std::string generator_fontface;
			int generator_font_point;
			std::string effect_fontface;
			int effect_font_point;

			std::string machine_skin;

			CFont seqFont;
			CFont generatorFont;
			CFont effectFont;

		public:
			class midi_type
			{
				public:
					midi_type() : groups_(16), velocity_(0x0c), pitch_(1), raw_()
					{
						for(std::size_t i(0) ; i < groups().size() ; ++i) group(i).message() = group(i).command() = static_cast<int>(i + 1);
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
			bool _linenumbers;
			bool _linenumbersHex;
			bool _linenumbersCursor;
			bool _followSong;

			AudioDriver** _ppOutputDrivers;
			int _numOutputDrivers;
			int _outputDriverIndex;

			CMidiInput* _pMidiInput;
			int _numMidiDrivers;
			int _midiDriverIndex;
			int _syncDriverIndex;
			int _midiHeadroom;

			bool bShowSongInfoOnLoad;
			bool bFileSaveReminders;
			bool autosaveSong;
			int autosaveSongTime;

#endif // !defined WINAMP_PLUGIN

		public:
			int defaultPatLines;
			bool autoStopMachines;

			AudioDriver* _pOutputDriver;

			bool Initialized() { return _initialized; }
			bool Read();
			void Write();
			bool ReadVersion17();
			void SetSkinDefaults();
			inline int GetSamplesPerSec() const throw()
			{
				return _pOutputDriver->_samplesPerSec;
			}
			inline void SetSamplesPerSec(int samplerate) const throw()
			{
				_pOutputDriver->_samplesPerSec = samplerate;
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

		public:
			std::string const & GetWaveRecDir              () const throw() { return wave_rec_dir_; }
			void SetWaveRecDir              (std::string const &);
			std::string const & GetCurrentWaveRecDir      () const throw() { return current_wave_rec_dir_; }
			void SetCurrentWaveRecDir      (std::string const &);
		private:
			std::string wave_rec_dir_;
			std::string current_wave_rec_dir_;

		protected:
			void Error(std::string const &);
		};
	}
}
