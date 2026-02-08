#include <SDL3/SDL.h>
#include "SDLImage.h"
#include "SexyAppFramework/graphics/TriVertex.h"
#include "SexyAppFramework/misc/SexyMatrix.h"
#include <vector>

using namespace Sexy;

SDL_Renderer* Sexy::gRenderer;

bool SDLImage::Check3D(Image* image)
{
    return dynamic_cast<SDLImage*>(image) != nullptr;
}

SDLImage::SDLImage()
{
	mTexture = nullptr;
	mHasTrans = false;
	mHasAlpha = false;
	mBits = nullptr;
	mPurgeBits = false;
	mBitsChangedCount = 0;
}

SDL_Texture *SDLImage::GetTexture(Image *image)
{
    auto memoryImage = dynamic_cast<SDLImage*>(image);

    if (memoryImage == nullptr)
        return nullptr;

    if (memoryImage->mTexture == nullptr)
    {
        auto bits = memoryImage->GetBits();
        auto surface = SDL_CreateSurfaceFrom(memoryImage->mWidth, memoryImage->mHeight, SDL_PIXELFORMAT_ARGB8888, bits, memoryImage->mWidth * 4);
        memoryImage->mTexture = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_DestroySurface(surface);
    }

    return memoryImage->mTexture;
}


SDLImage::~SDLImage()
{
    if (mTexture)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
}

bool SDLImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty, bool convex)
{
    return false;
}

