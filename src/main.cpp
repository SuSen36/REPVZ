#define SDL_MAIN_HANDLED
#include "LawnApp.h"
#include "../src/Resources.h"
#include "../../Sexy.TodLib/TodStringFile.h"
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

// Logging function
void log(const std::string& message) {
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "LawnApp", "%s", message.c_str());
#else
	std::cout << message << std::endl;
#endif
}

// Game run function
void runGame() {
	// Initialize SDL
	SDL_SetMainReady();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		log("SDL Initialization failed: " + std::string(SDL_GetError()));
		return;
	}

	// Initialize necessary settings
	TodStringListSetColors(gLawnStringFormats, gLawnStringFormatCount);
	gGetCurrentLevelName = LawnGetCurrentLevelName;
	gAppCloseRequest = LawnGetCloseRequest;
	gAppHasUsedCheatKeys = LawnHasUsedCheatKeys;
	gExtractResourcesByName = Sexy::ExtractResourcesByName;

	// Create an instance of LawnApp
	std::unique_ptr<LawnApp> gLawnApp(new LawnApp());
	if (gLawnApp) {
		gLawnApp->Init();
		gLawnApp->Start();
		gLawnApp->Shutdown();
	} else {
		log("Failed to create LawnApp instance.");
	}

	SDL_Quit();  // Quit SDL
}

// Android JNI implementation
#ifdef ANDROID

int SDL_main(int argc, char *argv[]) {
	runGame();  // Run the game on Windows
	return 0;  // Exit the program
}
#else

// Windows entry point
int main(int argc, char *argv[]) {
	runGame();  // Run the game on Windows
	return 0;  // Exit the program
}
#endif
