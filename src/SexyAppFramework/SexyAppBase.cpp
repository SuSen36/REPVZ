//#define SEXY_TRACING_ENABLED
//#define SEXY_PERF_ENABLED
//#define SEXY_MEMTRACE

#include <string>
#include <fstream>
#include <ctime>
#include <cmath>
#include <codecvt>
#include <locale>
#include <unistd.h>

#include <SDL.h>

#include <iostream>
#include "SexyAppBase.h"
#include "SexyAppFramework/widget/WidgetManager.h"
#include "SexyAppFramework/widget/Widget.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "SexyAppFramework/graphics/MemoryImage.h"
#include <unistd.h>
#include "SexyAppFramework/widget/Dialog.h"
#include "SexyAppFramework/imagelib/ImageLib.h"
#include "SexyAppFramework/sound/SDLSoundManager.h"
#include "SexyAppFramework/sound/SDLSoundInstance.h"
#include "SexyAppFramework/misc/Rect.h"
#include "SexyAppFramework/misc/PropertiesParser.h"
#include "SexyAppFramework/misc/MTRand.h"
#include "SexyAppFramework/misc/ModVal.h"
//#include "SexyAppFramework/graphics/SysFont.h"
#include "SexyAppFramework/misc/ResourceManager.h"
#include "SexyAppFramework/sound/SDLMusicInterface.h"
#include "SexyAppFramework/misc/AutoCrit.h"
#include "SexyAppFramework/paklib/PakInterface.h"
#include "SexyAppFramework/misc/fcaseopen.h"

#include <unordered_set>

#include "SexyAppFramework/misc/RegEmu.h"
#include "SexyAppFramework/sound/DummyMusicInterface.h"

#include <filesystem>

using namespace Sexy;

SexyAppBase* Sexy::gSexyAppBase = NULL;

//////////////////////////////////////////////////////////////////////////
SexyAppBase::SexyAppBase()
{
	gSexyAppBase = this;

	SDL_Init(SDL_INIT_TIMER);
	//mMutex = NULL;
	mNotifyGameMessage = 0;

#ifdef _DEBUG
	mOnlyAllowOneCopyToRun = false;
#else
	mOnlyAllowOneCopyToRun = true;
#endif

    // Initialize mTabletPC based on platform:
    // - Android/iOS: Touch devices, enable tablet mode for drag-and-drop planting and touch input
    // - PC: Traditional mouse/keyboard, disable tablet mode (can be detected later via SDL if needed)
#ifdef ANDROID
    // Android is a touch platform, enable tablet PC mode by default
	mTabletPC = true;
#elif defined(__APPLE__) || defined(__IOS__) || defined(TARGET_OS_IPHONE)
    // iOS/Apple platforms support touch, enable tablet PC mode by default
	mTabletPC = true;
#else
    // Default to false, can be detected later after SDL initialization
    mTabletPC = false;
#endif

#ifdef ANDROID
    mChangeDirTo = "/storage/emulated/0/Android/data/com.popcap.pvz/files/";
#else
	mChangeDirTo = "./";
#endif

	mNoDefer = false;
	mFullScreenPageFlip = true; // should we page flip in fullscreen?
	mTimeLoaded = SDL_GetTicks();
	mProdName = "Product";
	mTitle = __S("SexyApp");
	mShutdown = false;
	mExitToTop = false;
    mWidth = 1066;
    mHeight = 600;
	mFullscreenBits = 16;
	mIsWindowed = true;
	mIsPhysWindowed = true;
	mFullScreenWindow = false;
	mPreferredX = -1;
	mPreferredY = -1;
	mAllowMonitorPowersave = true;
	//mHWnd = NULL;
	mMusicInterface = NULL;
	mFrameTime = 10;
	mNonDrawCount = 0;
	mDrawCount = 0;
	mSleepCount = 0;
	mUpdateCount = 0;
	mUpdateAppState = 0;
	mUpdateAppDepth = 0;
	mPendingUpdatesAcc = 0.0;
	mUpdateFTimeAcc = 0.0;
	mHasPendingDraw = true;
	mIsDrawing = false;
	mLastDrawWasEmpty = false;
	mLastTimeCheck = 0;
	mUpdateMultiplier = 1;
	mPaused = false;
	mSoundManager = NULL;
	mCursorNum = CURSOR_POINTER;
	mMouseIn = false;
	mRunning = false;
	mActive = true;
	mProcessInTimer = false;
	mMinimized = false;
	mPhysMinimized = false;
	mIsDisabled = false;
	mLoaded = false;
	mYieldMainThread = false;
	mLoadingFailed = false;
	mLoadingThreadStarted = false;
	mAutoStartLoadingThread = true;
	mLoadingThreadCompleted = false;
	mNumLoadingThreadTasks = 0;
	mCompletedLoadingThreadTasks = 0;
	mLastDrawTick = SDL_GetTicks();
	mNextDrawTick = SDL_GetTicks();
	mForceFullscreen = false;
	mForceWindowed = false;
	mHasFocus = true;
	mCustomCursorsEnabled = false;
	mCustomCursorDirty = false;
	//mOverrideCursor = NULL;
	mIsOpeningURL = false;
	mInitialized = false;
	mLastShutdownWasGraceful = true;
	mReadFromRegistry = false;
	mSkipSignatureChecks = false;
	mCtrlDown = false;
	mAltDown = false;
	mAllowAltEnter = true;
	mStepMode = 0;
	mCleanupSharedImages = false;
	mStandardWordWrap = true;
	mbAllowExtendedChars = true;
	mEnableMaximizeButton = false;
	mWriteToSexyCache = true;
	mSexyCacheBuffers = false;

	mMusicVolume = 0.85;
	mSfxVolume = 0.85;
	mMuteCount = 0;
	mAutoMuteCount = 0;
	mMuteOnLostFocus = false;
	mCurHandleNum = 0;
	mFPSTime = 0;
	mFPSStartTick = SDL_GetTicks();
	mFPSFlipCount = 0;
	mFPSCount = 0;
	mFPSDirtyCount = 0;
	mDrawTime = 0;
	mScreenBltTime = 0;
	mAlphaDisabled = false;
	mDebugKeysEnabled = false;
	mOldWndProc = 0;
	mNoSoundNeeded = false;
	mWantFMod = false;

	mSyncRefreshRate = 100;
	mVSyncUpdates = false;
	mVSyncBroken = false;
	mVSyncBrokenCount = 0;
	mVSyncBrokenTestStartTick = 0;
	mVSyncBrokenTestUpdates = 0;
	mWaitForVSync = false;
	mSoftVSyncWait = true;
	mRelaxUpdateBacklogCount = 0;
	mWidescreenAware = false;
	mEnableWindowAspect = false;
	mWindowAspect.Set(4, 3);
	mIsWideWindow = false;

	mWindow = 0;
	mContext = 0;
	mSurface = 0;

	int i;

	for (i = 0; i < NUM_CURSORS; i++)
		mCursorImages[i] = NULL;

	for (i = 0; i < 256; i++)
		mAdd8BitMaxTable[i] = i;

	for (i = 256; i < 512; i++)
		mAdd8BitMaxTable[i] = 255;

	mManualShutdown = false;

	mWidgetManager = new WidgetManager(this);
	mResourceManager = new ResourceManager(this);

	mPrimaryThreadId = 0;
}

SexyAppBase::~SexyAppBase()
{
	Shutdown();



	DialogMap::iterator aDialogItr = mDialogMap.begin();
	while (aDialogItr != mDialogMap.end())
	{
		mWidgetManager->RemoveWidget(aDialogItr->second);
		delete aDialogItr->second;
		++aDialogItr;
	}
	mDialogMap.clear();
	mDialogList.clear();

	/*
	if (mInvisHWnd != NULL)
	{
		HWND aWindow = mInvisHWnd;
		mInvisHWnd = NULL;
		SetWindowLongPtr(aWindow, GWLP_USERDATA, 0);
		DestroyWindow(aWindow);
	}
*/

	delete mWidgetManager;
	delete mResourceManager;

	SharedImageMap::iterator aSharedImageItr = mSharedImageMap.begin();
	while (aSharedImageItr != mSharedImageMap.end())
	{
		SharedImage* aSharedImage = &aSharedImageItr->second;
		//TOD_ASSERT(aSharedImage->mRefCount == 0);
		delete aSharedImage->mImage;
		mSharedImageMap.erase(aSharedImageItr++);
	}

	delete mMusicInterface;
	delete mSoundManager;

	if (mSurface)
	{
		SDL_DestroyTexture((SDL_Texture*)mSurface);
		mSurface = NULL;
	}

	if (mContext)
	{
		SDL_DestroyRenderer((SDL_Renderer*)mContext);
		mContext = NULL;
	}

	WaitForLoadingThread();

	//DestroyCursor(mHandCursor);
	//DestroyCursor(mDraggingCursor);

	gSexyAppBase = NULL;


	//if (mMutex != NULL)
		//::CloseHandle(mMutex);

	/*
	FreeLibrary(gDDrawDLL);
	FreeLibrary(gDSoundDLL);
	FreeLibrary(gVersionDLL);
	*/
}

void SexyAppBase::ClearUpdateBacklog(bool relaxForASecond)
{
	mLastTimeCheck = SDL_GetTicks();
	mUpdateFTimeAcc = 0.0;

	if (relaxForASecond)
		mRelaxUpdateBacklogCount = 1000;
}

bool SexyAppBase::AppCanRestore()
{
	return !mIsDisabled;
}

Dialog* SexyAppBase::NewDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode)
{
	Dialog* aDialog = new Dialog(NULL, NULL, theDialogId, isModal, theDialogHeader,	theDialogLines, theDialogFooter, theButtonMode);
	return aDialog;
}

Dialog* SexyAppBase::DoDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode)
{
	KillDialog(theDialogId);

	Dialog* aDialog = NewDialog(theDialogId, isModal, theDialogHeader, theDialogLines, theDialogFooter, theButtonMode);

	AddDialog(theDialogId, aDialog);

	return aDialog;
}


Dialog*	SexyAppBase::GetDialog(int theDialogId)
{
	DialogMap::iterator anItr = mDialogMap.find(theDialogId);

	if (anItr != mDialogMap.end())
		return anItr->second;

	return NULL;
}

bool SexyAppBase::KillDialog(int theDialogId, bool removeWidget, bool deleteWidget)
{
	DialogMap::iterator anItr = mDialogMap.find(theDialogId);

	if (anItr != mDialogMap.end())
	{
		Dialog* aDialog = anItr->second;

		// set the result to something else so DoMainLoop knows that the dialog is gone
		// in case nobody else sets mResult
		if (aDialog->mResult == -1)
			aDialog->mResult = 0;

		DialogList::iterator aListItr = std::find(mDialogList.begin(),mDialogList.end(),aDialog);
		if (aListItr != mDialogList.end())
			mDialogList.erase(aListItr);

		mDialogMap.erase(anItr);

		if (removeWidget || deleteWidget)
		mWidgetManager->RemoveWidget(aDialog);

		if (aDialog->IsModal())
		{
			ModalClose();
			mWidgetManager->RemoveBaseModal(aDialog);
		}

		if (deleteWidget)
		SafeDeleteWidget(aDialog);

		return true;
	}

	return false;
}

