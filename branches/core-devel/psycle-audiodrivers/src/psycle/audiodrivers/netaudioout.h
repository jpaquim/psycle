/***************************************************************************
*   Copyright (C) 2007 by Psycledelics  , Johan Boule                  *
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
#ifndef PSYCLE__AUDIO_DRIVERS__NET_AUDIO_OUT
#define PSYCLE__AUDIO_DRIVERS__NET_AUDIO_OUT
#if defined PSYCLE__NET_AUDIO_AVAILABLE
#include "audiodriver.h"
#include <audio/audiolib.h>
#include <pthread.h>
#include <exception>
namespace psy
{
	namespace core
	{

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

		// Sound sources : 
		//----------------
		//ImportClient -> create a sound source, providing from the client (application)
		//ImportDevice -> create a sound source, providing from a server physical input.
		//ImportBucket -> create a sound source, providing from a "bucket" (sample stored in the server).
		//ImportWaveForm -> create a sound source, providing from a wave file in the client's side.
		//
		// Interconnect elements:
		//------------------
		//TypeBundle -> two branches are bundled(?) to generate a single one ( two mono branches -> one stereo branch?)
		//TypeSum -> add (sum) the values of two branches
		//TypeMultiplyConstant -> multiply the branch values by a constant.
		//TypeAddConstant -> add a constant value to the branch values.
		//
		// Sound destinations:
		//--------------------
		//ExportClient -> send the sound to a client (application)
		//ExportDevice -> send the sound to a physical output.
		//ExportBucket -> store the branch values to a bucket.
		//ExportMonitor -> generates events from the audio so that application can get information ( monitor/vumeter.. ) ( destination or interconnect element? )


/*																																
		Working with the low-level functions
		------------------------------------
		1) Creating the Server
		----------------------
		AuOpenServer() opens a connection with it.
		AuCreateFlow() prepares a "workspace" where to put the elements.
		AuRegisterEventHandler() setup the callback handler so that we can get notified of events

			aud = AuOpenServer(server, 0, NULL, 0, NULL, NULL);
			flow = AuCreateFlow(aud, NULL);
			handler = AuRegisterEventHandler(aud, 0, 0, 0, EventHandlerFunc,(AuPointer) shareddata)))

		2) Find the physical devices, if needed
		----------------------------------------
		AuServerNumDevices() gives the number of devices of the server
		AuServerDevice() gets the ID of the device
		AuDeviceKind() identifies the kind of device.

			AuDeviceID device = AuNone;
			for (i=0; i < AuServerNumDevices(aud); i++) {
				AuDeviceAttributes *dev = AuServerDevice(aud, i);
				if ((AuDeviceKind(dev) == AuComponentKindPhysicalOutput) &&
					AuDeviceNumTracks(dev) == 2) {
					device = AuDeviceIdentifier(dev);
					break;
				}
			}

		3) Create the elements, setup and start it
		------------------------------------------
			A) for a continuous flow
			------------------------
			AuMakeElementImportClient() create an audio entry point coming from the application
			(Optional)AuMakeElementMultiplyConstant() use some operations between the application and the soundcard.
			AuMakeElementExportDevice() create an audio output to the soundcard.
			AuSetElements() Pass the elements to the server to build the flow
			AuStartFlow() Start playback


			B) for a play-from-bucket flow
			------------------------------
			AuMakeElementImportBucket() create one (several) bucket(s) as a place for audio storage.
			(Optional)AuMakeElementMultiplyConstant() use some operations between the application and the soundcard.
			AuMakeElementExportDevice() create an audio output to the soundcard.
			AuSetElements() Pass the elements to the server to build the flow
			AuStartFlow() Start playback

				AuMakeElementImportBucket(&elements[0], ringoutRate, ringoutBucket,
					AuUnlimitedSamples, 0, 2, actions);
				AuMakeElementMultiplyConstant(&elements[1], 0,
					AuFixedPointFromFraction(DEFAULT_RING_VOLUME, 100));
				AuMakeElementExportDevice(&elements[2], 1, device, ringoutRate,
					AuUnlimitedSamples, 0, NULL);
				AuSetElements(aud, flow, AuTrue, 3, elements, NULL);
				AuStartFlow(aud, flow, NULL);

		4) Send the audio data ( callback function or loop )
		----------------------------------------------------
			A) for a continuous flow (using the callback function)
			------------------------------------------------------
			case AuElementNotifyKindLowWater: The server is ready to receive data from the application via the ImportClient.
			case AuElementNotifyKindHighWater: The server has data in in ExportClient ready for the application to read.
			case AuElementNotifyKindState: A state of an element has changed.
			AuWriteElement() Write the data to the ImportClient.
			AuReadElement() Read the data from the ExportClient.

				OurSharedDataType  *shared = (OurSharedDataType*) handler->data;
				AuElementNotifyEvent *event = (AuElementNotifyEvent *) ev;
				if (event->type == AuEventTypeElementNotify)
				{
					switch (event->kind)
					{
					case AuElementNotifyKindLowWater:
						// You will need to write to the shared buffer some audio, of course.
						// Also, note the bufferpos, which is assumed to allow a circular buffer.
						AuWriteElement(aud, flow, 0, event->num_bytes, shared->buffer + shared->bufferpos,																AuFalse, NULL);
						shared->bufferpos += event->num_bytes;
						break;
					case AuElementNotifyKindState:
						switch (event->cur_state)
						{
							case AuStateStop:
								shared->stopcallback(shared);
								break;
						}
						break;
					}
				}

			B) for a play-from-bucket flow
			------------------------------


			C) continuous flow in a syncronous loop.
			------------------------------------------
			AuSync() syncronizes with the server and waits so that the application can receive server events.

				while (1) {
					AuStatus as = AuBadValue;
					long writelen = BUFFER_SIZE;
	
					AuWriteElement(aud, flow, 0, writelen, buffer, AuFalse, &as);
					AuSync(nas_server, AuTrue);
					if (as == AuBadValue) {
						// Does not fit in remaining buffer space, wait a bit and try again
						(void)usleep(1000);
						continue;
					}
					if (as != AuSuccess) {
						// Unexpected error happened
						break;
					}
					// update buffer here.
				}
*/


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

		class NetAudioOut : public AudioDriver
		{
			public:
				NetAudioOut();
				~NetAudioOut();
				
				virtual AudioDriverInfo info() const;

			public:
				virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
				virtual bool Initialized();																																																
			private:
				bool initialized_;
				
			public:																																
				virtual void Configure();
				virtual bool Enable(bool e);
				virtual AudioDriver* clone() const;

				
			private:
				
				void setDefaults();
				bool open();
				bool close();

				std::string hostPort();
				std::string host_;
				int port_;
				AuServer* aud_;
				AuFlowID flow_;
				AuDeviceID device_;
				AuEventHandlerRec* handler_;
				AuElement nas_elements[3];

				static AuBool EventHandlerFunc(AuServer *aud, AuEvent *ev, AuEventHandlerRec *handler);
				static int audioOutThreadStatic(void*);
				void audioOutThread();
				pthread_t threadId_;
				bool threadRunning_;
				bool killThread_;

//																																																																int writeBuffer(char * buffer, long size);
				AUDIODRIVERWORKFN callback_;
				void* callbackContext_; // Player callback
				short buf[48000];

				int latencyInBytes();
				int latency_; // in samples

				static char *nas_error(AuServer* aud,AuStatus status);
				unsigned char toNasFormat();
			
		};
	}
}
#endif // defined PSYCLE__NET_AUDIO_AVAILABLE
#endif
