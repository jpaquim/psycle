/////////////////////////////////////////////////////////////////////
// Main Song class

class psySong
{
public:
	CCriticalSection door;
	
	psySong();
	~psySong();
	void newSong();	
	void Reset(void);
	void UpdateVolume(void);
	void updatePanflags(void);
	char Name[32];								// Song Name
	char Author[32];							// Song Author
	char Comment[128];							// Song Comment

	psyPat *pPatterns[256];
	bool pStatus[256];

	// Bus properties
	int eBus;
	int bIntVol[MAX_TRACKS];
	int bIntPan[MAX_TRACKS];
	int bIntFx1[MAX_TRACKS];
	int bIntFx2[MAX_TRACKS];
	float bRealVol[MAX_TRACKS];
	float bRealPanL[MAX_TRACKS];
	float bRealPanR[MAX_TRACKS];
	float bRealFx1[MAX_TRACKS];
	float bRealFx2[MAX_TRACKS];
	bool bMute[MAX_TRACKS];
	bool Activemachine[256];

	psyGear* machine[256];
};
