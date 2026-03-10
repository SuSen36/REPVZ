#ifndef __SEXYAPPBASE_H__
#define __SEXYAPPBASE_H__

#include "Common.h"
#include <SDL_blendmode.h>
#include "SexyAppFramework/misc/Rect.h"
#include "SexyAppFramework/graphics/Color.h"
#include "SexyAppFramework/widget/ButtonListener.h"
#include "SexyAppFramework/widget/DialogListener.h"
#include "SexyAppFramework/misc/Buffer.h"
#include "SexyAppFramework/misc/CritSect.h"
#include "SexyAppFramework/graphics/SharedImage.h"
#include "SexyAppFramework/misc/Ratio.h"
#include "SexyAppFramework/misc/ResourceManager.h"

namespace ImageLib
{
	class Image;
};

struct SDL_Renderer;

namespace Sexy
{

extern SDL_Renderer* gRenderer;
extern SDL_BlendMode gPremultipliedBlendMode;
extern SDL_BlendMode gAdditiveBlendMode;

class WidgetManager;
class Image;
class Widget;
class SoundManager;
class MusicInterface;
class MemoryImage;
class HTTPTransfer;
class Dialog;

class WidgetSafeDeleteInfo
{
public:
	int						mUpdateAppDepth;
	Widget*					mWidget;
};


typedef std::list<WidgetSafeDeleteInfo> WidgetSafeDeleteList;
typedef std::set<MemoryImage*> MemoryImageSet;
typedef std::map<int, Dialog*> DialogMap;
typedef std::list<Dialog*> DialogList;
//typedef std::list<MSG> WindowsMessageList;
typedef std::vector<std::string> StringVector;
//typedef std::basic_string<TCHAR> tstring; // string of TCHARs

typedef std::map<std::string, SexyString> StringSexyStringMap;
typedef std::map<std::string, std::string> StringStringMap;
typedef std::map<std::string, std::wstring> StringWStringMap;
typedef std::map<std::string, bool> StringBoolMap;
typedef std::map<std::string, int> StringIntMap;
typedef std::map<std::string, double> StringDoubleMap;
typedef std::map<std::string, StringVector> StringStringVectorMap;

enum
{
	CURSOR_POINTER,
	CURSOR_HAND,
	CURSOR_DRAGGING,
	CURSOR_TEXT,
	CURSOR_CIRCLE_SLASH,
	CURSOR_SIZEALL,
	CURSOR_SIZENESW,
	CURSOR_SIZENS,
	CURSOR_SIZENWSE,
	CURSOR_SIZEWE,	
	CURSOR_WAIT,
	CURSOR_NONE,
	CURSOR_CUSTOM,
	NUM_CURSORS
};

enum {
	FPS_ShowFPS,
	FPS_ShowCoords,
	Num_FPS_Types
};

enum
{
	UPDATESTATE_MESSAGES,
	UPDATESTATE_PROCESS_1,
	UPDATESTATE_PROCESS_2,
	UPDATESTATE_PROCESS_DONE
};

typedef std::map<HANDLE, int> HandleToIntMap;

class SexyAppBase : public ButtonListener, public DialogListener
{
public:
	void*					mWindow;
	void*					mContext;
	void*					mSurface; // for EGL

	uint32_t				mRandSeed{};
		
	std::string				mCompanyName;
	std::string				mFullCompanyName;
	std::string				mProdName;	
	SexyString				mTitle;	
	std::string				mRegKey;
	std::string				mChangeDirTo;
	
	int						mRelaxUpdateBacklogCount; // app doesn't try to catch up for this many frames
	int						mPreferredX;
	int						mPreferredY;
	int						mWidth;
	int						mHeight;
	int						mFullscreenBits;
	double					mMusicVolume;
	double					mSfxVolume;
	bool					mNoSoundNeeded;
	bool					mWantFMod;
	bool					mSkipSignatureChecks;
	bool					mStandardWordWrap;
	bool					mbAllowExtendedChars;

