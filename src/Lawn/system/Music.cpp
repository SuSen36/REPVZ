#include "Music.h"
#include "../Board.h"
#include "PlayerInfo.h"
#include "../../LawnApp.h"
#include "SexyAppFramework/paklib/PakInterface.h"
#include "../../Sexy.TodLib/TodDebug.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "SexyAppFramework/sound/SDLMusicInterface.h"
#include "SexyAppFramework/misc/ResourceManager.h"
#include "../../Resources.h"

using namespace Sexy;

//0x45A260
Music::Music()
{
	mApp = (LawnApp*)gSexyAppBase;
	mMusicInterface = gSexyAppBase->mMusicInterface;
	mCurMusicTune = MusicTune::MUSIC_TUNE_NONE;
	mCurMusicFileMain = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileDrums = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileHihats = MusicFile::MUSIC_FILE_NONE;
	mBurstOverride = -1;
	mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_OFF;
	mQueuedDrumTrackPackedOrder = -1;
	mBaseBPM = 155;
	mBaseModSpeed = 3;
	mMusicBurstState = MusicBurstState::MUSIC_BURST_OFF;
	mPauseOffset = 0;
	mPauseOffsetDrums = 0;
	mPaused = false;
	mMusicDisabled = false;
	mFadeOutCounter = 0;
	mFadeOutDuration = 0;

    for (int i = 0; i < MusicFile::NUM_MUSIC_FILES; i++)
        mMusicMap[i] = -1;
}

//0x45A2C0
bool Music::TodLoadMusic(MusicFile theMusicFile, const std::string& theFileName)
{
	TodTraceAndLog("TodLoadMusic: 尝试加载音乐资源: %s", theFileName.c_str());
    
    ResourceManager::ResMap::iterator anItr = mApp->mResourceManager->mMusicMap.find(theFileName);
    if (anItr == mApp->mResourceManager->mMusicMap.end())
    {
        TodTraceAndLog("TodLoadMusic: Resource not found: %s", theFileName.c_str());
        return false;
    }

    ResourceManager::MusicRes* aRes = (ResourceManager::MusicRes*)anItr->second;
    if (aRes->mMusicId == -1)
    {
        if (!mApp->mResourceManager->DoLoadMusic(aRes))
        {
             TodTraceAndLog("TodLoadMusic: Failed to load music: %s", theFileName.c_str());
             return false;
        }
    }

    mMusicMap[(int)theMusicFile] = aRes->mMusicId;
    return true;
}

//0x45A6C0
void Music::SetupMusicFileForTune(MusicFile theMusicFile, MusicTune theMusicTune)
{
	int aTrackCount = 0;
	int aTrackStart1 = -1, aTrackEnd1 = -1, aTrackStart2 = -1, aTrackEnd2 = -1;

	switch (theMusicTune)
	{
	case MusicTune::MUSIC_TUNE_DAY_GRASSWALK:
		switch (theMusicFile) {
		case MusicFile::MUSIC_FILE_MAIN_MUSIC:		aTrackCount = 29;	aTrackStart1 = 0;	aTrackEnd1 = 23;											break;
		case MusicFile::MUSIC_FILE_HIHATS:			aTrackCount = 29;	aTrackStart1 = 27;	aTrackEnd1 = 27;											break;
		case MusicFile::MUSIC_FILE_DRUMS:			aTrackCount = 29;	aTrackStart1 = 24;	aTrackEnd1 = 26;											break;
		default: break;
		} break;
	case MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES:
		switch (theMusicFile) {
		case MusicFile::MUSIC_FILE_MAIN_MUSIC:		aTrackCount = 29;	aTrackStart1 = 0;	aTrackEnd1 = 17;											break;
		case MusicFile::MUSIC_FILE_HIHATS:			aTrackCount = 29;	aTrackStart1 = 18;	aTrackEnd1 = 24;	aTrackStart2 = 29;	aTrackEnd2 = 29;	break;
		case MusicFile::MUSIC_FILE_DRUMS:			aTrackCount = 29;	aTrackStart1 = 25;	aTrackEnd1 = 28;											break;
		default: break;
		} break;
	case MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST:
		switch (theMusicFile) {
		case MusicFile::MUSIC_FILE_MAIN_MUSIC:		aTrackCount = 29;	aTrackStart1 = 0;	aTrackEnd1 = 15;											break;
		case MusicFile::MUSIC_FILE_HIHATS:			aTrackCount = 29;	aTrackStart1 = 23;	aTrackEnd1 = 23;											break;
		case MusicFile::MUSIC_FILE_DRUMS:			aTrackCount = 29;	aTrackStart1 = 16;	aTrackEnd1 = 22;											break;
		default: break;
		} break;
	case MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF:
		switch (theMusicFile) {
		case MusicFile::MUSIC_FILE_MAIN_MUSIC:		aTrackCount = 29;	aTrackStart1 = 0;	aTrackEnd1 = 17;											break;
		case MusicFile::MUSIC_FILE_HIHATS:			aTrackCount = 29;	aTrackStart1 = 21;	aTrackEnd1 = 21;											break;
		case MusicFile::MUSIC_FILE_DRUMS:			aTrackCount = 29;	aTrackStart1 = 18;	aTrackEnd1 = 20;											break;
		default: break;
		} break;
	default:
		if (theMusicFile == MusicFile::MUSIC_FILE_MAIN_MUSIC || theMusicFile == MusicFile::MUSIC_FILE_DRUMS)
		{
			aTrackCount = 29;
			aTrackStart1 = 0;
			aTrackEnd1 = 29;
		}
		break;
	}

    Mix_Music* aHMusic = GetMusicHandle(theMusicFile);
    if (!aHMusic) return;
    for (int aTrack = 0; aTrack < aTrackCount; aTrack++)
    {
        float aVolume;
        if (aTrack >= aTrackStart1 && aTrack <= aTrackEnd1)
            aVolume = 1;
        else if (aTrack >= aTrackStart2 && aTrack <= aTrackEnd2)
            aVolume = 1;
        else
            aVolume = 0;

        int finalVolume = (int)(aVolume*128);
        TodTrace("Setting track %d for MusicFile %d volume to %d (raw: %f)", aTrack, (int)theMusicFile, finalVolume, aVolume);
        Mix_SetMusicTrackMute(aHMusic, aTrack, finalVolume);
    }
}

