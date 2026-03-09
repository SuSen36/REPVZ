#include <SDL.h>
#include "SexyAppFramework/SexyAppBase.h"
#include "SexyAppFramework/widget/WidgetManager.h"
#include "SexyAppFramework/graphics/MemoryImage.h"
#include "SexyAppFramework/paklib/PakInterface.h"
#include "Sexy.TodLib/TodDebug.h"

using namespace Sexy;

SDL_Renderer* Sexy::gRenderer = nullptr;

extern SDL_Surface* LoadWindowIcon(const char* iconPath);

void SexyAppBase::MakeWindow()
{
	if (mWindow)
	{
		// Avoid fullscreen desktop scaling; toggle true fullscreen only when requested
		SDL_SetWindowFullscreen((SDL_Window*)mWindow, (!mIsWindowed ? SDL_WINDOW_FULLSCREEN : 0));
	}
	else
	{
		SDL_Init(SDL_INIT_VIDEO);

		// 线性缩放，避免拉伸锯齿
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

        // Android specific hints
        SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
        SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");

		Uint32 windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#ifdef ANDROID
        windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
		if (!mIsWindowed)
			windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP; // 桌面全屏，按比例拉伸
#endif

		mWindow = (void*)SDL_CreateWindow(
			SexyStringToStringFast(mTitle).c_str(),
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			mWidth, mHeight,
			windowFlags
		);

#ifndef ANDROID
		// Load icon image
		SDL_Surface* iconSurface = LoadWindowIcon("properties/icon.bmp");
		if (iconSurface) {
			SDL_SetWindowIcon(static_cast<SDL_Window *>(mWindow), iconSurface);
			SDL_FreeSurface(iconSurface);
		} else {
			TodLog("Icon loading failed: ", SDL_GetError());
		}
#endif

        // 使用 SDL_Renderer 进行软件渲染后的缩放展示
        gRenderer = SDL_CreateRenderer((SDL_Window*)mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
        if (!gRenderer) {
            gRenderer = SDL_CreateRenderer((SDL_Window*)mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
        }
        if (!gRenderer) {
            gRenderer = SDL_CreateRenderer((SDL_Window*)mWindow, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE);
        }
        if (!gRenderer) {
            gRenderer = SDL_CreateRenderer((SDL_Window*)mWindow, -1, 0);
        }
        mContext = (void*)gRenderer;
        SDL_RenderSetLogicalSize(gRenderer, mWidth, mHeight);
	}

	bool isActive = mActive;
	mActive = !!(SDL_GetWindowFlags((SDL_Window*)mWindow) & SDL_WINDOW_INPUT_FOCUS);

	mPhysMinimized = false;
	if (mMinimized)
	{
		if (mMuteOnLostFocus)
			Unmute(true);

		mMinimized = false;
		isActive = mActive; // set this here so we don't call RehupFocus again.
		RehupFocus();
#ifndef ANDROID
        EnforceCursor();
#endif
	}

	if (isActive != mActive)
		RehupFocus();

	ReInitImages();

    if (mWidgetManager->mImage == NULL)
    {
        MemoryImage* anImage = new MemoryImage(this);
        anImage->Create(mWidth, mHeight);
        mWidgetManager->mImage = anImage;
    }
	mWidgetManager->MarkAllDirty();
}