bool SexyAppBase::KillDialog(int theDialogId)
{
	return KillDialog(theDialogId,true,true);
}

bool SexyAppBase::KillDialog(Dialog* theDialog)
{
	return KillDialog(theDialog->mId);
}

int SexyAppBase::GetDialogCount()
{
	return mDialogMap.size();
}

void SexyAppBase::AddDialog(int theDialogId, Dialog* theDialog)
{
	KillDialog(theDialogId);

	if (theDialog->mWidth == 0)
	{
		// Set the dialog position ourselves
		int aWidth = mWidth/2;
		theDialog->Resize((mWidth - aWidth)/2, mHeight / 5, aWidth, theDialog->GetPreferredHeight(aWidth));
	}

	mDialogMap.insert(DialogMap::value_type(theDialogId, theDialog));
	mDialogList.push_back(theDialog);

	mWidgetManager->AddWidget(theDialog);
	if (theDialog->IsModal())
	{
		mWidgetManager->AddBaseModal(theDialog);
		ModalOpen();
	}
}

void SexyAppBase::AddDialog(Dialog* theDialog)
{
	AddDialog(theDialog->mId, theDialog);
}

void SexyAppBase::ModalOpen()
{
}

void SexyAppBase::ModalClose()
{
}

void SexyAppBase::DialogButtonPress(int theDialogId, int theButtonId)
{
	if (theButtonId == Dialog::ID_YES)
		ButtonPress(2000 + theDialogId);
	else if (theButtonId == Dialog::ID_NO)
		ButtonPress(3000 + theDialogId);
}

void SexyAppBase::DialogButtonDepress(int theDialogId, int theButtonId)
{
	if (theButtonId == Dialog::ID_YES)
		ButtonDepress(2000 + theDialogId);
	else if (theButtonId == Dialog::ID_NO)
		ButtonDepress(3000 + theDialogId);
}

void SexyAppBase::GotFocus()
{
}

void SexyAppBase::LostFocus()
{
}

void SexyAppBase::URLOpenFailed(const std::string& theURL)
{
	(void)theURL;
	mIsOpeningURL = false;
}

void SexyAppBase::URLOpenSucceeded(const std::string& theURL)
{
	(void)theURL;
	mIsOpeningURL = false;

	if (mShutdownOnURLOpen)
		Shutdown();
}

bool SexyAppBase::OpenURL(const std::string& theURL, bool shutdownOnOpen)
{
	if ((!mIsOpeningURL) || (theURL != mOpeningURL))
	{
		mShutdownOnURLOpen = shutdownOnOpen;
		mIsOpeningURL = true;
		mOpeningURL = theURL;
		mOpeningURLTime = SDL_GetTicks();

        /*
		if ((intptr_t) ShellExecuteA(NULL, "open", theURL.c_str(), NULL, NULL, SW_SHOWNORMAL) > 32)
		{
			return true;
		}
		else
		{
			URLOpenFailed(theURL);
			return false;
		}
        */
	}

	return true;
}

std::string SexyAppBase::GetProductVersion(const std::string& thePath)
{
	return "0";
}

void SexyAppBase::WaitForLoadingThread()
{
    int ms = 20;
    while ((mLoadingThreadStarted) && (!mLoadingThreadCompleted))
        SDL_Delay(ms);
}

void SexyAppBase::SetCursorImage(int theCursorNum, Image* theImage)
{
	if ((theCursorNum >= 0) && (theCursorNum < NUM_CURSORS))
	{
		mCursorImages[theCursorNum] = theImage;
		EnforceCursor();
	}
}

double SexyAppBase::GetLoadingThreadProgress()
{
	if (mLoaded)
		return 1.0;
	if (!mLoadingThreadStarted)
		return 0.0;
	if (mNumLoadingThreadTasks == 0)
		return 0.0;
	return std::min(mCompletedLoadingThreadTasks / (double) mNumLoadingThreadTasks, 1.0);
}

bool SexyAppBase::RegistryWrite(const std::string& theValueName, uint32_t theType, const uchar* theValue, uint32_t theLength)
{
	if (mRegKey.length() == 0)
		return false;


	//HKEY aGameKey;

	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
	std::string aValueName;

	int aSlashPos = (int) theValueName.rfind('\\');
	if (aSlashPos != -1)
	{
		aKeyName += "\\" + theValueName.substr(0, aSlashPos);
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

	/*
	int aResult = RegOpenKeyExA(HKEY_CURRENT_USER, aKeyName.c_str(), 0, KEY_WRITE, &aGameKey);
	if (aResult != ERROR_SUCCESS)
	{
		uint32_t aDisp;
		aResult = RegCreateKeyExA(HKEY_CURRENT_USER, aKeyName.c_str(), 0, (char *)"Key", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, NULL, &aGameKey, &aDisp);
	}

	if (aResult != ERROR_SUCCESS)
	{
		return false;
	}

	RegSetValueExA(aGameKey, aValueName.c_str(), 0, theType, theValue, theLength);
	RegCloseKey(aGameKey);
	*/

	if (!regemu::RegistryWrite(aKeyName, aValueName, theType, theValue, theLength))
	{
		return false;
	}

	return true;
}

bool SexyAppBase::RegistryWriteString(const std::string& theValueName, const std::string& theString)
{
	return RegistryWrite(theValueName, regemu::REGEMU_SZ, (uchar*) theString.c_str(), theString.length());
}

bool SexyAppBase::RegistryWriteInteger(const std::string& theValueName, int theValue)
{
	return RegistryWrite(theValueName, regemu::REGEMU_DWORD, (uchar*) &theValue, sizeof(int));
}

bool SexyAppBase::RegistryWriteBoolean(const std::string& theValueName, bool theValue)
{
	int aValue = theValue ? 1 : 0;
	return RegistryWrite(theValueName, regemu::REGEMU_DWORD, (uchar*) &aValue, sizeof(int));
}

bool SexyAppBase::RegistryWriteData(const std::string& theValueName, const uchar* theValue, uint32_t theLength)
{
	return RegistryWrite(theValueName, regemu::REGEMU_BINARY, (uchar*) theValue, theLength);
}

void SexyAppBase::WriteToRegistry()
{
	RegistryWriteInteger("MusicVolume", (int) (mMusicVolume * 100));
	RegistryWriteInteger("SfxVolume", (int) (mSfxVolume * 100));
	RegistryWriteInteger("Muted", (mMuteCount - mAutoMuteCount > 0) ? 1 : 0);
	RegistryWriteInteger("ScreenMode", mIsWindowed ? 0 : 1);
	RegistryWriteInteger("PreferredX", mPreferredX);
	RegistryWriteInteger("PreferredY", mPreferredY);
	RegistryWriteInteger("CustomCursors", mCustomCursorsEnabled ? 1 : 0);
	RegistryWriteInteger("InProgress", 0);
	RegistryWriteBoolean("WaitForVSync", mWaitForVSync);
}

bool SexyAppBase::RegistryEraseKey(const SexyString& _theKeyName)
{
	std::string theKeyName = SexyStringToStringFast(_theKeyName);
	if (mRegKey.length() == 0)
		return false;
	
	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey) + "\\" + theKeyName;

	if (!regemu::RegistryEraseKey(aKeyName))
	{
		return false;
	}

	return true;
}

void SexyAppBase::RegistryEraseValue(const SexyString& _theValueName)
{
	std::string theValueName = SexyStringToStringFast(_theValueName);
	if (mRegKey.length() == 0)
		return;

	//HKEY aGameKey;
	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
	std::string aValueName;

	int aSlashPos = (int) theValueName.rfind('\\');
	if (aSlashPos != -1)
	{
		aKeyName += "\\" + theValueName.substr(0, aSlashPos);
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}

	/*
	int aResult = RegOpenKeyExA(HKEY_CURRENT_USER, aKeyName.c_str(), 0, KEY_WRITE, &aGameKey);
	if (aResult == ERROR_SUCCESS)
	{
		RegDeleteValueA(aGameKey, aValueName.c_str());
		RegCloseKey(aGameKey);
	}
	*/

	regemu::RegistryEraseValue(aKeyName, aValueName);
}

/*
bool SexyAppBase::RegistryGetSubKeys(const std::string& theKeyName, StringVector* theSubKeys)
{
	theSubKeys->clear();

	if (mRegKey.length() == 0)
		return false;

	if (mPlayingDemoBuffer)
	{
		if (mManualShutdown)
			return true;

		PrepareDemoCommand(true);
		mDemoNeedsCommand = true;

		TOD_ASSERT(!mDemoIsShortCmd);
		TOD_ASSERT(mDemoCmdNum == DEMO_REGISTRY_GETSUBKEYS);

		bool success = mDemoBuffer.ReadNumBits(1, false) != 0;
		if (!success)
			return false;

		int aNumKeys = mDemoBuffer.ReadLong();

		for (int i = 0; i < aNumKeys; i++)
			theSubKeys->push_back(mDemoBuffer.ReadString());

		return true;
	}
	else
	{
		HKEY aKey;

		std::string aKeyName = RemoveTrailingSlash(RemoveTrailingSlash("SOFTWARE\\" + mRegKey) + "\\" + theKeyName);
		int aResult = RegOpenKeyExA(HKEY_CURRENT_USER, aKeyName.c_str(), 0, KEY_READ, &aKey);

		if (aResult == ERROR_SUCCESS)
		{
			for (int anIdx = 0; ; anIdx++)
			{
				char aStr[1024];

				aResult = RegEnumKeyA(aKey, anIdx, aStr, 1024);
				if (aResult != ERROR_SUCCESS)
					break;

				theSubKeys->push_back(aStr);
			}

			RegCloseKey(aKey);

			if (mRecordingDemoBuffer)
			{
				WriteDemoTimingBlock();
				mDemoBuffer.WriteNumBits(0, 1);
				mDemoBuffer.WriteNumBits(DEMO_REGISTRY_GETSUBKEYS, 5);
				mDemoBuffer.WriteNumBits(1, 1); // success
				mDemoBuffer.WriteLong(theSubKeys->size());

				for (int i = 0; i < (int) theSubKeys->size(); i++)
					mDemoBuffer.WriteString((*theSubKeys)[i]);
			}

			return true;
		}
		else
		{
			if (mRecordingDemoBuffer)
			{
				WriteDemoTimingBlock();
				mDemoBuffer.WriteNumBits(0, 1);
				mDemoBuffer.WriteNumBits(DEMO_REGISTRY_GETSUBKEYS, 5);
				mDemoBuffer.WriteNumBits(0, 1); // failure
			}

			return false;
		}
	}
}
*/

bool SexyAppBase::RegistryRead(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength)
{
	return RegistryReadKey(theValueName, theType, theValue, theLength);
}

