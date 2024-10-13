#include <iostream>
#include <SDL.h>
#include "SexyAppFramework/SexyAppBase.h"
#include "../graphics/GLInterface.h"
#include "SexyAppFramework/graphics/GLImage.h"
#include "SexyAppFramework/widget/WidgetManager.h"

using namespace Sexy;

void SexyAppBase::MakeWindow()
{
    if (mWindow)
    {
        // 切换全屏模式
        SDL_SetWindowFullscreen((SDL_Window*)mWindow, (!mIsWindowed ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    }
    else
    {
        // 初始化SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return; // SDL 初始化失败
        }

        // 设置OpenGL版本
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES); // 使用 OpenGL ES

        // 创建SDL窗口
        mWindow = (void*)SDL_CreateWindow(
                SexyStringToStringFast(mTitle).c_str(),
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                mWidth, mHeight,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | (!mIsWindowed ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
        );

        if (!mWindow) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return; // 窗口创建失败
        }

        // 创建OpenGL上下文
        mContext = (void*)SDL_GL_CreateContext((SDL_Window*)mWindow);
        if (!mContext) {
            std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return; // 上下文创建失败
        }

        SDL_GL_SetSwapInterval(1); // 开启垂直同步

        mGLInterface = new GLInterface(this); // 创建GLInterface实例
        InitGLInterface(); // 初始化OpenGL接口

        mPhysMinimized = false;
        mMinimized = false; // 确保最小化状态为假
    }

    // 处理窗口激活状态
    bool isActive = mActive;
    mActive = !!(SDL_GetWindowFlags((SDL_Window*)mWindow) & SDL_WINDOW_INPUT_FOCUS);

    if (isActive != mActive)
        RehupFocus(); // 更新焦点状态

    ReInitImages(); // 重新初始化图像资源

    // 更新小部件管理器的图像
    if (mWidgetManager)
    {
        mWidgetManager->mImage = mGLInterface->GetScreenImage();
        mWidgetManager->MarkAllDirty();
    }
}


