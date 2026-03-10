#include "MemoryImage.h"
#include <SDL.h>
#include <vector>

#include "../misc/CritSect.h"
#include "../misc/SexyMatrix.h"
#include "../SexyAppBase.h"
#include "Graphics.h"
#include "NativeDisplay.h"
#include "Quantize.h"
#include "SWTri.h"
#include "../../GameConstants.h"
#include <cmath>

namespace Sexy
{

MemoryImage::MemoryImage()
{	
	mApp = gSexyAppBase;
	
	Init();
}

MemoryImage::MemoryImage(SexyAppBase* theApp) 
{
	mApp = theApp;
	Init();
}

MemoryImage::MemoryImage(const MemoryImage& theMemoryImage) :
	Image(theMemoryImage)
{
	mBits = NULL;
	mColorTable = NULL;
	mColorIndices = NULL;
	mHasTrans = theMemoryImage.mHasTrans;
	mHasAlpha = theMemoryImage.mHasAlpha;
	mBitsChanged = false;
	mForcedMode = theMemoryImage.mForcedMode;
	mIsVolatile = theMemoryImage.mIsVolatile;
	mD3DData = NULL;
	mD3DFlags = 0;
	mBitsChangedCount = 0;
	mPurgeBits = false;
	mWantPal = theMemoryImage.mWantPal;
	mTexture = NULL;
	mApp = theMemoryImage.mApp;

	bool deleteBits = false;

	MemoryImage* aNonConstMemoryImage = (MemoryImage*) &theMemoryImage;

	if ((theMemoryImage.mBits == NULL) && (theMemoryImage.mColorTable == NULL))
	{
		// Must be a DDImage with only a DDSurface
		aNonConstMemoryImage->GetBits();
		deleteBits = true;
	}

	if (theMemoryImage.mBits != NULL)
	{
		mBits = new uint32_t[mWidth*mHeight];
		memcpy(mBits, theMemoryImage.mBits, (mWidth*mHeight)*sizeof(uint32_t));
	}
	else
		mBits = NULL;

	if (deleteBits)
	{
		// Remove the temporary source bits
		delete [] aNonConstMemoryImage->mBits;
		aNonConstMemoryImage->mBits = NULL;
	}

	if (theMemoryImage.mColorTable != NULL)
	{
		mColorTable = new uint32_t[256];
		memcpy(mColorTable, theMemoryImage.mColorTable, 256*sizeof(uint32_t));
	}
	else
		mColorTable = NULL;

	if (theMemoryImage.mColorIndices != NULL)
	{
		mColorIndices = new uchar[mWidth*mHeight];
		memcpy(mColorIndices, theMemoryImage.mColorIndices, mWidth*mHeight*sizeof(uchar));
	}
	else
		mColorIndices = NULL;

	mApp->AddMemoryImage(this);
}

MemoryImage::~MemoryImage()
{	
	mApp->RemoveMemoryImage(this);
	
    if (mTexture) {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
    }

	delete [] mBits;
	delete [] mColorIndices;
	delete [] mColorTable;
}

void MemoryImage::Init()
{
	mBits = NULL;
	mColorTable = NULL;
	mColorIndices = NULL;

	mHasTrans = false;
	mHasAlpha = false;	
	mBitsChanged = false;
	mForcedMode = false;
	mIsVolatile = false;

	mD3DData = NULL;
	mD3DFlags = 0;
	mBitsChangedCount = 0;

	mPurgeBits = false;
	mWantPal = false;

    mTexture = NULL;

	mApp->AddMemoryImage(this);
}

void MemoryImage::BitsChanged()
{
	mBitsChanged = true;
	mBitsChangedCount++;
}

SDL_Texture* MemoryImage::GetTexture()
{
    if (mTexture == nullptr)
    {
        mTexture = SDL_CreateTexture(Sexy::gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, mWidth, mHeight);
        if (mBits)
        {
            SDL_UpdateTexture(mTexture, NULL, mBits, mWidth * 4);
        }
        mBitsChanged = false;
    }
    else if (mBitsChanged)
    {
        if (mBits)
        {
            SDL_UpdateTexture(mTexture, NULL, mBits, mWidth * 4);
        }
        mBitsChanged = false;
    }
    return mTexture;
}

void MemoryImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
    DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
}

