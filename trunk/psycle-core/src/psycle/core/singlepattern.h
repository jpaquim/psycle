/**************************************************************************
*   Copyright (C) 2007-2008 Psycledelics http://psycle.sourceforge.net    *
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

#include "patternevent.h"
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

		class TweakTrackInfo {
		public:

			enum TweakType { twk, tws, mdi, aut };

			TweakTrackInfo();
			TweakTrackInfo( int mac, int param, TweakType type );

			~TweakTrackInfo();

			int machineIdx()    const;
			int parameterIdx()  const;
			TweakType type()          const;

			bool operator<(const TweakTrackInfo & key) const;

		private:

			int macIdx_;
			int paramIdx_;
			TweakType type_;

		};


		class SinglePattern {
		public:
			typedef std::multimap<double, PatternEvent>::iterator iterator;
			typedef std::multimap<double, PatternEvent>::const_iterator const_iterator;
			typedef std::multimap<double, PatternEvent>::const_reverse_iterator const_reverse_iterator;
			typedef std::multimap<double, PatternEvent>::reverse_iterator reverse_iterator;

			SinglePattern();
			SinglePattern(const SinglePattern& other);

			virtual ~SinglePattern();

			boost::signal<void (SinglePattern*)> wasDeleted;

			void setID(int id);
			int id() const;

			void setBeatZoom(int zoom);
			int beatZoom() const;


			void setEvent( int line, int track, const PatternEvent & event );
			PatternEvent event( int line, int track );

			void setTweakEvent( int line, int track, const PatternEvent & event );
			PatternEvent tweakEvent( int line, int track );


			void addBar( const TimeSignature & signature );
			void removeBar( float pos);

			float beats() const;

			bool barStart(double pos, TimeSignature & signature) const;
			void clearBars();

			const TimeSignature & playPosTimeSignature(double pos) const;

			void setName(const std::string & name);
			const std::string & name() const;

			void setCategory(PatternCategory* category);
			PatternCategory* category();

			float beatsPerLine() const;

			void clearTrack( int linenr , int tracknr );
			void clearTweakTrack( int linenr , int tracknr );
			bool lineIsEmpty( int linenr ) const;

/*			iterator find_nearest( int linenr );
			const_iterator find_nearest( int linenr ) const;

			iterator find_lower_nearest( int linenr );
			const_iterator find_lower_nearest( int linenr ) const;*/
			

			void clearEmptyLines();

			void scaleBlock(int left, int right, double top, double bottom, float factor);
			void transposeBlock(int left, int right, double top, double bottom, int trp);
			void deleteBlock(int left, int right, double top, double bottom);
			
			void blockSetInstrument( int left, int right, double top, double bottom, std::uint8_t newInstrument );
			void blockSetMachine( int left, int right, double top, double bottom, std::uint8_t newMachine );

			std::vector<TimeSignature> &  timeSignatures();
			const std::vector<TimeSignature> &  timeSignatures() const;

			std::string toXml() const;

			std::auto_ptr<SinglePattern> block( int left, int right, int top, int bottom );
			void copyBlock(int left, int top, const SinglePattern & pattern, int tracks, float maxBeats);
			void mixBlock(int left, int top, const SinglePattern & pattern, int tracks, float maxBeats);

			void deleteBlock( int left, int right, int top, int bottom );

			TweakTrackInfo tweakInfo( int track ) const;
			int tweakTrack( const TweakTrackInfo & info);


			iterator begin() { return lines_.begin(); }
			const_iterator begin() const { return lines_.begin(); }
			iterator end() { return lines_.end(); }
			const_iterator end() const { return lines_.end(); }
  
			reverse_iterator rbegin() { return lines_.rbegin(); }
			const_reverse_iterator rbegin() const { return lines_.rbegin();}
			reverse_iterator rend() { return lines_.rend(); }
			const_reverse_iterator rend() const { return lines_.rend(); }

			iterator lower_bound(double pos) { return lines_.lower_bound(pos); }
			iterator upper_bound(double pos) { return lines_.upper_bound(pos); }

			iterator insert(double pos, const PatternEvent& ev) {
				return lines_.insert(std::pair<double, PatternEvent>(pos, ev));
			}

			std::map<double, PatternEvent>::size_type size() const { lines_.size(); }
			

		private:

			int beatZoom_;
			std::string name_;

			PatternCategory* category_;
			std::vector<TimeSignature> timeSignatures_;

			TimeSignature zeroTime;

			int id_;
			static int idCounter;
			static int genId();

			std::map<TweakTrackInfo, int> tweakInfoMap;

			std::multimap<double, PatternEvent> lines_;

		};

	}
}

#endif
