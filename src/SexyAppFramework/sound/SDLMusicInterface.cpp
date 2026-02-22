#include "SDLMusicInterface.h"
#include "SexyAppFramework/paklib/PakInterface.h"

using namespace Sexy;

SDLMusicInfo::SDLMusicInfo()
{
	mVolume = 0.0;
	mVolumeAdd = 0.0;
	mVolumeCap = 1.0;
	mStopOnFade = false;
	mHMusic = 0;
	mData = nullptr;
}

SDLMusicInterface::SDLMusicInterface()
{
	mGlobalVolume = MIX_MAX_VOLUME;
}

SDLMusicInterface::~SDLMusicInterface()
{
	Mix_HaltMusic();
}

bool SDLMusicInterface::LoadMusic(int theSongId, const std::string& theFileName)
{
	Mix_Music* aHMusic = nullptr;
    uint8_t* aData = nullptr;

	const char* extensions[] = { ".ogg", ".mp3", ".wav", ".mo3", "" };
	for (int i = 0; i < 5; i++)
	{
		std::string aTryPath = theFileName + extensions[i];
		PFILE* fp = p_fopen(aTryPath.c_str(), "rb");
		if (fp)
		{
            p_fseek(fp, 0, SEEK_END);
            size_t aSize = p_ftell(fp);
            p_fseek(fp, 0, SEEK_SET);

            if (aSize > 0)
            {
                aData = new uint8_t[aSize];
                p_fread(aData, 1, aSize, fp);
                p_fclose(fp);

                aHMusic = Mix_LoadMUS_RW(SDL_RWFromConstMem(aData, aSize), 1);
                if (aHMusic)
                    break;
                
                delete[] aData;
                aData = nullptr;
            }
            else
            {
			    p_fclose(fp);
            }
		}
	}

	if (aHMusic == nullptr)
		return false;
	
	SDLMusicInfo aMusicInfo;	
	aMusicInfo.mHMusic = aHMusic;
    aMusicInfo.mData = aData;
	mMusicMap.insert(SDLMusicMap::value_type(theSongId, aMusicInfo));

	return true;
}

void SDLMusicInterface::PlayMusic(int theSongId, int theOffset, bool noLoop)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = aMusicInfo->mVolumeCap;
		aMusicInfo->mVolumeAdd = 0.0;
		aMusicInfo->mStopOnFade = noLoop;

		Mix_HaltMusicStream(aMusicInfo->mHMusic);
		Mix_PlayMusicStream(aMusicInfo->mHMusic, (noLoop) ? 0 : -1);
		if (theOffset > 0)
			Mix_ModMusicStreamJumpToOrder(aMusicInfo->mHMusic, theOffset);
    }
}

void SDLMusicInterface::StopMusic(int theSongId)
{
	Mix_HaltMusic();

	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = 0.0;
		Mix_HaltMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::PauseMusic(int theSongId)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		Mix_PauseMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::ResumeMusic(int theSongId)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		//gBass->BASS_ChannelResume(aMusicInfo->GetHandle());
		Mix_ResumeMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::StopAllMusic()
{
	SDLMusicMap::iterator anItr = mMusicMap.begin();
	while (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		aMusicInfo->mVolume = 0.0;
		Mix_HaltMusicStream(aMusicInfo->mHMusic);
		++anItr;
	}
}

void SDLMusicInterface::UnloadMusic(int theSongId)
{
	StopMusic(theSongId);
	
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		Mix_FreeMusic(aMusicInfo->mHMusic);
        if (aMusicInfo->mData)
            delete[] aMusicInfo->mData;

		mMusicMap.erase(anItr);
	}
}

void SDLMusicInterface::UnloadAllMusic()
{
	StopAllMusic();
	for (SDLMusicMap::iterator anItr = mMusicMap.begin(); anItr != mMusicMap.end(); ++anItr)
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		Mix_FreeMusic(aMusicInfo->mHMusic);
        if (aMusicInfo->mData)
            delete[] aMusicInfo->mData;
	}
	mMusicMap.clear();
}

void SDLMusicInterface::PauseAllMusic()
{
	for (SDLMusicMap::iterator anItr = mMusicMap.begin(); anItr != mMusicMap.end(); ++anItr)
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		if (Mix_PlayingMusicStream(aMusicInfo->mHMusic) && !Mix_PausedMusicStream(aMusicInfo->mHMusic))
			Mix_PauseMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::ResumeAllMusic()
{
	for (SDLMusicMap::iterator anItr = mMusicMap.begin(); anItr != mMusicMap.end(); ++anItr)
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		if (Mix_PausedMusicStream(aMusicInfo->mHMusic))
			Mix_ResumeMusicStream(aMusicInfo->mHMusic);
	}
}