bool SexyAppBase::RegistryReadKey(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength)
{
	if (mRegKey.length() == 0)
		return false;
		
	std::string aKeyName = RemoveTrailingSlash("SOFTWARE\\" + mRegKey);
	std::string aValueName;

	int aSlashPos = (int) theValueName.rfind('\\');
	if (aSlashPos != -1)
	{
		aKeyName += "\\" + theValueName.substr(0, aSlashPos);
		aValueName = theValueName.substr(aSlashPos + 1);
	}
	else
	{
		aValueName = theValueName;
	}		

	if (regemu::RegistryRead(aKeyName, aValueName, (uint32_t*)theType, theValue, (uint32_t*)theLength))
	{
		return true;
	}
	
	return false;
}

// aStr isn't initialised lmao
bool SexyAppBase::RegistryReadString(const std::string& theKey, std::string* theString)
{
	char aStr[1024];

	uint32_t aType;
	uint32_t aLen = sizeof(aStr) - 1;
	if (!RegistryRead(theKey, &aType, (uchar*) aStr, &aLen))
		return false;

	if (aType != regemu::REGEMU_SZ)
		return false;

	aStr[aLen] = 0;

	*theString = aStr;
	return true;
}

bool SexyAppBase::RegistryReadInteger(const std::string& theKey, int* theValue)
{
	uint32_t aType;
	uint32_t aLong;
	uint32_t aLen = 4;
	if (!RegistryRead(theKey, &aType, (uchar*) &aLong, &aLen))
		return false;

	if (aType != regemu::REGEMU_DWORD)
		return false;

	*theValue = aLong;
	return true;
}

bool SexyAppBase::RegistryReadBoolean(const std::string& theKey, bool* theValue)
{
	int aValue;
	if (!RegistryReadInteger(theKey, &aValue))
		return false;

	*theValue = aValue != 0;
	return true;
}

bool SexyAppBase::RegistryReadData(const std::string& theKey, uchar* theValue, uint32_t* theLength)
{
	uint32_t aType;
	if (!RegistryRead(theKey, &aType, (uchar*) theValue, theLength))
		return false;

	if (aType != regemu::REGEMU_BINARY)
		return false;

	return true;
}

void SexyAppBase::ReadFromRegistry()
{
	mReadFromRegistry = true;
	mRegKey = SexyStringToString(GetString("RegistryKey", StringToSexyString(mRegKey)));

	if (mRegKey.length() == 0)
		return;

	regemu::SetRegFile("config.regemu");

	int anInt;
	if (RegistryReadInteger("MusicVolume", &anInt))
		mMusicVolume = anInt / 100.0;

	if (RegistryReadInteger("SfxVolume", &anInt))
		mSfxVolume = anInt / 100.0;

	if (RegistryReadInteger("Muted", &anInt))
		mMuteCount = anInt;

	if (RegistryReadInteger("ScreenMode", &anInt))
		mIsWindowed = anInt == 0;

	RegistryReadInteger("PreferredX", &mPreferredX);
	RegistryReadInteger("PreferredY", &mPreferredY);

	if (RegistryReadInteger("CustomCursors", &anInt))
		EnableCustomCursors(anInt != 0);

	RegistryReadBoolean("WaitForVSync", &mWaitForVSync);

	if (RegistryReadInteger("InProgress", &anInt))
		mLastShutdownWasGraceful = anInt == 0;

	RegistryWriteInteger("InProgress", 1);
}

bool SexyAppBase::WriteBytesToFile(const std::string& theFileName, const void *theData, ulong theDataLen)
{
	MkDir(GetFileDir(theFileName));
	FILE* aFP = fcaseopen(theFileName.c_str(), "w+b");

	if (aFP == NULL)
	{
		return false;
	}

	fwrite(theData, 1, theDataLen, aFP);
	fclose(aFP);

	if (mSexyCacheBuffers && mWriteToSexyCache)
	{
		GetFullPath(theFileName);
		/*
		unsigned char* aSetData = (unsigned char*)gSexyCache->AllocSetData(GetFullPath(theFileName), "Buffer", theDataLen + 1);
		if (aSetData)
		{
			*aSetData = 1;
			memcpy(aSetData, theData, theDataLen);
			gSexyCache->SetData(aSetData);
			gSexyCache->FreeSetData(aSetData);
			gSexyCache->SetFileDeps(GetFullPath(theFileName), "Buffer", GetFullPath(theFileName));
		}
		*/
	}

	return true;
}

bool SexyAppBase::WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer)
{
	return WriteBytesToFile(theFileName,theBuffer->GetDataPtr(),theBuffer->GetDataLen());
}

bool SexyAppBase::ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer)
{
	PFILE* aFP = p_fopen(theFileName.c_str(), "rb");

	if (aFP == NULL)
	{
		return false;
	}
	
	p_fseek(aFP, 0, SEEK_END);
	int aFileSize = p_ftell(aFP);
	p_fseek(aFP, 0, SEEK_SET);
	
	uchar* aData = new uchar[aFileSize];

	p_fread(aData, 1, aFileSize, aFP);
	p_fclose(aFP);

	theBuffer->Clear();
	theBuffer->SetData(aData, aFileSize);

	delete [] aData;

	return true;
}

bool SexyAppBase::FileExists(const std::string& theFileName)
{
	PFILE* aFP = p_fopen(theFileName.c_str(), "rb");

	if (aFP == NULL)		
		return false;		
	
	p_fclose(aFP);
	return true;
}

bool SexyAppBase::EraseFile(const std::string& theFileName)
{
	return remove(theFileName.c_str()) == 0;
}

void SexyAppBase::ShutdownHook()
{
}

void SexyAppBase::Shutdown()
{
	if ((mPrimaryThreadId != 0) && ((void*)SDL_GetThreadID != mPrimaryThreadId))
	{
		mLoadingFailed = true;
	}
	else if (!mShutdown)
	{
		mExitToTop = true;
		mShutdown = true;
		ShutdownHook();

		if (mMusicInterface != NULL)
			mMusicInterface->StopAllMusic();

		/*
		if (mDDInterface->mDD != NULL)
		{
			mDDInterface->mDD->RestoreDisplayMode();
		}

		if (mHWnd != NULL)
		{
			ShowWindow(mHWnd, SW_HIDE);
		}
		*/

		RestoreScreenResolution();

		if (mReadFromRegistry)
			WriteToRegistry();

		//ImageLib::CloseJPEG2000();
	}
}

void SexyAppBase::RestoreScreenResolution()
{
    /*
	if (mFullScreenWindow)
	{
		EnumWindows(ChangeDisplayWindowEnumProc,0); // get any windows that appeared while we were running
		ChangeDisplaySettings(NULL,0);
		EnumWindows(ChangeDisplayWindowEnumProc,1); // restore window pos
		mFullScreenWindow = false;
	}
    */
}

void SexyAppBase::DoExit(int theCode)
{
	RestoreScreenResolution();
	exit(theCode);
}

void SexyAppBase::UpdateFrames()
{
	mUpdateCount++;

	if (!mMinimized)
	{
		if (mWidgetManager->UpdateFrame())
			++mFPSDirtyCount;
	}

	mMusicInterface->Update();
	CleanSharedImages();
}

void SexyAppBase::DoUpdateFramesF(float theFrac)
{
	if ((mVSyncUpdates) && (!mMinimized))
		mWidgetManager->UpdateFrameF(theFrac);
}

bool SexyAppBase::DoUpdateFrames()
{
	if ((mLoadingThreadCompleted) && (!mLoaded))
	{
		//::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		mLoaded = true;
		mYieldMainThread = false;
		LoadingThreadCompleted();
	}

	UpdateFrames();
	return true;
}

bool gIsFailing = false;

void SexyAppBase::Redraw(Rect* theClipRect)
{
	// Do mIsDrawing check because we could enter here at a bad time if any windows messages
	//  are processed during WidgetManager->Draw
	if ((mIsDrawing) || (mShutdown))
		return;

	if (mContext)
	{
		SDL_Renderer* aRenderer = (SDL_Renderer*)mContext;
		MemoryImage* aScreenImage = dynamic_cast<MemoryImage*>(mWidgetManager->mImage);
		if (aScreenImage)
		{
			SDL_Texture* aTexture = aScreenImage->GetTexture();
			if (aTexture)
			{
				SDL_SetRenderTarget(aRenderer, NULL);
				SDL_SetRenderDrawColor(aRenderer, 0, 0, 0, 255);
				SDL_RenderClear(aRenderer);
				SDL_RenderCopy(aRenderer, aTexture, NULL, NULL);
				SDL_RenderPresent(aRenderer);
			}
		}
	}
	mFPSFlipCount++;
}

bool SexyAppBase::DrawDirtyStuff()
{
	MTAutoDisallowRand aDisallowRand;

	if (gIsFailing) // just try to reinit
	{
		Redraw(nullptr);
		mHasPendingDraw = false;
		mLastDrawWasEmpty = true;
		return false;
	}

	DWORD aStartTime = SDL_GetTicks();

	mIsDrawing = true;
	bool drewScreen = mWidgetManager->DrawScreen();
	mIsDrawing = false;

	if ((drewScreen || (aStartTime - mLastDrawTick >= 1000) || (mCustomCursorDirty)) &&
		((int) (aStartTime - mNextDrawTick) >= 0))
	{
		mLastDrawWasEmpty = false;

		mDrawCount++;

		DWORD aMidTime = SDL_GetTicks();

		mFPSCount++;
		mFPSTime += aMidTime - aStartTime;

		mDrawTime += aMidTime - aStartTime;

		DWORD aPreScreenBltTime = SDL_GetTicks();
		mLastDrawTick = aPreScreenBltTime;

		Redraw(NULL);

		// This is our one UpdateFTimeAcc if we are vsynched
		UpdateFTimeAcc();

		DWORD aEndTime = SDL_GetTicks();

		mScreenBltTime = aEndTime - aPreScreenBltTime;


		if ((mLoadingThreadStarted) && (!mLoadingThreadCompleted))
		{
			int aTotalTime = aEndTime - aStartTime;

			mNextDrawTick += 35 + std::max(aTotalTime, 15);

			if ((int) (aEndTime - mNextDrawTick) >= 0)
				mNextDrawTick = aEndTime;
		}
		else
			mNextDrawTick = aEndTime;

		mHasPendingDraw = false;
		mCustomCursorDirty = false;

		return true;
	}
	else
	{
		mHasPendingDraw = false;
		mLastDrawWasEmpty = true;
		return false;
	}
}


void SexyAppBase::BeginPopup()
{
}

void SexyAppBase::EndPopup()
{
	if (!mIsPhysWindowed)
		mNoDefer = false;

	ClearUpdateBacklog();
	ClearKeysDown();

	if (mWidgetManager->mDownButtons)
	{
		mWidgetManager->DoMouseUps();
		//ReleaseCapture();
	}
}

