#ifndef __MUSIC_H__
#define __MUSIC_H__

#include <string>
#include <SDL_mixer_ext/SDL_mixer_ext.h>

class LawnApp;
namespace Sexy
{
	class MusicInterface;
}

enum MusicTune
{
	MUSIC_TUNE_NONE = -1,
	MUSIC_TUNE_DAY_GRASSWALK = 1,				// 白天草地关卡
	MUSIC_TUNE_NIGHT_MOONGRAINS,				// 黑夜草地关卡
	MUSIC_TUNE_POOL_WATERYGRAVES,				// 白天泳池关卡
	MUSIC_TUNE_FOG_RIGORMORMIST,				// 黑夜泳池关卡
	MUSIC_TUNE_ROOF_GRAZETHEROOF,				// 屋顶关卡
	MUSIC_TUNE_CHOOSE_YOUR_SEEDS,				// 选卡界面/小游戏界面
	MUSIC_TUNE_TITLE_CRAZY_DAVE_MAIN_THEME,		// 主菜单
	MUSIC_TUNE_ZEN_GARDEN,						// 禅境花园
	MUSIC_TUNE_PUZZLE_CEREBRAWL,				// 解谜模式
	MUSIC_TUNE_MINIGAME_LOONBOON,				// 小游戏
	MUSIC_TUNE_CONVEYER,						// 传送带关卡
	MUSIC_TUNE_FINAL_BOSS_BRAINIAC_MANIAC,		// 僵王博士关卡
	MUSIC_TUNE_CREDITS_ZOMBIES_ON_YOUR_LAWN,	// MV
	NUM_MUSIC_TUNES
};

enum MusicFile
{
	MUSIC_FILE_NONE = -1,
	MUSIC_FILE_MAIN_MUSIC,
	MUSIC_FILE_DRUMS,
	MUSIC_FILE_HIHATS,
	MUSIC_FILE_CREDITS_ZOMBIES_ON_YOUR_LAWN,
	MUSIC_FILE_ZEN_GARDEN,
	MUSIC_FILE_ZEN_GARDEN_DRUMS,
	MUSIC_FILE_ZEN_GARDEN_HIHATS,
	MUSIC_FILE_ROOF,
	MUSIC_FILE_ROOF_DRUMS,
	MUSIC_FILE_ROOF_HIHATS,
	MUSIC_FILE_POOL,
	MUSIC_FILE_POOL_DRUMS,
	MUSIC_FILE_POOL_HIHATS,
	MUSIC_FILE_NIGHT,
	MUSIC_FILE_NIGHT_DRUMS,
	MUSIC_FILE_NIGHT_HIHATS,
	MUSIC_FILE_LOONBOON,
	MUSIC_FILE_LOONBOON_DRUMS,
	MUSIC_FILE_LOONBOON_HIHATS,
	MUSIC_FILE_FOG,
	MUSIC_FILE_FOG_DRUMS,
	MUSIC_FILE_FOG_HIHATS,
	MUSIC_FILE_DAY,
	MUSIC_FILE_DAY_DRUMS,
	MUSIC_FILE_DAY_HIHATS,
	MUSIC_FILE_CRAZY_DAVE,
	MUSIC_FILE_CRAZY_DAVE_DRUMS,
	MUSIC_FILE_CRAZY_DAVE_HIHATS,
	MUSIC_FILE_CONVEYOR,
	MUSIC_FILE_CONVEYOR_DRUMS,
	MUSIC_FILE_CONVEYOR_HIHATS,
	MUSIC_FILE_CHOOSE_YOUR_SEEDS,
	MUSIC_FILE_CHOOSE_YOUR_SEEDS_DRUMS,
	MUSIC_FILE_CHOOSE_YOUR_SEEDS_HIHATS,
	MUSIC_FILE_CEREBRAWL,
	MUSIC_FILE_CEREBRAWL_DRUMS,
	MUSIC_FILE_CEREBRAWL_HIHATS,
	MUSIC_FILE_BOSS,
	MUSIC_FILE_BOSS_DRUMS,
	MUSIC_FILE_BOSS_HIHATS,
	NUM_MUSIC_FILES
};

enum MusicBurstState
{
	MUSIC_BURST_OFF,
	MUSIC_BURST_STARTING,
	MUSIC_BURST_ON,
	MUSIC_BURST_FINISHING
};

enum MusicDrumsState
{
	MUSIC_DRUMS_OFF,
	MUSIC_DRUMS_ON_QUEUED,
	MUSIC_DRUMS_ON,
	MUSIC_DRUMS_OFF_QUEUED,
	MUSIC_DRUMS_FADING
};

class Music
{
public:
	LawnApp*					mApp;
	Sexy::MusicInterface*		mMusicInterface;
	MusicTune					mCurMusicTune;
	MusicFile					mCurMusicFileMain;
	MusicFile					mCurMusicFileDrums;
	MusicFile					mCurMusicFileHihats;
	int							mBurstOverride;
	float						mBaseBPM;
	float						mBaseModSpeed;
	MusicBurstState				mMusicBurstState;
	int							mBurstStateCounter;
	MusicDrumsState				mMusicDrumsState;
	double						mQueuedDrumTrackPackedOrder;
	int							mDrumsStateCounter;
	double						mPauseOffset;
	double						mPauseOffsetDrums;
	bool						mPaused;
	bool						mMusicDisabled;
	int							mFadeOutCounter;
	int							mFadeOutDuration;
    int                         mMusicMap[MusicFile::NUM_MUSIC_FILES];

public:
	Music();

	void						MusicInit();
	void						MusicDispose() { }
	void						MusicUpdate();
	void						StopAllMusic();
	/*inline*/ void				PlayMusic(MusicTune theMusicTune, double theOffset = -1.0, double theDrumsOffset = -1.0);
    /*inline*/ Mix_Music* 		GetMusicHandle(MusicFile theMusicFile);
    void						StartGameMusic();
	/*inline*/ void				LoadSong(MusicFile theMusicFile, const std::string& theFileName);
	void						MusicResync();
	void						UpdateMusicBurst();
	/*inline*/ void				StartBurst();
	void						GameMusicPause(bool thePause);
	void						PlayFromOffset(MusicFile theMusicFile, double theOffset, double theVolume);
	void						PlayMusicNoOffset(MusicFile theMusicFile, double theVolume);
	void						MusicResyncChannel(MusicFile theMusicFileToMatch, MusicFile theMusicFileToSync);
	bool						TodLoadMusic(MusicFile theMusicFile, const std::string& theFileName);
	void						MusicTitleScreenInit();
	/*inline*/ void				MakeSureMusicIsPlaying(MusicTune theMusicTune);
	/*inline*/ void				FadeOut(int theFadeOutDuration);
	void						SetupMusicFileForTune(MusicFile theMusicFile, MusicTune theMusicTune);
	double						GetMusicOrder(MusicFile theMusicFile);
	void						MusicCreditScreenInit();
	int							GetNumLoadingTasks();
};

#endif