void SDLMusicInterface::FadeIn(int theSongId, int theOffset, double theSpeed, bool noLoop)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
				
		aMusicInfo->mVolumeAdd = theSpeed;
		aMusicInfo->mStopOnFade = noLoop;

		Mix_HaltMusicStream(aMusicInfo->mHMusic);
		Mix_PlayMusicStream(aMusicInfo->mHMusic, (noLoop) ? 0 : -1);
		if (theOffset > 0)
			Mix_ModMusicStreamJumpToOrder(aMusicInfo->mHMusic, theOffset);

		/*
		gBass->BASS_ChannelStop(aMusicInfo->GetHandle());
		gBass->BASS_ChannelSetAttribute(aMusicInfo->GetHandle(), BASS_ATTRIB_VOL, (int) (aMusicInfo->mVolume));
		if (aMusicInfo->mHMusic)
		{
			if (theOffset == -1)
				gBass->BASS_MusicPlay(aMusicInfo->mHMusic);
			else
			{
				gBass->BASS_MusicPlayEx(aMusicInfo->mHMusic, theOffset, BASS2_MUSIC_RAMP | (noLoop ? 0 : BASS_MUSIC_LOOP), TRUE);
			}
		}
		else
		{
			BOOL flush = theOffset == -1 ? FALSE : TRUE;
			gBass->BASS_StreamPlay(aMusicInfo->mHStream, flush, noLoop ? 0 : BASS_MUSIC_LOOP);
			if (theOffset > 0)
				gBass->BASS_ChannelSetPosition(aMusicInfo->mHStream, theOffset, BASS_POS_BYTE);
		}
		*/

	}
}

void SDLMusicInterface::FadeOut(int theSongId, bool stopSong, double theSpeed)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{		
		SDLMusicInfo* aMusicInfo = &anItr->second;
		
		if (aMusicInfo->mVolume != 0.0)
		{
			aMusicInfo->mVolumeAdd = -theSpeed;			
		}

		aMusicInfo->mStopOnFade = stopSong;
	}
}

void SDLMusicInterface::FadeOutAll(bool stopSong, double theSpeed)
{
	for (SDLMusicMap::iterator anItr = mMusicMap.begin(); anItr != mMusicMap.end(); ++anItr)
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		
		if (aMusicInfo->mVolume != 0.0)
		{
			aMusicInfo->mVolumeAdd = -theSpeed;			
		}

		aMusicInfo->mStopOnFade = stopSong;
	}
}

void SDLMusicInterface::SetSongVolume(int theSongId, double theVolume)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		
		aMusicInfo->mVolume = theVolume;
		aMusicInfo->mVolumeCap = theVolume;

		Mix_SetMusicTrackMute(aMusicInfo->mHMusic, -1, (int)(theVolume * 128));
		//gBass->BASS_ChannelSetAttribute(aMusicInfo->GetHandle(), BASS_ATTRIB_VOL, (int) (aMusicInfo->mVolume));
	}
}

void SDLMusicInterface::SetSongMaxVolume(int theSongId, double theMaxVolume)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		
		aMusicInfo->mVolumeCap = theMaxVolume;
	}
}

bool SDLMusicInterface::IsPlaying(int theSongId)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		//return gBass->BASS_ChannelIsActive(aMusicInfo->GetHandle()) == BASS_ACTIVE_PLAYING;
		return Mix_PlayingMusicStream(aMusicInfo->mHMusic);
	}

	return false;
}

void SDLMusicInterface::SetVolume(double theVolume)
{
	mGlobalVolume = (int) (theVolume * MIX_MAX_VOLUME);
	//gBass->BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (int) (theVolume * 10000));
	//gBass->BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, (int) (theVolume * 10000));
	Mix_VolumeMusic(mGlobalVolume);
}

void SDLMusicInterface::SetMusicAmplify(int theSongId, double theAmp)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		
		//gBass->BASS_ChannelSetAttribute(aMusicInfo->GetHandle(), BASS_ATTRIB_MUSIC_AMPLIFY, (int) (theAmp * 100));
	}
}

void SDLMusicInterface::Update()
{
	for (SDLMusicMap::iterator anItr = mMusicMap.begin(); anItr != mMusicMap.end(); ++anItr)
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		
		if (aMusicInfo->mVolumeAdd != 0.0)
		{
			aMusicInfo->mVolume += aMusicInfo->mVolumeAdd;
			
			if (aMusicInfo->mVolume < 0.0)
			{
				aMusicInfo->mVolume = 0.0;
				aMusicInfo->mVolumeAdd = 0.0;
				
				if (aMusicInfo->mStopOnFade)
				{
					//gBass->BASS_ChannelStop(aMusicInfo->GetHandle());
					Mix_HaltMusicStream(aMusicInfo->mHMusic);
				}
			}
			else if (aMusicInfo->mVolume > aMusicInfo->mVolumeCap)
			{
				aMusicInfo->mVolume = aMusicInfo->mVolumeCap;
				aMusicInfo->mVolumeAdd = 0.0;
			}
			
			//gBass->BASS_ChannelSetAttribute(aMusicInfo->GetHandle(), BASS_ATTRIB_VOL, (int) (aMusicInfo->mVolume));
			Mix_SetMusicTrackMute(aMusicInfo->mHMusic, -1, (int)(aMusicInfo->mVolume * 128));
		}
	}
}

int SDLMusicInterface::GetMusicOrder(int theSongId)
{
	SDLMusicMap::iterator anItr = mMusicMap.find(theSongId);
	if (anItr != mMusicMap.end())
	{
		SDLMusicInfo* aMusicInfo = &anItr->second;
		//return gBass->BASS_ChannelGetPosition(aMusicInfo->GetHandle(), BASS_POS_MUSIC_ORDER);
	}
	
	return 0;
}

double SDLMusicInterface::GetMusicPosition(int theSongId)
{
    return 0;
}

int SDLMusicInterface::GetFreeMusicId()
{
	int aId = 0;
	while (mMusicMap.find(aId) != mMusicMap.end())
		aId++;
	return aId;
}
