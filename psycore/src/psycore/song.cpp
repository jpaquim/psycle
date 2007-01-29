///\file
///\brief implementation file for psy::host::Song
#include "song.h"
#include "machine.h"

#include <cassert>
#include <algorithm>
#include <sstream>


namespace psy {
  namespace core
  {

    // some song infos
    SongInfo::SongInfo() 
    {
      name_ = "untitled";
    }
    
    SongInfo::~SongInfo() {
    }

    void SongInfo::setName( const std::string& name ) {
      name_ = name;
    }
    
    const std::string& SongInfo::name() const {
      return name_;
    }
    // end of songinfo


    // the song class
    Song::Song() 
      : patternSequence_(patternData_)
    {
      init();
    }

    Song::~Song()
    {
    }

    void Song::init() {
      patternSequence_.patternData().newPattern();
      std::list<SinglePattern>::iterator it = patternSequence_.patternData().begin();
      SinglePattern& pat = *it;
      pat.setName("/pattern0");
    }

    PatternSequence& Song::patternSequence() {
      return patternSequence_;
    }

    const SongInfo& Song::info() const {
      return info_;
    }


  }
}
