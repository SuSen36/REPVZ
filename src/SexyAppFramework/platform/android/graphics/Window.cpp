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
        // �����д��ڵ�����£�����ȫ��״̬
        SDL_SetWindowFullscreen((SDL_Window*)mWindow, (!mIsWindowed ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    }
    else
    {
        // ��ʼ��SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            return; // SDL��ʼ��ʧ�ܣ�����
        }

        // ����OpenGL�汾
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

        // ����SDL����
        mWindow = (void*)SDL_CreateWindow(
            SexyStringToStringFast(mTitle).c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            mWidth, mHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | (!mIsWindowed ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
        );

        // ����OpenGL������
        mContext = (void*)SDL_GL_CreateContext((SDL_Window*)mWindow);
        SDL_GL_SetSwapInterval(1); // �趨�������

        mGLInterface = new GLInterface(this); // ����GLInterfaceʵ��
        InitGLInterface(); // ��ʼ��OpenGL�ӿ�

        // Android���еĴ����������
        mPhysMinimized = false;
        mMinimized = false; // ȷ������û�б���С��
    }

    // ��鴰�ڵĻ״̬
    bool isActive = mActive;
    mActive = !!(SDL_GetWindowFlags((SDL_Window*)mWindow) & SDL_WINDOW_INPUT_FOCUS);

    if (isActive != mActive)
        RehupFocus(); // ��������״̬

    ReInitImages(); // ���³�ʼ��ͼ��

    // ����С������������ͼ��
    if (mWidgetManager)
    {
        mWidgetManager->mImage = mGLInterface->GetScreenImage();
        mWidgetManager->MarkAllDirty();
    }
}
