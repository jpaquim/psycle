// PCSignalProcessor.h: interface for the PCSignalProcessor class.
//
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_PCSIGNALPROCESSOR_H__37637AF1_6353_4B0B_89CF_27BFE4394F7A__INCLUDED_)
#define AFX_PCSIGNALPROCESSOR_H__37637AF1_6353_4B0B_89CF_27BFE4394F7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef float* soundBuffer;
typedef soundBuffer* sBufferArray;


//
//   PsyClean Machine Specification Version  0.1 (May 2002)
//   ------------------------------------------------------
//

#define PCINTERFACE_VERSION 0x1010 //  1st 10 = Version 1.0 , 2nd 10 = subversion 1.0

#define MAX_EVENTS 64

typedef struct
{
	long deltaFrames;	// event delay (in sample frames) related to the current sample block start
	long noteOffset;	// note start (sample ) offset (opposite of delay). else 0

	char midiData[4];	// 0, 1, 2 midi bytes; midiData[3] is reserved (zero)
	char detune;		// -64 to +63 cents; for scales other than 'well-tempered' ('microtuning')	
} PCMidiEvent;


enum PCMessageCode
{

//	Messages that you can use in the function "QueryProcessorToHost()".
////////////////////////////////////////////////////////////////////////////
	PCGetHostName,
	PCGetHostVersion,
	PCGetHostCompatibleVersion,

	PCPlugHasChanged,

//	Messages that you can receive via the "QueryHostToProcessor()" function.
////////////////////////////////////////////////////////////////////////////
	
	// Queries (info variable becomes the result)
	PCGetPluginName,				// Plugin name/description. Max 64 chars.
	PCGetPluginVersion,				// Plugin version (chars) Max 16 chars.
	PCGetPluginCompatibleVersion,	// Interface version. Return  PCINTERFACE_VERSION
	
	//Notifications
	PCBufferSizeChange,				// Host has changed the audio buffer size (and buffersize variable)
	PCProcessIdle,					// Host gives you Idle time to work. Wait for this call to do any File read or
									// processes that can take long, instead of doing it in ProcessSound.
									// It will be called 0 or 1 times for each ProcessSound
									// (if 0, then, the CPU can't cope)
	PCRefreshGUI,					// This will be called by a timer to refresh the GUI. It is your decision to 
									// recalc it everytime, or maitain a doublebuffer.
	//

};




class PCSignalProcessor  
{
protected:
	int buffer_size;		// Indicates the sound buffer size (of each output).
	float wet;			// Wet component (0.0f to 2.0f) multiply your volume by this when generating sound.

	int	queue_size;
	PCMidiEvent midievents[MAX_EVENTS];


public:
	CFrameWnd* guiWnd;

	sBufferArray inputs;
	sBufferArray outputs;
	int numInputs;
	int numOutputs;
	CList outputs;

	PCSignalProcessor();
	virtual ~PCSignalProcessor();

/////////////////////////////////////////////////////////////////////
// Functions called by the Host (Implement those that you'll respond to) 
/////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	// Called to get or send Information, or just to notify the Processor of something.
	// Both message and info variables are provided
	// The list of MessageCodes is on top of this file.
	virtual PCError QueryHostToProcessor(PCMessage &message,PCQueryResult &result);
	virtual PCError QueryHostToProcessor(PCMessageCode &code, PCMessageBody &message,PCQueryResult &result);
	virtual PCError QueryHostToProcessor(PCMessageCode &code, float, float, void*, float);


	/////////////////////////////////////////////////////////////////////
	// General call for Processing. Buffers are provided.
	// Size is indicated in "buffersize" variable. You will receive a
	// "PCBufferSizeChange" message just after this variable is changed.
	// Return true if you have filled outputbuffer with data. False otherwise.
	// Note: ALWAYS fill the entire buffer (pad with zeroes if needed)
	virtual bool ProcessSoundReplacing(sBufferArray inputbuffers,sBufferArray outputbuffers){return false;}


	////////////////////////////////////////////////////////////////////////
	// Called when the mixing (like when outputing to Master). Add instead of replace.
	// Buffers are provided. Size is provided by the "buffersize" variable,
	// which is changed just previous to the "PCBufferSizeChange" message.
	// Return true if you have filled outputbuffer with data. False otherwise.
	virtual bool ProcessSoundAdding(sBufferArray inputbuffers,sBufferArray outputbuffers){return false;}
	


/////////////////////////////////////////////////////////////////////
// Functions called by the Processor (Implemented by the Host)
/////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	// Call this to get or send Information, or just to notify the Host of something.
	// Provide both, message and info variables.
	// The list of MessageCodes is on top of this file.
	virtual PCError QueryProcessorToHost(PCMessage &message,PCQueryReply &result);

	
/////////////////////////////////////////////////////////////////////
// Other?
/////////////////////////////////////////////////////////////////////

	virtual int SaveDataSize();
	virtual void Save(void *data);
	virtual void Load(void *data);

	virtual void AddMidiEvent(PCMidiEvent &midievent);
};






#endif // !defined(AFX_PCSIGNALPROCESSOR_H__37637AF1_6353_4B0B_89CF_27BFE4394F7A__INCLUDED_)
