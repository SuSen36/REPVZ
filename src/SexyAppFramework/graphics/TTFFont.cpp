#include "TTFFont.h"
#include "Graphics.h"
#include "Image.h"
#include "SexyAppFramework/SexyAppBase.h"
#include "Sexy.TodLib/TodCommon.h"
#include <SDL3/SDL.h>
#include "SexyAppFramework/graphics/SDLImage.h"
#include "SexyAppFramework/paklib/PakInterface.h"
#include "Sexy.TodLib/TodDebug.h"

using namespace Sexy;

extern PakInterface* gPakInterface;

TTFFont::TTFFont(const std::string& theFontPath, int thePointSize)
    : mFont(nullptr), mPointSize(thePointSize)
{
    if (!Load(theFontPath))
    {
        TodTrace("Failed to load font: %s", theFontPath.c_str());
    }
}

TTFFont::~TTFFont()
{
    if (mFont)
    {
        TTF_CloseFont(mFont);
    }
    for (auto& aPair : mCharGlyphMap)
    {
        delete aPair.second.mImage;
    }
    mCharGlyphMap.clear();
}

bool TTFFont::Load(const std::string& theFontPath)
{
    mFilePath = theFontPath;

    // 1. Try to load from standard file system first
    mFont = TTF_OpenFont(theFontPath.c_str(), mPointSize);

    if (mFont)
    {
        // Disable hinting to ensure metrics match rendered bitmaps (fixes dancing letters)
        TTF_SetFontHinting(mFont, TTF_HINTING_NONE);
    }

    // 2. If standard load fails, try loading from PAK
    if (!mFont && gPakInterface)
    {
        PFILE* aPakFile = gPakInterface->FOpen(theFontPath.c_str(), "rb");
        if (aPakFile)
        {
            // Read entire font into memory buffer
            mFontData.clear();
            uint8_t aChunk[4096];
            size_t aReadBytes = 0;
            while ((aReadBytes = gPakInterface->FRead(aChunk, 1, sizeof(aChunk), aPakFile)) > 0)
            {
                mFontData.insert(mFontData.end(), aChunk, aChunk + aReadBytes);
            }
            gPakInterface->FClose(aPakFile);

            if (!mFontData.empty())
            {
                SDL_IOStream* aIO = SDL_IOFromConstMem(mFontData.data(), mFontData.size());
                if (aIO)
                {
                    mFont = TTF_OpenFontIO(aIO, true, mPointSize);
                    if (mFont)
                    {
                        TTF_SetFontHinting(mFont, TTF_HINTING_NONE);
                    }
                }
            }
        }
    }

    if (mFont)
    {
        mAscent = TTF_GetFontAscent(mFont);
        mHeight = TTF_GetFontHeight(mFont);
        return true;
    }

    TodTrace("TTF_OpenFont failed for %s: %s", theFontPath.c_str(), SDL_GetError());
    return false;
}

Font* TTFFont::Duplicate()
{
    return new TTFFont(mFilePath, mPointSize);
}

int TTFFont::StringWidth(const SexyString& theString)
{
    if (!mFont) return 0;
    
    int aWidth = 0;
    int aHeight = 0;
    if (TTF_GetStringSize(mFont, theString.c_str(), theString.length(), &aWidth, &aHeight))
    {
        return aWidth;
    }
    return 0;
}

int TTFFont::CharWidth(SexyChar theChar)
{
    if (!mFont) return 0;
    int aAdvance = 0;
    TTF_GetGlyphMetrics(mFont, (uint32_t)(unsigned char)theChar, nullptr, nullptr, nullptr, nullptr, &aAdvance);
    return aAdvance;
}

int TTFFont::CharWidthKern(SexyChar theChar, SexyChar thePrevChar)
{
    return CharWidth(theChar);
}

