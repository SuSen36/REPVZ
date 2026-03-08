#include "MemoryImage.h"
#include <SDL.h>

#include "../misc/CritSect.h"
#include "../misc/SexyMatrix.h"
#include "../SexyAppBase.h"
#include "Graphics.h"
#include "NativeDisplay.h"
#include "Quantize.h"
#include "SWTri.h"

#include <cmath>

using namespace Sexy;

// Disable macro redefinition warning
#pragma warning(disable:4005)

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
	mNativeAlphaData = NULL;
	mRLAlphaData = NULL;
	mRLAdditiveData = NULL;
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
		mBits = new uint32_t[mWidth*mHeight + 1];
		mBits[mWidth*mHeight] = MEMORYCHECK_ID;
		memcpy(mBits, theMemoryImage.mBits, (mWidth*mHeight + 1)*sizeof(uint32_t));
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

	if (theMemoryImage.mNativeAlphaData != NULL)
	{
		if (theMemoryImage.mColorTable == NULL)
		{
			mNativeAlphaData = new uint32_t[mWidth*mHeight];
			memcpy(mNativeAlphaData, theMemoryImage.mNativeAlphaData, mWidth*mHeight*sizeof(uint32_t));
		}
		else
		{
			mNativeAlphaData = new uint32_t[256];
			memcpy(mNativeAlphaData, theMemoryImage.mNativeAlphaData, 256*sizeof(uint32_t));
		}
	}
	else
		mNativeAlphaData = NULL;

	if (theMemoryImage.mRLAlphaData != NULL)
	{
		mRLAlphaData = new uchar[mWidth*mHeight];
		memcpy(mRLAlphaData, theMemoryImage.mRLAlphaData, mWidth*mHeight);
	}
	else
		mRLAlphaData = NULL;

	if (theMemoryImage.mRLAdditiveData != NULL)
	{
		mRLAdditiveData = new uchar[mWidth*mHeight];
		memcpy(mRLAdditiveData, theMemoryImage.mRLAdditiveData, mWidth*mHeight);
	}
	else
		mRLAdditiveData = NULL;	

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
	delete [] mNativeAlphaData;	
	delete [] mRLAlphaData;
	delete [] mRLAdditiveData;
	delete [] mColorIndices;
	delete [] mColorTable;
}

void MemoryImage::Init()
{
	mBits = NULL;
	mColorTable = NULL;
	mColorIndices = NULL;

	mNativeAlphaData = NULL;
	mRLAlphaData = NULL;
	mRLAdditiveData = NULL;
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

	delete [] mNativeAlphaData;
	mNativeAlphaData = NULL;

	delete [] mRLAlphaData;
	mRLAlphaData = NULL;

	delete [] mRLAdditiveData;
	mRLAdditiveData = NULL;

	// Verify secret value at end to protect against overwrite
	if (mBits != NULL)
	{
		TOD_ASSERT(mBits[mWidth*mHeight] == MEMORYCHECK_ID);
	}
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
    SDL_SetRenderDrawBlendMode(Sexy::gRenderer, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_RenderDrawLineF(Sexy::gRenderer, (float)theStartX, (float)theStartY, (float)theEndX, (float)theEndY);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, Graphics::DRAWMODE_NORMAL);
}

void MemoryImage::AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor)
{
	DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, Graphics::DRAWMODE_ADDITIVE);
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

