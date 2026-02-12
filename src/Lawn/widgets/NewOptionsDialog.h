#ifndef __NEWOPTIONSDIALOG_H__
#define __NEWOPTIONSDIALOG_H__

#include "SexyAppFramework/widget/Dialog.h"
#include "SexyAppFramework/widget/SliderListener.h"

class LawnApp;
class LawnStoneButton;
class NewLawnButton;
namespace Sexy
{
	class Slider;
};

class NewOptionsDialog : public Sexy::Dialog, public Sexy::SliderListener
{
protected:
	enum
	{
		NewOptionsDialog_MusicVolume,
		NewOptionsDialog_SoundVolume,
		NewOptionsDialog_Almanac,
		NewOptionsDialog_MainMenu,
		NewOptionsDialog_Restart,
		NewOptionsDialog_Update,
		NewOptionsDialog_CheatCode,
	};

public:
	LawnApp*			    mApp;
	Sexy::Slider*			mMusicVolumeSlider;
	Sexy::Slider*			mSfxVolumeSlider;
    LawnStoneButton*		mCheatButton;
	LawnStoneButton*		mAlmanacButton;
	LawnStoneButton*		mBackToMainButton;
	LawnStoneButton*		mRestartButton;
	NewLawnButton*			mBackToGameButton;
	bool					mFromGameSelector;

public:
	NewOptionsDialog(LawnApp* theApp, bool theFromGameSelector);
	~NewOptionsDialog();

	int						GetPreferredHeight(int theWidth);
	void					AddedToManager(Sexy::WidgetManager* theWidgetManager);
	void					RemovedFromManager(Sexy::WidgetManager* theWidgetManager);
	void					Resize(int theX, int theY, int theWidth, int theHeight);
	void					Draw(Sexy::Graphics* g);
	void					SliderVal(int theId, double theVal);
	void					ButtonPress(int theId);
	void					ButtonDepress(int theId);
	void					KeyDown(Sexy::KeyCode theKey);
};

#endif
