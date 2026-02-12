#ifndef __IMAGELIB_H__
#define __IMAGELIB_H__

#include <cstdint>
#include <memory>
#include <string>
#include <cstring>

#include "SexyAppFramework/misc/ResourceManager.h"

namespace ImageLib {
    class Image {
    public:
        int mWidth = 0;
        int mHeight = 0;
        std::unique_ptr<uint32_t[]> mBits = nullptr;

    public:
        Image() = default;
        virtual ~Image() = default;

        Image(int width, int height)
                : mWidth(width), mHeight(height), mBits(std::make_unique<uint32_t[]>(mWidth * mHeight)) {
            memset(mBits.get(), 0, mWidth * mHeight * sizeof(uint32_t));
        }

        Image(int width, int height, std::unique_ptr<uint32_t[]> bits)
                : mWidth(width), mHeight(height), mBits(std::move(bits)) {}

        int GetWidth() { return mWidth; }
        int GetHeight() { return mHeight; }
        uint32_t* GetBits() { return mBits.get(); }
    };

    bool WriteJPEGImage(const std::string& theFileName, const Image* theImage);
    bool WritePNGImage(const std::string& theFileName, const Image* theImage);
    bool WriteTGAImage(const std::string& theFileName, const Image* theImage);
    bool WriteBMPImage(const std::string& theFileName, const Image* theImage);
    
    extern int gAlphaComposeColor;
    extern bool gAutoLoadAlpha;
    extern bool gIgnoreJPEG2000Alpha;

    // Overload for string path
    Image* GetImage(const std::string& theFileName, bool lookForAlphaImage = true);

    // Primary overload used by SexyAppBase
    Image* GetImage(const Sexy::ResourceManager::ImageRes &theRes, bool lookForAlphaImage = true);
}

#endif //__IMAGELIB_H__
