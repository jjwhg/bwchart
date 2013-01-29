#include"stdafx.h"
#include"replayinterface.h"
#include"bwrepapi.h"

// replay factory interface
class StarcraftReplayFactory : public IStarcraftReplayFactory
{
public:
	// create replay instance
	virtual IStarcraftReplay *CreateReplayInstance(const char *filename)
	{
		const char *p = strrchr(filename,'.');
		if(p==0) return 0;
		if(_stricmp(p,".rep")==0)
			return new BWrepFile;
		return 0;
	}
};

// globale methods to get a factory object
StarcraftReplayFactory gRepFactory;
IStarcraftReplayFactory *QueryFactory() {return &gRepFactory;}

//-----------------------------------------------
