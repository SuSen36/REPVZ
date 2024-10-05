#include <SDL.h>
#include "../SexyAppBase.h"
#include "../graphics/GLInterface.h"
#include "../graphics/GLImage.h"
#include "widget/WidgetManager.h"

using namespace Sexy;

void SexyAppBase::InitInput()
{
    SDL_Init(SDL_INIT_EVENTS);
}

bool SexyAppBase::StartTextInput(std::string& theInput)
{
    SDL_StartTextInput();
    return false;
}

void SexyAppBase::StopTextInput()
{
    SDL_StopTextInput();
}

bool SexyAppBase::ProcessDeferredMessages(bool singleMessage)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                mShutdown = true;
                break;

            case SDL_WINDOWEVENT:
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                        mGLInterface->UpdateViewport();
                        mWidgetManager->Resize(mScreenBounds, mGLInterface->mPresentationRect);
                        break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        mActive = event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED;
                        RehupFocus();
                        break;
                }
                break;

            case SDL_FINGERDOWN:
            {
                int x = static_cast<int>(event.tfinger.x * mScreenBounds.w); // ʹ����Ļ���
                int y = static_cast<int>(event.tfinger.y * mScreenBounds.h); // ʹ����Ļ�߶�
                mWidgetManager->RemapMouse(x, y);

                mLastUserInputTick = mLastTimerTime;
                mMouseIn = true; // ����������ʱ������ mouseIn Ϊ true
                mWidgetManager->MouseDown(x, y, 1); // 1 Ϊ�������
                break;
            }

            case SDL_FINGERUP:
            {
                int x = static_cast<int>(event.tfinger.x * mScreenBounds.w);
                int y = static_cast<int>(event.tfinger.y * mScreenBounds.h;
                mWidgetManager->RemapMouse(x, y);

                mLastUserInputTick = mLastTimerTime;
                mWidgetManager->MouseUp(x, y, 1); // 1 Ϊ����ɿ�
                break;
            }

            case SDL_FINGERMOTION:
            {
                int x = static_cast<int>(event.tfinger.x * mScreenBounds.w);
                int y = static_cast<int>(event.tfinger.y * mScreenBounds.h);
                mWidgetManager->RemapMouse(x, y);

                mLastUserInputTick = mLastTimerTime;
                mWidgetManager->MouseMove(x, y);
                break;
            }

            case SDL_KEYDOWN:
                mLastUserInputTick = mLastTimerTime;
                mWidgetManager->KeyDown((KeyCode)event.key.keysym.sym);
                break;

            case SDL_KEYUP:
                mLastUserInputTick = mLastTimerTime;
                mWidgetManager->KeyUp((KeyCode)event.key.keysym.sym);
                break;

            case SDL_TEXTINPUT:
                mLastUserInputTick = mLastTimerTime;
                mWidgetManager->KeyChar((SexyChar)event.text.text[0]);
                break;

        }
    }

    return SDL_HasEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
}

