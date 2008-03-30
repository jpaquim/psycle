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

#include "w_directmidi_CDMRecv.h"
//SysEx
void CDMReceiver::RecvMidiMsg(REFERENCE_TIME lprt,DWORD dwChannel, DWORD dwBytesRead,BYTE *lpBuffer)
{
	DWORD dwBytecount;
	
	//Print The received buffer
	for (dwBytecount = 0;dwBytecount < dwBytesRead;dwBytecount++)
    {    
        cout.width(2);
        cout.precision(2);
        cout.fill('0');        
        cout << hex << static_cast<int>(lpBuffer[dwBytecount]) << " ";
        if ((dwBytecount % 20) == 0) cout << endl;
        if (lpBuffer[dwBytecount] == END_SYS_EX)
            cout << "\nSystem memory dumped" << endl;
    }
}

//Structured MIDI
void CDMReceiver::RecvMidiMsg(REFERENCE_TIME lprt,DWORD dwChannel,
                               DWORD dwMsg)
{
    unsigned char Command,Channel,Note,Velocity;
    
    // Extract MIDI parameters from a MIDI message    

    CInputPort::DecodeMidiMsg(dwMsg,&Command,&Channel,&Note,&Velocity);
  
    if (Command == NOTE_ON) //Channel #0 Note-On

        {                    
        cout << "Received on channel " << static_cast<int>(Channel) << 
                " Note " << static_cast<int>(Note) 
             << " with velocity " << static_cast<int>(Velocity) << endl;
    }
} 
   