void* MemoryImage::GetNativeAlphaData(NativeDisplay *theDisplay)
{
	if (mNativeAlphaData != NULL)
		return mNativeAlphaData;

	CommitBits();

	const int rRightShift = 16 + (8-theDisplay->mRedBits);
	const int gRightShift = 8 + (8-theDisplay->mGreenBits);
	const int bRightShift = 0 + (8-theDisplay->mBlueBits);

	const int rLeftShift = theDisplay->mRedShift;
	const int gLeftShift = theDisplay->mGreenShift;
	const int bLeftShift = theDisplay->mBlueShift;

	const int rMask = theDisplay->mRedMask;
	const int gMask = theDisplay->mGreenMask;
	const int bMask = theDisplay->mBlueMask;

	if (mColorTable == NULL)
	{
		uint32_t* aSrcPtr = GetBits();

		uint32_t* anAlphaData = new uint32_t[mWidth*mHeight];	

		uint32_t* aDestPtr = anAlphaData;
		int aSize = mWidth*mHeight;
		for (int i = 0; i < aSize; i++)
		{
			uint32_t val = *(aSrcPtr++);

			int anAlpha = val >> 24;			

			uint32_t r = ((val & 0xFF0000) * (anAlpha+1)) >> 8;
			uint32_t g = ((val & 0x00FF00) * (anAlpha+1)) >> 8;
			uint32_t b = ((val & 0x0000FF) * (anAlpha+1)) >> 8;

			*(aDestPtr++) =
				(((r >> rRightShift) << rLeftShift) & rMask) |
				(((g >> gRightShift) << gLeftShift) & gMask) |
				(((b >> bRightShift) << bLeftShift) & bMask) |
				(anAlpha << 24);
		}
		
		mNativeAlphaData = anAlphaData;	
	}
	else
	{
		uint32_t* aSrcPtr = mColorTable;		

		uint32_t* anAlphaData = new uint32_t[256];
		
		for (int i = 0; i < 256; i++)
		{
			uint32_t val = *(aSrcPtr++);

			int anAlpha = val >> 24;

			uint32_t r = ((val & 0xFF0000) * (anAlpha+1)) >> 8;
			uint32_t g = ((val & 0x00FF00) * (anAlpha+1)) >> 8;
			uint32_t b = ((val & 0x0000FF) * (anAlpha+1)) >> 8;

			anAlphaData[i] =
				(((r >> rRightShift) << rLeftShift) & rMask) |
				(((g >> gRightShift) << gLeftShift) & gMask) |
				(((b >> bRightShift) << bLeftShift) & bMask) |
				(anAlpha << 24);
		}
		
		
		mNativeAlphaData = anAlphaData;	
	}

	return mNativeAlphaData;
}


uchar* MemoryImage::GetRLAlphaData()
{
	CommitBits();

	if (mRLAlphaData == NULL)
	{
		mRLAlphaData = new uchar[mWidth*mHeight];

		if (mColorTable == NULL)
		{
			uint32_t* aSrcPtr;
			if (mNativeAlphaData != NULL)
				aSrcPtr = (uint32_t*) mNativeAlphaData;
			else
				aSrcPtr = GetBits();

			GenerateRLAlphaData(aSrcPtr, NULL, mWidth, mHeight);
		}
		else
		{
			uchar* aSrcPtr = mColorIndices;
			uint32_t* aColorTable = mColorTable;

			GenerateRLAlphaData(aSrcPtr, aColorTable, mWidth, mHeight);
		}
	}

	return mRLAlphaData;
}

void MemoryImage::GenerateRLAlphaData(void* aSrcPtr, uint32_t* aColorTable, int theWidth, int theHeight)
{		
	if (theWidth==1)
	{
		memset(mRLAlphaData,1,theHeight);
	}
	else
	{
		for (int aRow = 0; aRow < theHeight; aRow++)			
		{
			int aRCount = 1;
			int aRLCount = 1;

			int anAVal;
			if (aColorTable == NULL)
				anAVal = ((uint32_t*)aSrcPtr)[aRow * theWidth] >> 24;
			else
				anAVal = aColorTable[((uchar*)aSrcPtr)[aRow * theWidth]] >> 24;

			int aLastAClass = (anAVal == 0) ? 0 : (anAVal == 255) ? 1 : 2;

			while (aRCount < theWidth)
			{				
				if (aColorTable == NULL)
					anAVal = ((uint32_t*)aSrcPtr)[aRow * theWidth + aRCount] >> 24;
				else
					anAVal = aColorTable[((uchar*)aSrcPtr)[aRow * theWidth + aRCount]] >> 24;

				int aThisAClass = (anAVal == 0) ? 0 : (anAVal == 255) ? 1 : 2;

				if ((aThisAClass != aLastAClass) || (aRCount == theWidth))
				{
					if (aThisAClass == aLastAClass)
						aRLCount++;

					for (int i = aRLCount; i > 0; i--)
					{
						if (i >= 255)
							*(mRLAlphaData + aRow * theWidth + aRCount++) = 255;
						else
							*(mRLAlphaData + aRow * theWidth + aRCount++) = i;					
					}

					if ((aRCount == theWidth) && (aThisAClass != aLastAClass))
						*(mRLAlphaData + aRow * theWidth + aRCount++) = 1;

					aLastAClass = aThisAClass;
					aRLCount = 1;
				}
				else
				{
					aRLCount++;
				}
			}
		}
	}
}