void SDLImage::FillRect(const Rect &theRect, const Color &theColor, int theDrawMode)
{
    SDL_FRect rect = {(float)theRect.mX, (float)theRect.mY, (float)theRect.mWidth, (float)theRect.mHeight};
    SDL_SetRenderDrawColor(gRenderer, theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha);
    SDL_SetRenderDrawBlendMode(gRenderer, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_RenderFillRect(gRenderer, &rect);
}

void SDLImage::DrawRect(const Rect &theRect, const Color &theColor, int theDrawMode)
{
    SDL_FRect rect = {(float)theRect.mX, (float)theRect.mY, (float)theRect.mWidth + 1, (float)theRect.mHeight + 1};
    SDL_SetRenderDrawColor(gRenderer, theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha);
    SDL_SetRenderDrawBlendMode(gRenderer, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_RenderRect(gRenderer, &rect);
}

void SDLImage::ClearRect(const Rect &theRect)
{
}

void SDLImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor, int theDrawMode)
{
    SDL_SetRenderDrawColor(gRenderer, theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha);
    SDL_RenderLine(gRenderer, theStartX, theStartY, theEndX, theEndY);
}

void SDLImage::FillScanLines(Span *theSpans, int theSpanCount, const Color &theColor, int theDrawMode)
{
    if (theSpanCount <= 0) return;

    SDL_SetRenderDrawColor(gRenderer, theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha);
    SDL_SetRenderDrawBlendMode(gRenderer, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);

    for (int i = 0; i < theSpanCount; ++i)
        SDL_RenderLine(gRenderer, (float)theSpans[i].mX, (float)theSpans[i].mY, (float)(theSpans[i].mX + theSpans[i].mWidth - 1), (float)theSpans[i].mY);
}

void SDLImage::FillScanLinesWithCoverage(Span *theSpans, int theSpanCount, const Color &theColor, int theDrawMode, const BYTE *theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight)
{
    for (int i = 0; i < theSpanCount; ++i)
    {
        int aIndex = (theSpans[i].mY - theCoverY) * theCoverWidth + (theSpans[i].mX - theCoverX);
        Color aColor = theColor;
        for (int x = 0; x < theSpans[i].mWidth; ++x)
        {
            aColor.mAlpha = (theColor.mAlpha * theCoverage[aIndex++]) / 255;
            FillRect(Rect(theSpans[i].mX + x, theSpans[i].mY, 1, 1), aColor, theDrawMode);
        }
    }
}

void SDLImage::Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor, int theDrawMode)
{
    auto texture = GetTexture(theImage);

    if (texture == nullptr)
        return;

    SDL_FRect srcRect = { (float)theSrcRect.mX,(float) theSrcRect.mY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_FRect dstRect = { (float)theX, (float)theY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_SetTextureColorMod(texture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(texture, theColor.mAlpha);
    SDL_SetTextureBlendMode(texture, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_RenderTexture(gRenderer, texture, &srcRect, &dstRect);
}

void SDLImage::BltF(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect, const Color &theColor, int theDrawMode)
{
    auto texture = GetTexture(theImage);

    if (texture == nullptr)
        return;

    SDL_FRect srcRect = { (float)theSrcRect.mX,(float) theSrcRect.mY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_FRect dstRect = { theX, theY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
    SDL_SetTextureColorMod(texture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(texture, theColor.mAlpha);
    SDL_SetTextureBlendMode(texture, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_SetRenderClipRect(gRenderer, &clipRect);
    SDL_RenderTexture(gRenderer, texture, &srcRect, &dstRect);
    SDL_SetRenderClipRect(gRenderer, nullptr);
}

void SDLImage::BltRotated(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect, const Color &theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
    auto texture = GetTexture(theImage);

    if (texture == nullptr)
        return;

    SDL_FRect srcRect = { (float)theSrcRect.mX,(float) theSrcRect.mY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_FRect destRect = { theX, theY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
    SDL_FPoint center = { theRotCenterX, theRotCenterY };
    SDL_SetTextureColorMod(texture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(texture, theColor.mAlpha);
    SDL_SetTextureBlendMode(texture, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_SetRenderClipRect(gRenderer, &clipRect);
    SDL_RenderTextureRotated(gRenderer, texture, &srcRect, &destRect, theRot, &center, SDL_FLIP_NONE);
    SDL_SetRenderClipRect(gRenderer, nullptr);
}

void SDLImage::StretchBlt(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect &theClipRect, const Color &theColor, int theDrawMode, bool fastStretch)
{
    auto texture = GetTexture(theImage);

    if (texture == nullptr)
        return;

    SDL_FRect srcRect = { (float)theSrcRect.mX,(float) theSrcRect.mY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_FRect destRect = { (float)theDestRect.mX, (float)theDestRect.mY, (float)theDestRect.mWidth, (float)theDestRect.mHeight };
    SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
    SDL_SetTextureColorMod(texture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(texture, theColor.mAlpha);
    SDL_SetTextureBlendMode(texture, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_SetRenderClipRect(gRenderer, &clipRect);
    SDL_RenderTexture(gRenderer, texture, &srcRect, &destRect);
    SDL_SetRenderClipRect(gRenderer, nullptr);
}

void SDLImage::BltMatrix(Image *theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect &theClipRect, const Color &theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
    auto texture = GetTexture(theImage);

    if (texture == nullptr)
        return;

    float w2 = theSrcRect.mWidth / 2.0f;
    float h2 = theSrcRect.mHeight / 2.0f;

    float u0 = (float)theSrcRect.mX / theImage->mWidth;
    float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / theImage->mWidth;
    float v0 = (float)theSrcRect.mY / theImage->mHeight;
    float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / theImage->mHeight;

    SDL_FColor color = { theColor.mRed / 255.0f, theColor.mGreen / 255.0f, theColor.mBlue / 255.0f, theColor.mAlpha / 255.0f };

    SDL_Vertex vertices[4] =
    {
        { { -w2, -h2 }, color, { u0, v0 } },
        { { w2, -h2 }, color, { u1, v0 } },
        { { -w2, h2 }, color, { u0, v1 } },
        { { w2, h2 }, color, { u1, v1 } },
    };

    int indices[6] = { 0, 1, 2, 1, 3, 2 };

    for (int i = 0; i < 4; i++)
    {
        SexyVector3 v(vertices[i].position.x, vertices[i].position.y, 1);
        v = theMatrix * v;
        vertices[i].position.x = v.x + x;
        vertices[i].position.y = v.y + y;
    }

    SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };

    SDL_SetTextureBlendMode(texture, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_SetRenderClipRect(gRenderer, &clipRect);
    SDL_RenderGeometry(gRenderer, texture, vertices, 4, indices, 6);
    SDL_SetRenderClipRect(gRenderer, nullptr);
}

void SDLImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect &theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend)
{
    auto texture = GetTexture(theTexture);

    if (texture == nullptr)
        return;

    std::vector<SDL_Vertex> vertices;
    vertices.resize(theNumTriangles * 3);

    for (int i = 0; i < theNumTriangles; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            vertices[i * 3 + j].position.x = theVertices[i][j].x + tx;
            vertices[i * 3 + j].position.y = theVertices[i][j].y + ty;
            vertices[i * 3 + j].tex_coord.x = theVertices[i][j].u;
            vertices[i * 3 + j].tex_coord.y = theVertices[i][j].v;
            if (theVertices[i][j].color != 0)
            {
                vertices[i * 3 + j].color.r = ((theVertices[i][j].color >> 16) & 0xff) / 255.0f;
                vertices[i * 3 + j].color.g = ((theVertices[i][j].color >> 8) & 0xff) / 255.0f;
                vertices[i * 3 + j].color.b = (theVertices[i][j].color & 0xff) / 255.0f;
                vertices[i * 3 + j].color.a = ((theVertices[i][j].color >> 24) & 0xff) / 255.0f;
            }
            else
            {
                vertices[i * 3 + j].color.r = theColor.mRed / 255.0f;
                vertices[i * 3 + j].color.g = theColor.mGreen / 255.0f;
                vertices[i * 3 + j].color.b = theColor.mBlue / 255.0f;
                vertices[i * 3 + j].color.a = theColor.mAlpha / 255.0f;
            }
        }
    }

    SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };

    SDL_SetTextureBlendMode(texture, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_SetRenderClipRect(gRenderer, &clipRect);
    SDL_RenderGeometry(gRenderer, texture, vertices.data(), vertices.size(), nullptr, 0);
    SDL_SetRenderClipRect(gRenderer, nullptr);
}

void SDLImage::BltMirror(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor, int theDrawMode)
{
    auto texture = GetTexture(theImage);

    if (texture == nullptr)
        return;

    SDL_FRect srcRect = { (float)theSrcRect.mX,(float) theSrcRect.mY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_FRect destRect = { (float)theX, (float)theY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_SetTextureColorMod(texture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(texture, theColor.mAlpha);
    SDL_SetTextureBlendMode(texture, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_RenderTextureRotated(gRenderer, texture, &srcRect, &destRect, 0, nullptr, SDL_FLIP_HORIZONTAL);
}

void SDLImage::StretchBltMirror(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect &theClipRect, const Color &theColor, int theDrawMode, bool fastStretch)
{
    auto texture = GetTexture(theImage);

    if (texture == nullptr)
        return;

    SDL_FRect srcRect = { (float)theSrcRect.mX,(float) theSrcRect.mY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_FRect destRect = { (float)theDestRect.mX, (float)theDestRect.mY, (float)theDestRect.mWidth, (float)theDestRect.mHeight };
    SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
    SDL_SetTextureColorMod(texture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(texture, theColor.mAlpha);
    SDL_SetTextureBlendMode(texture, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_SetRenderClipRect(gRenderer, &clipRect);
    SDL_RenderTextureRotated(gRenderer, texture, &srcRect, &destRect, 0, nullptr, SDL_FLIP_HORIZONTAL);
    SDL_SetRenderClipRect(gRenderer, nullptr);
}

// SDLImage compatibility methods
void SDLImage::SetBits(uint32_t* theBits, int theWidth, int theHeight, bool commitBits)
{
    mWidth = theWidth;
    mHeight = theHeight;
    
    // Create SDL surface from bits
    SDL_Surface* surface = SDL_CreateSurfaceFrom(theWidth, theHeight, SDL_PIXELFORMAT_ARGB8888, theBits, theWidth * 4);
    if (surface)
    {
        // Create texture from surface
        if (mTexture)
            SDL_DestroyTexture((SDL_Texture*)mTexture);
        
        mTexture = SDL_CreateTextureFromSurface(gRenderer, surface);
        SDL_DestroySurface(surface);
    }
}

void SDLImage::Create(int theWidth, int theHeight)
{
    mWidth = theWidth;
    mHeight = theHeight;
    
    // Create empty texture
    if (mTexture)
        SDL_DestroyTexture((SDL_Texture*)mTexture);
    
    mTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, theWidth, theHeight);
}

uint32_t* SDLImage::GetBits()
{
    if (mBits)
        return mBits;

    if (!mTexture)
        return nullptr;

    mBits = new uint32_t[mWidth * mHeight];
    mPurgeBits = true;

    SDL_Texture* readTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, mWidth, mHeight);
    if (!readTexture)
    {
        delete[] mBits;
        mBits = nullptr;
        return nullptr;
    }

    SDL_Texture* prevTarget = SDL_GetRenderTarget(gRenderer);
    SDL_SetRenderTarget(gRenderer, readTexture);

    SDL_RenderTexture(gRenderer, mTexture, nullptr, nullptr);
    SDL_FlushRenderer(gRenderer);
    
    SDL_Surface* surface = SDL_RenderReadPixels(gRenderer, nullptr);
    if (surface)
    {
        SDL_memcpy(mBits, surface->pixels, mWidth * mHeight * 4);
        SDL_DestroySurface(surface);
    }

    SDL_SetRenderTarget(gRenderer, prevTarget);
    SDL_DestroyTexture(readTexture);

    return mBits;
}

void SDLImage::BitsChanged()
{
	// For SDLImage, we might need to update the texture if bits were modified
	// This is a placeholder - in a real implementation, we'd track if the bits
	// were modified and update the texture accordingly
    CommitBits();
	mBitsChangedCount++;
}

void SDLImage::CommitBits()
{
    if (mBits && mTexture)
        SDL_UpdateTexture(mTexture, nullptr, mBits, mWidth * 4);
}

void SDLImage::PurgeBits()
{
	// For SDLImage, this would free the bits memory
	// This is a placeholder for compatibility with SDLImage
	if (mBits)
	{
		delete[] mBits;
		mBits = nullptr;
	}
}

void SDLImage::DeleteNativeData()
{
	// For SDLImage, this would delete the native texture data
	if (mTexture)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = nullptr;
	}
}


void SDLImage::ReInit()
{
	// For SDLImage, this would reinitialize the image
	// Placeholder implementation
}