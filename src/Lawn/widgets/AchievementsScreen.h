#ifndef __ACHIEVEMENTSSCREEN_H__
#define __ACHIEVEMENTSSCREEN_H__

#include "../../ConstEnums.h"
#include "SexyAppFramework/widget/Widget.h"

class LawnApp;

using namespace Sexy;

enum AchievementId {
	HomeSecurity, //
	NovelPeasPrize, //
	BetterOffDead, //
	ChinaShop, //
	Spudow, //
	Explodonator, //
	Morticulturalist, //
	DontPea, //
	RollSomeHeads, //
	Grounded, //
	Zombologist, //
	PennyPincher, //
	SunnyDays, //
	PopcornParty, //
	GoodMorning, //
	NoFungusAmongUs, //
	BeyondTheGrave, //
	Immortal, //
	ToweringWisdom, //
	MustacheMode, //
    MAX_ACHIEVEMENTS
};

class AchievementItem {
public:
    std::string name;
    std::string description;
};

extern AchievementItem gAchievementList[MAX_ACHIEVEMENTS];

class AchievementsWidget : public Widget {
public:
	LawnApp*	mApp;
	int			mScrollDirection;
	Rect		mMoreRockRect;
	int			mScrollValue;
	int			mScrollDecay;
	int			mDefaultScrollValue;
	bool		mDidPressMoreButton;

	AchievementsWidget(LawnApp* theApp);
	virtual ~AchievementsWidget();

	virtual void                Update();
	virtual void                Draw(Graphics* g);
	virtual void                KeyDown(KeyCode theKey);
	virtual void                MouseDown(int x, int y, int theClickCount);
	virtual void                MouseUp(int x, int y, int theClickCount);
	virtual void				MouseWheel(int theDelta);
};

class ReportAchievement {
public:
	static void GiveAchievement(LawnApp* theApp, int theAchievement, bool theForceGive);
	static void AchievementInitForPlayer(LawnApp* theApp);
};

#endif