int SexyAppBase::MsgBox(const std::string& theText, const std::string& theTitle, int theFlags)
{
	BeginPopup();

	const int iconMask = theFlags & 0x000000F0; // MB_ICON* on Win32
	Uint32 sdlFlags = SDL_MESSAGEBOX_INFORMATION;
	switch (iconMask)
	{
	case 0x00000010: // MB_ICONHAND / ERROR
		sdlFlags = SDL_MESSAGEBOX_ERROR;
		break;
	case 0x00000020: // MB_ICONQUESTION
	case 0x00000030: // MB_ICONEXCLAMATION / WARNING
		sdlFlags = SDL_MESSAGEBOX_WARNING;
		break;
	case 0x00000040: // MB_ICONASTERISK / INFORMATION
	default:
		sdlFlags = SDL_MESSAGEBOX_INFORMATION;
		break;
	}

	if (SDL_ShowSimpleMessageBox(sdlFlags, theTitle.c_str(), theText.c_str(), nullptr) != 0)
	{
		// Fallback to console if SDL message box cannot be shown (e.g., headless)
		printf("%s\n===\n%s\n", theTitle.c_str(), theText.c_str());
	}

	EndPopup();

	return 0;
}

int SexyAppBase::MsgBox(const std::wstring& theText, const std::wstring& theTitle, int theFlags)
{

	BeginPopup();
	const int iconMask = theFlags & 0x000000F0; // MB_ICON* on Win32
	Uint32 sdlFlags = SDL_MESSAGEBOX_INFORMATION;
	switch (iconMask)
	{
	case 0x00000010: // MB_ICONHAND / ERROR
		sdlFlags = SDL_MESSAGEBOX_ERROR;
		break;
	case 0x00000020: // MB_ICONQUESTION
	case 0x00000030: // MB_ICONEXCLAMATION / WARNING
		sdlFlags = SDL_MESSAGEBOX_WARNING;
		break;
	case 0x00000040: // MB_ICONASTERISK / INFORMATION
	default:
		sdlFlags = SDL_MESSAGEBOX_INFORMATION;
		break;
	}

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf8conv;
	std::string utf8Title = utf8conv.to_bytes(theTitle);
	std::string utf8Text = utf8conv.to_bytes(theText);

	if (SDL_ShowSimpleMessageBox(sdlFlags, utf8Title.c_str(), utf8Text.c_str(), nullptr) != 0)
	{
		// Fallback to console if SDL message box cannot be shown (e.g., headless)
		wprintf(L"%s\n===\n%s\n", theTitle.c_str(), theText.c_str());
	}

	EndPopup();

	return 0;
}

void SexyAppBase::Popup(const std::string& theString)
{
	BeginPopup();
	if (!mShutdown)
		printf("FATAL ERROR\n===\n%s\n", theString.c_str());

	EndPopup();
}

void SexyAppBase::Popup(const std::wstring& theString)
{
	BeginPopup();
	if (!mShutdown)
		wprintf(L"FATAL ERROR\n===\n%s\n", theString.c_str());

	EndPopup();
}

void SexyAppBase::SafeDeleteWidget(Widget* theWidget)
{
	WidgetSafeDeleteInfo aWidgetSafeDeleteInfo;
	aWidgetSafeDeleteInfo.mUpdateAppDepth = mUpdateAppDepth;
	aWidgetSafeDeleteInfo.mWidget = theWidget;
	mSafeDeleteList.push_back(aWidgetSafeDeleteInfo);
}

void SexyAppBase::HandleNotifyGameMessage(int theType)
{
}

void SexyAppBase::RehupFocus()
{
	bool wantHasFocus = mActive && !mMinimized;

	if (wantHasFocus != mHasFocus)
	{
		mHasFocus = wantHasFocus;

		if (mHasFocus)
		{
			if (mMuteOnLostFocus)
				Unmute(true);

			mWidgetManager->GotFocus();
			GotFocus();
		}
		else
		{
			if (mMuteOnLostFocus)
				Mute(true);

			mWidgetManager->LostFocus();
			LostFocus();

			//ReleaseCapture();
			mWidgetManager->DoMouseUps();
		}
	}
}

void SexyAppBase::ClearKeysDown()
{
	if (mWidgetManager != NULL) // fix stuck alt-key problem
	{
		for (int aKeyNum = 0; aKeyNum < 0xFF; aKeyNum++)
			mWidgetManager->mKeyDown[aKeyNum] = false;
	}
	mCtrlDown = false;
	mAltDown = false;
}

bool SexyAppBase::DebugKeyDown(int theKey)
{
	return false;
}


void SexyAppBase::CloseRequestAsync()
{
}

// return file name that you want to upload
std::string	SexyAppBase::NotifyCrashHook()
{
	return "";
}

void SexyAppBase::DeleteNativeImageData()
{
	AutoCrit anAutoCrit(mCritSect);

	for (MemoryImage* aMemoryImage : mMemoryImageSet)
	{
		if (aMemoryImage)
			aMemoryImage->DeleteNativeData();
	}
}

void SexyAppBase::DeleteExtraImageData()
{
	AutoCrit anAutoCrit(mCritSect);

	for (MemoryImage* aMemoryImage : mMemoryImageSet)
	{
		if (aMemoryImage)
			aMemoryImage->DeleteExtraBuffers();
	}
}

void SexyAppBase::ReInitImages()
{
	AutoCrit anAutoCrit(mCritSect);
    
    MemoryImageSet::iterator anItr = mMemoryImageSet.begin();
	while (anItr != mMemoryImageSet.end())
	{
		MemoryImage* aMemoryImage = *anItr;
		aMemoryImage->ReInit();
		++anItr;
	}
}

void SexyAppBase::LoadingThreadProc()
{
}

void SexyAppBase::LoadingThreadCompleted()
{
}

int SexyAppBase::LoadingThreadProcStub(void *theArg)
{
	SexyAppBase* aSexyApp = (SexyAppBase*) theArg;

	aSexyApp->LoadingThreadProc();

	printf("Resource Loading Time: %d\r\n", (SDL_GetTicks() - aSexyApp->mTimeLoaded));

	aSexyApp->mLoadingThreadCompleted = true;
	return 0;
}

void SexyAppBase::StartLoadingThread()
{
    if (!mLoadingThreadStarted)
    {
        mYieldMainThread = true;
        mLoadingThreadStarted = true;

        // 创建线程并分离
        SDL_Thread* thread = SDL_CreateThread(LoadingThreadProcStub, "LoadingThread", this);
        if (thread) {
            SDL_DetachThread(thread);
        }
    }
}

void SexyAppBase::SwitchScreenMode(bool wantWindowed, bool force)
{
	if (mForceFullscreen)
		wantWindowed = false;

	if (mIsWindowed == wantWindowed && !force)
	{
		return;
	}
	// Always make the app windowed when playing demos, in order to
	//  make it easier to track down bugs.  We place this after the
	//  sanity check just so things get re-initialized and stuff
	//if (mPlayingDemoBuffer)
	//	wantWindowed = true;

	mIsWindowed = wantWindowed;

	MakeWindow();

	// We need to do this check to allow IE to get focus instead of
	//  stealing it away for ourselves
	if (!mIsOpeningURL)
	{
		//::ShowWindow(mHWnd, SW_NORMAL);
		//::SetForegroundWindow(mHWnd);
	}
	else
	{
		// Show it but don't activate it
		//::ShowWindow(mHWnd, SW_SHOWNOACTIVATE);
	}

	if (mSoundManager!=NULL)
	{
		//mSoundManager->SetCooperativeWindow(mHWnd);
	}

	mLastTime = SDL_GetTicks();
}

void SexyAppBase::SwitchScreenMode(bool wantWindowed)
{
	SwitchScreenMode(wantWindowed, true);
}

void SexyAppBase::SwitchScreenMode()
{
	SwitchScreenMode(mIsWindowed, true);
}

void SexyAppBase::SetAlphaDisabled(bool isDisabled)
{
	if (mAlphaDisabled != isDisabled)
	{
		mAlphaDisabled = isDisabled;
		mWidgetManager->MarkAllDirty();
	}
}

void SexyAppBase::EnforceCursor()
{
	// Handle error state or mouse not in window
	if (!mMouseIn)
	{
		SDL_Cursor* aCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		if (aCursor)
		{
			SDL_SetCursor(aCursor);
			SDL_ShowCursor(SDL_ENABLE);
		}
		return;
	}

	// Check if we should use custom cursor image (game-drawn cursor)
	if ((mCursorImages[mCursorNum] != nullptr) &&
		((mCustomCursorsEnabled) || (mCursorNum == CURSOR_CUSTOM)))
	{
		// Using custom cursor, hide system cursor so game can draw its own
		SDL_ShowCursor(SDL_DISABLE);
		return;
	}

	// Use SDL system cursor based on cursor type
	SDL_SystemCursor aSystemCursor = SDL_SYSTEM_CURSOR_ARROW;
	bool shouldShowCursor = true;

	switch (mCursorNum)
	{
	case CURSOR_POINTER:
		aSystemCursor = SDL_SYSTEM_CURSOR_ARROW;
		break;
	case CURSOR_HAND:
		aSystemCursor = SDL_SYSTEM_CURSOR_HAND;
		break;
	case CURSOR_TEXT:
		aSystemCursor = SDL_SYSTEM_CURSOR_IBEAM;
		break;
	case CURSOR_DRAGGING:
		aSystemCursor = SDL_SYSTEM_CURSOR_SIZEALL;
		break;
	case CURSOR_CIRCLE_SLASH:
		aSystemCursor = SDL_SYSTEM_CURSOR_NO;
		break;
	case CURSOR_SIZEALL:
		aSystemCursor = SDL_SYSTEM_CURSOR_SIZEALL;
		break;
	case CURSOR_SIZENESW:
		aSystemCursor = SDL_SYSTEM_CURSOR_SIZENESW;
		break;
	case CURSOR_SIZENS:
		aSystemCursor = SDL_SYSTEM_CURSOR_SIZENS;
		break;
	case CURSOR_SIZENWSE:
		aSystemCursor = SDL_SYSTEM_CURSOR_SIZENWSE;
		break;
	case CURSOR_SIZEWE:
		aSystemCursor = SDL_SYSTEM_CURSOR_SIZEWE;
		break;
	case CURSOR_WAIT:
		aSystemCursor = SDL_SYSTEM_CURSOR_WAIT;
		break;
	case CURSOR_CUSTOM:
	case CURSOR_NONE:
		SDL_ShowCursor(SDL_DISABLE);
		return;
	default:
		aSystemCursor = SDL_SYSTEM_CURSOR_ARROW;
		break;
	}

	// Set the system cursor
	SDL_Cursor* aCursor = SDL_CreateSystemCursor(aSystemCursor);
	if (aCursor)
	{
		SDL_SetCursor(aCursor);
		if (shouldShowCursor)
		{
			SDL_ShowCursor(SDL_ENABLE);
		}
	}
}