uchar* MemoryImage::GetRLAdditiveData(NativeDisplay *theNative)
{
	if (mRLAdditiveData == NULL)
	{
		if (mColorTable == NULL)
		{
			uint32_t* aBits = (uint32_t*) GetNativeAlphaData(theNative);

			mRLAdditiveData = new uchar[mWidth*mHeight];

			uchar* aWPtr = mRLAdditiveData;
			uint32_t* aRPtr = aBits;

			if (mWidth==1)
			{
				memset(aWPtr,1,mHeight);
			}
			else
			{
				for (int aRow = 0; aRow < mHeight; aRow++)			
				{
					int aRCount = 1;
					int aRLCount = 1;
					
					int aLastAClass = (((*aRPtr++) & 0xFFFFFF) != 0) ? 1 : 0;

					while (aRCount < mWidth)
					{
						aRCount++;				

						int aThisAClass = (((*aRPtr++) & 0xFFFFFF) != 0) ? 1 : 0;				

						if ((aThisAClass != aLastAClass) || (aRCount == mWidth))
						{
							if (aThisAClass == aLastAClass)
								aRLCount++;

							for (int i = aRLCount; i > 0; i--)
							{
								if (i >= 255)
									*aWPtr++ = 255;
								else
									*aWPtr++ = i;
							}					

							if ((aRCount == mWidth) && (aThisAClass != aLastAClass))
								*aWPtr++ = 1;

							aLastAClass = aThisAClass;
							aRLCount = 1;
						}
						else
						{
							aRLCount++;
						}
					}
				}
			}
		}
		else
		{
			uint32_t* aNativeColorTable = (uint32_t*) GetNativeAlphaData(theNative);

			mRLAdditiveData = new uchar[mWidth*mHeight];

			uchar* aWPtr = mRLAdditiveData;
			uchar* aRPtr = mColorIndices;

			if (mWidth==1)
			{
				memset(aWPtr,1,mHeight);
			}
			else
			{
				for (int aRow = 0; aRow < mHeight; aRow++)			
				{
					int aRCount = 1;
					int aRLCount = 1;
					
					int aLastAClass = (((aNativeColorTable[*aRPtr++]) & 0xFFFFFF) != 0) ? 1 : 0;

					while (aRCount < mWidth)
					{
						aRCount++;				

						int aThisAClass = (((aNativeColorTable[*aRPtr++]) & 0xFFFFFF) != 0) ? 1 : 0;				

						if ((aThisAClass != aLastAClass) || (aRCount == mWidth))
						{
							if (aThisAClass == aLastAClass)
								aRLCount++;

							for (int i = aRLCount; i > 0; i--)
							{
								if (i >= 255)
									*aWPtr++ = 255;
								else
									*aWPtr++ = i;
							}					

							if ((aRCount == mWidth) && (aThisAClass != aLastAClass))
								*aWPtr++ = 1;

							aLastAClass = aThisAClass;
							aRLCount = 1;
						}
						else
						{
							aRLCount++;
						}
					}
				}
			}
		}
	}

	return mRLAdditiveData;
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
	
	delete [] mNativeAlphaData;
	mNativeAlphaData = NULL;

	delete [] mRLAdditiveData;
	mRLAdditiveData = NULL;

	delete [] mRLAlphaData;
	mRLAlphaData = NULL;
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
	
	delete [] mNativeAlphaData;
	mNativeAlphaData = NULL;

	delete [] mRLAdditiveData;
	mRLAdditiveData = NULL;	
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
			mBits = new uint32_t[theWidth*theHeight + 1];
			mWidth = theWidth;
			mHeight = theHeight;
		}
		memcpy(mBits, theBits, mWidth*mHeight*sizeof(uint32_t));
		mBits[mWidth*mHeight] = MEMORYCHECK_ID;

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

		mBits = new uint32_t[aSize+1];		
		mBits[aSize] = MEMORYCHECK_ID;		

		if (mColorTable != NULL)
		{
			for (int i = 0; i < aSize; i++)
				mBits[i] = mColorTable[mColorIndices[i]];

			delete [] mColorIndices;
			mColorIndices = NULL;

			delete [] mColorTable;
			mColorTable = NULL;

			delete [] mNativeAlphaData;
			mNativeAlphaData = NULL;
		}
		else if (mNativeAlphaData != NULL)
			{
			NativeDisplay* aDisplay = nullptr;

			if (aDisplay != nullptr)
			{
				const int rMask = aDisplay->mRedMask;
				const int gMask = aDisplay->mGreenMask;
				const int bMask = aDisplay->mBlueMask;

				const int rLeftShift = aDisplay->mRedShift + (aDisplay->mRedBits);
				const int gLeftShift = aDisplay->mGreenShift + (aDisplay->mGreenBits);
				const int bLeftShift = aDisplay->mBlueShift + (aDisplay->mBlueBits);			

				uint32_t* aDestPtr = mBits;
				uint32_t* aSrcPtr = mNativeAlphaData;

				int aSize = mWidth*mHeight;
				for (int i = 0; i < aSize; i++)
				{
					uint32_t val = *(aSrcPtr++);

					int anAlpha = val >> 24;			

					uint32_t r = (((((val & rMask) << 8) / (anAlpha+1)) & rMask) << 8) >> rLeftShift;
					uint32_t g = (((((val & gMask) << 8) / (anAlpha+1)) & gMask) << 8) >> gLeftShift;
					uint32_t b = (((((val & bMask) << 8) / (anAlpha+1)) & bMask) << 8) >> bLeftShift;

					*(aDestPtr++) = (r << 16) | (g << 8) | (b) | (anAlpha << 24);
				}
			}
			else
			{
				memset(mBits, 0, aSize*sizeof(uint32_t));
			}
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
    SDL_SetRenderDrawBlendMode(Sexy::gRenderer, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
    SDL_RenderFillRectF(Sexy::gRenderer, &rect);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::DrawRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
    SDL_Texture* oldTarget = SDL_GetRenderTarget(Sexy::gRenderer);
    SDL_SetRenderTarget(Sexy::gRenderer, GetTexture());

    SDL_FRect rect = { (float)theRect.mX, (float)theRect.mY, (float)theRect.mWidth, (float)theRect.mHeight };
    SDL_SetRenderDrawColor(Sexy::gRenderer, theColor.mRed, theColor.mGreen, theColor.mBlue, theColor.mAlpha);
    SDL_SetRenderDrawBlendMode(Sexy::gRenderer, theDrawMode == 0 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD);
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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

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
bool MemoryImage::BltRotatedClipHelper(float &theX, float &theY, const Rect &theSrcRect, const Rect &theClipRect, double theRot, FRect &theDestRect, float theRotCenterX, float theRotCenterY)
{
	// Clipping Code (this used to be in Graphics::DrawImageRotated)
	float aCos = cosf(theRot);
	float aSin = sinf(theRot);

	// Map the four corners and find the bounding rectangle
	float px[4] = { 0, (float)theSrcRect.mWidth, (float)theSrcRect.mWidth, 0 };
	float py[4] = { 0, 0, (float)theSrcRect.mHeight, (float)theSrcRect.mHeight };
	float aMinX = 10000000;
	float aMaxX = -10000000;
	float aMinY = 10000000;
	float aMaxY = -10000000;

	for (int i=0; i<4; i++)
	{
		float ox = px[i] - theRotCenterX;
		float oy = py[i] - theRotCenterY;

		px[i] = (theRotCenterX + ox*aCos + oy*aSin) + theX;
		py[i] = (theRotCenterY + oy*aCos - ox*aSin) + theY;

		if (px[i] < aMinX)
			aMinX = px[i];
		if (px[i] > aMaxX)
			aMaxX = px[i];
		if (py[i] < aMinY)
			aMinY = py[i];
		if (py[i] > aMaxY)
			aMaxY = py[i];
	}



	FRect aClipRect(theClipRect.mX,theClipRect.mY,theClipRect.mWidth,theClipRect.mHeight);

	FRect aDestRect = FRect(aMinX, aMinY, aMaxX-aMinX, aMaxY-aMinY).Intersection(aClipRect);	
	if ((aDestRect.mWidth <= 0) || (aDestRect.mHeight <= 0)) // nothing to draw
		return false;

	theDestRect = aDestRect;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool MemoryImage::StretchBltClipHelper(const Rect &theSrcRect, const Rect &theClipRect, const Rect &theDestRect, FRect &theSrcRectOut, Rect &theDestRectOut)
{
	theDestRectOut = Rect(theDestRect.mX , theDestRect.mY, theDestRect.mWidth, theDestRect.mHeight).Intersection(theClipRect);	

	double aXFactor = theSrcRect.mWidth / (double) theDestRect.mWidth;
	double aYFactor = theSrcRect.mHeight / (double) theDestRect.mHeight;

	theSrcRectOut = FRect(theSrcRect.mX + (theDestRectOut.mX - theDestRect.mX)*aXFactor, 
				   theSrcRect.mY + (theDestRectOut.mY - theDestRect.mY)*aYFactor, 
				   theSrcRect.mWidth + (theDestRectOut.mWidth - theDestRect.mWidth)*aXFactor, 
				   theSrcRect.mHeight + (theDestRectOut.mHeight - theDestRect.mHeight)*aYFactor);

	return theSrcRectOut.mWidth>0 && theSrcRectOut.mHeight>0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool MemoryImage::StretchBltMirrorClipHelper(const Rect &theSrcRect, const Rect &theClipRect, const Rect &theDestRect, FRect &theSrcRectOut, Rect &theDestRectOut)
{
	theDestRectOut = Rect(theDestRect.mX, theDestRect.mY, theDestRect.mWidth, theDestRect.mHeight).Intersection(theClipRect);	

	double aXFactor = theSrcRect.mWidth / (double) theDestRect.mWidth;
	double aYFactor = theSrcRect.mHeight / (double) theDestRect.mHeight;

	int aTotalClip = theDestRect.mWidth - theDestRectOut.mWidth;
	int aLeftClip = theDestRectOut.mX - theDestRect.mX;
	int aRightClip = aTotalClip-aLeftClip;

	theSrcRectOut = FRect(theSrcRect.mX + (aRightClip)*aXFactor, 
				   theSrcRect.mY + (theDestRectOut.mY - theDestRect.mY)*aYFactor, 
				   theSrcRect.mWidth + (theDestRectOut.mWidth - theDestRect.mWidth)*aXFactor, 
				   theSrcRect.mHeight + (theDestRectOut.mHeight - theDestRect.mHeight)*aYFactor);

	return theSrcRectOut.mWidth>0 && theSrcRectOut.mHeight>0;
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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {
        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &clipRect);
    }

    SDL_RenderCopyExF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect, theRot * 180.0 / 3.14159265358979323846, &center, SDL_FLIP_NONE);

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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

    SDL_RenderCopyF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

//TODO: Make the special version
void MemoryImage::FastStretchBlt(Image* theImage, const Rect& theDestRect, const FRect& theSrcRect, const Color& theColor, int theDrawMode)
{
    SlowStretchBlt(theImage, theDestRect, theSrcRect, theColor, theDrawMode);
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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

    if (theClipRect.mWidth > 0 && theClipRect.mHeight > 0)
    {        SDL_Rect clipRect = { theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight };
        SDL_RenderSetClipRect(Sexy::gRenderer, &clipRect);
    }

    SDL_RenderCopyF(Sexy::gRenderer, aSrcTexture, &srcRect, &destRect);
    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::BltMatrixHelper(Image* theImage, float x, float y, const SexyMatrix3 &theMatrix, const Rect& theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, void *theSurface, int theBytePitch, int thePixelFormat, bool blend)
{
	MemoryImage *anImage = dynamic_cast<MemoryImage*>(theImage);
	if (anImage==NULL)
		return;
 
	float w2 = theSrcRect.mWidth/2.0f;
	float h2 = theSrcRect.mHeight/2.0f;

	float u0 = (float)theSrcRect.mX/theImage->mWidth;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth)/theImage->mWidth;
	float v0 = (float)theSrcRect.mY/theImage->mHeight;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight)/theImage->mHeight;

	SWHelper::XYZStruct aVerts[4] =
	{
		{ -w2,	-h2,	u0, v0, static_cast<ulong>(0xFFFFFFFF) },
		{ w2,	-h2,	u1,	v0,	static_cast<ulong>(0xFFFFFFFF) },
		{ -w2,	h2,		u0,	v1,	static_cast<ulong>(0xFFFFFFFF) },
		{ w2,	h2,		u1,	v1,	static_cast<ulong>(0xFFFFFFFF) }
	};

	for (int i=0; i<4; i++)
	{
		SexyVector3 v(aVerts[i].mX, aVerts[i].mY, 1);
		v = theMatrix*v;
		aVerts[i].mX = v.x + x - 0.5f;
		aVerts[i].mY = v.y + y - 0.5f;
	}

	SWHelper::SWDrawShape(aVerts, 4, anImage, theColor, theDrawMode, theClipRect, theSurface, theBytePitch, thePixelFormat, blend,false);
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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

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
        verts[i].color.r = 255; verts[i].color.g = 255; verts[i].color.b = 255; verts[i].color.a = 255;
    }

    int indices[6] = { 0, 1, 2, 1, 2, 3 };
    SDL_RenderGeometry(Sexy::gRenderer, aSrcTexture, verts, 4, indices, 6);

    SDL_SetRenderTarget(Sexy::gRenderer, oldTarget);
}

void MemoryImage::BltTrianglesTexHelper(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect &theClipRect, const Color &theColor, int theDrawMode, void *theSurface, int theBytePitch, int thePixelFormat, float tx, float ty, bool blend)
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
    SDL_SetTextureBlendMode(aSrcTexture, (theDrawMode == Graphics::DRAWMODE_ADDITIVE) ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);

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

void MemoryImage::FillScanLinesWithCoverage(Span* theSpans, int theSpanCount, const Color& theColor, int theDrawMode, const BYTE* theCoverage, int theCoverX, int theCoverY, int theCoverWidth, int theCoverHeight)
{
	(void)theDrawMode;(void)theCoverHeight;
	uint32_t* theBits = GetBits();
	uint32_t src = theColor.ToInt();
	for (int i = 0; i < theSpanCount; ++i)
	{
		Span* aSpan = &theSpans[i];
		int x = aSpan->mX - theCoverX;
		int y = aSpan->mY - theCoverY;

		uint32_t* aDestPixels = &theBits[aSpan->mY*mWidth + aSpan->mX];
		const BYTE* aCoverBits = &theCoverage[y*theCoverWidth+x];
		for (int w = 0; w < aSpan->mWidth; ++w)
		{
			int cover = *aCoverBits++ + 1;
			int a = (cover * theColor.mAlpha) >> 8;
			int oma;
			uint32_t dest = *aDestPixels;
							
			if (a > 0)
			{
				int aDestAlpha = dest >> 24;
				int aNewDestAlpha = aDestAlpha + ((255 - aDestAlpha) * a) / 255;
				
				a = 255 * a / aNewDestAlpha;
				oma = 256 - a;
				*(aDestPixels++) = (aNewDestAlpha << 24) |
					((((dest & 0x0000FF) * oma + (src & 0x0000FF) * a) >> 8) & 0x0000FF) |
					((((dest & 0x00FF00) * oma + (src & 0x00FF00) * a) >> 8) & 0x00FF00) |
					((((dest & 0xFF0000) * oma + (src & 0xFF0000) * a) >> 8) & 0xFF0000);
			}
		}
	}
	BitsChanged();
}

void MemoryImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles, const Rect& theClipRect, const Color &theColor, int theDrawMode, float tx, float ty, bool blend)
{
    BltTrianglesTexHelper(theTexture, theVertices, theNumTriangles, theClipRect, theColor, theDrawMode, NULL, 0, 0, tx, ty, blend);
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

	delete [] mNativeAlphaData;
	mNativeAlphaData = nullptr;

	mWantPal = true;

	return true;
}
