#ifndef __SOUNDINSTANCE_H__
#define __SOUNDINSTANCE_H__

#include "SexyAppFramework/Common.h"

namespace Sexy {
class SoundInstance {
public:
    SoundInstance() {}

    virtual ~SoundInstance() {}

    virtual void Release() = 0;

    virtual void SetBaseVolume(double theBaseVolume) = 0;
    virtual void SetBasePan(int theBasePan) = 0;

    virtual void AdjustPitch(double theNumSteps) = 0;

    virtual void SetVolume(double theVolume) = 0;
    virtual void SetPan(int thePosition) = 0; //-hundredth db to +hundredth db = left to right

    virtual int GetSoundPosition() = 0;
    virtual void SetSoundPosition(int thePosition) = 0;

    virtual bool Play(bool looping, bool autoRelease) = 0;
    virtual void Stop() = 0;
    virtual bool IsPlaying() = 0;
    virtual bool IsReleased() = 0;
    virtual double GetVolume() = 0;
};
} // namespace Sexy

#endif //__SOUNDINSTANCE_H__
