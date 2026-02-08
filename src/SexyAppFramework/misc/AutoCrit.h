#ifndef __AUTOCRIT_INCLUDED__
#define __AUTOCRIT_INCLUDED__

#include "SexyAppFramework/Common.h"
#include "CritSect.h"
#include <SDL3/SDL_mutex.h>

namespace Sexy
{

class AutoCrit
{
	SDL_Mutex* mCritSec;

public:
	AutoCrit(SDL_Mutex* theCritSec) :
		mCritSec(theCritSec)
	{
		SDL_LockMutex(mCritSec);
	}

	AutoCrit(CritSect& theCritSect) : 
		mCritSec(theCritSect.mCriticalSection)
	{
		SDL_LockMutex(mCritSec);
	}

	~AutoCrit()
	{
		SDL_UnlockMutex(mCritSec);
	}
};

}

#endif //__AUTOCRIT_INCLUDED__