	HANDLE					mMutex{};
	bool					mOnlyAllowOneCopyToRun;
	UINT					mNotifyGameMessage;
	CritSect				mCritSect;
	uchar					mAdd8BitMaxTable[512]{};
	WidgetManager*			mWidgetManager;
	DialogMap				mDialogMap;
	DialogList				mDialogList;
	void*					mPrimaryThreadId;
	bool					mShutdown;
	bool					mExitToTop;
	bool					mIsWindowed;
	bool					mIsPhysWindowed;
	bool					mFullScreenWindow; // uses ChangeDisplaySettings to run fullscreen with mIsWindowed true
	bool					mForceFullscreen;
	bool					mForceWindowed;	
	bool					mInitialized;	
	bool					mProcessInTimer;
	DWORD					mTimeLoaded;
	//HWND					mHWnd;
	//HWND					mInvisHWnd;
	bool					mAllowMonitorPowersave;
	//WindowsMessageList		mDeferredMessages;
	bool					mNoDefer;	
	bool					mFullScreenPageFlip;	
	// mTabletPC: Indicates if the device is a tablet/touch device.
	// When true: Enables touch-specific features like drag-and-drop planting, virtual keyboard support.
	// When false: Uses traditional mouse/keyboard input (PC mode).
	// Android/iOS platforms default to true, PC platforms default to false.
	bool					mTabletPC;
	bool					mAlphaDisabled;
	MusicInterface*			mMusicInterface;	
	bool					mReadFromRegistry;
	std::string				mRegisterLink;
	std::string				mProductVersion;	
	Image*					mCursorImages[NUM_CURSORS]{};
	//HCURSOR					mOverrideCursor;
	bool					mIsOpeningURL;
	bool					mShutdownOnURLOpen{};
	std::string				mOpeningURL;
	DWORD					mOpeningURLTime{};
	DWORD					mLastTimerTime{};
	DWORD					mLastBigDelayTime{};
	double					mUnmutedMusicVolume{};
	double					mUnmutedSfxVolume{};
	int						mMuteCount;
	int						mAutoMuteCount;
	bool					mMuteOnLostFocus;
	MemoryImageSet			mMemoryImageSet;
	SharedImageMap			mSharedImageMap;
	bool					mCleanupSharedImages;
	
	int						mNonDrawCount;
	int						mFrameTime;

	bool					mIsDrawing;
	bool					mLastDrawWasEmpty;
	bool					mHasPendingDraw;
	double					mPendingUpdatesAcc;
	double					mUpdateFTimeAcc;
	time_t					mLastTimeCheck;
	time_t					mLastTime{};
	DWORD					mLastUserInputTick{};

	int						mSleepCount;
	int						mDrawCount;
	int						mUpdateCount;
	int						mUpdateAppState;
	int						mUpdateAppDepth;
	double					mUpdateMultiplier;		
	bool					mPaused;
	int						mFastForwardToUpdateNum{};
	bool					mFastForwardToMarker{};
	bool					mFastForwardStep{};
	DWORD					mLastDrawTick;
	DWORD					mNextDrawTick;
	int						mStepMode;  // 0 = off, 1 = step, 2 = waiting for step

	int						mCursorNum;
	SoundManager*			mSoundManager;
	//HCURSOR					mHandCursor;
	//HCURSOR					mDraggingCursor;
	WidgetSafeDeleteList	mSafeDeleteList;
	bool					mMouseIn;	
	bool					mRunning;
	bool					mActive;
	bool					mMinimized;
	bool					mPhysMinimized;
	bool					mIsDisabled;
	bool					mHasFocus;
	int						mDrawTime;
	uint32_t					mFPSStartTick;
	int						mFPSFlipCount;
	int						mFPSDirtyCount;
	int						mFPSTime;
	int						mFPSCount;
	int						mScreenBltTime;
	bool					mAutoStartLoadingThread;
	bool					mLoadingThreadStarted;
	bool					mLoadingThreadCompleted;
	bool					mLoaded;
	bool					mYieldMainThread;
	bool					mLoadingFailed;
	bool					mCustomCursorsEnabled;
	bool					mCustomCursorDirty;	
	bool					mLastShutdownWasGraceful;
	bool					mIsWideWindow;
	bool					mWriteToSexyCache;
	bool					mSexyCacheBuffers;

