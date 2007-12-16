//
// File: alsaseqin.cc
// Created by: User <Email>
// Created on: Thu Jul 20 13:35:01 2007
//
#include <qpsyclePch.hpp>

#include "alsaseqin.hpp"

#if defined PSYCLE__ALSA_AVAILABLE

AlsaSeqIn::AlsaSeqIn()
{
	// TODO: put constructor code here
}


AlsaSeqIn::~AlsaSeqIn()
{
	// TODO: put destructor code here
}

bool AlsaSeqIn::Open()
{
	//\todo: change "default" to something configurable.
		if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
				fprintf(stderr, "Error opening ALSA sequencer.\n");
				return false;
		}
		snd_seq_set_client_name(seq_handle, "xpsycle");
		if (snd_seq_create_simple_port(seq_handle, "midiIO",
				SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
				SND_SEQ_PORT_TYPE_APPLICATION) < 0) {
				fprintf(stderr, "Error creating sequencer port.\n");
				return false;
		}
		npolldesc = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
		ppolldesc = (struct pollfd *)alloca(sizeof(struct pollfd) * npolldesc);
		snd_seq_poll_descriptors(seq_handle, ppolldesc,npolldesc, POLLIN);
	
	return true;
}

bool AlsaSeqIn::Close()
{
	return    snd_seq_close (seq_handle);
}

///\todo: this is currently called directly from Player. It should actually be a separate thread, just like alsaout.
void AlsaSeqIn::MidiCallback(int numSamples)
{
		if (poll (ppolldesc, npolldesc, 1000) > 0) {
						for (int l1 = 0; l1 < npolldesc; l1++) {
								if (ppolldesc[l1].revents > 0) ProcessEvent();
						}
		}
}

void AlsaSeqIn::ProcessEvent()
{
	snd_seq_event_t *ev;
		int l1;
		
	do{
				snd_seq_event_input(seq_handle, &ev);
				switch (ev->type) {
					case SND_SEQ_EVENT_PITCHBEND:
						//pitch = (double)ev->data.control.value / 8192.0;
					break;
				
					case SND_SEQ_EVENT_CONTROLLER:
						if (ev->data.control.param == 1) {
							//pitch = (double)ev->data.control.value / 8192.0;
						}
					break;

					case SND_SEQ_EVENT_NOTEON:
						if ( ev->data.note.velocity != 0 ) {
							//note[l1] = ev->data.note.note;
							//velocity[l1] = ev->data.note.velocity / 127.0;
						}
					break;
					// else, we do NOTEOFF
					case SND_SEQ_EVENT_NOTEOFF:
						//ev->data.note.note
					break;
					case SND_SEQ_EVENT_NOTE:
						//note[l1] = ev->data.note.note;
						//velocity[l1] = ev->data.note.velocity / 127.0;
					break;
		}
		snd_seq_free_event(ev);
	} while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}
