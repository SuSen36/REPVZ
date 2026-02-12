#ifndef __CHEATDIALOG_H__
#define __CHEATDIALOG_H__

#include "LawnDialog.h"
#include "SexyAppFramework/widget/EditListener.h"

class CheatDialog : public LawnDialog, public EditListener
{
public:
	LawnApp*			mApp;
	EditWidget*			mCheatEditWidget;
	bool				mRestartRequired;		// 标记是否需要重新开始游戏

public:
	CheatDialog(LawnApp* theApp);
	virtual ~CheatDialog();

	virtual int			GetPreferredHeight(int theWidth);
	virtual void		Resize(int theX, int theY, int theWidth, int theHeight);
	virtual void		AddedToManager(WidgetManager* theWidgetManager);
	virtual void		RemovedFromManager(WidgetManager* theWidgetManager);
	virtual void		Draw(Graphics* g);
	virtual void		EditWidgetText(int theId, const SexyString& theString);
	virtual bool		AllowChar(int theId, SexyChar theChar);
	bool				ApplyCheat();
};

#endif