	int						mNumLoadingThreadTasks;
	int						mCompletedLoadingThreadTasks;

	bool					mManualShutdown;
	HandleToIntMap			mHandleToIntMap; // For waiting on handles
	int						mCurHandleNum;

	bool					mDebugKeysEnabled;
	bool					mEnableMaximizeButton;
	bool					mCtrlDown;
	bool					mAltDown;
	bool					mAllowAltEnter;
	
	int						mSyncRefreshRate;
	bool					mVSyncUpdates;
	bool					mVSyncBroken;
	int						mVSyncBrokenCount;
	DWORD					mVSyncBrokenTestStartTick;
	DWORD					mVSyncBrokenTestUpdates;
	bool					mWaitForVSync;
	bool					mSoftVSyncWait;

	bool					mWidescreenAware;
	Rect					mScreenBounds;
	bool					mEnableWindowAspect;
	Ratio					mWindowAspect;

	StringWStringMap		mStringProperties;
	StringBoolMap			mBoolProperties;
	StringIntMap			mIntProperties;
	StringDoubleMap			mDoubleProperties;
	StringStringVectorMap	mStringVectorProperties;
	ResourceManager*		mResourceManager;

	LONG					mOldWndProc;

protected:	
	void					RehupFocus();
	void					ClearKeysDown();
	bool					ProcessDeferredMessages(bool singleMessage);
	void					UpdateFTimeAcc();
	virtual bool			Process(bool allowSleep = true);		
	virtual void			UpdateFrames();
	virtual bool			DoUpdateFrames();
	virtual void			DoUpdateFramesF(float theFrac);
	virtual void			MakeWindow();
	virtual void			EnforceCursor();
	virtual void			ReInitImages();
	virtual void			DeleteNativeImageData();	
	virtual void			DeleteExtraImageData();
	
	// Loading thread methods	
	virtual void			LoadingThreadCompleted();
	static int			    LoadingThreadProcStub(void *theArg);

	// Cursor thread methods
	
	void					WaitForLoadingThread();				
	void					ProcessSafeDeleteList();	
	void					RestoreScreenResolution();
	void					DoExit(int theCode);

	// Registry helpers
	bool					RegistryRead(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength);
	bool					RegistryReadKey(const std::string& theValueName, uint32_t* theType, uchar* theValue, uint32_t* theLength);
	bool					RegistryWrite(const std::string& theValueName, uint32_t theType, const uchar* theValue, uint32_t theLength);


public:
	SexyAppBase();
	virtual ~SexyAppBase();

	// Common overrides:
	virtual MusicInterface*	CreateMusicInterface();
	virtual void			InitHook();
	virtual void			ShutdownHook();	
	virtual void			PreTerminate();
	virtual void			LoadingThreadProc();
	virtual void			WriteToRegistry();
	virtual void			ReadFromRegistry();
	virtual Dialog*			NewDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode);		
	virtual void			PreDisplayHook();

	// Public methods
	virtual void			BeginPopup();
	virtual void			EndPopup();
	virtual int				MsgBox(const std::string &theText, const std::string &theTitle = "Message", int theFlags = 0);
	virtual int				MsgBox(const std::wstring &theText, const std::wstring &theTitle = L"Message", int theFlags = 0);
	virtual void			Popup(const std::string& theString);
	virtual void			Popup(const std::wstring& theString);
	virtual void			SafeDeleteWidget(Widget* theWidget);	

