#include <SDL.h>
#include "SexyAppFramework/SexyAppBase.h"
#include "graphics/GLInterface.h"
#include "SexyAppFramework/graphics/GLImage.h"
#include "SexyAppFramework/widget/WidgetManager.h"

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

            case SDL_MOUSEMOTION:
            {
                if (!mMouseIn)
                    mMouseIn = true;

                int x = event.motion.x;
                int y = event.motion.y;
                mWidgetManager->RemapMouse(x, y);

                mLastUserInputTick = mLastTimerTime;

                mWidgetManager->MouseMove(x, y);
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            {
                if (!mMouseIn)
                    mMouseIn = true;

                int x = event.button.x;
                int y = event.button.y;
                mWidgetManager->RemapMouse(x, y);

                mLastUserInputTick = mLastTimerTime;

                mWidgetManager->MouseMove(x, y);
                int btn =
                        (event.button.button == SDL_BUTTON_LEFT) ? 1 :
                        (event.button.button == SDL_BUTTON_RIGHT) ? -1 :
                        3;
                if (event.button.clicks == 2)
                    btn = (event.button.button == SDL_BUTTON_LEFT) ? 2 : -2;

                mWidgetManager->MouseDown(x, y, btn);
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if (!mMouseIn)
                    mMouseIn = true;

                int x = event.button.x;
                int y = event.button.y;
                mWidgetManager->RemapMouse(x, y);

                mLastUserInputTick = mLastTimerTime;

                mWidgetManager->MouseMove(x, y);
                int btn =
                        (event.button.button == SDL_BUTTON_LEFT) ? 1 :
                        (event.button.button == SDL_BUTTON_RIGHT) ? -1 :
                        3;

                mWidgetManager->MouseUp(x, y, btn);
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