void MemoryImage::NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, Graphics::DRAWMODE_NORMAL);
}

void MemoryImage::AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, Graphics::DRAWMODE_ADDITIVE);
}


void MemoryImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_SetRenderDrawColor(Sexy::gRenderer, theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha);
    SDL_SetRenderDrawBlendMode(Sexy::gRenderer, theDrawMode == 0 ? Sexy::gPremultipliedBlendMode : Sexy::gAdditiveBlendMode);
    SDL_RenderDrawLineF(Sexy::gRenderer, (float)theStartX, (float)theStartY, (float)theEndX, (float)theEndY);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::CommitBits()
{
	//if (gDebug)
	//	mApp->CopyToClipboard("+MemoryImage::CommitBits");
	
	if ((mBitsChanged) && (!mForcedMode))
	{			
		// Analyze 
		if (mBits != NULL)
		{
			mHasTrans = false;
			mHasAlpha = false;
			
			int aSize = mWidth*mHeight;
			uint32_t* ptr = mBits;
			
			for (int i = 0; i < aSize; i++)
			{
				uchar anAlpha = (uchar) (*ptr++ >> 24);

				if (anAlpha == 0)
					mHasTrans = true;
				else if (anAlpha != 255)
					mHasAlpha = true;
			}
		}
		else if (mColorTable != NULL)
		{
			mHasTrans = false;
			mHasAlpha = false;
			
			int aSize = 256;
			uint32_t* ptr = mColorTable;
			
			for (int i = 0; i < aSize; i++)
			{
				uchar anAlpha = (uchar) (*ptr++ >> 24);

				if (anAlpha == 0)
					mHasTrans = true;
				else if (anAlpha != 255)
					mHasAlpha = true;
			}
		}
		else
		{
			mHasTrans = true;
			mHasAlpha = false;
		}

		mBitsChanged = false;
	}

	//if (gDebug)
	//	mApp->CopyToClipboard("-MemoryImage::CommitBits");
}

void MemoryImage::SetImageMode(bool hasTrans, bool hasAlpha)
{
	mForcedMode = true;	
	mHasTrans = hasTrans;
	mHasAlpha = hasAlpha;	
}

void MemoryImage::SetVolatile(bool isVolatile)
{
	mIsVolatile = isVolatile;
}

void MemoryImage::PurgeBits()
{
	mPurgeBits = true;

	if (true)
	{
		// Due to potential D3D threading issues we have to defer the texture creation
		//  and therefore the actual purging
		if (mD3DData == NULL)
			return;
	}
	else
	{
		if ((mBits == NULL) && (mColorIndices == NULL))
			return;
		
		if (false)
		{
			// Purge bits from GLInterface
		}
	}		
	
	delete [] mBits;
	mBits = NULL;
	
	if (mD3DData != NULL)
	{
		delete [] mColorIndices;
		mColorIndices = NULL;

		delete [] mColorTable;
		mColorTable = NULL;
	}	
}

void MemoryImage::DeleteSWBuffers()
{
	if ((mBits == NULL) && (mColorIndices == NULL))
		GetBits();
}

void MemoryImage::Delete3DBuffers()
{
	mApp->Remove3DData(this);
}

void MemoryImage::DeleteExtraBuffers()
{
	DeleteSWBuffers();
	Delete3DBuffers();
}

void MemoryImage::ReInit()
{
	// Fix any un-palletizing
	if (mWantPal)
		Palletize();
			
	if (mPurgeBits)
		PurgeBits();
}

void MemoryImage::DeleteNativeData()
{
	if ((mBits == NULL) && (mColorIndices == NULL))
		GetBits(); // We need to keep the bits around
}

