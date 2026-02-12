#ifndef __GAMESELECTOR_H__
#define __GAMESELECTOR_H__

#include "../../ConstEnums.h"
#include "SexyAppFramework/widget/Widget.h"
#include "SexyAppFramework/widget/ButtonListener.h"
#include "AchievementsScreen.h"
#include "GameButton.h"

class LawnApp;
class ToolTipWidget;
namespace Sexy
{
    class DialogButton;
}

using namespace Sexy;

enum SelectorAnimState
{
    SELECTOR_OPEN,
    SELECTOR_NEW_USER,
    SELECTOR_SHOW_SIGN,
    SELECTOR_IDLE
};

class GameSelector : public Widget, public ButtonListener
{
private:
    enum
    {
        GameSelector_Adventure = 100,
        GameSelector_Minigame = 101,
        GameSelector_Puzzle = 102,
        GameSelector_Options = 103,
        GameSelector_Help = 104,
        GameSelector_Quit = 105,
        GameSelector_ChangeUser = 106,
        GameSelector_Store = 107,
        GameSelector_Almanac = 108,
        GameSelector_ZenGarden = 109,
        GameSelector_Survival = 110,
        GameSelector_Zombatar = 111,
        GameSelector_AchievementsBack = 112,
        GameSelector_Achievements = 113,
        GameSelector_QuickPlay = 114
    };

public:
    LawnApp*                    mApp;
    NewLawnButton*              mAdventureButton;
    NewLawnButton*              mMinigameButton;
    NewLawnButton*              mPuzzleButton;
    NewLawnButton*              mOptionsButton;
    NewLawnButton*              mQuitButton;
    NewLawnButton*              mHelpButton;
    NewLawnButton*              mStoreButton;
    NewLawnButton*              mAlmanacButton;
    NewLawnButton*              mZenGardenButton;
    NewLawnButton*              mSurvivalButton;
    NewLawnButton*              mChangeUserButton;
    NewLawnButton*              mZombatarButton;
    NewLawnButton*              mAchievementsButton;
    NewLawnButton*              mQuickPlayButton;
    Widget*                     mOverlayWidget;
    bool                        mStartingGame;
    int                         mStartingGameCounter;
    bool                        mMinigamesLocked;
    bool                        mPuzzleLocked;
    bool                        mSurvivalLocked;
    bool                        mShowStartButton;
    ParticleSystemID            mTrophyParticleID;
    ReanimationID               mSelectorReanimID;
    ReanimationID               mCloudReanimID[6];
    int                         mCloudCounter[6];
    ReanimationID               mFlowerReanimID[3];
    ReanimationID               mLeafReanimID;
    ReanimationID               mHandReanimID;
    int                         mLeafCounter;
    SelectorAnimState           mSelectorState;
    int                         mLevel;
    bool                        mLoading;
    ToolTipWidget*              mToolTip;
    bool                        mHasTrophy;
    bool                        mUnlockSelectorCheat;
    int                         mSlideCounter;
    int                         mStartX;
    int                         mStartY;
    int                         mDestX;
    int                         mDestY;
    AchievementsWidget*       mAchievementsWidget;

public:
    GameSelector(LawnApp* theApp, bool skipAnimation = false);
    virtual ~GameSelector();

    void                        SyncProfile(bool theShowLoading);
    virtual void                Draw(Graphics* g);
    virtual void                DrawOverlay(Graphics* g);
    virtual void                Update();
    virtual void                AddedToManager(WidgetManager* theWidgetManager);
    virtual void                RemovedFromManager(WidgetManager* theWidgetManager);
    virtual void                OrderInManagerChanged();
    virtual void                ButtonMouseEnter(int theId);
    virtual void                ButtonPress(int theId);
    virtual void                ButtonDepress(int theId);
    virtual void                ButtonDownTick(int){}
    virtual void                ButtonMouseLeave(int){}
    virtual void                ButtonMouseMove(int, int, int){}
    virtual void                KeyDown(KeyCode theKey);
    virtual void                KeyChar(char theChar);
    virtual void                MouseDown(int x, int y, int theClickCount);
    void                        TrackButton(DialogButton* theButton, const char* theTrackName, float theOffsetX, float theOffsetY);
    void                        SyncButtons();
    void                        AddTrophySparkle();
    void                        ClickedAdventure();
    void                        UpdateTooltip();
    /*inline*/ bool             ShouldDoZenTuturialBeforeAdventure();
    void                        AddPreviewProfiles();
    /*inline*/ void             SlideTo(int theX, int theY);
    void                        ShowAchievementsScreen();
};

class GameSelectorOverlay : public Widget
{
public:
    GameSelector*               mParent;

public:
    GameSelectorOverlay(GameSelector* theGameSelector);
    virtual ~GameSelectorOverlay() { }

    virtual void Draw(Graphics* g);
};

#endif
