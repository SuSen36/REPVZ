#include <SDL.h>
#include <SDL_opengl.h>
#include "SexyAppFramework/SexyAppBase.h"
#include "../graphics/GLInterface.h"
#include "SexyAppFramework/graphics/GLImage.h"
#include "SexyAppFramework/widget/WidgetManager.h"

using namespace Sexy;

void SexyAppBase::MakeWindow()
{
    if (mWindow)
    {
        SDL_SetWindowFullscreen((SDL_Window*)mWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else
    {
        // 初始化 SDL 视频子系统
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            return;
        }
        SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft");
        SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
        // 设置 OpenGL ES 1.0 属性
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

        // 创建窗口
        mWindow = (void*)SDL_CreateWindow(
            SexyStringToStringFast(mTitle).c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            mWidth, mHeight,
            SDL_WINDOW_OPENGL  | SDL_WINDOW_FULLSCREEN_DESKTOP
        );

        if (!mWindow)
        {
            return;
        }

        // 创建 OpenGL 上下文
        mContext = (void*)SDL_GL_CreateContext((SDL_Window*)mWindow);
        if (!mContext)
        {
            return;
        }

        SDL_GL_SetSwapInterval(1); // 启用垂直同步
    }

    if (mGLInterface == NULL)
    {
        mGLInterface = new GLInterface(this);
        InitGLInterface();
    }

    bool isActive = mActive;
    mActive = !!(SDL_GetWindowFlags((SDL_Window*)mWindow) & SDL_WINDOW_INPUT_FOCUS);

    mPhysMinimized = false;
    if (mMinimized)
    {
        if (mMuteOnLostFocus)
            Unmute(true);

        mMinimized = false;
        isActive = mActive;
        RehupFocus();
    }

    if (isActive != mActive)
        RehupFocus();

    ReInitImages();

    mWidgetManager->mImage = mGLInterface->GetScreenImage();
    mWidgetManager->MarkAllDirty();
}