void MemoryImage::SetBits(uint32_t* theBits, int theWidth, int theHeight, bool commitBits)
{	
	if (theBits != mBits)
	{
		delete [] mColorIndices;
		mColorIndices = NULL;

		delete [] mColorTable;
		mColorTable = NULL;

		if (theWidth != mWidth || theHeight != mHeight)
		{
			delete [] mBits;
			mBits = new uint32_t[theWidth*theHeight];
			mWidth = theWidth;
			mHeight = theHeight;
		}
		memcpy(mBits, theBits, mWidth*mHeight*sizeof(uint32_t));

		BitsChanged();
		if (commitBits)
			CommitBits();
	}
}

void MemoryImage::Create(int theWidth, int theHeight)
{
	delete [] mBits;
	mBits = NULL;

	mWidth = theWidth;
	mHeight = theHeight;	

	// All zeros --> trans + alpha
	mHasTrans = true;
	mHasAlpha = true;

	BitsChanged();	
}

uint32_t* MemoryImage::GetBits()
{
	if (mBits == NULL)
	{
		int aSize = mWidth*mHeight;

		mBits = new uint32_t[aSize];		

		if (mColorTable != NULL)
		{
			for (int i = 0; i < aSize; i++)
				mBits[i] = mColorTable[mColorIndices[i]];

			delete [] mColorIndices;
			mColorIndices = NULL;

			delete [] mColorTable;
			mColorTable = NULL;
		}
		else if (mD3DData == NULL || mApp == NULL)
		{
			memset(mBits, 0, aSize*sizeof(uint32_t));
		}
		else
		{
			// 始终使用软件渲染：直接清空 mBits
			memset(mBits, 0, aSize*sizeof(uint32_t));
		}
	}	

	return mBits;
}