void SexyAppBase::ProcessSafeDeleteList()
{
	MTAutoDisallowRand aDisallowRand;

	WidgetSafeDeleteList::iterator anItr = mSafeDeleteList.begin();
	while (anItr != mSafeDeleteList.end())
	{
		WidgetSafeDeleteInfo* aWidgetSafeDeleteInfo = &(*anItr);
		if (mUpdateAppDepth <= aWidgetSafeDeleteInfo->mUpdateAppDepth)
		{
			delete aWidgetSafeDeleteInfo->mWidget;
			anItr = mSafeDeleteList.erase(anItr);
		}
		else
			++anItr;
	}
}

void SexyAppBase::UpdateFTimeAcc()
{
	DWORD aCurTime = SDL_GetTicks();

	if (mLastTimeCheck != 0)
	{
		int aDeltaTime = aCurTime - mLastTimeCheck;

		mUpdateFTimeAcc = std::min(mUpdateFTimeAcc + aDeltaTime, 200.0);

		if (mRelaxUpdateBacklogCount > 0)
			mRelaxUpdateBacklogCount = std::max(mRelaxUpdateBacklogCount - aDeltaTime, 0);
	}

	mLastTimeCheck = aCurTime;
}

//int aNumCalls = 0;
//DWORD aLastCheck = 0;

bool SexyAppBase::Process(bool allowSleep)
{
	/*DWORD aTimeNow = GetTickCount();
	if (aTimeNow - aLastCheck >= 10000)
	{
		OutputDebugString(StrFormat(__S("FUpdates: %d\n"), aNumCalls).c_str());
		aLastCheck = aTimeNow;
		aNumCalls = 0;
	}*/

	if (mLoadingFailed)
		Shutdown();

	bool isVSynched = (mVSyncUpdates) && (!mLastDrawWasEmpty) && (!mVSyncBroken) &&
		((!mIsPhysWindowed) || (mIsPhysWindowed && mWaitForVSync && !mSoftVSyncWait));
	double aFrameFTime;
	double anUpdatesPerUpdateF;

	if (mVSyncUpdates)
	{
		aFrameFTime = (1000.0 / mSyncRefreshRate) / mUpdateMultiplier;
		anUpdatesPerUpdateF = (float) (1000.0 / (mFrameTime * mSyncRefreshRate));
	}
	else
	{
		aFrameFTime = mFrameTime / mUpdateMultiplier;
		anUpdatesPerUpdateF = 1.0;
	}

	// Make sure we're not paused
	if ((!mPaused) && (mUpdateMultiplier > 0))
	{
		uint32_t aStartTime = SDL_GetTicks();

		// uint32_t aCurTime = aStartTime; // Unused
		int aCumSleepTime = 0;

		// When we are VSynching, only calculate this FTimeAcc right after drawing

		if (!isVSynched)
			UpdateFTimeAcc();

		// mNonDrawCount is used to make sure we draw the screen at least
		// 10 times per second, even if it means we have to slow down
		// the updates to make it draw 10 times per second in "game time"

		bool didUpdate = false;

		if (mUpdateAppState == UPDATESTATE_PROCESS_1)
		{
			if ((++mNonDrawCount < (int) ceil(10*mUpdateMultiplier)) || (!mLoaded))
			{
				bool doUpdate = false;

				if (isVSynched)
				{
					// Synch'ed to vertical refresh, so update as soon as possible after draw
					doUpdate = (!mHasPendingDraw) || (mUpdateFTimeAcc >= (int) (aFrameFTime * 0.75));
				}
				else if (mUpdateFTimeAcc >= aFrameFTime)
				{
					doUpdate = true;
				}

				if (doUpdate)
				{
					// Do VSyncBroken test.  This test fails if we're in fullscreen and
					// "don't vsync" has been forced in Advanced settings up Display Properties
					if (mUpdateMultiplier == 1.0)
					{
						mVSyncBrokenTestUpdates++;
						if (mVSyncBrokenTestUpdates >= (DWORD) ((1000+mFrameTime-1)/mFrameTime))
						{
							// It has to be running 33% fast to be "broken" (25% = 1/0.800)
							if (aStartTime - mVSyncBrokenTestStartTick <= 800)
							{
								// The test has to fail 3 times in a row before we decide that
								//  vsync is broken overall
								mVSyncBrokenCount++;
								if (mVSyncBrokenCount >= 3)
									mVSyncBroken = true;
							}
							else
								mVSyncBrokenCount = 0;

							mVSyncBrokenTestStartTick = aStartTime;
							mVSyncBrokenTestUpdates = 0;
						}
					}

					bool hadRealUpdate = DoUpdateFrames();
					if (hadRealUpdate)
						mUpdateAppState = UPDATESTATE_PROCESS_2;

					mHasPendingDraw = true;
					didUpdate = true;
				}
			}
		}
		else if (mUpdateAppState == UPDATESTATE_PROCESS_2)
		{
			mUpdateAppState = UPDATESTATE_PROCESS_DONE;

			mPendingUpdatesAcc += anUpdatesPerUpdateF;
			mPendingUpdatesAcc -= 1.0;
			ProcessSafeDeleteList();

			// Process any extra updates
			while (mPendingUpdatesAcc >= 1.0)
			{
				++mNonDrawCount;
				bool hasRealUpdate = DoUpdateFrames();
				TOD_ASSERT(hasRealUpdate);

				if (!hasRealUpdate)
					break;

				ProcessSafeDeleteList();
				mPendingUpdatesAcc -= 1.0;
			}

			//aNumCalls++;
			DoUpdateFramesF((float) anUpdatesPerUpdateF);
			ProcessSafeDeleteList();

			// Don't let mUpdateFTimeAcc dip below 0
			//  Subtract an extra 0.2ms, because sometimes refresh rates have some
			//  fractional component that gets truncated, and it's better to take off
			//  too much to keep our timing tending toward occuring right after
			//  redraws
			if (isVSynched)
				mUpdateFTimeAcc = std::max(mUpdateFTimeAcc - aFrameFTime - 0.2f, 0.0);
			else
				mUpdateFTimeAcc -= aFrameFTime;

			if (mRelaxUpdateBacklogCount > 0)
				mUpdateFTimeAcc = 0;

			didUpdate = true;
		}

		if (!didUpdate)
		{
			mUpdateAppState = UPDATESTATE_PROCESS_DONE;

			mNonDrawCount = 0;

			if (mHasPendingDraw)
			{
				DrawDirtyStuff();
			}
			else
			{
				// Let us take into account the time it took to draw dirty stuff
				int aTimeToNextFrame = (int) (aFrameFTime - mUpdateFTimeAcc);
				if (aTimeToNextFrame > 0)
				{
					if (!allowSleep)
						return false;

					// Wait till next processing cycle
					++mSleepCount;

                    SDL_Delay(aTimeToNextFrame);

                    aCumSleepTime += aTimeToNextFrame;
				}
			}
		}

		if (mYieldMainThread)
		{
			// This is to make sure that the title screen doesn't take up any more than
			// 1/3 of the processor time

			uint32_t anEndTime = SDL_GetTicks();
			int anElapsedTime = (anEndTime - aStartTime) - aCumSleepTime;
			int aLoadingYieldSleepTime = std::min(250, (anElapsedTime * 2) - aCumSleepTime);

			if (aLoadingYieldSleepTime >= 0)
			{
				if (!allowSleep)
					return false;
                SDL_Delay(aLoadingYieldSleepTime);
			}
		}
	}

	ProcessSafeDeleteList();
	return true;
}

void SexyAppBase::DoMainLoop()
{
	while (!mShutdown)
	{
		if (mExitToTop)
			mExitToTop = false;
		UpdateApp();
	}
}

bool SexyAppBase::UpdateAppStep(bool* updated)
{
	if (updated != NULL)
		*updated = false;

	if (mExitToTop)
		return false;

	if (mUpdateAppState == UPDATESTATE_PROCESS_DONE)
		mUpdateAppState = UPDATESTATE_MESSAGES;

	mUpdateAppDepth++;

	// We update in two stages to avoid doing a Process if our loop termination
	//  condition has already been met by processing windows messages
	if (mUpdateAppState == UPDATESTATE_MESSAGES)
	{
		/*
		MSG msg;
		while ((PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) && (!mShutdown))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		*/

		if (!ProcessDeferredMessages(true))
		{
			mUpdateAppState = UPDATESTATE_PROCESS_1;
		}
	}
	else
	{
		// Process changes state by itself
		if (mStepMode)
		{
			if (mStepMode==2)
			{
                SDL_Delay(mFrameTime);
				mUpdateAppState = UPDATESTATE_PROCESS_DONE; // skip actual update until next step
			}
			else
			{
				mStepMode = 2;
				DoUpdateFrames();
				DoUpdateFramesF(1.0f);
				DrawDirtyStuff();
			}
		}
		else
		{
			int anOldUpdateCnt = mUpdateCount;
			Process();
			if (updated != NULL)
				*updated = mUpdateCount != anOldUpdateCnt;
		}
	}

	mUpdateAppDepth--;

	return true;
}

bool SexyAppBase::UpdateApp()
{
	bool updated;
	for (;;)
	{
		if (!UpdateAppStep(&updated))
			return false;
		if (updated)
			return true;
	}
}

int SexyAppBase::InitGraphics()
{
    // 预初始化钩子
    PreGLInterfaceInitHook();

    // 删除之前的图像数据
    DeleteNativeImageData();

    mScreenBounds.mX = 0;
    mScreenBounds.mY = 0;
    mScreenBounds.mWidth = mWidth;
    mScreenBounds.mHeight = mHeight;

    // 始终使用软件渲染：调整小部件管理器的大小
    mWidgetManager->Resize(mScreenBounds, mScreenBounds);

    // 后初始化钩子
    PostGLInterfaceInitHook();

    return 0; // 返回初始化成功
}


void SexyAppBase::PreTerminate()
{
}

void SexyAppBase::Start()
{
	if (mShutdown)
		return;

	if (mAutoStartLoadingThread)
		StartLoadingThread();

	//::ShowWindow(mHWnd, SW_SHOW);
	//::SetFocus(mHWnd);

	//timeBeginPeriod(1);

	//int aCount = 0; // unused
	//int aSleepCount = 0; // unused

	DWORD aStartTime = SDL_GetTicks();

	mRunning = true;
	mLastTime = aStartTime;
	mLastUserInputTick = aStartTime;
	mLastTimerTime = aStartTime;

	DoMainLoop();
	ProcessSafeDeleteList();

	mRunning = false;

	WaitForLoadingThread();

	printf("Seconds       = %g\r\n", (SDL_GetTicks() - aStartTime) / 1000.0);
	//printf("Count         = %d\r\n", aCount);
	printf("Sleep Count   = %d\r\n", mSleepCount);
	printf("Update Count  = %d\r\n", mUpdateCount);
	printf("Draw Count    = %d\r\n", mDrawCount);
	printf("Draw Time     = %d\r\n", mDrawTime);
	printf("Screen Blt    = %d\r\n", mScreenBltTime);
	if (mDrawTime+mScreenBltTime > 0)
	{
		printf("Avg FPS       = %d\r\n", (mDrawCount*1000)/(mDrawTime+mScreenBltTime));
	}

	//timeEndPeriod(1);

	PreTerminate();

	WriteToRegistry();
}

