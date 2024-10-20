#ifndef __CURSORWIDGET_H__
#define __CURSORWIDGET_H__

#include "SexyAppFramework/widget/Widget.h"
#include "SexyAppFramework/misc/Point.h"

namespace Sexy
{

class Image;

class CursorWidget : public Widget
{
public:
	Image*					mImage;

public:
	CursorWidget();

	virtual void			Draw(Graphics* g);
	void					SetImage(Image* theImage);
	Point					GetHotspot();
	
};

}

#endif //__CURSORWIDGET_H__