void TTFFont::DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect)
{
    if (!mFont) return;

    // Save original state to avoid side effects on subsequent drawing operations
    bool aOldColorizeImages = g->GetColorizeImages();
    Color aOldColor = g->GetColor();

    int aX = theX;
    SDL_Renderer* aRenderer = gRenderer;

    const char* aStr = theString.c_str();
    size_t aLen = theString.length();
    size_t aI = 0;

    while (aI < aLen)
    {
        uint32_t aChar = 0;
        unsigned char aByte = aStr[aI];
        int aBytes = 0;

        if (aByte < 0x80)
        {
            aChar = aByte;
            aBytes = 1;
        }
        else if ((aByte & 0xE0) == 0xC0)
        {
            aChar = aByte & 0x1F;
            aBytes = 2;
        }
        else if ((aByte & 0xF0) == 0xE0)
        {
            aChar = aByte & 0x0F;
            aBytes = 3;
        }
        else if ((aByte & 0xF8) == 0xF0)
        {
            aChar = aByte & 0x07;
            aBytes = 4;
        }
        else
        {
            // Invalid UTF-8, skip
            aI++;
            continue;
        }

        if (aBytes > 1)
        {
            if (aI + aBytes > aLen) break;
            for (int aK = 1; aK < aBytes; aK++)
            {
                aChar = (aChar << 6) | (aStr[aI + aK] & 0x3F);
            }
        }
        aI += aBytes;

        // Simple whitespace handling
        if (aChar == ' ' || aChar == '\t' || aChar == '\n')
        {
            aX += CharWidth(aChar);
            continue;
        }

        GlyphInfo* aGlyph = nullptr;
        auto aIt = mCharGlyphMap.find(aChar);

        if (aIt != mCharGlyphMap.end())
        {
            aGlyph = &aIt->second;
        }
        else
        {
            SDL_Color aSdlColor = { 255, 255, 255, 255 }; // Always render white, use Colorize to tint
            SDL_Surface* aSurface = TTF_RenderGlyph_Blended(mFont, aChar, aSdlColor);
            if (aSurface)
            {
                SDL_Texture* aTexture = SDL_CreateTextureFromSurface(aRenderer, aSurface);
                if (aTexture)
                {
                    SDLImage* aNewImage = new SDLImage();
                    aNewImage->mWidth = aSurface->w;
                    aNewImage->mHeight = aSurface->h;
                    aNewImage->mTexture = aTexture;
                    aNewImage->mHasAlpha = true;
                    aNewImage->mHasTrans = true;
                    aNewImage->mBits = nullptr;
                    
                    int aMinX = 0, aMaxY = 0, aAdvance = 0;
                    // Note: We use mMinY to store MaxY (bearing top) for simplified calc
                    TTF_GetGlyphMetrics(mFont, aChar, &aMinX, nullptr, nullptr, &aMaxY, &aAdvance);
                    
                    GlyphInfo aInfo;
                    aInfo.mImage = reinterpret_cast<Image *>(aNewImage);
                    aInfo.mAdvance = aAdvance;
                    aInfo.mOffsetX = aMinX;
                    aInfo.mMinY = aMaxY; // Storing MaxY here
                    
                    mCharGlyphMap[aChar] = aInfo;
                    aGlyph = &mCharGlyphMap[aChar];
                }
                SDL_DestroySurface(aSurface);
            }
        }

        if (aGlyph && aGlyph->mImage)
        {
            g->SetColor(theColor);
            g->SetColorizeImages(true);
            
            // Draw relative to baseline: Top = Baseline - MaxY (stored in mMinY)
            // Baseline = theY + mAscent
            // Note: TTF_SetFontHinting(TTF_HINTING_NONE) is crucial for this to match
            g->DrawImage(aGlyph->mImage, aX + aGlyph->mOffsetX, theY + mAscent - aGlyph->mMinY);
            
            aX += aGlyph->mAdvance;
        }
        else
        {
             // If rendering failed, still advance cursor to avoid overlapping
             aX += CharWidth(aChar);
        }
    }

    // Restore original state
    g->SetColorizeImages(aOldColorizeImages);
    g->SetColor(aOldColor);
}
