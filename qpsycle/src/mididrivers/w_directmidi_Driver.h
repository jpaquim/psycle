/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#ifndef DMIDIDRIVER_H
#define DMIDIDRIVER_H

#include "w_directmidi_CDMRecv.h"

#include <CDirectMidi.h>
#include <QString>

class CDMDirverHandler
{
private:
	INFOPORT _portInfo;
	const int SYSTEM_EXCLUSIVE_MEM = 48000;
	int _portNumber;
	CDirectMusic CDMusic;
    CInputPort   CInPort;
    CDMReceiver  Receiver;    

public:
	void Initialize();
	void SetPort(int portNumber);
	int GetCurrentPortNumber();
	QString GetCurrentPortName();
	QString[] GetPorts();
};

#endif