bool SexyAppBase::LoadProperties(const std::string& theFileName, bool required, bool checkSig)
{
	Buffer aBuffer;
	if (!ReadBufferFromFile(theFileName, &aBuffer))
	{
		if (!required)
			return true;
		else
		{
			Popup(GetString("UNABLE_OPEN_PROPERTIES", __S("Unable to open properties file ")) + StringToSexyString(theFileName));
			return false;
		}
	}
	if (checkSig)
	{
		//if (!CheckSignature(aBuffer, theFileName))
		//{
			//Popup(GetString("PROPERTIES_SIG_FAILED", __S("Signature check failed on ")) + StringToSexyString(theFileName + "'"));
			//return false;
		//}
	}

	PropertiesParser aPropertiesParser(this);

	// Load required language-file properties
		if (!aPropertiesParser.ParsePropertiesBuffer(aBuffer))
		{
			Popup(aPropertiesParser.GetErrorText());
			return false;
		}
		else
			return true;
}

bool SexyAppBase::LoadProperties()
{
	// Load required language-file properties
	return LoadProperties("properties/default.xml", true, false);
}

void SexyAppBase::LoadResourceManifest()
{
	if (!mResourceManager->ParseResourcesFile("properties/resources.xml"))
		ShowResourceError(true);
}

void SexyAppBase::ShowResourceError(bool doExit)
{
	Popup(mResourceManager->GetErrorText());
	if (doExit)
		DoExit(0);
}

bool SexyAppBase::GetBoolean(const std::string& theId)
{
	StringBoolMap::iterator anItr = mBoolProperties.find(theId);
	TOD_ASSERT(anItr != mBoolProperties.end());

	if (anItr != mBoolProperties.end())
		return anItr->second;
	else
		return false;
}

bool SexyAppBase::GetBoolean(const std::string& theId, bool theDefault)
{
	StringBoolMap::iterator anItr = mBoolProperties.find(theId);

	if (anItr != mBoolProperties.end())
		return anItr->second;
	else
		return theDefault;
}

int SexyAppBase::GetInteger(const std::string& theId)
{
	StringIntMap::iterator anItr = mIntProperties.find(theId);
	TOD_ASSERT(anItr != mIntProperties.end());

	if (anItr != mIntProperties.end())
		return anItr->second;
	else
		return false;
}

int SexyAppBase::GetInteger(const std::string& theId, int theDefault)
{
	StringIntMap::iterator anItr = mIntProperties.find(theId);

	if (anItr != mIntProperties.end())
		return anItr->second;
	else
		return theDefault;
}

double SexyAppBase::GetDouble(const std::string& theId)
{
	StringDoubleMap::iterator anItr = mDoubleProperties.find(theId);
	TOD_ASSERT(anItr != mDoubleProperties.end());

	if (anItr != mDoubleProperties.end())
		return anItr->second;
	else
		return false;
}

double SexyAppBase::GetDouble(const std::string& theId, double theDefault)
{
	StringDoubleMap::iterator anItr = mDoubleProperties.find(theId);

	if (anItr != mDoubleProperties.end())
		return anItr->second;
	else
		return theDefault;
}

SexyString SexyAppBase::GetString(const std::string& theId)
{
	StringWStringMap::iterator anItr = mStringProperties.find(theId);
	TOD_ASSERT(anItr != mStringProperties.end());

	if (anItr != mStringProperties.end())
		return WStringToSexyString(anItr->second);
	else
		return __S("");
}

SexyString SexyAppBase::GetString(const std::string& theId, const SexyString& theDefault)
{
	StringWStringMap::iterator anItr = mStringProperties.find(theId);

	if (anItr != mStringProperties.end())
		return WStringToSexyString(anItr->second);
	else
		return theDefault;
}

StringVector SexyAppBase::GetStringVector(const std::string& theId)
{
	StringStringVectorMap::iterator anItr = mStringVectorProperties.find(theId);
	TOD_ASSERT(anItr != mStringVectorProperties.end());

	if (anItr != mStringVectorProperties.end())
		return anItr->second;
	else
		return StringVector();
}

void SexyAppBase::SetString(const std::string& theId, const std::wstring& theValue)
{
	std::pair<StringWStringMap::iterator, bool> aPair = mStringProperties.insert(StringWStringMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}


void SexyAppBase::SetBoolean(const std::string& theId, bool theValue)
{
	std::pair<StringBoolMap::iterator, bool> aPair = mBoolProperties.insert(StringBoolMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}

void SexyAppBase::SetInteger(const std::string& theId, int theValue)
{
	std::pair<StringIntMap::iterator, bool> aPair = mIntProperties.insert(StringIntMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}

void SexyAppBase::SetDouble(const std::string& theId, double theValue)
{
	std::pair<StringDoubleMap::iterator, bool> aPair = mDoubleProperties.insert(StringDoubleMap::value_type(theId, theValue));
	if (!aPair.second) // Found it, change value
		aPair.first->second = theValue;
}


void SexyAppBase::PreDisplayHook()
{
}

void SexyAppBase::PreGLInterfaceInitHook()
{
}

void SexyAppBase::PostGLInterfaceInitHook()
{
}

bool SexyAppBase::ChangeDirHook(const char *theIntendedPath)
{
	(void)theIntendedPath;
	return false;
}

MusicInterface* SexyAppBase::CreateMusicInterface()
{
	if (mNoSoundNeeded)
		return new DummyMusicInterface();
	else
		return new SDLMusicInterface();
}

void SexyAppBase::InitPropertiesHook()
{
}

void SexyAppBase::InitHook()
{
}

void SexyAppBase::Init()
{
	mPrimaryThreadId = (void*)SDL_ThreadID;

	if (mShutdown)
		return;

	InitPropertiesHook();
	ReadFromRegistry();


	// Change directory
	if (!ChangeDirHook(mChangeDirTo.c_str()))
		chdir(mChangeDirTo.c_str());

	char aPath[512];
	getcwd(aPath, 512);
	strcat(aPath, "/savedata/");
	SetAppDataFolder(aPath);


    //gPakInterface->AddPakFile("main.pak", 0); // 设置 main.pak 的优先级为 99
    
    char aPakPath[512];
    getcwd(aPakPath, sizeof(aPakPath));
    strcat(aPakPath, "/paks/");
    SetPakFolder(aPakPath);

    if (std::filesystem::exists(GetPakFolder())) {
        std::vector<std::string> pakList; // 存储 pak_list.txt 中的文件名
        std::ifstream pakListFile("paks/pak_list.txt"); // 打开 pak_list.txt 文件

        if (pakListFile.is_open()) {
            std::string line;
            // 读取每一行并检查文件是否存在
            while (std::getline(pakListFile, line)) {
                if (std::filesystem::exists(line)) {
                    pakList.push_back(line); // 只添加存在的文件
                }
            }
            pakListFile.close();
        }

        std::unordered_set<std::string> pakListSet(pakList.begin(), pakList.end()); // 使用集合快速查找
        std::unordered_set<std::string> addedPakFiles; // 存储已经添加的文件路径

        std::reverse(pakList.begin(), pakList.end());
        int priority = 0;

        // 首先，遍历 paks/ 文件夹中的所有文件，添加未在 pak_list.txt 中且尚未添加的 .pak 文件
        for (const auto &entry : std::filesystem::directory_iterator(GetPakFolder())) {
            // 检查是否为常规文件且扩展名为 .pak
            if (entry.is_regular_file() && entry.path().extension() == ".pak") {
                // 获取相对于根目录的路径
                std::string pakFilePath = std::filesystem::relative(entry.path(), mChangeDirTo).string();
                std::replace(pakFilePath.begin(), pakFilePath.end(), '\\', '/');

                // 确保该文件不在 pak_list 中且尚未添加
                if (pakListSet.find(pakFilePath) == pakListSet.end() && addedPakFiles.insert(pakFilePath).second) {
                    // 添加文件到接口并增加优先级
                    gPakInterface->AddPakFile(pakFilePath, priority++); // 直接使用当前优先级
                }
            }
        }

       // 然后，遍历 pak_list 中的文件，添加已经存在的 .pak 文件
        for (const auto &pakFileName : pakList) {
            if (addedPakFiles.insert(pakFileName).second) { // 检查是否已经添加过
                gPakInterface->AddPakFile(pakFileName, priority); // 使用相对路径添加文件
                priority++; // 增加优先级
            }
        }

        // 更新 pak_list.txt
        std::ofstream outputPakListFile("paks/pak_list.txt", std::ios::out | std::ios::trunc); // 打开文件进行写入
        if (outputPakListFile.is_open()) {
            // 写入已添加的 PAK 文件
            for (const auto& pakFile : addedPakFiles) {
                outputPakListFile << pakFile << std::endl; // 写入每个添加的 PAK 文件
            }
            outputPakListFile.close(); // 关闭文件
        }

    } else {
        MkDir(GetPakFolder());
    }

	mRandSeed = SDL_GetTicks();
	SRand(mRandSeed);


	srand(SDL_GetTicks());

	mIsWideWindow = false;

	// Let app do something before showing window, or switching to fullscreen mode
	// NOTE: Moved call to PreDisplayHook above mIsWindowed and GetSystemsMetrics
	// checks because the checks below use values that could change in PreDisplayHook.
	// PreDisplayHook must call mWidgetManager->Resize if it changes mWidth or mHeight.
	PreDisplayHook();
	mWidgetManager->Resize(Rect(0, 0, mWidth, mHeight), Rect(0, 0, mWidth, mHeight));

	MakeWindow();

	if (mSoundManager == nullptr)
		mSoundManager = new SDLSoundManager();

	SetSfxVolume(mSfxVolume);

	mMusicInterface = CreateMusicInterface();

	SetMusicVolume(mMusicVolume);


	InitHook();

	InitInput();

	mInitialized = true;
}

void SexyAppBase::HandleGameAlreadyRunning()
{
	if(mOnlyAllowOneCopyToRun)
	{
		/*
		// Notify the other window and then shut ourselves down
		if (mNotifyGameMessage != 0)
			PostMessage(HWND_BROADCAST, mNotifyGameMessage, 0, 0);
		*/

		DoExit(0);
	}
}

void SexyAppBase::CopyToClipboard(const std::string& theString)
{
	// SDL handles the platform-specific clipboard implementation
	if (SDL_SetClipboardText(theString.c_str()) != 0)
	{
		// Best-effort: if setting fails, just return
		return;
	}
}

std::string	SexyAppBase::GetClipboard()
{
	if (!SDL_HasClipboardText())
		return "";

	char* aClipboardText = SDL_GetClipboardText();

	if (aClipboardText == nullptr)
		return "";

	std::string aResult(aClipboardText);
	SDL_free(aClipboardText);

	return aResult;
}

void SexyAppBase::SetCursor(int theCursorNum)
{
	mCursorNum = theCursorNum;
	EnforceCursor();
}

int SexyAppBase::GetCursor()
{
	return mCursorNum;
}

void SexyAppBase::EnableCustomCursors(bool enabled)
{
	mCustomCursorsEnabled = enabled;
	EnforceCursor();
}

Image* SexyAppBase::GetImage(const std::string& theFileName, bool commitBits)
{
	ImageLib::Image* aLoadedImage = ImageLib::GetImage(theFileName, true);

	if (aLoadedImage == NULL)
		return NULL;

	MemoryImage* anImage = new MemoryImage(this);
	anImage->mFilePath = theFileName;
	anImage->SetBits(aLoadedImage->GetBits(), aLoadedImage->GetWidth(), aLoadedImage->GetHeight(), commitBits);
	anImage->mFilePath = theFileName;
	delete aLoadedImage;

	return anImage;
}

Image* SexyAppBase::CreateCrossfadeImage(Sexy::Image* theImage1, const Rect& theRect1, Sexy::Image* theImage2, const Rect& theRect2, double theFadeFactor)
{
	MemoryImage* aMemoryImage1 = dynamic_cast<MemoryImage*>(theImage1);
	MemoryImage* aMemoryImage2 = dynamic_cast<MemoryImage*>(theImage2);

	if ((aMemoryImage1 == NULL) || (aMemoryImage2 == NULL))
		return NULL;

	if ((theRect1.mX < 0) || (theRect1.mY < 0) ||
		(theRect1.mX + theRect1.mWidth > theImage1->GetWidth()) ||
		(theRect1.mY + theRect1.mHeight > theImage1->GetHeight()))
	{
		TOD_ASSERT(false, "Crossfade Rect1 out of bounds");
		return NULL;
	}

	if ((theRect2.mX < 0) || (theRect2.mY < 0) ||
		(theRect2.mX + theRect2.mWidth > theImage2->GetWidth()) ||
		(theRect2.mY + theRect2.mHeight > theImage2->GetHeight()))
	{
		TOD_ASSERT(false, "Crossfade Rect2 out of bounds");
		return NULL;
	}

	int aWidth = theRect1.mWidth;
	int aHeight = theRect1.mHeight;

	MemoryImage* anImage = new MemoryImage(this);
	anImage->Create(aWidth, aHeight);

	uint32_t* aDestBits = anImage->GetBits();
	uint32_t* aSrcBits1 = aMemoryImage1->GetBits();
	uint32_t* aSrcBits2 = aMemoryImage2->GetBits();

	int aSrc1Width = aMemoryImage1->GetWidth();
	int aSrc2Width = aMemoryImage2->GetWidth();
	uint32_t aMult = (int) (theFadeFactor*256);
	uint32_t aOMM = (256 - aMult);

	for (int y = 0; y < aHeight; y++)
	{
		uint32_t* s1 = &aSrcBits1[(y+theRect1.mY)*aSrc1Width+theRect1.mX];
		uint32_t* s2 = &aSrcBits2[(y+theRect2.mY)*aSrc2Width+theRect2.mX];
		uint32_t* d = &aDestBits[y*aWidth];

		for (int x = 0; x < aWidth; x++)
		{
			uint32_t p1 = *s1++;
			uint32_t p2 = *s2++;

			//p1 = 0;
			//p2 = 0xFFFFFFFF;

			*d++ =
				((((p1 & 0x000000FF)*aOMM + (p2 & 0x000000FF)*aMult)>>8) & 0x000000FF) |
				((((p1 & 0x0000FF00)*aOMM + (p2 & 0x0000FF00)*aMult)>>8) & 0x0000FF00) |
				((((p1 & 0x00FF0000)*aOMM + (p2 & 0x00FF0000)*aMult)>>8) & 0x00FF0000) |
				((((p1 >> 24)*aOMM + (p2 >> 24)*aMult)<<16) & 0xFF000000);
		}
	}

	anImage->BitsChanged();

	return anImage;
}

void SexyAppBase::ColorizeImage(Image* theImage, const Color& theColor)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	if (aSrcMemoryImage == NULL)
		return;

	uint32_t* aBits;
	int aNumColors;

	if (aSrcMemoryImage->mColorTable == NULL)
	{
		aBits = aSrcMemoryImage->GetBits();
		aNumColors = theImage->GetWidth()*theImage->GetHeight();
	}
	else
	{
		aBits = aSrcMemoryImage->mColorTable;
		aNumColors = 256;
	}

	if ((theColor.mAlpha <= 255) && (theColor.mRed <= 255) &&
		(theColor.mGreen <= 255) && (theColor.mBlue <= 255))
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32_t aColor = aBits[i];

			aBits[i] =
				((((aColor & 0xFF000000) >> 8) * theColor.mAlpha) & 0xFF000000) |
				((((aColor & 0x00FF0000) * theColor.mRed) >> 8) & 0x00FF0000) |
				((((aColor & 0x0000FF00) * theColor.mGreen) >> 8) & 0x0000FF00)|
				((((aColor & 0x000000FF) * theColor.mBlue) >> 8) & 0x000000FF);
		}
	}
	else
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32_t aColor = aBits[i];

			int aAlpha = ((aColor >> 24) * theColor.mAlpha) / 255;
			int aRed = (((aColor >> 16) & 0xFF) * theColor.mRed) / 255;
			int aGreen = (((aColor >> 8) & 0xFF) * theColor.mGreen) / 255;
			int aBlue = ((aColor & 0xFF) * theColor.mBlue) / 255;

			if (aAlpha > 255)
				aAlpha = 255;
			if (aRed > 255)
				aRed = 255;
			if (aGreen > 255)
				aGreen = 255;
			if (aBlue > 255)
				aBlue = 255;

			aBits[i] = (aAlpha << 24) | (aRed << 16) | (aGreen << 8) | (aBlue);
		}
	}

	aSrcMemoryImage->BitsChanged();
}