void Music::LoadSong(MusicFile theMusicFile, const std::string& theFileName)
{
	TodHesitationTrace("preloadsong");
	if (!TodLoadMusic(theMusicFile, theFileName))
	{
		TodTrace("music failed to load\n");
		mMusicDisabled = true;
	}
	else
	{
		//gBass->BASS_ChannelSetAttribute(GetBassMusicHandle(theMusicFile), BASS_ATTRIB_MUSIC_PSCALER, 4);  // 设置音乐定位精确度属性
		TodHesitationTrace("song '%s'", theFileName.c_str());
	}
}

//0x45A8A0
void Music::MusicTitleScreenInit()
{
	TodTraceAndLog("MusicTitleScreenInit called.");
	LoadSong(MusicFile::MUSIC_FILE_CRAZY_DAVE, "MUSIC_TITLE_CRAZY_DAVE_MAIN_THEME");
    // LoadSong(MusicFile::MUSIC_FILE_CRAZY_DAVE_DRUMS, "MUSIC_TITLE_CRAZY_DAVE_MAIN_THEME_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_CRAZY_DAVE_HIHATS, "MUSIC_TITLE_CRAZY_DAVE_MAIN_THEME_HIHATS");
	// LoadSong(MusicFile::MUSIC_FILE_MAIN_MUSIC, "sounds/mainmusic.mo3");
	TodTraceAndLog("MusicTitleScreenInit: crazydave.ogg 加载完成.");
	MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME);
	TodTraceAndLog("MusicTitleScreenInit: MakeSureMusicIsPlaying called.");
}

