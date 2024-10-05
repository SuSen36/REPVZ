#ifndef __AUTOCRIT_INCLUDED__
#define __AUTOCRIT_INCLUDED__

#include "../Common.h"
#include "CritSect.h"

namespace Sexy
{

class AutoCrit
{
	pthread_mutex_t*		mCritSec;
public:
	AutoCrit(pthread_mutex_t* theCritSec) : 
		mCritSec(theCritSec)
	{
		pthread_mutex_trylock(mCritSec);
	}

	AutoCrit(CritSect& theCritSect) : 
		mCritSec(&theCritSect.mCriticalSection)
	{
		pthread_mutex_trylock(mCritSec);
	}

	~AutoCrit()
	{
		pthread_mutex_unlock(mCritSec);
	}
};

}

#endif //__AUTOCRIT_INCLUDED__