void MemoryImage::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_FRect rect = { (float)theRect.mX, (float)theRect.mY, (float)theRect.mWidth, (float)theRect.mHeight };
    SDL_SetRenderDrawColor(Sexy::gRenderer, theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha);
    SDL_SetRenderDrawBlendMode(Sexy::gRenderer, theDrawMode == 0 ? Sexy::gPremultipliedBlendMode : Sexy::gAdditiveBlendMode);
    SDL_RenderFillRectF(Sexy::gRenderer, &rect);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::DrawRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_FRect rect = { (float)theRect.mX, (float)theRect.mY, (float)theRect.mWidth, (float)theRect.mHeight };
    SDL_SetRenderDrawColor(Sexy::gRenderer, theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha);
    SDL_SetRenderDrawBlendMode(Sexy::gRenderer, theDrawMode == 0 ? Sexy::gPremultipliedBlendMode : Sexy::gAdditiveBlendMode);
    SDL_RenderDrawRectF(Sexy::gRenderer, &rect);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::ClearRect(const Rect& theRect)
{
    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_FRect rect = { (float)theRect.mX, (float)theRect.mY, (float)theRect.mWidth, (float)theRect.mHeight };
    SDL_SetRenderDrawColor(Sexy::gRenderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(Sexy::gRenderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRectF(Sexy::gRenderer, &rect);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::Clear()
{
    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_SetRenderDrawColor(Sexy::gRenderer, 0, 0, 0, 0);
    SDL_RenderClear(Sexy::gRenderer);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::AdditiveBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
    Blt(theImage, theX, theY, theSrcRect, theColor, Graphics::DRAWMODE_ADDITIVE);
}

void MemoryImage::NormalBlt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor)
{
    Blt(theImage, theX, theY, theSrcRect, theColor, Graphics::DRAWMODE_NORMAL);
}

void MemoryImage::Blt(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
    theImage->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_Rect srcRect = { theSrcRect.mX, theSrcRect.mY, theSrcRect.mWidth, theSrcRect.mHeight };
    SDL_FRect destRect = { (float)theX, (float)theY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    SDL_RenderCopyF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MemoryImage::BltF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode)
{
    theImage->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_Rect srcRect = { theSrcRect.mX, theSrcRect.mY, theSrcRect.mWidth, theSrcRect.mHeight };
    SDL_FRect destRect = { theX, theY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {
        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &clipRect);
    }

    SDL_RenderCopyF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect);
    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MemoryImage::BltRotated(Image* theImage, float theX, float theY, const Rect &theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
    theImage->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_Rect srcRect = { theSrcRect.mX, theSrcRect.mY, theSrcRect.mWidth, theSrcRect.mHeight };
    SDL_FRect destRect = { theX, theY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };
    SDL_FPoint center = { theRotCenterX, theRotCenterY };

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {
        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &clipRect);
    }

    static const double aRadToDeg = 180.0 / PI;
    SDL_RenderCopyExF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect, theRot * aRadToDeg, &center, SDL_FLIP_NONE);

    SDL_RenderSetClipRect(Sexy::gRenderer, NULL);
    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::BltMirror(Image* theImage, int theX, int theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode)
{
    theImage->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_Rect srcRect = { theSrcRect.mX, theSrcRect.mY, theSrcRect.mWidth, theSrcRect.mHeight };
    SDL_FRect destRect = { (float)theX, (float)theY, (float)theSrcRect.mWidth, (float)theSrcRect.mHeight };

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    SDL_RenderCopyExF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect, 0, NULL, SDL_FLIP_HORIZONTAL);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::StretchBltMirror(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fast)
{
    theImage->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_Rect srcRect = { theSrcRect.mX, theSrcRect.mY, theSrcRect.mWidth, theSrcRect.mHeight };
    SDL_FRect destRect = { (float)theDestRect.mX, (float)theDestRect.mY, (float)theDestRect.mWidth, (float)theDestRect.mHeight };

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {
        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &clipRect);
    }

    SDL_RenderCopyExF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::SlowStretchBlt(Image* theImage, const Rect& theDestRect, const FRect& theSrcRect, const Color& theColor, int theDrawMode)
{
    theImage->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_Rect srcRect = { (int)theSrcRect.mX, (int)theSrcRect.mY, (int)theSrcRect.mWidth, (int)theSrcRect.mHeight };
    SDL_FRect destRect = { (float)theDestRect.mX, (float)theDestRect.mY, (float)theDestRect.mWidth, (float)theDestRect.mHeight };

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    SDL_RenderCopyF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, bool fast)
{
    theImage->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_Rect srcRect = { theSrcRect.mX, theSrcRect.mY, theSrcRect.mWidth, theSrcRect.mHeight };
    SDL_FRect destRect = { (float)theDestRect.mX, (float)theDestRect.mY, (float)theDestRect.mWidth, (float)theDestRect.mHeight };

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &clipRect);
    }

    SDL_RenderCopyF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect);
    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::BltMatrix(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
    theImage->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {
        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &clipRect);
    }

    float w2 = theSrcRect.mWidth / 2.0f;
    float h2 = theSrcRect.mHeight / 2.0f;

    float u0 = (float)theSrcRect.mX / theImage->mWidth;
    float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / theImage->mWidth;
    float v0 = (float)theSrcRect.mY / theImage->mHeight;
    float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / theImage->mHeight;

    SDL_Vertex verts[4];
    float px[4] = { -w2, w2, -w2, w2 };
    float py[4] = { -h2, -h2, h2, h2 };
    float pu[4] = { u0, u1, u0, u1 };
    float pv[4] = { v0, v0, v1, v1 };

    for (int i = 0; i < 4; i++)
    {
        SexyVector3 v(px[i], py[i], 1);
        v = theMatrix * v;
        verts[i].position.x = v.x + x;
        verts[i].position.y = v.y + y;
        verts[i].tex_coord.x = pu[i];
        verts[i].tex_coord.y = pv[i];
        verts[i].color.r = theColor.mRed;
        verts[i].color.g = theColor.mGreen;
        verts[i].color.b = theColor.mBlue;
        verts[i].color.a = theColor.mAlpha;
    }

    int indices[6] = { 0, 1, 2, 1, 2, 3 };
    SDL_RenderGeometry(Sexy::gRenderer, aSrcTexture, verts, 4, indices, 6);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {
        SDL_RenderSetClipRect(Sexy::gRenderer, NULL);
    }
    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