Image* SexyAppBase::CreateColorizedImage(Image* theImage, const Color& theColor)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	if (aSrcMemoryImage == NULL)
		return NULL;

	MemoryImage* anImage = new MemoryImage(this);

	anImage->Create(theImage->GetWidth(), theImage->GetHeight());

	uint32_t* aSrcBits;
	uint32_t* aDestBits;
	int aNumColors;

	if (aSrcMemoryImage->mColorTable == NULL)
	{
		aSrcBits = aSrcMemoryImage->GetBits();
		aDestBits = anImage->GetBits();
		aNumColors = theImage->GetWidth()*theImage->GetHeight();
	}
	else
	{
		aSrcBits = aSrcMemoryImage->mColorTable;
		aDestBits = anImage->mColorTable = new uint32_t[256];
		aNumColors = 256;

		anImage->mColorIndices = new uchar[anImage->mWidth*theImage->mHeight];
		memcpy(anImage->mColorIndices, aSrcMemoryImage->mColorIndices, anImage->mWidth*theImage->mHeight);
	}

	if ((theColor.mAlpha <= 255) && (theColor.mRed <= 255) &&
		(theColor.mGreen <= 255) && (theColor.mBlue <= 255))
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32_t aColor = aSrcBits[i];

			aDestBits[i] =
				((((aColor & 0xFF000000) >> 8) * theColor.mAlpha) & 0xFF000000) |
				((((aColor & 0x00FF0000) * theColor.mRed) >> 8) & 0x00FF0000) |
				((((aColor & 0x0000FF00) * theColor.mGreen) >> 8) & 0x0000FF00)|
				((((aColor & 0x000000FF) * theColor.mBlue) >> 8) & 0x000000FF);
		}
	}
	else
	{
		for (int i = 0; i < aNumColors; i++)
		{
			uint32_t aColor = aSrcBits[i];

			int aAlpha = ((aColor >> 24) * theColor.mAlpha) / 255;
			int aRed = (((aColor >> 16) & 0xFF) * theColor.mRed) / 255;
			int aGreen = (((aColor >> 8) & 0xFF) * theColor.mGreen) / 255;
			int aBlue = ((aColor & 0xFF) * theColor.mBlue) / 255;

			if (aAlpha > 255)
				aAlpha = 255;
			if (aRed > 255)
				aRed = 255;
			if (aGreen > 255)
				aGreen = 255;
			if (aBlue > 255)
				aBlue = 255;

			aDestBits[i] = (aAlpha << 24) | (aRed << 16) | (aGreen << 8) | (aBlue);
		}
	}

	anImage->BitsChanged();

	return anImage;
}

Image* SexyAppBase::CopyImage(Image* theImage, const Rect& theRect)
{
	MemoryImage* anImage = new MemoryImage(this);

	anImage->Create(theRect.mWidth, theRect.mHeight);

	Graphics g(anImage);
	g.DrawImage(theImage, -theRect.mX, -theRect.mY);

	anImage->CopyAttributes(theImage);

	return anImage;
}

Image* SexyAppBase::CopyImage(Image* theImage)
{
	return CopyImage(theImage, Rect(0, 0, theImage->GetWidth(), theImage->GetHeight()));
}

void SexyAppBase::MirrorImage(Image* theImage)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	uint32_t* aSrcBits = aSrcMemoryImage->GetBits();

	int aPhysSrcWidth = aSrcMemoryImage->mWidth;
	for (int y = 0; y < aSrcMemoryImage->mHeight; y++)
	{
		uint32_t* aLeftBits = aSrcBits + (y * aPhysSrcWidth);
		uint32_t* aRightBits = aLeftBits + (aPhysSrcWidth - 1);

		for (int x = 0; x < (aPhysSrcWidth >> 1); x++)
		{
			uint32_t aSwap = *aLeftBits;

			*(aLeftBits++) = *aRightBits;
			*(aRightBits--) = aSwap;
		}
	}

	aSrcMemoryImage->BitsChanged();
}

void SexyAppBase::FlipImage(Image* theImage)
{
	MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

	uint32_t* aSrcBits = aSrcMemoryImage->GetBits();

	int aPhysSrcHeight = aSrcMemoryImage->mHeight;
	int aPhysSrcWidth = aSrcMemoryImage->mWidth;
	for (int x = 0; x < aPhysSrcWidth; x++)
	{
		uint32_t* aTopBits    = aSrcBits + x;
		uint32_t* aBottomBits = aTopBits + (aPhysSrcWidth * (aPhysSrcHeight - 1));

		for (int y = 0; y < (aPhysSrcHeight >> 1); y++)
		{
			uint32_t aSwap = *aTopBits;

			*aTopBits = *aBottomBits;
			aTopBits += aPhysSrcWidth;
			*aBottomBits = aSwap;
			aBottomBits -= aPhysSrcWidth;
		}
	}

	aSrcMemoryImage->BitsChanged();
}

