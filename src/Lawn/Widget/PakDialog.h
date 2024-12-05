#ifndef __PAKDIALOG_H__
#define __PAKDIALOG_H__

#include "LawnDialog.h"
#include "SexyAppFramework/widget/ListListener.h"
#include "SexyAppFramework/widget/EditListener.h"

namespace Sexy
{
	class ListWidget;
};

class PakDialog : public LawnDialog, public ListListener, public EditListener
{
protected:
	enum
	{
		PakDialog_MoveUpPak,
		PakDialog_MoveDownPak
	};

public:
	ListWidget*			mPakList;				//+0x174
	DialogButton*		mMoveUpButton;			//+0x178
	DialogButton*		mMoveDownButton;			//+0x17C
	int					mNumPaks;				//+0x180

public:
    PakDialog(LawnApp* theApp);
	virtual ~PakDialog();

	virtual void		Resize(int theX, int theY, int theWidth, int theHeight);
	virtual int			GetPreferredHeight(int theWidth);
	virtual void		AddedToManager(WidgetManager* theWidgetManager);
	virtual void		RemovedFromManager(WidgetManager* theWidgetManager);
	virtual void 		ListClicked(int theId, int theIdx, int theClickCount);
	virtual void 		ListClosed(int){}
	virtual void 		ListHiliteChanged(int, int, int){}
	virtual void		ButtonDepress(int theId);
	virtual void		EditWidgetText(int theId, const SexyString& theString);

	virtual bool		AllowChar(int theId, SexyChar theChar);
	virtual void		Draw(Graphics* g);
    void                MovePakUp();
    void                MovePakDown();
    void                SavePakList();
	SexyString			GetSelName();


};

#endif
