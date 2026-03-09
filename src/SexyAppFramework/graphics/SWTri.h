#pragma once

#include "Color.h"
#include "MemoryImage.h"
#include "../misc/Rect.h"
#include "../misc/SexyMatrix.h"

namespace Sexy
{

class SWHelper {
public:
    struct XYZStruct {
        float mX;
        float mY;
        float mU;
        float mV;
        slong mDiffuse;
    };

    struct SWVertex {
        int x, y;
        int a, r, g, b;
        int u, v;
    };
    struct SWTextureInfo {
        const unsigned int *pTexture;
        unsigned int vShift, uMask, vMask;
        int pitch;
        unsigned int endpos;
        int height;
    };
    struct SWDiffuse {
        unsigned int a, r, g, b;
    };
};
} // namespace Sexy