	virtual void			URLOpenFailed(const std::string& theURL);
	virtual void			URLOpenSucceeded(const std::string& theURL);
	virtual bool			OpenURL(const std::string& theURL, bool shutdownOnOpen = false);	
	virtual std::string		GetProductVersion(const std::string& thePath);	

	virtual void			Shutdown();	

	virtual void			HandleNotifyGameMessage(int theType); // for HWND_BROADCAST of mNotifyGameMessage (0-1000 are reserved for SexyAppBase for theType)
	virtual void			HandleGameAlreadyRunning(); 

	virtual void			Start();	
	virtual void			Init();
	virtual void			PreGLInterfaceInitHook();
	virtual void			PostGLInterfaceInitHook();
	virtual bool			ChangeDirHook(const char *theIntendedPath);
	virtual void			PlaySample(int theSoundNum);
	virtual void			PlaySample(int theSoundNum, int thePan);

	virtual double			GetMasterVolume();
	virtual double			GetMusicVolume();
	virtual double			GetSfxVolume();
	virtual bool			IsMuted();

	virtual void			SetMasterVolume(double theVolume);
	virtual void			SetMusicVolume(double theVolume);
	virtual void			SetSfxVolume(double theVolume);	
	virtual void			Mute(bool autoMute = false);
	virtual void			Unmute(bool autoMute = false);

	void					StartLoadingThread();
	virtual double			GetLoadingThreadProgress();	

	void					CopyToClipboard(const std::string& theString);
	std::string				GetClipboard();

	void					SetCursor(int theCursorNum);
	int						GetCursor();
	void					EnableCustomCursors(bool enabled);	
	virtual Image*			GetImage(const std::string& theFileName, bool commitBits = true);	
	virtual SharedImageRef	SetSharedImage(const std::string& theFileName, const std::string& theVariant, Image* theImage, bool* isNew);
	virtual SharedImageRef	GetSharedImage(const std::string& theFileName, const std::string& theVariant = "", bool* isNew = NULL);
	void					CleanSharedImages();

	void					SetCursorImage(int theCursorNum, Image* theImage);

	Image*					CreateCrossfadeImage(Image* theImage1, const Rect& theRect1, Image* theImage2, const Rect& theRect2, double theFadeFactor);
	void					ColorizeImage(Image* theImage, const Color& theColor);
	Image*					CreateColorizedImage(Image* theImage, const Color& theColor);
	Image*					CopyImage(Image* theImage, const Rect& theRect);
	Image*					CopyImage(Image* theImage);
	void					MirrorImage(Image* theImage);
	void					FlipImage(Image* theImage);
	void					RotateImageHue(Sexy::MemoryImage *theImage, int theDelta);
	uint32_t				HSLToRGB(int h, int s, int l);
	uint32_t				RGBToHSL(int r, int g, int b);
	void					HSLToRGB(const uint32_t* theSource, uint32_t* theDest, int theSize);
	void					RGBToHSL(const uint32_t* theSource, uint32_t* theDest, int theSize);

	void					AddMemoryImage(MemoryImage* theMemoryImage);
	void					RemoveMemoryImage(MemoryImage* theMemoryImage);
	void					Remove3DData(MemoryImage* theMemoryImage);
	virtual void			SwitchScreenMode();
	virtual void			SwitchScreenMode(bool wantWindowed);
	virtual void			SwitchScreenMode(bool wantWindowed,bool force = false);
	virtual void			SetAlphaDisabled(bool isDisabled);
	
	virtual Dialog*			DoDialog(int theDialogId, bool isModal, const SexyString& theDialogHeader, const SexyString& theDialogLines, const SexyString& theDialogFooter, int theButtonMode);
	virtual Dialog*			GetDialog(int theDialogId);
	virtual void			AddDialog(int theDialogId, Dialog* theDialog);
	virtual void			AddDialog(Dialog* theDialog);
	virtual bool			KillDialog(int theDialogId, bool removeWidget, bool deleteWidget);
	virtual bool			KillDialog(int theDialogId);
	virtual bool			KillDialog(Dialog* theDialog);
	virtual int				GetDialogCount();
	virtual void			ModalOpen();
	virtual void			ModalClose();	
	virtual void			DialogButtonPress(int theDialogId, int theButtonId);
	virtual void			DialogButtonDepress(int theDialogId, int theButtonId);