/*
SND_SEQ_EVENT_SYSTEM                 system status; event data type = snd_seq_result_t
SND_SEQ_EVENT_RESULT                 returned result status; event data type = snd_seq_result_t
SND_SEQ_EVENT_NOTE                   note on and off with duration; event data type = snd_seq_ev_note_t
SND_SEQ_EVENT_NOTEON                 note on; event data type = snd_seq_ev_note_t
SND_SEQ_EVENT_NOTEOFF                note off; event data type = snd_seq_ev_note_t
SND_SEQ_EVENT_KEYPRESS               key pressure change (aftertouch); event data type = snd_seq_ev_note_t
SND_SEQ_EVENT_CONTROLLER             controller; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_PGMCHANGE              program change; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_CHANPRESS              channel pressure; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_PITCHBEND              pitchwheel; event data type = snd_seq_ev_ctrl_t; data is from -8192 to 8191)
SND_SEQ_EVENT_CONTROL14              14 bit controller value; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_NONREGPARAM            14 bit NRPN; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_REGPARAM               14 bit RPN; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_SONGPOS                SPP with LSB and MSB values; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_SONGSEL                Song Select with song ID number; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_QFRAME                 midi time code quarter frame; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_TIMESIGN               SMF Time Signature event; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_KEYSIGN                SMF Key Signature event; event data type = snd_seq_ev_ctrl_t
SND_SEQ_EVENT_START                  MIDI Real Time Start message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_CONTINUE               MIDI Real Time Continue message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_STOP                   MIDI Real Time Stop message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_SETPOS_TICK            Set tick queue position; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_SETPOS_TIME            Set real-time queue position; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_TEMPO                  (SMF) Tempo event; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_CLOCK                  MIDI Real Time Clock message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_TICK MIDI              Real Time Tick message; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_QUEUE_SKEW             Queue timer skew; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_SYNC_POS               Sync position changed; event data type = snd_seq_ev_queue_control_t
SND_SEQ_EVENT_TUNE_REQUEST           Tune request; event data type = none
SND_SEQ_EVENT_RESET                  Reset to power-on state; event data type = none
SND_SEQ_EVENT_SENSING                Active sensing event; event data type = none
SND_SEQ_EVENT_ECHO                   Echo-back event; event data type = any type
SND_SEQ_EVENT_OSS                    OSS emulation raw event; event data type = any type
SND_SEQ_EVENT_CLIENT_START           New client has connected; event data type = snd_seq_addr_t
SND_SEQ_EVENT_CLIENT_EXIT            Client has left the system; event data type = snd_seq_addr_t
SND_SEQ_EVENT_CLIENT_CHANGE          Client status/info has changed; event data type = snd_seq_addr_t
SND_SEQ_EVENT_PORT_START             New port was created; event data type = snd_seq_addr_t
SND_SEQ_EVENT_PORT_EXIT              Port was deleted from system; event data type = snd_seq_addr_t
SND_SEQ_EVENT_PORT_CHANGE            Port status/info has changed; event data type = snd_seq_addr_t
SND_SEQ_EVENT_PORT_SUBSCRIBED        Ports connected; event data type = snd_seq_connect_t
SND_SEQ_EVENT_PORT_UNSUBSCRIBED      Ports disconnected; event data type = snd_seq_connect_t
SND_SEQ_EVENT_SAMPLE                 Sample select; event data type = snd_seq_ev_sample_control_t
SND_SEQ_EVENT_SAMPLE_CLUSTER         Sample cluster select; event data type = snd_seq_ev_sample_control_t
SND_SEQ_EVENT_SAMPLE_START           voice start
SND_SEQ_EVENT_SAMPLE_STOP            voice stop
SND_SEQ_EVENT_SAMPLE_FREQ            playback frequency
SND_SEQ_EVENT_SAMPLE_VOLUME          volume and balance
SND_SEQ_EVENT_SAMPLE_LOOP            sample loop
SND_SEQ_EVENT_SAMPLE_POSITION        sample position
SND_SEQ_EVENT_SAMPLE_PRIVATE1        private (hardware dependent) event
SND_SEQ_EVENT_USR0                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR1                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR2                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR3                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR4                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR5                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR6                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR7                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR8                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_USR9                   user-defined event; event data type = any (fixed size)
SND_SEQ_EVENT_INSTR_BEGIN            begin of instrument management
SND_SEQ_EVENT_INSTR_END              end of instrument management
SND_SEQ_EVENT_INSTR_INFO             query instrument interface info
SND_SEQ_EVENT_INSTR_INFO_RESULT      result of instrument interface info
SND_SEQ_EVENT_INSTR_FINFO            query instrument format info
SND_SEQ_EVENT_INSTR_FINFO_RESULT     result of instrument format info
SND_SEQ_EVENT_INSTR_RESET            reset instrument instrument memory
SND_SEQ_EVENT_INSTR_STATUS           get instrument interface status
SND_SEQ_EVENT_INSTR_STATUS_RESULT    result of instrument interface status
SND_SEQ_EVENT_INSTR_PUT              put an instrument to port
SND_SEQ_EVENT_INSTR_GET              get an instrument from port
SND_SEQ_EVENT_INSTR_GET_RESULT       result of instrument query
SND_SEQ_EVENT_INSTR_FREE             free instrument(s)
SND_SEQ_EVENT_INSTR_LIST             get instrument list
SND_SEQ_EVENT_INSTR_LIST_RESULT      result of instrument list
SND_SEQ_EVENT_INSTR_CLUSTER          set cluster parameters
SND_SEQ_EVENT_INSTR_CLUSTER_GET      get cluster parameters
SND_SEQ_EVENT_INSTR_CLUSTER_RESULT   result of cluster parameters
SND_SEQ_EVENT_INSTR_CHANGE           instrument change
SND_SEQ_EVENT_SYSEX                  system exclusive data (variable length); event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_BOUNCE                 error event; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR0               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR1               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR2               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR3               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_USR_VAR4               reserved for user apps; event data type = snd_seq_ev_ext_t
SND_SEQ_EVENT_NONE                   NOP; ignored in any case
*/

#endif // defined PSYCLE__ALSA_AVAILABLE
