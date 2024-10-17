#include <SDL.h>
#include <iostream>  // 用于 std::cerr
#include "SexyAppFramework/SexyAppBase.h"
#include "../graphics/GLInterface.h"
#include "SexyAppFramework/graphics/GLImage.h"
#include "SexyAppFramework/widget/WidgetManager.h"

using namespace Sexy;

void SexyAppBase::MakeWindow()
{
    if (mWindow)
    {
        // 如果窗口已经存在，就切换全屏
        SDL_SetWindowFullscreen((SDL_Window*)mWindow, (!mIsWindowed ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    }
    else
    {
        // 初始化SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return;
        }

        // 设置OpenGL ES上下文属性
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

        // 创建窗口
        mWindow = (void*)SDL_CreateWindow(
                SexyStringToStringFast(mTitle).c_str(),
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                mWidth, mHeight,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | (!mIsWindowed ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
        );

        if (!mWindow)
        {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            SDL_Quit();  // 确保资源被释放
            return;
        }

        // 创建OpenGL上下文
        mContext = (void*)SDL_GL_CreateContext((SDL_Window*)mWindow);
        if (!mContext)
        {
            std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow((SDL_Window*)mWindow);  // 销毁窗口
            SDL_Quit();
            return;
        }

        SDL_GL_SetSwapInterval(1);  // 启用垂直同步
    }

    // 创建OpenGL接口
    if (mGLInterface == NULL)
    {
        mGLInterface = new GLInterface(this);
        InitGLInterface();
    }

    // 激活窗口
    bool isActive = mActive;
    mActive = !!(SDL_GetWindowFlags((SDL_Window*)mWindow) & SDL_WINDOW_INPUT_FOCUS);

    mPhysMinimized = false;

    if (isActive != mActive)
       RehupFocus();

    ReInitImages();

    mWidgetManager->mImage = mGLInterface->GetScreenImage();
    mWidgetManager->MarkAllDirty();
}
