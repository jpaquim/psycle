/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
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
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "audiodriver.h"
#include "inputhandler.h"
#include <ncolor.h>
#include <vector>
#include <nobject.h>

/**
@author Stefan
*/



class Configuration : public NObject {
public:
    Configuration();

    ~Configuration();

    void loadConfig();

    bool autoStopMachines;
    std::string iconPath;
    std::string pluginPath;
    std::string prsPath;


    InputHandler inputHandler;

    NColor vu1;
    NColor vu2;
    NColor vu3;

    NColor pvc_separator;
    NColor pvc_separator2;
    NColor pvc_background;
    NColor pvc_background2;
    NColor pvc_row4beat;
    NColor pvc_row4beat2;
    NColor pvc_rowbeat;
    NColor pvc_rowbeat2;
    NColor pvc_row;
    NColor pvc_row2;
    NColor pvc_font;
    NColor pvc_font2;
    NColor pvc_fontPlay;
    NColor pvc_fontPlay2;
    NColor pvc_fontCur;
    NColor pvc_fontCur2;
    NColor pvc_fontSel;
    NColor pvc_fontSel2;
    NColor pvc_selection;
    NColor pvc_selection2;
    NColor pvc_playbar;
    NColor pvc_playbar2;
    NColor pvc_cursor;
    NColor pvc_cursor2;

    NColor machineGUITopColor;
    NColor machineGUIFontTopColor;
    NColor machineGUIBottomColor;
    NColor machineGUIFontBottomColor;

    NColor machineGUIHTopColor;
    NColor machineGUIHFontTopColor;
    NColor machineGUIHBottomColor;
    NColor machineGUIHFontBottomColor;

    NColor machineGUITitleColor;
    NColor machineGUITitleFontColor;

    NColor mv_polycolour;

    int  mv_wirewidth;
    int mv_triangle_size;

    int pattern_font_x;
    int pattern_font_y;

    bool _linenumbers;
    bool _linenumbersHex;
    bool _linenumbersCursor;
    bool _followSong;

    int defaultPatLines;
    int pv_timesig;

    bool _centerCursor;

    AudioDriver** _ppOutputDrivers;
    AudioDriver* _pOutputDriver;
    int _numOutputDrivers;
    int _outputDriverIndex;

    bool enableSound;

    inline int GetSamplesPerSec() const throw()
    {
      return _pOutputDriver->_samplesPerSec;
    }

    class midi_type
    {
      public:
        midi_type() : groups_(16), velocity_(0x0c), pitch_(1), raw_()
        {
          for(std::size_t i(0) ; i < groups().size() ; ++i) group(i).message() = group(i).command() = i + 1;
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
            groups_type              const inline & groups()                  const throw() { return groups_        ; }
            groups_type                    inline & groups()                  throw() { return groups_        ; }
            groups_type::value_type  const inline & group (std::size_t const & index) const throw() { return groups()[index]; }
            groups_type::value_type        inline & group (std::size_t const & index)       throw() { return groups()[index]; }
         private:
             groups_type groups_;

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

private:
    void setSkinDefaults();

    void onConfigTagParse(const std::string & tagName);
};

#endif
