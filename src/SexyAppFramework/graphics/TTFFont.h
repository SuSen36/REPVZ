#ifndef __SEXY_TTFFONT_H__
#define __SEXY_TTFFONT_H__

#include "Font.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <map>
#include <vector>
#include <string>

namespace Sexy
{

class Graphics;
class Image;

class TTFFont : public Font
{
    struct GlyphInfo
    {
        Image* mImage;
        int mAdvance;
        int mOffsetX;
        int mMinY;
    };

    TTF_Font* mFont;
    int mPointSize;
    std::string mFilePath;

    std::vector<uint8_t> mFontData; // Buffer for font data loaded from PAK

    std::map<uint32_t, GlyphInfo> mCharGlyphMap;

public:
    TTFFont(const std::string& theFontPath, int thePointSize);
    virtual ~TTFFont();

    // Disable copy/assign to avoid double-free of mFont
    TTFFont(const TTFFont&) = delete;
    TTFFont& operator=(const TTFFont&) = delete;

    virtual int StringWidth(const SexyString& theString) override;
    virtual int CharWidth(SexyChar theChar) override;
    virtual int CharWidthKern(SexyChar theChar, SexyChar thePrevChar) override;
    virtual void DrawString(Graphics* g, int theX, int theY, const SexyString& theString, const Color& theColor, const Rect& theClipRect) override;
    
    virtual Font* Duplicate() override;

    bool Load(const std::string& theFontPath);
    bool IsLoaded() const { return mFont != nullptr; }
};

}

#endif //__SEXY_TTFFONT_H__
