#include <SDL.h>
#include "../SexyAppBase.h"
#include "../graphics/GLInterface.h"
#include "../graphics/GLImage.h"
#include "widget/WidgetManager.h"

using namespace Sexy;

void SexyAppBase::MakeWindow()
{
    if (mWindow)
    {
        // 在已有窗口的情况下，设置全屏状态
        SDL_SetWindowFullscreen((SDL_Window*)mWindow, (!mIsWindowed ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    }
    else
    {
        // 初始化SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            return; // SDL初始化失败，返回
        }

        // 设置OpenGL版本
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

        // 创建SDL窗口
        mWindow = (void*)SDL_CreateWindow(
            SexyStringToStringFast(mTitle).c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            mWidth, mHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | (!mIsWindowed ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
        );

        // 创建OpenGL上下文
        mContext = (void*)SDL_GL_CreateContext((SDL_Window*)mWindow);
        SDL_GL_SetSwapInterval(1); // 设定交换间隔

        mGLInterface = new GLInterface(this); // 创建GLInterface实例
        InitGLInterface(); // 初始化OpenGL接口

        // Android特有的窗口面积管理
        mPhysMinimized = false;
        mMinimized = false; // 确保窗口没有被最小化
    }

    // 检查窗口的活动状态
    bool isActive = mActive;
    mActive = !!(SDL_GetWindowFlags((SDL_Window*)mWindow) & SDL_WINDOW_INPUT_FOCUS);

    if (isActive != mActive)
        RehupFocus(); // 调整焦点状态

    ReInitImages(); // 重新初始化图像

    // 更新小部件管理器的图像
    if (mWidgetManager)
    {
        mWidgetManager->mImage = mGLInterface->GetScreenImage();
        mWidgetManager->MarkAllDirty();
    }
}
