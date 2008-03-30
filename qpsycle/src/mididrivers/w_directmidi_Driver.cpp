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

#include "w_directmidi_Driver.h"

void CDMDirverHandler::Initialize()
{
	try
	{
		CDMusic.Initialize();
		
		CInPort.Initialize(CDMusic);
		CInPort.SetReceiver(Receiver);
	}
	catch (CDMusicException& DMex)
	{
		//Adding error handling to the class;
	}
}

int CDMDirverHandler::GetCurrentPortNumer()
{
	return _portNumber;
}

void CDMDirverHandler::SetPort(int portNumber);
{
	_portNumber = portNumber;
	CInPort.ReleasePort();
	CinPort.GetPortInfo(_portNumber, &portInfo);
	CInPort.ActivatePort(&_portInfo, SYSTEM_EXCLUSIVE_MEM);
}

QString CDMDirverHandler::GetCurrentPortName()
{
	INFOPORT infoport; //we need a temporary infoport because we don't want to change our main infoport
	CInPort.GetActivatedPortInfo(&infoport);
	return infoport.szPortDescription();
}

QString[] CDMDirverHandler::GetPorts()
{
	QString[] ports;
	int i;
	INFOPORT infoport; //we need a temporary infoport to enum our ports because we don't want to change our main infoport
	
	for {i=0,  i<(int)CInPort.GetNumPorts()-1. i++)
	{
		CInPort.GetPortInfo(i+1, &infoport);
		ports[i] = infoport.szPortDescription();
	}
	return ports;
}