/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#ifndef PATTERNEVENT_H
#define PATTERNEVENT_H

#include "pattern.h"
#include <cstdint>


/**
@author Stefan Nattkemper
*/
class PatternEvent{
public:
    PatternEvent();

    ~PatternEvent();

    void setNote(std::uint8_t value);
    std::uint8_t note() const;

    void setInstrument(std::uint8_t instrument);
    std::uint8_t instrument() const;

    void setMachine(std::uint8_t machine);
    std::uint8_t machine() const;

    void setCommand(std::uint8_t command);
    std::uint8_t command() const;

    void setParameter(std::uint8_t parameter);
    std::uint8_t parameter() const;

    psycle::host::PatternEntry* entry();
		psycle::host::PatternEntry* entry() const;

private:

    std::uint8_t _note;
    std::uint8_t _inst;
    std::uint8_t _mach;
    std::uint8_t _cmd;
    std::uint8_t _parameter;

};

#endif
