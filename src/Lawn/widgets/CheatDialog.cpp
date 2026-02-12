#include "CheatDialog.h"
#include "../Board.h"
#include "LawnApp.h"
#include "Lawn/LawnCommon.h"
#include "ChallengeScreen.h"
#include "../../Resources.h"
#include "GameConstants.h"
#include "Lawn/system/PlayerInfo.h"
#include "SexyAppFramework/widget/WidgetManager.h"

CheatDialog::CheatDialog(LawnApp* theApp) : LawnDialog(theApp, Dialogs::DIALOG_CHEAT, true, __S("CHEAT"), __S("Enter Your Cheat Code:"), __S(""), Dialog::BUTTONS_OK_CANCEL)
{
	mApp = theApp;
	mVerticalCenterText = false;
	mRestartRequired = true;
    mCheatEditWidget = CreateEditWidget(0, this, this);
    mCheatEditWidget->mMaxChars = 20;
    mCheatEditWidget->AddWidthCheckFont(FONT_BRIANNETOD12, 220);

	SexyString aCheatStr;
	if (mApp->mGameMode != GameMode::GAMEMODE_ADVENTURE)
	{
		aCheatStr = StrFormat(__S("C%d"), (int)mApp->mGameMode);
	}
	else if (mApp->mPlayerInfo && mApp->HasFinishedAdventure())
	{
		aCheatStr = StrFormat(__S("F%s"), mApp->GetStageString(mApp->mPlayerInfo->GetLevel()).c_str());
	}
	else if (mApp->mPlayerInfo)
	{
		aCheatStr = mApp->GetStageString(mApp->mPlayerInfo->GetLevel());
	}
	else
	{
		aCheatStr = __S("1-1");
	}
    mCheatEditWidget->SetText(aCheatStr, true);

	CalcSize(110, 40);
}

CheatDialog::~CheatDialog()
{
	delete mCheatEditWidget;
}

int CheatDialog::GetPreferredHeight(int theWidth)
{
	return LawnDialog::GetPreferredHeight(theWidth);
}

void CheatDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
	LawnDialog::Resize(theX, theY, theWidth, theHeight);
    mCheatEditWidget->Resize(mContentInsets.mLeft + 12, mHeight - 155, mWidth - mContentInsets.mLeft - mContentInsets.mRight - 24, 28);
}

void CheatDialog::AddedToManager(WidgetManager* theWidgetManager)
{
	LawnDialog::AddedToManager(theWidgetManager);
	AddWidget(mCheatEditWidget);
	theWidgetManager->SetFocus(mCheatEditWidget);
}

void CheatDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
	LawnDialog::RemovedFromManager(theWidgetManager);
	RemoveWidget(mCheatEditWidget);
}

void CheatDialog::Draw(Graphics* g)
{
	LawnDialog::Draw(g);
	DrawEditBox(g, mCheatEditWidget);
}

void CheatDialog::EditWidgetText(int theId, const SexyString& theString)
{
	(void)theId;(void)theString;
	mApp->ButtonDepress(mId + 2000);
}

bool CheatDialog::AllowChar(int theId, SexyChar theChar)
{
	(void)theId;
	// 允许数字、连字符和所有字母（支持彩蛋代码）
	return sexyisdigit(theChar) || theChar == __S('-') || (theChar >= __S('a') && theChar <= __S('z')) || (theChar >= __S('A') && theChar <= __S('Z'));
}

static ZombieType ZombieTypeFromString(const char* theString)
{
    if (strcmp(theString, "normal") == 0) return ZombieType::ZOMBIE_NORMAL;
    if (strcmp(theString, "flag") == 0) return ZombieType::ZOMBIE_FLAG;
    if (strcmp(theString, "cone") == 0) return ZombieType::ZOMBIE_TRAFFIC_CONE;
    if (strcmp(theString, "pole") == 0) return ZombieType::ZOMBIE_POLEVAULTER;
    if (strcmp(theString, "pail") == 0) return ZombieType::ZOMBIE_PAIL;
    if (strcmp(theString, "newspaper") == 0) return ZombieType::ZOMBIE_NEWSPAPER;
    if (strcmp(theString, "door") == 0) return ZombieType::ZOMBIE_DOOR;
    if (strcmp(theString, "football") == 0) return ZombieType::ZOMBIE_FOOTBALL;
    if (strcmp(theString, "dancer") == 0) return ZombieType::ZOMBIE_DANCER;
    if (strcmp(theString, "backup") == 0) return ZombieType::ZOMBIE_BACKUP_DANCER;
    if (strcmp(theString, "ducky") == 0) return ZombieType::ZOMBIE_DUCKY_TUBE;
    if (strcmp(theString, "snorkel") == 0) return ZombieType::ZOMBIE_SNORKEL;
    if (strcmp(theString, "zamboni") == 0) return ZombieType::ZOMBIE_ZAMBONI;
    if (strcmp(theString, "bobsled") == 0) return ZombieType::ZOMBIE_BOBSLED;
    if (strcmp(theString, "dolphin") == 0) return ZombieType::ZOMBIE_DOLPHIN_RIDER;
    if (strcmp(theString, "jack") == 0) return ZombieType::ZOMBIE_JACK_IN_THE_BOX;
    if (strcmp(theString, "balloon") == 0) return ZombieType::ZOMBIE_BALLOON;
    if (strcmp(theString, "digger") == 0) return ZombieType::ZOMBIE_DIGGER;
    if (strcmp(theString, "pogo") == 0) return ZombieType::ZOMBIE_POGO;
    if (strcmp(theString, "yeti") == 0) return ZombieType::ZOMBIE_YETI;
    if (strcmp(theString, "bungee") == 0) return ZombieType::ZOMBIE_BUNGEE;
    if (strcmp(theString, "ladder") == 0) return ZombieType::ZOMBIE_LADDER;
    if (strcmp(theString, "catapult") == 0) return ZombieType::ZOMBIE_CATAPULT;
    if (strcmp(theString, "gargantuar") == 0) return ZombieType::ZOMBIE_GARGANTUAR;
    if (strcmp(theString, "imp") == 0) return ZombieType::ZOMBIE_IMP;
    if (strcmp(theString, "boss") == 0) return ZombieType::ZOMBIE_BOSS;
    if (strcmp(theString, "pea") == 0) return ZombieType::ZOMBIE_PEA_HEAD;
    if (strcmp(theString, "wallnut") == 0) return ZombieType::ZOMBIE_WALLNUT_HEAD;
    if (strcmp(theString, "jalapeno") == 0) return ZombieType::ZOMBIE_JALAPENO_HEAD;
    if (strcmp(theString, "gatling") == 0) return ZombieType::ZOMBIE_GATLING_HEAD;
    if (strcmp(theString, "squash") == 0) return ZombieType::ZOMBIE_SQUASH_HEAD;
    if (strcmp(theString, "tallnut") == 0) return ZombieType::ZOMBIE_TALLNUT_HEAD;
    if (strcmp(theString, "redeye") == 0) return ZombieType::ZOMBIE_REDEYE_GARGANTUAR;
    return ZombieType::ZOMBIE_INVALID;
}