bool MemoryImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty)
{
    if (theNumVertices < 3) return true;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    if (theClipRect)
    {
        SDL_Rect aClipRect = { theClipRect->mX, theClipRect->mY, theClipRect->mWidth, theClipRect->mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &aClipRect);
    }

    std::vector<SDL_Vertex> aVertices(theNumVertices);
    for (int i = 0; i < theNumVertices; ++i)
    {
        aVertices[i].position.x = (float)theVertices[i].mX + tx;
        aVertices[i].position.y = (float)theVertices[i].mY + ty;
        aVertices[i].color.r = theColor.mRed;
        aVertices[i].color.g = theColor.mGreen;
        aVertices[i].color.b = theColor.mBlue;
        aVertices[i].color.a = theColor.mAlpha;
        aVertices[i].tex_coord.x = 0;
        aVertices[i].tex_coord.y = 0;
    }

    int aNumTriangles = theNumVertices - 2;
    std::vector<int> aIndices(aNumTriangles * 3);
    for (int i = 0; i < aNumTriangles; ++i)
    {
        aIndices[i * 3 + 0] = 0;
        aIndices[i * 3 + 1] = i + 1;
        aIndices[i * 3 + 2] = i + 2;
    }

    SDL_SetRenderDrawBlendMode(Sexy::gRenderer, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_RenderGeometry(Sexy::gRenderer, NULL, aVertices.data(), (int)aVertices.size(), aIndices.data(), (int)aIndices.size());

    SDL_RenderSetClipRect(Sexy::gRenderer, NULL);
    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);

    return true;
}

void MemoryImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend)
{
    theTexture->mDrawn = true;
    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theTexture);
    if (!aSrcMemoryImage) return;

    SDL_Texture* aSrcTexture = aSrcMemoryImage->GetTexture();
    if (!aSrcTexture) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_SetTextureColorMod(aSrcTexture, theColor.mRed, theColor.mGreen, theColor.mBlue);
    SDL_SetTextureAlphaMod(aSrcTexture, theColor.mAlpha);
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? Sexy::gAdditiveBlendMode : Sexy::gPremultipliedBlendMode);
    SDL_SetTextureScaleMode(aSrcTexture, SDL_ScaleModeLinear);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {
        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &clipRect);
    }

    for (int i = 0; i < theNumTriangles; i++)
    {
        SDL_Vertex v[3];
        for (int j = 0; j < 3; j++)
        {
            v[j].position.x = theVertices[i][j].x + tx;
            v[j].position.y = theVertices[i][j].y + ty;
            v[j].tex_coord.x = theVertices[i][j].u;
            v[j].tex_coord.y = theVertices[i][j].v;

            uint32_t c = theVertices[i][j].color;
            if (c == 0) {
                v[j].color.r = 255;
                v[j].color.g = 255;
                v[j].color.b = 255;
                v[j].color.a = 255;
            } else {
                v[j].color.r = (c >> 16) & 0xFF;
                v[j].color.g = (c >> 8) & 0xFF;
                v[j].color.b = c & 0xFF;
                v[j].color.a = (c >> 24) & 0xFF;
            }
        }
        SDL_RenderGeometry(Sexy::gRenderer, aSrcTexture, v, 3, NULL, 0);
    }

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

bool MemoryImage::Palletize()
{
	CommitBits();
	
	if (mColorTable != nullptr)
		return true;

	GetBits();

	if (mBits == nullptr)
		return false;

	mColorIndices = new uchar[mWidth*mHeight];
	mColorTable = new uint32_t[256];

	if (!Quantize8Bit(mBits, mWidth, mHeight, mColorIndices, mColorTable))
	{
		delete [] mColorIndices;
		mColorIndices = nullptr;

		delete [] mColorTable;
		mColorTable = nullptr;

		mWantPal = false;

		return false;
	}
	
	delete [] mBits;
	mBits = nullptr;

	mWantPal = true;

	return true;
}

} // namespace Sexy
