#include <memory>
#include "ResourceManager.h"
#include "XMLParser.h"
#include "SexyAppFramework/sound/SoundManager.h"
#include "SexyAppFramework/graphics/SDLImage.h"
#include "SexyAppFramework/graphics/TTFFont.h"
//#include "SexyAppFramework/graphics/SysFont.h"
#include "SexyAppFramework/imagelib/ImageLib.h"
#include "SexyAppFramework/SexyAppBase.h"


using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::ImageRes::DeleteResource()
{	
	mImage.Release();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::SoundRes::DeleteResource()
{
	if (mSoundId >= 0)
		gSexyAppBase->mSoundManager->ReleaseSound(mSoundId);

	mSoundId = -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::FontRes::DeleteResource()
{
	delete mFont;
	mFont = NULL;

	delete mImage;
	mImage = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ResourceManager::ResourceManager(SexyAppBase *theApp) 
{
	mApp = theApp;
	mHasFailed = false;
	mXMLParser = NULL;

	mAllowMissingProgramResources = false;
	mAllowAlreadyDefinedResources = false;
	mCurResGroupList = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ResourceManager::~ResourceManager()
{
	DeleteMap(mImageMap);
	DeleteMap(mSoundMap);
	DeleteMap(mFontMap);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::IsGroupLoaded(const std::string &theGroup)
{
	return mLoadedGroups.find(theGroup)!=mLoadedGroups.end();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteMap(ResMap &theMap)
{
	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		anItr->second->DeleteResource();
		delete anItr->second;
	}

	theMap.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteResources(ResMap &theMap, const std::string &theGroup)
{
	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		if (theGroup.empty() || anItr->second->mResGroup==theGroup)
			anItr->second->DeleteResource();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteResources(const std::string &theGroup)
{
	DeleteResources(mImageMap,theGroup);
	DeleteResources(mSoundMap,theGroup);
	DeleteResources(mFontMap,theGroup);
	mLoadedGroups.erase(theGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteExtraImageBuffers(const std::string &theGroup)
{
	for (ResMap::iterator anItr = mImageMap.begin(); anItr != mImageMap.end(); ++anItr)
	{
		if (theGroup.empty() || anItr->second->mResGroup==theGroup)
		{
			ImageRes *aRes = (ImageRes*)anItr->second;
			SDLImage *anImage = (SDLImage*)aRes->mImage;
			//if (anImage != NULL)
			//	anImage->DeleteExtraBuffers();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string ResourceManager::GetErrorText()
{
	return mError;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::HadError()
{
	return mHasFailed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::Fail(const std::string& theErrorText)
{
	if (!mHasFailed)
	{
		mHasFailed = true;
		if (mXMLParser==NULL)
		{
			mError = theErrorText;
			return false;
		}

		int aLineNum = mXMLParser->GetCurrentLineNum();

		char aLineNumStr[16];
		sprintf(aLineNumStr, "%d", aLineNum);	

		mError = theErrorText;

		if (aLineNum > 0)
			mError += std::string(" on Line ") + aLineNumStr;

		if (mXMLParser->GetFileName().length() > 0)
			mError += " in File '" + mXMLParser->GetFileName() + "'";
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseCommonResource(XMLElement &theElement, BaseRes *theRes, ResMap &theMap)
{
	mHadAlreadyDefinedError = false;

	const SexyString &aPath = theElement.mAttributes[__S("path")];
	if (aPath.empty())
		return Fail("No path specified.");

	theRes->mXMLAttributes = theElement.mAttributes;
	theRes->mFromProgram = false;
	if (aPath[0]==__S('!'))
	{
		theRes->mPath = SexyStringToStringFast(aPath);
		if (aPath==__S("!program"))
			theRes->mFromProgram = true;
	}
	else
		theRes->mPath = mDefaultPath + SexyStringToStringFast(aPath);

	
	std::string anId;
	XMLParamMap::iterator anItr = theElement.mAttributes.find(__S("id"));
	if (anItr == theElement.mAttributes.end())
		anId = mDefaultIdPrefix + GetFileName(theRes->mPath,true);
	else
		anId = mDefaultIdPrefix + SexyStringToStringFast(anItr->second);

	theRes->mResGroup = mCurResGroup;
	theRes->mId = anId;

	std::pair<ResMap::iterator,bool> aRet = theMap.insert(ResMap::value_type(anId,theRes));
	if (!aRet.second)
	{
		mHadAlreadyDefinedError = true;
		return Fail("Resource already defined.");
	}

	mCurResGroupList->push_back(theRes);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseSoundResource(XMLElement &theElement)
{
	SoundRes *aRes = new SoundRes;
	aRes->mSoundId = -1;
	aRes->mVolume = -1;
	aRes->mPanning = 0;

	if (!ParseCommonResource(theElement, aRes, mSoundMap))
	{
		if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources)
		{
			mError = "";
			mHasFailed = false;
			SoundRes *oldRes = aRes;
			aRes = (SoundRes*)mSoundMap[oldRes->mId];
			aRes->mPath = oldRes->mPath;
			aRes->mXMLAttributes = oldRes->mXMLAttributes;
			delete oldRes;
		}
		else			
		{
			delete aRes;
			return false;
		}
	}
	
	XMLParamMap::iterator anItr;

	anItr = theElement.mAttributes.find(__S("volume"));
	if (anItr != theElement.mAttributes.end())
		sexysscanf(anItr->second.c_str(),__S("%lf"),&aRes->mVolume);

	anItr = theElement.mAttributes.find(__S("pan"));
	if (anItr != theElement.mAttributes.end())
		sexysscanf(anItr->second.c_str(),__S("%d"),&aRes->mPanning);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void ReadIntVector(const SexyString &theVal, std::vector<int> &theVector)
{
	theVector.clear();

	std::string::size_type aPos = 0;
	while (true)
	{
		theVector.push_back(sexyatoi(theVal.c_str()+aPos));
		aPos = theVal.find_first_of(__S(','),aPos);
		if (aPos==std::string::npos)
			break;

		aPos++;
	}	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseImageResource(XMLElement &theElement)
{
	ImageRes *aRes = new ImageRes;
	if (!ParseCommonResource(theElement, aRes, mImageMap))
	{
		if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources)
		{
			mError = "";
			mHasFailed = false;
			ImageRes *oldRes = aRes;
			aRes = (ImageRes*)mImageMap[oldRes->mId];
			aRes->mPath = oldRes->mPath;
			aRes->mXMLAttributes = oldRes->mXMLAttributes;
			delete oldRes;
		}
		else			
		{
			delete aRes;
			return false;
		}
	}
	
	aRes->mPalletize = theElement.mAttributes.find(__S("nopal")) == theElement.mAttributes.end();
	aRes->mA4R4G4B4 = theElement.mAttributes.find(__S("a4r4g4b4")) != theElement.mAttributes.end();
	aRes->mDDSurface = theElement.mAttributes.find(__S("ddsurface")) != theElement.mAttributes.end();
	aRes->mPurgeBits = (theElement.mAttributes.find(__S("nobits")) != theElement.mAttributes.end()) ||
		((true) && (theElement.mAttributes.find(__S("nobits3d")) != theElement.mAttributes.end())) ||
		((!true) && (theElement.mAttributes.find(__S("nobits2d")) != theElement.mAttributes.end()));
	aRes->mA8R8G8B8 = theElement.mAttributes.find(__S("a8r8g8b8")) != theElement.mAttributes.end();
	aRes->mMinimizeSubdivisions = theElement.mAttributes.find(__S("minsubdivide")) != theElement.mAttributes.end();
	aRes->mAutoFindAlpha = theElement.mAttributes.find(__S("noalpha")) == theElement.mAttributes.end();	

	XMLParamMap::iterator anItr;
	anItr = theElement.mAttributes.find(__S("alphaimage"));
	if (anItr != theElement.mAttributes.end())
		aRes->mAlphaImage = mDefaultPath + SexyStringToStringFast(anItr->second);

	aRes->mAlphaColor = 0xFFFFFF;
	anItr = theElement.mAttributes.find(__S("alphacolor"));
	if (anItr != theElement.mAttributes.end())
		sexysscanf(anItr->second.c_str(),__S("%x"),&aRes->mAlphaColor);

	anItr = theElement.mAttributes.find(__S("variant"));
	if (anItr != theElement.mAttributes.end())
		aRes->mVariant = SexyStringToStringFast(anItr->second);

	anItr = theElement.mAttributes.find(__S("alphagrid"));
	if (anItr != theElement.mAttributes.end())
		aRes->mAlphaGridImage = mDefaultPath + SexyStringToStringFast(anItr->second);

	anItr = theElement.mAttributes.find(__S("rows"));
	if (anItr != theElement.mAttributes.end())
		aRes->mRows = sexyatoi(anItr->second.c_str());
	else
		aRes->mRows = 1;

	anItr = theElement.mAttributes.find(__S("cols"));
	if (anItr != theElement.mAttributes.end())
		aRes->mCols = sexyatoi(anItr->second.c_str());
	else
		aRes->mCols = 1;

	anItr = theElement.mAttributes.find(__S("anim"));
	AnimType anAnimType = AnimType_None;
	if (anItr != theElement.mAttributes.end())
	{
		const SexyChar *aType = anItr->second.c_str();

		if (strcasecmp(aType,__S("none"))==0) anAnimType = AnimType_None;
		else if (strcasecmp(aType,__S("once"))==0) anAnimType = AnimType_Once;
		else if (strcasecmp(aType,__S("loop"))==0) anAnimType = AnimType_Loop;
		else if (strcasecmp(aType,__S("pingpong"))==0) anAnimType = AnimType_PingPong;
		else 
		{
			Fail("Invalid animation type.");
			return false;
		}
	}
	aRes->mAnimInfo.mAnimType = anAnimType;
	if (anAnimType != AnimType_None)
	{
		int aNumCels = std::max(aRes->mRows,aRes->mCols);
		int aBeginDelay = 0, anEndDelay = 0;

		anItr = theElement.mAttributes.find(__S("framedelay"));
		if (anItr != theElement.mAttributes.end())
			aRes->mAnimInfo.mFrameDelay = sexyatoi(anItr->second.c_str());

		anItr = theElement.mAttributes.find(__S("begindelay"));
		if (anItr != theElement.mAttributes.end())
			aBeginDelay = sexyatoi(anItr->second.c_str());

		anItr = theElement.mAttributes.find(__S("enddelay"));
		if (anItr != theElement.mAttributes.end())
			anEndDelay = sexyatoi(anItr->second.c_str());

		anItr = theElement.mAttributes.find(__S("perframedelay"));
		if (anItr != theElement.mAttributes.end())
			ReadIntVector(anItr->second,aRes->mAnimInfo.mPerFrameDelay);

		anItr = theElement.mAttributes.find(__S("framemap"));
		if (anItr != theElement.mAttributes.end())
			ReadIntVector(anItr->second,aRes->mAnimInfo.mFrameMap);

		aRes->mAnimInfo.Compute(aNumCels,aBeginDelay,anEndDelay);
	}


	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseFontResource(XMLElement &theElement)
{
	FontRes *aRes = new FontRes;
	aRes->mFont = NULL;
	aRes->mImage = NULL;

	if (!ParseCommonResource(theElement, aRes, mFontMap))
	{
		if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources)
		{
			mError = "";
			mHasFailed = false;
			FontRes *oldRes = aRes;
			aRes = (FontRes*)mFontMap[oldRes->mId];
			aRes->mPath = oldRes->mPath;
			aRes->mXMLAttributes = oldRes->mXMLAttributes;
			delete oldRes;
		}
		else
		{
			delete aRes;
			return false;
		}
	}


	XMLParamMap::iterator anItr;
	anItr = theElement.mAttributes.find(__S("image"));
	if (anItr != theElement.mAttributes.end())
		aRes->mImagePath = SexyStringToStringFast(anItr->second);

	anItr = theElement.mAttributes.find(__S("tags"));
	if (anItr != theElement.mAttributes.end())
		aRes->mTags = SexyStringToStringFast(anItr->second);

	if (strncmp(aRes->mPath.c_str(),"!sys:",5)==0)
	{
		aRes->mSysFont = true;
		aRes->mPath = aRes->mPath.substr(5);

		anItr = theElement.mAttributes.find(__S("size"));
		if (anItr==theElement.mAttributes.end())
			return Fail("SysFont needs point size");

		aRes->mSize = sexyatoi(anItr->second.c_str());
		if (aRes->mSize<=0)
			return Fail("SysFont needs point size");

		aRes->mBold = theElement.mAttributes.find(__S("bold"))!=theElement.mAttributes.end();
		aRes->mItalic = theElement.mAttributes.find(__S("italic"))!=theElement.mAttributes.end();
		aRes->mShadow = theElement.mAttributes.find(__S("shadow"))!=theElement.mAttributes.end();
		aRes->mUnderline = theElement.mAttributes.find(__S("underline"))!=theElement.mAttributes.end();
	}
	else
		aRes->mSysFont = false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseSetDefaults(XMLElement &theElement)
{
	XMLParamMap::iterator anItr;
	anItr = theElement.mAttributes.find(__S("path"));
	if (anItr != theElement.mAttributes.end())
		mDefaultPath = RemoveTrailingSlash(SexyStringToStringFast(anItr->second)) + '/';

	anItr = theElement.mAttributes.find(__S("idprefix"));
	if (anItr != theElement.mAttributes.end())
		mDefaultIdPrefix = RemoveTrailingSlash(SexyStringToStringFast(anItr->second));	

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseResources()
{
	for (;;)
	{
		XMLElement aXMLElement;
		if (!mXMLParser->NextElement(&aXMLElement))
			return false;
		
		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue == __S("Image"))
			{
				if (!ParseImageResource(aXMLElement))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == __S("Sound"))
			{
				if (!ParseSoundResource(aXMLElement))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == __S("Font"))
			{
				if (!ParseFontResource(aXMLElement))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return Fail("Unexpected element found.");
			}
			else if (aXMLElement.mValue == __S("SetDefaults"))
			{
				if (!ParseSetDefaults(aXMLElement))
					return false;

				if (!mXMLParser->NextElement(&aXMLElement))
					return false;

				if (aXMLElement.mType != XMLElement::TYPE_END)
					return Fail("Unexpected element found.");		
			}
			else
			{
				Fail("Invalid Section '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
				return false;
			}
		}
		else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
		{
			Fail("Element Not Expected '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
			return false;
		}		
		else if (aXMLElement.mType == XMLElement::TYPE_END)
		{
			return true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoParseResources()
{
	if (!mXMLParser->HasFailed())
	{
		for (;;)
		{
			XMLElement aXMLElement;
			if (!mXMLParser->NextElement(&aXMLElement))
				break;

			if (aXMLElement.mType == XMLElement::TYPE_START)
			{
				if (aXMLElement.mValue == __S("Resources"))
				{
					mCurResGroup = SexyStringToStringFast(aXMLElement.mAttributes[__S("id")]);
					mCurResGroupList = &mResGroupMap[mCurResGroup];

					if (mCurResGroup.empty())
					{
						Fail("No id specified.");
						break;
					}

					if (!ParseResources())
						break;
				}
				else 
				{
					Fail("Invalid Section '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
					break;
				}
			}
			else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
			{
				Fail("Element Not Expected '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
				break;
			}
		}
	}

	if (mXMLParser->HasFailed())
		Fail(SexyStringToStringFast(mXMLParser->GetErrorText()));

	delete mXMLParser;
	mXMLParser = NULL;

	return !mHasFailed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseResourcesFile(const std::string& theFilename)
{
	mXMLParser = new XMLParser();
	if (!mXMLParser->OpenFile(theFilename))
		Fail("Resource file not found: " + theFilename);

	XMLElement aXMLElement;
	while (!mXMLParser->HasFailed())
	{
		if (!mXMLParser->NextElement(&aXMLElement))
			Fail(SexyStringToStringFast(mXMLParser->GetErrorText()));

		if (aXMLElement.mType == XMLElement::TYPE_START)
		{
			if (aXMLElement.mValue != __S("ResourceManifest"))
				break;
			else
				return DoParseResources();
		}
	}
		
	Fail("Expecting ResourceManifest tag");

	return DoParseResources();	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReparseResourcesFile(const std::string& theFilename)
{
	bool oldDefined = mAllowAlreadyDefinedResources;
	mAllowAlreadyDefinedResources = true;

	bool aResult = ParseResourcesFile(theFilename);

	mAllowAlreadyDefinedResources = oldDefined;

	return aResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadAlphaGridImage(ImageRes *theRes, SDLImage *theImage)
{
	return true;//已弃用
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadAlphaImage(ImageRes *theRes, SDLImage *theImage)
{
    //已弃用
	theImage->BitsChanged();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadImage(ImageRes *theRes)
{
	//bool lookForAlpha = theRes->mAlphaImage.empty() && theRes->mAlphaGridImage.empty() && theRes->mAutoFindAlpha; // unused
	
	//ImageLib::Image *anImage = ImageLib::GetImage(theRes->mPath, lookForAlpha);

	bool isNew;
	ImageLib::gAlphaComposeColor = theRes->mAlphaColor;
	SharedImageRef aSharedImageRef = gSexyAppBase->GetSharedImage(theRes->mPath, theRes->mVariant, &isNew);
	ImageLib::gAlphaComposeColor = 0xFFFFFF;

	SDLImage* aSDLImage = (SDLImage*) aSharedImageRef;
	
	if (aSDLImage == NULL)
		return Fail(StrFormat("Failed to load image: %s",theRes->mPath.c_str()));

	if (isNew)
	{
		if (!theRes->mAlphaImage.empty())
		{
			if (!LoadAlphaImage(theRes, aSharedImageRef))
				return false;
		}
		
		if (!theRes->mAlphaGridImage.empty())
		{
			if (!LoadAlphaGridImage(theRes, aSharedImageRef))
				return false;
		}
	}
	
	aSDLImage->CommitBits();
	theRes->mImage = aSharedImageRef;
	aSDLImage->mPurgeBits = theRes->mPurgeBits;

	if (theRes->mDDSurface)
	{
		aSDLImage->CommitBits();
				
		if (!aSDLImage->mHasAlpha)
		{
			//aSDLImage->mWantDDSurface = true;
			aSDLImage->mPurgeBits = true;
		}
	}	

	if (theRes->mAnimInfo.mAnimType != AnimType_None)
		aSDLImage->mAnimInfo = new AnimInfo(theRes->mAnimInfo);

	aSDLImage->mNumRows = theRes->mRows;
	aSDLImage->mNumCols = theRes->mCols;

	if (aSDLImage->mPurgeBits)
		aSDLImage->PurgeBits();

	ResourceLoadedHook(theRes);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteImage(const std::string &theName)
{
	ReplaceImage(theName,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SharedImageRef ResourceManager::LoadImage(const std::string &theName)
{
	ResMap::iterator anItr = mImageMap.find(theName);
	if (anItr == mImageMap.end())
		return NULL;

	ImageRes *aRes = (ImageRes*)anItr->second;
	if ((SDLImage*) aRes->mImage != NULL)
		return aRes->mImage;

	if (aRes->mFromProgram)
		return NULL;

	if (!DoLoadImage(aRes))
		return NULL;

	return aRes->mImage;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadSound(SoundRes* theRes)
{
	SoundRes *aRes = theRes;

	int aSoundId = mApp->mSoundManager->GetFreeSoundId();
	if (aSoundId<0)
		return Fail("Out of free sound ids");

	if(!mApp->mSoundManager->LoadSound(aSoundId, aRes->mPath))
		return Fail(StrFormat("Failed to load sound: %s",aRes->mPath.c_str()));

	if (aRes->mVolume >= 0)
		mApp->mSoundManager->SetBaseVolume(aSoundId, aRes->mVolume);

	if (aRes->mPanning != 0)
		mApp->mSoundManager->SetBasePan(aSoundId, aRes->mPanning);

	aRes->mSoundId = aSoundId;

	ResourceLoadedHook(theRes);
	return true;
}

#include <Sexy.TodLib/TodCommon.h>
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadFont(FontRes* theRes)
{
	Font *aFont = NULL;

	if (theRes->mSysFont)
	{
		/*
		bool bold = theRes->mBold, simulateBold = false;
		aFont = new SysFont(theRes->mPath,theRes->mSize,bold,theRes->mItalic,theRes->mUnderline);
		SysFont* aSysFont = (SysFont*)aFont;
		aSysFont->mDrawShadow = theRes->mShadow;
		aSysFont->mSimulateBold = simulateBold;
		*/
	}
	else if (theRes->mImagePath.empty())	
	{
		if (strncmp(theRes->mPath.c_str(),"!ref:",5)==0)
		{
			std::string aRefName = theRes->mPath.substr(5);
			Font *aRefFont = GetFont(aRefName);
			if (aRefFont==NULL)
				return Fail("Ref font not found: " + aRefName);

			aFont = aRefFont->Duplicate();
		}
		else
		{
			// Check for .ttf extension
			if (theRes->mPath.size() > 4 && theRes->mPath.substr(theRes->mPath.size() - 4) == ".ttf")
			{
				// It's a TTF font, use the new constructor
				// Assuming theRes->mSize holds the point size, which is parsed from XML for SysFonts
				int pointSize = theRes->mSize > 0 ? theRes->mSize : 14; // Default to 14 if not specified
				TTFFont* aTTFFont = new TTFFont(theRes->mPath, pointSize);
				if (!aTTFFont->IsLoaded())
				{
					delete aTTFFont;
					return Fail(StrFormat("Failed to load TTF font: %s", theRes->mPath.c_str()));
				}
				aFont = aTTFFont;
			}
			else
			{
				// Fallback to old TTFFont logic if you want to support both
				// For now, we assume we are fully switching to TTF and this path might be an error
				// If old fonts are still needed, the old TTFFont class would need to be preserved under a different name
				return Fail("Attempting to load a non-TTF font, which is not supported in this implementation.");
			}
		}
	}
	else
	{
		// This path was for image-based fonts with separate image and data files.
		// This is now deprecated in our TTF-only approach.
		return Fail("Loading fonts from separate image and data files is no longer supported.");
	}

	if (aFont == NULL)
	{
		return Fail(StrFormat("Failed to load font: %s", theRes->mPath.c_str()));
	}

	// The old checks for anImageFont and mFontData are no longer valid for the TTF implementation.
	// We can add new checks if necessary, e.g., check if the TTF_Font* inside is valid.

	theRes->mFont = aFont;

	ResourceLoadedHook(theRes);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Font* ResourceManager::LoadFont(const std::string &theName)
{
	ResMap::iterator anItr = mFontMap.find(theName);
	if (anItr == mFontMap.end())
		return NULL;

	FontRes *aRes = (FontRes*)anItr->second;
	if (aRes->mFont != NULL)
		return aRes->mFont;

	if (aRes->mFromProgram)
		return NULL;

	if (!DoLoadFont(aRes))
		return NULL;

	return aRes->mFont;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteFont(const std::string &theName)
{
	ReplaceFont(theName,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadNextResource()
{
	if (HadError())
		return false;

	if (mCurResGroupList==NULL)
		return false;

	while (mCurResGroupListItr!=mCurResGroupList->end())
	{
		BaseRes *aRes = *mCurResGroupListItr++;
		if (aRes->mFromProgram)
			continue;

		switch (aRes->mType)
		{
			case ResType_Image: 
			{
				ImageRes *anImageRes = (ImageRes*)aRes;
				if ((SDLImage*)anImageRes->mImage!=NULL)
					continue;

				return DoLoadImage(anImageRes); 
			}
			
			case ResType_Sound: 
			{
				SoundRes *aSoundRes = (SoundRes*)aRes;
				if (aSoundRes->mSoundId!=-1)
					continue;

				return DoLoadSound(aSoundRes); 
			}
			
			case ResType_Font: 
			{
				FontRes *aFontRes = (FontRes*)aRes;
				if (aFontRes->mFont!=NULL)
					continue;

				return DoLoadFont(aFontRes);
			}
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ResourceManager::ResourceLoadedHook(BaseRes*){}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::StartLoadResources(const std::string &theGroup)
{
	mError = "";
	mHasFailed = false;

	mCurResGroup = theGroup;
	mCurResGroupList = &mResGroupMap[theGroup];
	mCurResGroupListItr = mCurResGroupList->begin();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void ResourceManager::DumpCurResGroup(std::string& theDestStr)
{
	const ResList* rl = &mResGroupMap.find(mCurResGroup)->second;
	ResList::const_iterator it = rl->begin();
	theDestStr = StrFormat("About to dump %d elements from current res group name %s\r\n", rl->size(), mCurResGroup.c_str());
	
	ResList::const_iterator rl_end = rl->end();
	while (it != rl_end)
	{
		BaseRes* br = *it++;
		std::string prefix = StrFormat("%s: %s\r\n", br->mId.c_str(), br->mPath.c_str());
		theDestStr += prefix;
		if (br->mFromProgram)
			theDestStr += std::string("     res is from program\r\n");
		else if (br->mType == ResType_Image)
			theDestStr += std::string("     res is an image\r\n");
		else if (br->mType == ResType_Sound)
			theDestStr += std::string("     res is a sound\r\n");
		else if (br->mType == ResType_Font)
			theDestStr += std::string("     res is a font\r\n");

		if (it == mCurResGroupListItr)
			theDestStr += std::string("iterator has reached mCurResGroupItr\r\n");

	}

	theDestStr += std::string("Done dumping resources\r\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadResources(const std::string &theGroup)
{
	mError = "";
	mHasFailed = false;
	StartLoadResources(theGroup);
	while (LoadNextResource())
	{
	}

	if (!HadError())
	{
		mLoadedGroups.insert(theGroup);
		return true;
	}
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int	ResourceManager::GetNumResources(const std::string &theGroup, ResMap &theMap)
{
	if (theGroup.empty())
		return theMap.size();

	int aCount = 0;
	for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
	{
		BaseRes *aRes = anItr->second;
		if (aRes->mResGroup==theGroup && !aRes->mFromProgram)
			++aCount;
	}

	return aCount;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int	ResourceManager::GetNumImages(const std::string &theGroup)
{
	return GetNumResources(theGroup, mImageMap);
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int	ResourceManager::GetNumSounds(const std::string &theGroup)
{
	return GetNumResources(theGroup,mSoundMap);
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumFonts(const std::string &theGroup)
{
	return GetNumResources(theGroup, mFontMap);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int	ResourceManager::GetNumResources(const std::string &theGroup)
{
	return GetNumImages(theGroup) + GetNumSounds(theGroup) + GetNumFonts(theGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SharedImageRef ResourceManager::GetImage(const std::string &theId)
{
	ResMap::iterator anItr = mImageMap.find(theId);
	if (anItr != mImageMap.end())
		return ((ImageRes*)anItr->second)->mImage;
	else
		return NULL;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int	ResourceManager::GetSound(const std::string &theId)
{
	ResMap::iterator anItr = mSoundMap.find(theId);
	if (anItr != mSoundMap.end())
		return ((SoundRes*)anItr->second)->mSoundId;
	else
		return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Font* ResourceManager::GetFont(const std::string &theId)
{
	ResMap::iterator anItr = mFontMap.find(theId);
	if (anItr != mFontMap.end())
		return ((FontRes*)anItr->second)->mFont;
	else
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SharedImageRef ResourceManager::GetImageThrow(const std::string &theId)
{
	ResMap::iterator anItr = mImageMap.find(theId);
	if (anItr != mImageMap.end())
	{
		ImageRes *aRes = (ImageRes*)anItr->second;
		if ((SDLImage*) aRes->mImage != NULL)
			return aRes->mImage;

		if (mAllowMissingProgramResources && aRes->mFromProgram)
			return NULL;
	}


	Fail(StrFormat("Image resource not found: %s",theId.c_str()));
	throw ResourceManagerException(GetErrorText());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int	ResourceManager::GetSoundThrow(const std::string &theId)
{
	ResMap::iterator anItr = mSoundMap.find(theId);
	if (anItr != mSoundMap.end())
	{
		SoundRes *aRes = (SoundRes*)anItr->second;
		if (aRes->mSoundId!=-1)
			return aRes->mSoundId;

		if (mAllowMissingProgramResources && aRes->mFromProgram)
			return -1;
	}


	Fail(StrFormat("Sound resource not found: %s",theId.c_str()));
	throw ResourceManagerException(GetErrorText());		
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Font* ResourceManager::GetFontThrow(const std::string &theId)
{
	ResMap::iterator anItr = mFontMap.find(theId);
	if (anItr != mFontMap.end())
	{
		FontRes *aRes = (FontRes*)anItr->second;
		if (aRes->mFont!=nullptr)
			return aRes->mFont;

		if (mAllowMissingProgramResources && aRes->mFromProgram)
			return nullptr;
	}

	Fail(StrFormat("Font resource not found: %s",theId.c_str()));
	throw ResourceManagerException(GetErrorText());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::SetAllowMissingProgramImages(bool allow)
{
	mAllowMissingProgramResources = allow;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceImage(const std::string &theId, Image *theImage)
{

	ResMap::iterator anItr = mImageMap.find(theId);
	if (anItr != mImageMap.end())
	{
		anItr->second->DeleteResource();
		((ImageRes*)anItr->second)->mImage = (SDLImage*) theImage;
		((ImageRes*)anItr->second)->mImage.mOwnsUnshared = true;
		return true;
	}
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceSound(const std::string &theId, int theSound)
{
	ResMap::iterator anItr = mSoundMap.find(theId);
	if (anItr != mSoundMap.end())
	{
		anItr->second->DeleteResource();
		((SoundRes*)anItr->second)->mSoundId = theSound;
		return true;
	}
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceFont(const std::string &theId, Font *theFont)
{
	ResMap::iterator anItr = mFontMap.find(theId);
	if (anItr != mFontMap.end())
	{
		anItr->second->DeleteResource();
		((FontRes*)anItr->second)->mFont = theFont;
		return true;
	}
	else
		return false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const XMLParamMap& ResourceManager::GetImageAttributes(const std::string &theId)
{
	static XMLParamMap aStrMap;

	ResMap::iterator anItr = mImageMap.find(theId);
	if (anItr != mImageMap.end())
		return anItr->second->mXMLAttributes;
	else
		return aStrMap;
}