bool CheatDialog::ApplyCheat()
{
	int aChallengeIndex;
    std::string cheatInput = mCheatEditWidget->mString;
    std::transform(cheatInput.begin(), cheatInput.end(), cheatInput.begin(), ::tolower);
	
	// 重置标志
	mRestartRequired = true;
	
	// 检查是否是彩蛋代码（不需要重新进入关卡）
    if (mApp->mBoard && mApp->mBoard->DoTypingCheck(cheatInput))
    {
        mRestartRequired = false;
        return true;
    }

    int aSunAmount;
    if (sscanf(cheatInput.c_str(), "add sun %d", &aSunAmount) == 1)
    {
        if (mApp->mBoard)
        {
            mApp->mBoard->AddSunMoney(aSunAmount);
            mRestartRequired = false;
            return true;
        }
    }

    int aRow, aCol;
    char aType[32];
    if (sscanf(cheatInput.c_str(), "add %d %d %31s", &aRow, &aCol, aType) == 3)
    {
        ZombieType aZombieType = ZombieTypeFromString(aType);
        if (mApp->mBoard && aZombieType != ZombieType::ZOMBIE_INVALID)
        {
            if (aRow >= 0 && aRow < MAX_GRID_SIZE_Y && aCol >= 0 && aCol < MAX_GRID_SIZE_X)
            {
                Zombie* aZombie = mApp->mBoard->AddZombieInRow(aZombieType, aRow, 0);
                if (aZombie)
                {
                    if (aZombieType == ZombieType::ZOMBIE_BUNGEE)
                    {
                        aZombie->PickBungeeZombieTarget(aCol, aRow);
                    }
                    else
                    {
                        aZombie->mPosX = mApp->mBoard->GridToPixelX(aCol, aRow);
                    }
                }
            }
            mRestartRequired = false;
            return true;
        }
    }
	if (sexysscanf(cheatInput.c_str(), __S("c%d"), &aChallengeIndex) == 1 ||
		sexysscanf(cheatInput.c_str(), __S("C%d"), &aChallengeIndex) == 1)
	{
		mApp->mGameMode = (GameMode)std::clamp(aChallengeIndex, 0, NUM_CHALLENGE_MODES);
		return true;
	}

	int aLevel = -1;
	int aFinishedAdventure = 0;
	int aArea, aSubArea;
	if (sexysscanf(cheatInput.c_str(), __S("f%d-%d"), &aArea, &aSubArea) == 2 ||
		sexysscanf(cheatInput.c_str(), __S("F%d-%d"), &aArea, &aSubArea) == 2)
	{
		aLevel = (aArea - 1) * LEVELS_PER_AREA + aSubArea;
		aFinishedAdventure = 1;
	}
	else if (sexysscanf(cheatInput.c_str(), __S("f%d"), &aLevel) == 1 || sexysscanf(cheatInput.c_str(), __S("F%d"), &aLevel) == 1)
	{
		aFinishedAdventure = 1;
	}
	else if (sexysscanf(cheatInput.c_str(), __S("%d-%d"), &aArea, &aSubArea) == 2)
	{
		aLevel = (aArea - 1) * LEVELS_PER_AREA + aSubArea;
	}
	else
	{
		sexysscanf(cheatInput.c_str(), __S("%d"), &aLevel);
	}

	if (aLevel <= 0)
	{
		mApp->DoDialog(
			Dialogs::DIALOG_CHEATERROR, 
			true, 
			__S("Enter Level"), 
			__S("Invalid Level. Do 'number' or 'area-subarea' or 'Cnumber' or 'Farea-subarea'."), 
			__S("OK"), 
			Dialog::BUTTONS_FOOTER
		);
		return false;
	}

	mApp->mGameMode = GameMode::GAMEMODE_ADVENTURE;
	if (mApp->mPlayerInfo)
	{
		mApp->mPlayerInfo->SetLevel(aLevel);
		//mApp->mPlayerInfo->mFinishedAdventure = aFinishedAdventure;
		mApp->WriteCurrentUserConfig();
	}
	return true;
}
