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

    Song::Song()
    {
    }

    Song::~Song()
    {
    }

    PatternSequence* Song::patternSequence() {
      return &patternSequence_;
    }

    const PatternSequence& Song::patternSequence() const {
      return patternSequence_;
    }

  }
}