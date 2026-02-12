#ifndef __PLAYERINFO_H__
#define __PLAYERINFO_H__

#define MAX_POTTED_PLANTS 200
#define PURCHASE_COUNT_OFFSET 1000

#include <ctime>
#include "../../ConstEnums.h"
#include "../../SexyAppFramework/Common.h"

class PottedPlant
{
public:
    enum FacingDirection
    {
        FACING_RIGHT,
        FACING_LEFT
    };

public:
    SeedType            mSeedType;
    GardenType          mWhichZenGarden;
    int                 mX;
    int                 mY;
    FacingDirection     mFacing;

    time_t              mLastWateredTime;
    DrawVariation       mDrawVariation;
    PottedPlantAge      mPlantAge;
    int                 mTimesFed;
    int                 mFeedingsPerGrow;
    PottedPlantNeed     mPlantNeed;

    time_t              mLastNeedFulfilledTime;
    time_t              mLastFertilizedTime;
    time_t              mLastChocolateTime;
    time_t              mFutureAttribute[1];

public:
    void                InitializePottedPlant(SeedType theSeedType);
};

class DataSync;
class PlayerInfo
{
public:
    SexyString          mName;
    uint32_t            mUseSeq;
    uint32_t            mId;
    int                 mLevel;
    int                 mCoins;
    int                 mFinishedAdventure;
    int                 mChallengeRecords[100];
    slong               mPurchases[80];
    int                 mPlayTimeActivePlayer;
    int                 mPlayTimeInactivePlayer;
    int                 mHasUsedCheatKeys;
    int                 mHasWokenStinky;
    int                 mDidntPurchasePacketUpgrade;
    slong               mLastStinkyChocolateTime;
    int                 mStinkyPosX;
    int                 mStinkyPosY;
    int                 mHasUnlockedMinigames;
    int                 mHasUnlockedPuzzleMode;
    int                 mHasNewMiniGame;
    int                 mHasNewScaryPotter;
    int                 mHasNewIZombie;
    int                 mHasNewSurvival;
    int                 mHasUnlockedSurvivalMode;
    int                 mNeedsMessageOnGameSelector;
    int                 mNeedsMagicTacoReward;
    int                 mHasSeenStinky;
    int                 mHasSeenUpsell;
    int                 mPlaceHolderPlayerStats;            //+0x??????
    int                 mNumPottedPlants;
    PottedPlant         mPottedPlant[MAX_POTTED_PLANTS];
    bool                mEarnedAchievements[20];
    bool                mShownAchievements[20];

public:
    PlayerInfo();

    void                Reset();
    /*inline*/ void     AddCoins(int theAmount);
    void                SyncSummary(DataSync& theSync);
    void                SyncDetails(DataSync& theSync);
    void                DeleteUserFiles();
    void                LoadDetails();
    void                SaveDetails();
    inline int          GetLevel() const { return mLevel; }
    inline void         SetLevel(int theLevel) { mLevel = theLevel; }
    /*inline*/ void     ResetChallengeRecord(GameMode theGameMode);
};

#endif