//0x45A980
void Music::MusicInit()
{
	TodTraceAndLog("MusicInit called.");
#ifdef _DEBUG
	int aNumLoadingTasks = mApp->mCompletedLoadingThreadTasks + GetNumLoadingTasks();
#endif
	
	// MO3 Removed
	// LoadSong(MusicFile::MUSIC_FILE_DRUMS, "sounds/mainmusic.mo3");
	// mApp->mCompletedLoadingThreadTasks += /*原版*/3500;///*内测版*/800;
	// LoadSong(MusicFile::MUSIC_FILE_HIHATS, "sounds/mainmusic_hihats.mo3");
	// mApp->mCompletedLoadingThreadTasks += /*原版*/3500;///*内测版*/800;

#ifdef _DEBUG
	LoadSong(MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN, "MUSIC_CREDITS_ZOMBIES_ON_YOUR_LAWN");
	TodTraceAndLog("MusicInit: credits music 加载完成.");
	mApp->mCompletedLoadingThreadTasks += /*原版*/3500;///*内测版*/800;

	LoadSong(MusicFile::MUSIC_FILE_ZEN_GARDEN, "MUSIC_ZEN_GARDEN");
    // LoadSong(MusicFile::MUSIC_FILE_ZEN_GARDEN_DRUMS, "MUSIC_ZEN_GARDEN_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_ZEN_GARDEN_HIHATS, "MUSIC_ZEN_GARDEN_HIHATS");
	TodTraceAndLog("MusicInit: zengarden.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_ROOF, "MUSIC_ROOF_GRAZETHEROOF");
    LoadSong(MusicFile::MUSIC_FILE_ROOF_DRUMS, "MUSIC_ROOF_GRAZETHEROOF_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_ROOF_HIHATS, "MUSIC_ROOF_GRAZETHEROOF_HIHATS");
	TodTraceAndLog("MusicInit: roof.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_POOL, "MUSIC_POOL_WATERYGRAVES");
    LoadSong(MusicFile::MUSIC_FILE_POOL_DRUMS, "MUSIC_POOL_WATERYGRAVES_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_POOL_HIHATS, "MUSIC_POOL_WATERYGRAVES_HIHATS");
	TodTraceAndLog("MusicInit: pool.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_NIGHT, "MUSIC_NIGHT_MOONGRAINS");
    LoadSong(MusicFile::MUSIC_FILE_NIGHT_DRUMS, "MUSIC_NIGHT_MOONGRAINS_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_NIGHT_HIHATS, "MUSIC_NIGHT_MOONGRAINS_HIHATS");
	TodTraceAndLog("MusicInit: night.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_LOONBOON, "MUSIC_MINIGAME_LOONBOON");
    // LoadSong(MusicFile::MUSIC_FILE_LOONBOON_DRUMS, "MUSIC_MINIGAME_LOONBOON_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_LOONBOON_HIHATS, "MUSIC_MINIGAME_LOONBOON_HIHATS");
	TodTraceAndLog("MusicInit: loonboon.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_FOG, "MUSIC_FOG_RIGORMORMIST");
    LoadSong(MusicFile::MUSIC_FILE_FOG_DRUMS, "MUSIC_FOG_RIGORMORMIST_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_FOG_HIHATS, "MUSIC_FOG_RIGORMORMIST_HIHATS");
	TodTraceAndLog("MusicInit: fog.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_DAY, "MUSIC_DAY_GRASSWALK");
    LoadSong(MusicFile::MUSIC_FILE_DAY_DRUMS, "MUSIC_DAY_GRASSWALK_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_DAY_HIHATS, "MUSIC_DAY_GRASSWALK_HIHATS");
	TodTraceAndLog("MusicInit: day.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_CRAZY_DAVE, "MUSIC_TITLE_CRAZY_DAVE_MAIN_THEME");
    // LoadSong(MusicFile::MUSIC_FILE_CRAZY_DAVE_DRUMS, "MUSIC_TITLE_CRAZY_DAVE_MAIN_THEME_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_CRAZY_DAVE_HIHATS, "MUSIC_TITLE_CRAZY_DAVE_MAIN_THEME_HIHATS");
	TodTraceAndLog("MusicInit: crazydave.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_CONVEYOR, "MUSIC_CONVEYER");
    // LoadSong(MusicFile::MUSIC_FILE_CONVEYOR_DRUMS, "MUSIC_CONVEYER_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_CONVEYOR_HIHATS, "MUSIC_CONVEYER_HIHATS");
	TodTraceAndLog("MusicInit: conveyor.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_CHOOSE_YOUR_SEEDS, "MUSIC_CHOOSE_YOUR_SEEDS");
    // LoadSong(MusicFile::MUSIC_FILE_CHOOSE_YOUR_SEEDS_DRUMS, "MUSIC_CHOOSE_YOUR_SEEDS_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_CHOOSE_YOUR_SEEDS_HIHATS, "MUSIC_CHOOSE_YOUR_SEEDS_HIHATS");
	TodTraceAndLog("MusicInit: chooseyourseeds.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_CEREBRAWL, "MUSIC_PUZZLE_CEREBRAWL");
    // LoadSong(MusicFile::MUSIC_FILE_CEREBRAWL_DRUMS, "MUSIC_PUZZLE_CEREBRAWL_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_CEREBRAWL_HIHATS, "MUSIC_PUZZLE_CEREBRAWL_HIHATS");
	TodTraceAndLog("MusicInit: cerebrawl.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;

	LoadSong(MusicFile::MUSIC_FILE_BOSS, "MUSIC_FINAL_BOSS_BRAINIAC_MANIAC");
    // LoadSong(MusicFile::MUSIC_FILE_BOSS_DRUMS, "MUSIC_FINAL_BOSS_BRAINIAC_MANIAC_DRUMS");
	LoadSong(MusicFile::MUSIC_FILE_BOSS_HIHATS, "MUSIC_FINAL_BOSS_BRAINIAC_MANIAC_HIHATS");
	TodTraceAndLog("MusicInit: boss.ogg 加载完成.");
	mApp->mCompletedLoadingThreadTasks += 3500;
	if (mApp->mCompletedLoadingThreadTasks != aNumLoadingTasks)
		TodTraceAndLog("Didn\'t calculate loading task count correctly!!!!");
#endif
}

//0x45AAC0
void Music::MusicCreditScreenInit()
{
#ifndef _DEBUG
	SDLMusicInterface* anSDL = (SDLMusicInterface*)mApp->mMusicInterface;
    int aMusicId = mMusicMap[(int)MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN];
	if (aMusicId == -1 || anSDL->mMusicMap.find(aMusicId) == anSDL->mMusicMap.end())  // 如果尚未加载
		LoadSong(MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN, "MUSIC_CREDITS_ZOMBIES_ON_YOUR_LAWN");
#endif
}

//0x45ABB0
void Music::StopAllMusic()
{
	if (mMusicInterface != nullptr)
	{
		if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileMain] != -1)
			mMusicInterface->StopMusic(mMusicMap[(int)mCurMusicFileMain]);
		if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileDrums] != -1)
			mMusicInterface->StopMusic(mMusicMap[(int)mCurMusicFileDrums]);
		if (mCurMusicFileHihats != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileHihats] != -1)
			mMusicInterface->StopMusic(mMusicMap[(int)mCurMusicFileHihats]);
	}

	mCurMusicTune = MusicTune::MUSIC_TUNE_NONE;
	mCurMusicFileMain = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileDrums = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileHihats = MusicFile::MUSIC_FILE_NONE;
	mQueuedDrumTrackPackedOrder = -1;
	mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_OFF;
	mMusicBurstState = MusicBurstState::MUSIC_BURST_OFF;
	mPauseOffset = 0;
	mPauseOffsetDrums = 0;
	mPaused = false;
	mFadeOutCounter = 0;
}

//0x45AC20
Mix_Music* Music::GetMusicHandle(MusicFile theMusicFile) {
    if (!mApp || !mApp->mMusicInterface) return nullptr;
    
    int aMusicId = mMusicMap[(int)theMusicFile];
    if (aMusicId == -1) return nullptr;

    SDLMusicInterface* anSDL = (SDLMusicInterface*)mApp->mMusicInterface;
    auto anItr = anSDL->mMusicMap.find(aMusicId);
    if (anItr == anSDL->mMusicMap.end()) return nullptr;
    return anItr->second.mHMusic;
}

//0x45AC70
void Music::PlayFromOffset(MusicFile theMusicFile, double theOffset, double theVolume)
{
    TodTrace("PlayFromOffset called for MusicFile: %d, Offset: %f, Volume: %f", theMusicFile, theOffset, theVolume);
    
    if (!mApp || !mApp->mMusicInterface)
    {
        TodTrace("Music::PlayFromOffset: mApp or MusicInterface is invalid");
        return;
    }

    int aMusicId = mMusicMap[(int)theMusicFile];
    if (aMusicId == -1)
    {
        TodTrace("Music::PlayFromOffset: MusicFile %d not loaded", theMusicFile);
        return;
    }

    SDLMusicInterface* anSDL = (SDLMusicInterface*)mApp->mMusicInterface;
    auto anItr = anSDL->mMusicMap.find(aMusicId);
    if (anItr == anSDL->mMusicMap.end())
    {
        TodTrace("Music::PlayFromOffset: MusicId %d not found in map", aMusicId);
        return;
    }
    
    SDLMusicInfo* aMusicInfo = &anItr->second;

    {
        Mix_HaltMusicStream(aMusicInfo->mHMusic);
        aMusicInfo->mStopOnFade = false;
        aMusicInfo->mVolume = aMusicInfo->mVolumeCap * theVolume;
        aMusicInfo->mVolumeAdd = 0.0;
        TodTrace("Attempting to play music with volume: %f", aMusicInfo->mVolume);
        if (Mix_PlayMusicStream(aMusicInfo->mHMusic, -1) == -1)
        {
            TodTrace("Mix_PlayMusicStream failed: %s", Mix_GetError());
        }
        
        Mix_SetMusicPositionStream(aMusicInfo->mHMusic, theOffset);
        Mix_VolumeMusicStream(aMusicInfo->mHMusic, (int)(aMusicInfo->mVolume*128));
        SetupMusicFileForTune(theMusicFile, mCurMusicTune);  // 调整每条轨道的静音与否
    }
}

void Music::PlayMusicNoOffset(MusicFile theMusicFile, double theVolume)
{
    TodTrace("PlayMusicNoOffset called for MusicFile: %d, Volume: %f", theMusicFile, theVolume);
    
    if (!mApp || !mApp->mMusicInterface)
    {
        TodTrace("Music::PlayMusicNoOffset: mApp or MusicInterface is invalid");
        return;
    }

    int aMusicId = mMusicMap[(int)theMusicFile];
    if (aMusicId == -1)
    {
        TodTrace("Music::PlayMusicNoOffset: MusicFile %d not loaded", theMusicFile);
        return;
    }

    SDLMusicInterface* anSDL = (SDLMusicInterface*)mApp->mMusicInterface;
    auto anItr = anSDL->mMusicMap.find(aMusicId);
    if (anItr == anSDL->mMusicMap.end())
    {
        TodTrace("Music::PlayMusicNoOffset: MusicId %d not found in map", aMusicId);
        return;
    }

    SDLMusicInfo* aMusicInfo = &anItr->second;

    {
        Mix_HaltMusicStream(aMusicInfo->mHMusic);
        aMusicInfo->mStopOnFade = false;
        aMusicInfo->mVolume = aMusicInfo->mVolumeCap * theVolume;
        aMusicInfo->mVolumeAdd = 0.0;
        TodTrace("Attempting to play music with volume: %f", aMusicInfo->mVolume);
        if (Mix_PlayMusicStream(aMusicInfo->mHMusic, -1) == -1)
        {
            TodTrace("Mix_PlayMusicStream failed: %s", Mix_GetError());
        }
        Mix_VolumeMusicStream(aMusicInfo->mHMusic, (int)(aMusicInfo->mVolume*128));
        SetupMusicFileForTune(theMusicFile, mCurMusicTune);
    }
}

//0x45ADB0
void Music::PlayMusic(MusicTune theMusicTune, double theOffset, double theDrumsOffset)
{
	if (mMusicDisabled)
		return;

	mCurMusicTune = theMusicTune;
	mCurMusicFileMain = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileDrums = MusicFile::MUSIC_FILE_NONE;
	mCurMusicFileHihats = MusicFile::MUSIC_FILE_NONE;
	bool aRestartingSong = theOffset != -1.0;

	switch (theMusicTune)
	{
	case MusicTune::MUSIC_TUNE_DAY_GRASSWALK:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_DAY;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_DAY_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_DAY_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_NIGHT;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_NIGHT_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_NIGHT_HIHATS;
		if (theOffset == -1.0)
		{
			theOffset = 0.0;
			theDrumsOffset = 0.0;
		}
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theDrumsOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_POOL;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_POOL_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_POOL_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_FOG;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_FOG_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_FOG_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_ROOF;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_ROOF_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_ROOF_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_CHOOSE_YOUR_SEEDS:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_CHOOSE_YOUR_SEEDS;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_CHOOSE_YOUR_SEEDS_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_CHOOSE_YOUR_SEEDS_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_CRAZY_DAVE;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_CRAZY_DAVE_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_CRAZY_DAVE_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_ZEN_GARDEN:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_ZEN_GARDEN;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_ZEN_GARDEN_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_ZEN_GARDEN_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_PUZZLE_CEREBRAWL:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_CEREBRAWL;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_CEREBRAWL_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_CEREBRAWL_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_MINIGAME_LOONBOON:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_LOONBOON;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_LOONBOON_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_LOONBOON_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_CONVEYER:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_CONVEYOR;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_CONVEYOR_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_CONVEYOR_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_FINAL_BOSS_BRAINIAC_MANIAC:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_BOSS;
		mCurMusicFileDrums = MusicFile::MUSIC_FILE_BOSS_DRUMS;
		mCurMusicFileHihats = MusicFile::MUSIC_FILE_BOSS_HIHATS;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		PlayFromOffset(mCurMusicFileDrums, theOffset, 0.0);
		PlayFromOffset(mCurMusicFileHihats, theOffset, 0.0);
		break;

	case MusicTune::MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN:
		mCurMusicFileMain = MusicFile::MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN;
		if (theOffset == -1.0)
			theOffset = 0.0;
		PlayFromOffset(mCurMusicFileMain, theOffset, 1.0);
		break;

	default:
		TOD_ASSERT(false);
		break;
	}

    if (aRestartingSong) {
        if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE) {
            Mix_Music* aMusic = GetMusicHandle(mCurMusicFileMain);
            if (aMusic)
            {
                Mix_SetMusicTempo(aMusic, mBaseBPM);
                Mix_SetMusicSpeed(aMusic, mBaseModSpeed);
            }
        }
        if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE) {
            Mix_Music* aMusic = GetMusicHandle(mCurMusicFileDrums);
            if (aMusic)
            {
                Mix_SetMusicTempo(aMusic, mBaseBPM);
                Mix_SetMusicSpeed(aMusic, mBaseModSpeed);
            }
        }
        if (mCurMusicFileHihats != MusicFile::MUSIC_FILE_NONE) {
            Mix_Music* aMusic = GetMusicHandle(mCurMusicFileHihats);
            if (aMusic)
            {
                Mix_SetMusicTempo(aMusic, mBaseBPM);
                Mix_SetMusicSpeed(aMusic, mBaseModSpeed);
            }
        }
    } else {
        Mix_Music* aMusic = GetMusicHandle(mCurMusicFileMain);
        if (aMusic)
        {
            mBaseBPM = Mix_GetMusicTempo(aMusic);
            mBaseModSpeed = Mix_GetMusicSpeed(aMusic);
        }
    }
}

double Music::GetMusicOrder(MusicFile theMusicFile)
{
	TOD_ASSERT(theMusicFile != MusicFile::MUSIC_FILE_NONE);
    int aMusicId = mMusicMap[(int)theMusicFile];
    if (aMusicId == -1) return 0.0;
	return ((SDLMusicInterface*)mApp->mMusicInterface)->GetMusicPosition(aMusicId);
}

//0x45B1B0
void Music::MusicResyncChannel(MusicFile theMusicFileToMatch, MusicFile theMusicFileToSync)
{
	// TODO: REIMPLEMENT FOR STREAMING MUSIC (OGG)
	// Original logic used pattern/row order which is not applicable for OGG streams.
	/*
	unsigned int aPosToMatch = GetMusicOrder(theMusicFileToMatch);
	unsigned int aPosToSync = GetMusicOrder(theMusicFileToSync);
	int aDiff = (aPosToSync >> 16) - (aPosToMatch >> 16);  // 待同步的音乐与目标音乐的乐曲序号之差
	if (abs(aDiff) <= 128)  // 当前进行的乐曲序号之差超过 128 时，开摆（
	{
		//HMUSIC aHMusic = GetBassMusicHandle(theMusicFileToSync);

		int aBPM = mBaseBPM;
		if (aDiff > 2)
			aBPM -= 2;
		else if (aDiff > 0)
			aBPM -= 1;
		else if (aDiff < -2)
			aBPM += 2;
		else if (aDiff < 0)
			aBPM -= 1;

		//gBass->BASS_ChannelSetAttribute(aHMusic, BASS_ATTRIB_MUSIC_BPM, aBPM);  // 适当调整待同步音乐的速率以缩小差距
	}
	*/
}

void Music::MusicResync()
{
	if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE)
	{
		if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE)
			MusicResyncChannel(mCurMusicFileMain, mCurMusicFileDrums);
		if (mCurMusicFileHihats != MusicFile::MUSIC_FILE_NONE)
			MusicResyncChannel(mCurMusicFileMain, mCurMusicFileHihats);
	}
}

//0x45B240
void Music::StartBurst()
{ 
	if (mMusicBurstState == MusicBurstState::MUSIC_BURST_OFF)
	{ 
		mMusicBurstState = MusicBurstState::MUSIC_BURST_STARTING;
		mBurstStateCounter = 400;
	}
}

void Music::FadeOut(int theFadeOutDuration)
{ 
	if (mCurMusicTune != MusicTune::MUSIC_TUNE_NONE)
	{
		mFadeOutCounter = theFadeOutDuration;
		mFadeOutDuration = theFadeOutDuration;
	}
}

//0x45B260
void Music::UpdateMusicBurst()
{
	if (mApp->mBoard == nullptr)
		return;

	int aBurstScheme;
	if (mCurMusicTune == MusicTune::MUSIC_TUNE_DAY_GRASSWALK || mCurMusicTune == MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES ||
		mCurMusicTune == MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST || mCurMusicTune == MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF)
		aBurstScheme = 1;
	else if (mCurMusicTune == MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS)
		aBurstScheme = 2;
	else
		return;

	if (mBurstStateCounter > 0)
		mBurstStateCounter--;
	if (mDrumsStateCounter > 0)
		mDrumsStateCounter--;

	float aFadeTrackVolume = 0.0f;
	float aDrumsVolume = 0.0f;
	float aMainTrackVolume = 1.0f;
	switch (mMusicBurstState)
	{
		case MusicBurstState::MUSIC_BURST_OFF:
			if (mApp->mBoard->CountZombiesOnScreen() >= 10 || mBurstOverride == 1)
				StartBurst();
			break;
		case MusicBurstState::MUSIC_BURST_STARTING:
			if (aBurstScheme == 1)
			{
				aFadeTrackVolume = TodAnimateCurveFloat(400, 0, mBurstStateCounter, 0.0f, 1.0f, TodCurves::CURVE_LINEAR);
				if (mBurstStateCounter == 100)
				{
					mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_ON_QUEUED;
				}
				else if (mBurstStateCounter == 0)
				{
					mMusicBurstState = MusicBurstState::MUSIC_BURST_ON;
					mBurstStateCounter = 800;
				}
			}
			else if (aBurstScheme == 2)
			{
				if (mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_OFF)
				{
					mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_ON_QUEUED;
					mBurstStateCounter = 400;
				}
				else if (mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_ON_QUEUED)
					mBurstStateCounter = 400;
				else
				{
					aMainTrackVolume = TodAnimateCurveFloat(400, 0, mBurstStateCounter, 1.0f, 0.0f, TodCurves::CURVE_LINEAR);
					if (mBurstStateCounter == 0)
					{
						mMusicBurstState = MusicBurstState::MUSIC_BURST_ON;
						mBurstStateCounter = 800;
					}
				}
			}
			break;
		case MusicBurstState::MUSIC_BURST_ON:
			aFadeTrackVolume = 1.0f;
			if (aBurstScheme == 2)
				aMainTrackVolume = 0.0f;
			if (mBurstStateCounter == 0 && ((mApp->mBoard->CountZombiesOnScreen() < 4 && mBurstOverride == -1) || mBurstOverride == 2))
			{
				if (aBurstScheme == 1)
				{
					mMusicBurstState = MusicBurstState::MUSIC_BURST_FINISHING;
					mBurstStateCounter = 800;
					mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_OFF_QUEUED;
				}
				else if (aBurstScheme == 2)
				{
					mMusicBurstState = MusicBurstState::MUSIC_BURST_FINISHING;
					mBurstStateCounter = 1100;
					mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_FADING;
					mDrumsStateCounter = 800;
				}
			}
			break;
		case MusicBurstState::MUSIC_BURST_FINISHING:
			if (aBurstScheme == 1)
				aFadeTrackVolume = TodAnimateCurveFloat(800, 0, mBurstStateCounter, 1.0f, 0.0f, TodCurves::CURVE_LINEAR);
			else
				aMainTrackVolume = TodAnimateCurveFloat(400, 0, mBurstStateCounter, 0.0f, 1.0f, TodCurves::CURVE_LINEAR);
			if (mBurstStateCounter == 0 && mMusicDrumsState == MusicDrumsState::MUSIC_DRUMS_OFF)
				mMusicBurstState = MusicBurstState::MUSIC_BURST_OFF;
			break;
	}

	switch (mMusicDrumsState)
	{
		case MusicDrumsState::MUSIC_DRUMS_ON_QUEUED:
            // For OGG, we simply transition to ON immediately or after a short delay
            // Removing Order logic
            mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_ON;
            aDrumsVolume = 1.0f;
			break;
		case MusicDrumsState::MUSIC_DRUMS_ON:
			aDrumsVolume = 1.0f;
			break;
		case MusicDrumsState::MUSIC_DRUMS_OFF_QUEUED:
			aDrumsVolume = 1.0f;
            // Immediate transition to fading for OGG
            mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_FADING;
            mDrumsStateCounter = 50;
			break;
		case MusicDrumsState::MUSIC_DRUMS_FADING:
			if (aBurstScheme == 2)
				aDrumsVolume = TodAnimateCurveFloat(800, 0, mDrumsStateCounter, 1.0f, 0.0f, TodCurves::CURVE_LINEAR);
			else
				aDrumsVolume = TodAnimateCurveFloat(50, 0, mDrumsStateCounter, 1.0f, 0.0f, TodCurves::CURVE_LINEAR);
			if (mDrumsStateCounter == 0)
				mMusicDrumsState = MusicDrumsState::MUSIC_DRUMS_OFF;
			break;
		case MusicDrumsState::MUSIC_DRUMS_OFF:
			break;
	}

    if (aBurstScheme == 1) {
		if (mCurMusicFileHihats != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileHihats] != -1)
			mMusicInterface->SetSongVolume(mMusicMap[(int)mCurMusicFileHihats], aFadeTrackVolume);
		if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileDrums] != -1)
			mMusicInterface->SetSongVolume(mMusicMap[(int)mCurMusicFileDrums], aDrumsVolume);
	} else if (aBurstScheme == 2) {
		if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileMain] != -1)
			mMusicInterface->SetSongVolume(mMusicMap[(int)mCurMusicFileMain], aMainTrackVolume);
		if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileDrums] != -1)
			mMusicInterface->SetSongVolume(mMusicMap[(int)mCurMusicFileDrums], aDrumsVolume);
	}
}

