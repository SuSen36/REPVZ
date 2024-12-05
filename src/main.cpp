#define SDL_MAIN_HANDLED
#include "LawnApp.h"
#include "Resources.h"
#include "Sexy.TodLib/TodStringFile.h"
#include "Sexy.TodLib/TodDebug.h"
#include <SDL.h>
#include <iostream>
#include <memory>


using namespace Sexy;

// Function pointers for game operations
bool (*gAppCloseRequest)();
bool (*gAppHasUsedCheatKeys)();
SexyString (*gGetCurrentLevelName)();

// Game run function
void runGame() {
	// Initialize SDL
	SDL_SetMainReady();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        TodLog("SDL Initialization failed: " , SDL_GetError());
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
		TodLog("Failed to create LawnApp instance.");
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