void SexyAppBase::RotateImageHue(Sexy::MemoryImage *theImage, int theDelta)
{
	while (theDelta < 0)
		theDelta += 256;

	int aSize = theImage->mWidth * theImage->mHeight;
	uint32_t *aPtr = theImage->GetBits();
	for (int i=0; i<aSize; i++)
	{
		uint32_t aPixel = *aPtr;
		int alpha = aPixel&0xff000000;
		int r = (aPixel>>16)&0xff;
		int g = (aPixel>>8) &0xff;
		int b = aPixel&0xff;

		int maxval = std::max(r, std::max(g, b));
		int minval = std::min(r, std::min(g, b));
		int h = 0;
		int s = 0;
		int l = (minval+maxval)/2;
		int delta = maxval - minval;

		if (delta != 0)
		{
			s = (delta * 256) / ((l <= 128) ? (minval + maxval) : (512 - maxval - minval));

			if (r == maxval)
				h = (g == minval ? 1280 + (((maxval-b) * 256) / delta) :  256 - (((maxval - g) * 256) / delta));
			else if (g == maxval)
				h = (b == minval ?  256 + (((maxval-r) * 256) / delta) :  768 - (((maxval - b) * 256) / delta));
			else
				h = (r == minval ?  768 + (((maxval-g) * 256) / delta) : 1280 - (((maxval - r) * 256) / delta));

			h /= 6;
		}

		h += theDelta;
		if (h >= 256)
			h -= 256;

		double v= (l < 128) ? (l * (255+s))/255 :
				(l+s-l*s/255);

		int y = (int) (2*l-v);

		int aColorDiv = (6 * h) / 256;
		int x = (int)(y+(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
		if (x > 255)
			x = 255;

		int z = (int) (v-(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
		if (z < 0)
			z = 0;

		switch (aColorDiv)
		{
			case 0: r = (int) v; g = x; b = y; break;
			case 1: r = z; g= (int) v; b = y; break;
			case 2: r = y; g= (int) v; b = x; break;
			case 3: r = y; g = z; b = (int) v; break;
			case 4: r = x; g = y; b = (int) v; break;
			case 5: r = (int) v; g = y; b = z; break;
			default: r = (int) v; g = x; b = y; break;
		}

		*aPtr++ = alpha | (r<<16) | (g << 8) | (b);

	}

	theImage->BitsChanged();
}

uint32_t SexyAppBase::HSLToRGB(int h, int s, int l)
{
	int r;
	int g;
	int b;

	double v= (l < 128) ? (l * (255+s))/255 :
			(l+s-l*s/255);

	int y = (int) (2*l-v);

	int aColorDiv = (6 * h) / 256;
	int x = (int)(y+(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
	if (x > 255)
		x = 255;

	int z = (int) (v-(v-y)*((h - (aColorDiv * 256 / 6)) * 6)/255);
	if (z < 0)
		z = 0;

	switch (aColorDiv)
	{
		case 0: r = (int) v; g = x; b = y; break;
		case 1: r = z; g= (int) v; b = y; break;
		case 2: r = y; g= (int) v; b = x; break;
		case 3: r = y; g = z; b = (int) v; break;
		case 4: r = x; g = y; b = (int) v; break;
		case 5: r = (int) v; g = y; b = z; break;
		default: r = (int) v; g = x; b = y; break;
	}

	return 0xFF000000 | (r << 16) | (g << 8) | (b);
}

uint32_t SexyAppBase::RGBToHSL(int r, int g, int b)
{
	int maxval = std::max(r, std::max(g, b));
	int minval = std::min(r, std::min(g, b));
	int hue = 0;
	int saturation = 0;
	int luminosity = (minval+maxval)/2;
	int delta = maxval - minval;

	if (delta != 0)
	{
		saturation = (delta * 256) / ((luminosity <= 128) ? (minval + maxval) : (512 - maxval - minval));

		if (r == maxval)
			hue = (g == minval ? 1280 + (((maxval-b) * 256) / delta) :  256 - (((maxval - g) * 256) / delta));
		else if (g == maxval)
			hue = (b == minval ?  256 + (((maxval-r) * 256) / delta) :  768 - (((maxval - b) * 256) / delta));
		else
			hue = (r == minval ?  768 + (((maxval-g) * 256) / delta) : 1280 - (((maxval - r) * 256) / delta));

		hue /= 6;
	}

	return 0xFF000000 | (hue) | (saturation << 8) | (luminosity << 16);
}

void SexyAppBase::HSLToRGB(const uint32_t* theSource, uint32_t* theDest, int theSize)
{
	for (int i = 0; i < theSize; i++)
	{
		uint32_t src = theSource[i];
		theDest[i] = (src & 0xFF000000) | (HSLToRGB((src & 0xFF), (src >> 8) & 0xFF, (src >> 16) & 0xFF) & 0x00FFFFFF);
	}
}

void SexyAppBase::RGBToHSL(const uint32_t* theSource, uint32_t* theDest, int theSize)
{
	for (int i = 0; i < theSize; i++)
	{
		uint32_t src = theSource[i];
		theDest[i] = (src & 0xFF000000) | (RGBToHSL(((src >> 16) & 0xFF), (src >> 8) & 0xFF, (src & 0xFF)) & 0x00FFFFFF);
	}
}

void SexyAppBase::PrecacheAdditive(MemoryImage* theImage)
{
	theImage->GetRLAdditiveData(nullptr);
}

void SexyAppBase::PrecacheAlpha(MemoryImage* theImage)
{
	theImage->GetRLAlphaData();
}

void SexyAppBase::PrecacheNative(MemoryImage* theImage)
{
	theImage->GetNativeAlphaData(nullptr);
}


void SexyAppBase::PlaySample(int theSoundNum)
{
	if (!mSoundManager)
		return;

	SoundInstance* aSoundInstance = mSoundManager->GetSoundInstance(theSoundNum);
	if (aSoundInstance != NULL)
	{
		aSoundInstance->Play(false, true);
	}
}


void SexyAppBase::PlaySample(int theSoundNum, int thePan)
{
	if (!mSoundManager)
		return;

	SoundInstance* aSoundInstance = mSoundManager->GetSoundInstance(theSoundNum);
	if (aSoundInstance != NULL)
	{
		aSoundInstance->SetPan(thePan);
		aSoundInstance->Play(false, true);
	}
}

bool SexyAppBase::IsMuted()
{
	return mMuteCount > 0;
}

void SexyAppBase::Mute(bool autoMute)
{
	mMuteCount++;
	if (autoMute)
		mAutoMuteCount++;

	SetMusicVolume(mMusicVolume);
	SetSfxVolume(mSfxVolume);
}

void SexyAppBase::Unmute(bool autoMute)
{
	if (mMuteCount > 0)
	{
		mMuteCount--;
		if (autoMute)
			mAutoMuteCount--;
	}

	SetMusicVolume(mMusicVolume);
	SetSfxVolume(mSfxVolume);
}


double SexyAppBase::GetMusicVolume()
{
	return mMusicVolume;
}

void SexyAppBase::SetMusicVolume(double theVolume)
{
	mMusicVolume = theVolume;

	if (mMusicInterface != NULL)
		mMusicInterface->SetVolume((mMuteCount > 0) ? 0.0 : mMusicVolume);
}

double SexyAppBase::GetSfxVolume()
{
	return mSfxVolume;
}

void SexyAppBase::SetSfxVolume(double theVolume)
{
	mSfxVolume = theVolume;

	if (mSoundManager != NULL)
		mSoundManager->SetVolume((mMuteCount > 0) ? 0.0 : mSfxVolume);
}

double SexyAppBase::GetMasterVolume()
{
	return mSoundManager->GetMasterVolume();
}

void SexyAppBase::SetMasterVolume(double theMasterVolume)
{
	mSfxVolume = theMasterVolume;
	mSoundManager->SetMasterVolume(mSfxVolume);
}

void SexyAppBase::AddMemoryImage(MemoryImage* theMemoryImage)
{
	AutoCrit anAutoCrit(mCritSect);
	mMemoryImageSet.insert(theMemoryImage);
}

void SexyAppBase::RemoveMemoryImage(MemoryImage* theMemoryImage)
{
	AutoCrit anAutoCrit(mCritSect);
	MemoryImageSet::iterator anItr = mMemoryImageSet.find(theMemoryImage);
	if (anItr != mMemoryImageSet.end())
		mMemoryImageSet.erase(anItr);

	Remove3DData(theMemoryImage);
}

void SexyAppBase::Remove3DData(MemoryImage* theMemoryImage)
{
}


SharedImageRef SexyAppBase::SetSharedImage(const std::string& theFileName, const std::string& theVariant, Image* theImage, bool* isNew)
{
	std::string anUpperFileName = StringToUpper(theFileName);
	std::string anUpperVariant = StringToUpper(theVariant);

	std::pair<SharedImageMap::iterator, bool> aResultPair;
	SharedImageRef aSharedImageRef;

	{
		AutoCrit anAutoCrit(mCritSect);
		aResultPair = mSharedImageMap.insert(SharedImageMap::value_type(SharedImageMap::key_type(anUpperFileName, anUpperVariant), SharedImage()));
		aSharedImageRef = &aResultPair.first->second;
	}

	if (isNew != NULL)
		*isNew = aResultPair.second;

	if (aResultPair.second)
	{
		aSharedImageRef.mSharedImage->mImage = theImage;
	}

	return aSharedImageRef;
}

SharedImageRef SexyAppBase::GetSharedImage(const std::string& theFileName, const std::string& theVariant, bool* isNew)
{
	std::string anUpperFileName = StringToUpper(theFileName);
	std::string anUpperVariant = StringToUpper(theVariant);

	std::pair<SharedImageMap::iterator, bool> aResultPair;
	SharedImageRef aSharedImageRef;

	{
		AutoCrit anAutoCrit(mCritSect);
		aResultPair = mSharedImageMap.insert(SharedImageMap::value_type(SharedImageMap::key_type(anUpperFileName, anUpperVariant), SharedImage()));
		aSharedImageRef = &aResultPair.first->second;
	}

	if (isNew != NULL)
		*isNew = aResultPair.second;

	if (aResultPair.second)
	{
		// Pass in a '!' as the first char of the file name to create a new image
		if ((theFileName.length() > 0) && (theFileName[0] == '!'))
		{
			MemoryImage* anImage = new MemoryImage(this);
			aSharedImageRef.mSharedImage->mImage = anImage;
		}
		else
			aSharedImageRef.mSharedImage->mImage = GetImage(theFileName,false);
	}

	return aSharedImageRef;
}

void SexyAppBase::CleanSharedImages()
{
	AutoCrit anAutoCrit(mCritSect);

	if (mCleanupSharedImages)
	{
		// Delete shared images with reference counts of 0
		// This doesn't occur in ~SharedImageRef because sometimes we can not only access the image
		//  through the SharedImageRef returned by GetSharedImage, but also by calling GetSharedImage
		//  again with the same params -- so we can have instances where we do the 'final' deref on
		//  an image but immediately re-request it via GetSharedImage
		SharedImageMap::iterator aSharedImageItr = mSharedImageMap.begin();
		while (aSharedImageItr != mSharedImageMap.end())
		{
			SharedImage* aSharedImage = &aSharedImageItr->second;
			if (aSharedImage->mRefCount == 0)
			{
				delete aSharedImage->mImage;
				mSharedImageMap.erase(aSharedImageItr++);
			}
			else
				++aSharedImageItr;
		}

		mCleanupSharedImages = false;
	}
}
