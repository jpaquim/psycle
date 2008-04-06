/***************************************************************************
	*   Copyright (C) 2007 Psycledelics     *
	*   psycle.sf.net   *
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	*   This program is distributed in the hope that it will be useful,       *
	*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
	*   GNU General Public License for more details.                          *
	*                                                                         *
	*   You should have received a copy of the GNU General Public License     *
	*   along with this program; if not, write to the                         *
	*   Free Software Foundation, Inc.,                                       *
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/
#ifndef SINGLEPATTERN_H
#define SINGLEPATTERN_H

#include "patternline.h"
#include "signalslib.h"
#include "timesignature.h"

/**
@author  Psycledelics  
*/

namespace psy
{
	namespace core
	{
		class PatternCategory;

		/*!
		 * \brief
		 * Data Holder for the type of tweak.
		 * 
		 * TweakTrackInfo holds the parameter and type of tweak for a specific column.
		 * 
		 * \see
		 * TrackInfo|TweakEvent
		 */
		class TweakTrackInfo {
		public:

			enum TweakType { twk, tws, mdi, mdis, wire, wires, aut };
			static char *tweakTypeText[];

			TweakTrackInfo();
			TweakTrackInfo( TweakType type , std::uint16_t param);

			~TweakTrackInfo(){;}

			TweakType type()     const { return type_; }
			std::uint16_t parameterIdx()  const { return paramIdx_; }

			bool operator<(const TweakTrackInfo & key) const;

			std::string toXml() const;

		private:

			TweakType type_;
			std::uint16_t paramIdx_;
		};
		class TrackInfo {
		public:

			TrackInfo();
			TrackInfo( std::uint8_t trackidx, std::uint16_t mac, bool muted);

			inline void setMachineIdx(std::uint16_t macindex, std::uint8_t trackidx);
			inline std::uint16_t machineIdx()    const { return macIdx_; }


			///\todo: Add Pattern:: member which changes this value, while adding/removing the columns in
			/// the patternlines aswell
			void addtweakinfo(TweakTrackInfo& tinfo);
			void changetweakinfo(std::uint8_t index,TweakTrackInfo& tinfo);
			void removetweakinfo(TweakTrackInfo& tinfo);
			void removetweakinfo(std::uint8_t tinfo);
			inline TweakTrackInfo tweak(std::uint8_t idx) const { assert(idx<tweakinfos.size()); return tweakinfos[idx]; }
			inline const std::uint8_t tweaks() const { return tweakinfos.size(); }

			inline void mute(bool value) { ismuted_=value; }
			inline bool muted() const { return ismuted_; }

			std::string toXml( int track ) const;

		private:

			std::uint8_t index_;
			std::uint16_t macIdx_;
			std::uint8_t numnotes_;
			std::uint8_t numtweaks_;

			bool ismuted_;
			std::vector<TweakTrackInfo> tweakinfos;

		};


		class Pattern {
		public:
			typedef  std::multimap<double, PatternEvent>::iterator iterator;
			typedef  std::multimap<double, PatternEvent>::const_iterator const_iterator;
			typedef  std::multimap<double, PatternEvent>::reverse_iterator reverse_iterator;
			typedef  std::multimap<double, PatternEvent>::const_reverse_iterator const_reverse_iterator;

			Pattern();
			Pattern(Pattern const& other);

			virtual ~Pattern();

			inline iterator upper_bound(const double& _Keyval) { return events.upper_bound(_Keyval); }
			inline const_iterator upper_bound(const double& _Keyval) const { return events.upper_bound(_Keyval); }
			inline iterator lower_bound(const double& _Keyval) { return events.lower_bound(_Keyval); }
			inline const_iterator lower_bound(const double& _Keyval) const{ return events.lower_bound(_Keyval); }
			inline iterator begin() { return events.begin(); }
			inline const_iterator begin() const { return events.begin(); }
			inline iterator end() { return events.end(); }
			inline const_iterator end() const { return events.end(); }
			inline reverse_iterator rbegin() { return events.rbegin(); }
			inline const_reverse_iterator rbegin() const { return events.rbegin(); }
			inline reverse_iterator rend() { return events.rend(); }
			inline const_reverse_iterator rend() const { return events.rend(); }
			
//			TweakTrackInfo tweakInfo( int track ) const;
//			int tweakTrack( const TweakTrackInfo & info);

			void setID(int id);
			int id() const { return id_; }

			void setBeatZoom(int zoom) { beatZoom_ = zoom; }
			int beatZoom() const { return beatZoom_; }

			void setName(const std::string & name) { name_ = name; }
			const std::string & name() const { return name_; }

			void setCategory(PatternCategory* category) { category_ = category; }
			PatternCategory* category() { return category_; }

			///\todo: we have to work out better the "time signature" feature. I don't find useful to have one signature each
			/// signature end. I'd better have an initial signature, and the user add the variations he finds appropiate. 
			/// Concretely, signature affects mostly the visuals, and concretely, where the notes are placed relative to a beat.
			void addBar( const TimeSignature & signature );
			void removeBar( float pos);

			bool barStart(double pos, TimeSignature & signature) const;
			void clearBars();

			std::vector<TimeSignature> &  timeSignatures() { return timeSignatures_; }
			const std::vector<TimeSignature> &  timeSignatures() const { return timeSignatures_; }
			const TimeSignature & timeSignatureAt(double pos) const;

			///\todo: work out beats().
			float beats() const;
			float beatsPerLine() const { return 1 / (float) beatZoom(); }

			iterator find_nearest( int linenr );
			const_iterator find_nearest( int linenr ) const;

			iterator find_lower_nearest( int linenr );
			const_iterator find_lower_nearest( int linenr ) const;

			void setEvent( int line, int track, const NoteEvent & event );
			NoteEvent event( int line, int track );

			void setTweakEvent( int line, int track, const TweakEvent & event );
			TweakEvent tweakEvent( int line, int track );

			void clearTrack( int linenr , int tracknr );
			void clearTweakTrack( int linenr , int tracknr );
			bool lineIsEmpty( int linenr ) const;
		
/*			void clearEmptyLines();

			std::auto_ptr<Pattern> block( int left, int right, int top, int bottom );
			void scaleBlock(int left, int right, double top, double bottom, float factor);
			void transposeBlock(int left, int right, double top, double bottom, int trp);
			void copyBlock(int left, int top, const Pattern & pattern, int tracks, float maxBeats);
			void mixBlock(int left, int top, const Pattern & pattern, int tracks, float maxBeats);
			void deleteBlock(int left, int right, double top, double bottom);
			void deleteBlock( int left, int right, int top, int bottom );
*/
			std::string toXml() const;

			boost::signal<void (Pattern*)> wasDeleted;

		private:

			int beatZoom_;
			int id_;
			std::string name_;
			PatternCategory* category_;

			std::vector<TimeSignature> timeSignatures_;
			static TimeSignature defaultSignature;

			std::map<int,TrackInfo> trackInfoMap;
			std::multimap<double, PatternEvent> events;

		};

	}
}

#endif
