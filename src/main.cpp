#include "LawnApp.h"
#include "Resources.h"
#include "Sexy.TodLib/TodStringFile.h"
#include "Sexy.TodLib/TodDebug.h"
#include <SDL.h>
#include <iostream>
#include <memory>

#ifdef ANDROID
#include <android/log.h>  // Android logging
#include <jni.h>
#include <android/asset_manager_jni.h>
#endif

using namespace Sexy;

// Function pointers for game operations
bool (*gAppCloseRequest)();
bool (*gAppHasUsedCheatKeys)();
SexyString (*gGetCurrentLevelName)();

void runGame() {
    // Initialize SDL
    SDL_SetMainReady();

    // Initialize necessary settings
    TodStringListSetColors(gLawnStringFormats, gLawnStringFormatCount);
    gGetCurrentLevelName = LawnGetCurrentLevelName;
    gAppCloseRequest = LawnGetCloseRequest;
    gAppHasUsedCheatKeys = LawnHasUsedCheatKeys;
    gExtractResourcesByName = Sexy::ExtractResourcesByName;

    // Create an instance of LawnApp using the global pointer
    gLawnApp = new LawnApp();
    if (gLawnApp) {
        gLawnApp->Init();
        gLawnApp->Start();
        gLawnApp->Shutdown();
    } else {
        TodLog("Failed to create LawnApp instance.");
    }

    // Clean up the global LawnApp instance
    delete gLawnApp;
    gLawnApp = nullptr;

    SDL_Quit();  // Quit SDL
}

// Android JNI implementation
#ifdef ANDROID
extern "C" int SDL_main(int argc, char *argv[]) {
	runGame();  // Run the game on Windows
	return 0;  // Exit the program
}
#else
// Windows entry point
extern "C" int main(int argc, char *argv[]) {
	runGame();  // Run the game on Windows
	return 0;  // Exit the program
}
#endif
