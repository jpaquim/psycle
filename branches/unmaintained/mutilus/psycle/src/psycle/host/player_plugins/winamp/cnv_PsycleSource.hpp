// cnv_PsycleSource.h: interface for the cnv_PsycleSource class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <Wasabi_SDK/studio/studio/services/svc_mediaconverter.h>


class svc_PsycleSource : public svc_mediaConverterI, public TimerClientI, public DependentI {

public:
/*  enum {
    WIDTH = 320,
    HEIGHT = 200,
    THE_COLOR_PURPLE = 0x00800080,
    MS_DELAY = 33, // 30 fps
  };*/

public:
	svc_PsycleSource();
	virtual ~svc_PsycleSource();

	static const char *getServiceName()
	{
		return "Psycle Player";
	}

  virtual int canConvertFrom(svc_fileReader *reader, const char *name, const char *chunktype) {
	  if (name && !stricmp(name,"purple://")) return 1;
	  if (chunktype && !stricmp(chunktype,"purple")) return 1;
	  return 0;
  }

  virtual const char *getConverterTo() {
    return "VIDEO";
  }
  
  virtual int getInfos(MediaInfo *infos) {
    return 0;
  }

  virtual int processData(MediaInfo *infos, ChunkList *chunk_list, bool *killswitch);
  virtual void timerclient_timerCallback(int id) {
    if (id == THE_COLOR_PURPLE) {
      okToBlit = 1;
      timecount += (1 + timerclient_getSkipped()); // make sure we count skips. 
    }
    TimerClientI::timerclient_timerCallback(id);
  }

  virtual Dependent *timerclient_getDependencyPtr() {
    return this;
  }

  // =========================================================================
  //
  //  The COMPLETE virtual method table from "../../studio/corecb.h" 
  //    (We're going to override all of them!  Woo hoo!)
  //
  
  // Basic Events
  virtual int corecb_onStarted();
  virtual int corecb_onStopped();
  virtual int corecb_onPaused();
  virtual int corecb_onUnpaused();
  virtual int corecb_onSeeked(int newpos);
  
  // Volume & EQ Events
  virtual int corecb_onVolumeChange(int newvol);
  virtual int corecb_onEQStatusChange(int newval);
  virtual int corecb_onEQPreampChange(int newval);
  virtual int corecb_onEQBandChange(int band, int newval);
  
  // Info Events
  virtual int corecb_onTitleChange(const char *title);
  virtual int corecb_onTitle2Change(const char *title, const char *title2);
  virtual int corecb_onInfoChange(const char *info);
  virtual int corecb_onUrlChange(const char *url);
  
  // Source Events
  virtual int corecb_onNextFile();
  virtual int corecb_onNeedNextFile(int fileid);
  virtual int corecb_onAbortCurrentSong();
  virtual int corecb_onEndOfDecode();
  
  // Bad Events.
  virtual int corecb_onErrorOccured(int severity, const char *text);
  virtual int corecb_onStatusMsg(const char *text);
  virtual int corecb_onWarningMsg(const char *text);
  virtual int corecb_onErrorMsg(const char *text);
  


private:
  MemBlock<ARGB32> purplebuf;
  int okToBlit;
  int timecount;
};