//0x45B670
void Music::MusicUpdate()
{
	if (mFadeOutCounter > 0)
	{
		mFadeOutCounter--;
		if (mFadeOutCounter == 0)
			StopAllMusic();
		else
		{
			float aFadeLevel = TodAnimateCurveFloat(mFadeOutDuration, 0, mFadeOutCounter, 1.0f, 0.0f, TodCurves::CURVE_LINEAR);
			if (mCurMusicFileMain != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileMain] != -1)
				mMusicInterface->SetSongVolume(mMusicMap[(int)mCurMusicFileMain], aFadeLevel);
		}
	}

	if (mApp->mBoard == nullptr || !mApp->mBoard->mPaused)
	{
		UpdateMusicBurst();
		MusicResync();
	}
}

//0x45B750
// GOTY @Patoke: 0x45EFA0
void Music::MakeSureMusicIsPlaying(MusicTune theMusicTune)
{
	if (mCurMusicTune != theMusicTune)
	{
		StopAllMusic();
		PlayMusic(theMusicTune, -1, -1);
	}
}

//0x45B770
void Music::StartGameMusic()
{
	TOD_ASSERT(mApp->mBoard);

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN || mApp->mGameMode == GameMode::GAMEMODE_TREE_OF_WISDOM)
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_ZEN_GARDEN);
	else if (mApp->IsFinalBossLevel())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_FINAL_BOSS_BRAINIAC_MANIAC);
	else if (mApp->IsWallnutBowlingLevel() || mApp->IsWhackAZombieLevel() || mApp->IsLittleTroubleLevel() || mApp->IsBungeeBlitzLevel() ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_SPEED)
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_MINIGAME_LOONBOON);
	else if ((mApp->IsAdventureMode() && (mApp->mPlayerInfo->GetLevel() == 10 || mApp->mPlayerInfo->GetLevel() == 20 || mApp->mPlayerInfo->GetLevel() == 30)) ||
		mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_COLUMN)
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_CONVEYER);
	else if (mApp->IsStormyNightLevel())
		StopAllMusic();
	else if (mApp->IsScaryPotterLevel() || mApp->IsIZombieLevel())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_PUZZLE_CEREBRAWL);
	else if (mApp->mBoard->StageHasFog())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST);
	else if (mApp->mBoard->StageIsNight())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS);
	else if (mApp->mBoard->StageHas6Rows())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES);
	else if (mApp->mBoard->StageHasRoof())
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF);
	else
		MakeSureMusicIsPlaying(MusicTune::MUSIC_TUNE_DAY_GRASSWALK);
}

