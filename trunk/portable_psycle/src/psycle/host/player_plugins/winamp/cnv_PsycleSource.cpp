// cnv_PsycleSource.cpp: implementation of the cnv_PsycleSource class.
//
//////////////////////////////////////////////////////////////////////

#include <project.private.hpp>
#include "cnv_PsycleSource.hpp"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cnv_PsycleSource::cnv_PsycleSource()
{

}

cnv_PsycleSource::~cnv_PsycleSource()
{

}

class svc_PsycleSource : public svc_mediaConverterI, public TimerClientI, public DependentI {

public:
/*  enum {
    WIDTH = 320,
    HEIGHT = 200,
    THE_COLOR_PURPLE = 0x00800080,
    MS_DELAY = 33, // 30 fps
  };*/

public:
	svc_purpleVid() {
    // Setup the buffer we're going to blit continually.
    purplebuf.setSize(WIDTH * HEIGHT);
    // Fill it with the color purple.
    MEMFILL<ARGB32>(purplebuf.getMemory(), THE_COLOR_PURPLE, WIDTH * HEIGHT);
    // Get your timer going.
    timerclient_setTimer(THE_COLOR_PURPLE, MS_DELAY);
    // And be ready to blit (this is for the timer)
    okToBlit = 1;
    // And, for fun let's count how often our timer callback hits.
    timecount = 0;   
	}

	~svc_purpleVid() {
	}

	static const char *getServiceName() {
    return "Purple Video"; 
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

  virtual int processData(MediaInfo *infos, ChunkList *chunk_list, bool *killswitch) {

    if (!okToBlit) {
      chunk_list->setChunk("VIDEO", NULL, NULL);
      // If you don't Sleep(1) you kill the CPU.
      Sleep(1);
		  return 1;
    }
    okToBlit = 0;

    OutputDebugString(StringPrintf("REAL PURPLE BLIT #%04d\n",timecount));

    MEMFILL<ARGB32>(purplebuf.getMemory(), purples[timecount%numPurples], WIDTH * HEIGHT);

		ChunkInfosI *chunkinfo = new ChunkInfosI();

    // 32bpp video
    chunkinfo->addInfo("type", (int)"RGB32"); 

    // Aspect ratio is a float packed into an int.
    float aspectFloat = 1.33333F;
    chunkinfo->addInfo("aspect", *((int*)&aspectFloat));

    // Width is whatever your video source is.  For us, 320
    chunkinfo->addInfo("width", WIDTH);

    // Height is whatever your video source is.  For us, 200
    chunkinfo->addInfo("height", HEIGHT);

    // Flipv specifies whether or not to flip your video vertically.  For us, no
    chunkinfo->addInfo("flipv", 0);

    // Yes, all of our frames are keyframes.
    chunkinfo->addInfo("keyframe", 1);

    // How much time in ms has elapsed?
    chunkinfo->addInfo("time", timecount * MS_DELAY);

    // After we setup the chunk, feed it back to the core for more processing.
    chunk_list->setChunk("VIDEO", (void *)purplebuf.getMemory(), WIDTH * HEIGHT, chunkinfo);

		return 1;
	}

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

private:
  MemBlock<ARGB32> purplebuf;
  int okToBlit;
  int timecount;
};