	virtual void			GotFocus();
	virtual void			LostFocus();
	virtual bool			DebugKeyDown(int theKey);
//	virtual bool			DebugKeyDownAsync(int theKey, bool ctrlDown, bool altDown);
	virtual void			CloseRequestAsync();
	void					InitInput();
	bool					StartTextInput(std::string& theInput); // set theInput and return true if using soft keyboard capability and user pressed OK (e.g. Switch libnx swkbd)
	void					StopTextInput();
	virtual std::string		NotifyCrashHook(); // return file name that you want to upload
	
	virtual bool			DrawDirtyStuff();
	virtual void			Redraw(Rect* theClipRect);

	// Properties access methods
	bool					LoadProperties(const std::string& theFileName, bool required, bool checkSig);
	bool					LoadProperties();
	virtual void			InitPropertiesHook();

	// Resource access methods
	void					LoadResourceManifest();
	void					ShowResourceError(bool doExit = false);
	
	bool					GetBoolean(const std::string& theId);
	bool					GetBoolean(const std::string& theId, bool theDefault);	
	int						GetInteger(const std::string& theId);
	int						GetInteger(const std::string& theId, int theDefault);
	double					GetDouble(const std::string& theId);
	double					GetDouble(const std::string& theId, double theDefault);
	SexyString				GetString(const std::string& theId);
	SexyString				GetString(const std::string& theId, const SexyString& theDefault);

	StringVector			GetStringVector(const std::string& theId);

	void					SetBoolean(const std::string& theId, bool theValue);
	void					SetInteger(const std::string& theId, int theValue);
	void					SetDouble(const std::string& theId, double theValue);
	void					SetString(const std::string& theId, const std::wstring& theValue);
	
	

	// Registry access methods
	//bool					RegistryGetSubKeys(const std::string& theKeyName, StringVector* theSubKeys);
	bool					RegistryReadString(const std::string& theValueName, std::string* theString);
	bool					RegistryReadInteger(const std::string& theValueName, int* theValue);
	bool					RegistryReadBoolean(const std::string& theValueName, bool* theValue);
	bool					RegistryReadData(const std::string& theValueName, uchar* theValue, uint32_t* theLength);
	bool					RegistryWriteString(const std::string& theValueName, const std::string& theString);
	bool					RegistryWriteInteger(const std::string& theValueName, int theValue);
	bool					RegistryWriteBoolean(const std::string& theValueName, bool theValue);
	bool					RegistryWriteData(const std::string& theValueName, const uchar* theValue, uint32_t theLength);	
	bool					RegistryEraseKey(const SexyString& theKeyName);
	void					RegistryEraseValue(const SexyString& theValueName);

	// File access methods
	bool					WriteBufferToFile(const std::string& theFileName, const Buffer* theBuffer);
    bool                    ReadBufferFromFile(const std::string& theFileName, Buffer* theBuffer);
	bool					WriteBytesToFile(const std::string& theFileName, const void *theData, ulong theDataLen);
	bool					FileExists(const std::string& theFileName);
	bool					EraseFile(const std::string& theFileName);

	// Misc methods
	virtual void			DoMainLoop();
	virtual bool			UpdateAppStep(bool* updated);
	virtual bool			UpdateApp();
	int						InitGraphics();
	void					ClearUpdateBacklog(bool relaxForASecond = false);
	virtual bool			AppCanRestore();
};

extern SexyAppBase* gSexyAppBase;

};

#endif //__SEXYAPPBASE_H__