//0x45B930
void Music::GameMusicPause(bool thePause) {
    if (thePause) {
        if (!mPaused && mCurMusicTune != MusicTune::MUSIC_TUNE_NONE) {
            SDLMusicInterface* anSDL = (SDLMusicInterface*)mMusicInterface;
            int aMainMusicId = mMusicMap[(int)mCurMusicFileMain];
            
            if (aMainMusicId != -1) {
                auto anItr = anSDL->mMusicMap.find(aMainMusicId);
                // TOD_ASSERT(anItr != anSDL->mMusicMap.end()); // Soften assert
                if (anItr != anSDL->mMusicMap.end()) {
                    SDLMusicInfo* aMusicInfo = &anItr->second;
                    mPauseOffset = GetMusicOrder(mCurMusicFileMain);
                    mMusicInterface->StopMusic(aMainMusicId);
                }
            }

            if (mCurMusicTune == MusicTune::MUSIC_TUNE_DAY_GRASSWALK ||
                mCurMusicTune == MusicTune::MUSIC_TUNE_POOL_WATERYGRAVES ||
                mCurMusicTune == MusicTune::MUSIC_TUNE_FOG_RIGORMORMIST ||
                mCurMusicTune == MusicTune::MUSIC_TUNE_ROOF_GRAZETHEROOF) {
                
                if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileDrums] != -1)
                    mMusicInterface->StopMusic(mMusicMap[(int)mCurMusicFileDrums]);
                
                if (mCurMusicFileHihats != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileHihats] != -1)
                    mMusicInterface->StopMusic(mMusicMap[(int)mCurMusicFileHihats]);
                    
            } else if (mCurMusicTune == MusicTune::MUSIC_TUNE_NIGHT_MOONGRAINS) {
                if (mCurMusicFileDrums != MusicFile::MUSIC_FILE_NONE && mMusicMap[(int)mCurMusicFileDrums] != -1) {
                    mPauseOffsetDrums = GetMusicOrder(mCurMusicFileDrums);
                    mMusicInterface->StopMusic(mMusicMap[(int)mCurMusicFileDrums]);
                }
            }
        }
        mPaused = true;
    } else if (mPaused) {
        if (mCurMusicTune != MusicTune::MUSIC_TUNE_NONE) PlayMusic(mCurMusicTune, mPauseOffset, mPauseOffsetDrums);
        mPaused = false;
    }
}

int Music::GetNumLoadingTasks()
{
	//return 800 * 3;  // 内测版
	return 3500 * 13;  // 原版
}
