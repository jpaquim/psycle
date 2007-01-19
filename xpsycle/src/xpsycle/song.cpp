///\file
///\brief implementation file for psycle::host::Song
#include "song.h"
#include "machine.h"

#include <cassert>
#include <algorithm>
#include <sstream>


namespace psycle {
  namespace host
  {

    Song::Song()
    {
    }

    Song::~Song()
    {
    }

    PatternSequence& Song::patternSequence() {
      return patternSequence_;
    }

  }
}