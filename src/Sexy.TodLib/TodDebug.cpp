#include <time.h>
#include "TodDebug.h"
#include "TodCommon.h"
#include "SexyAppFramework/SexyAppBase.h"
#include "SDL.h"

using namespace Sexy;

static char gLogFileName[512];
static char gDebugDataFolder[512];

void TodErrorMessageBox(const char* theMessage, const char* theTitle)
{
    TodTraceAndLog("%s.%s", theMessage, theTitle);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, theTitle, theMessage, nullptr);
}

void TodTraceMemory()
{
}

void* TodMalloc(int theSize)
{
	TOD_ASSERT(theSize > 0);
	return malloc(theSize);
}

void TodFree(void* theBlock)
{
	if (theBlock != nullptr)
	{
		free(theBlock);
	}
}

void TodAssertFailed(const char* theCondition, const char* theFile, int theLine, const char* theMsg, ...)
{
#ifdef _DEBUG
    char aFormattedMsg[1024];
    va_list argList;
    va_start(argList, theMsg);
    int aCount = TodVsnprintf(aFormattedMsg, sizeof(aFormattedMsg), theMsg, argList);
    va_end(argList);

    if (aCount != 0) {
        if (aFormattedMsg[aCount - 1] != '\n')
        {
            if (aCount + 1 < 1024)
            {
                aFormattedMsg[aCount] = '\n';
                aFormattedMsg[aCount + 1] = '\0';
            }
            else
            {
                aFormattedMsg[aCount - 1] = '\n';
            }
        }
    }

    char aBuffer[1024];
    if (*theCondition != '\0')
    {
        TodSnprintf(aBuffer, sizeof(aBuffer), "\n%s(%d)\nassertion failed: '%s'\n%s\n", theFile, theLine, theCondition, aFormattedMsg);
    }
    else
    {
        TodSnprintf(aBuffer, sizeof(aBuffer), "\n%s(%d)\nassertion failed: %s\n", theFile, theLine, aFormattedMsg);
    }
    TodTrace("%s", aBuffer);

    TodTraceAndLog(aBuffer, "Assertion failed");
#endif
}

void TodLog(const char* theFormat, ...)
{
#ifdef _DEBUG
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	TodLogString(aButter);
#endif
}

void TodLogString(const char* theMsg)
{
#ifdef _DEBUG
	FILE* f = fopen(gLogFileName, "a");
	if (f == nullptr)
	{
		fprintf(stderr, __S("Failed to open log file '%s'\n"), gLogFileName);
		return;
	}

	if (fwrite(theMsg, strlen(theMsg), 1, f) != 1)
	{
		fprintf(stderr, __S("Failed to write to log file\n"));
	}

	fclose(f);
#endif
}

void TodTrace(const char* theFormat, ...)
{
#ifdef _DEBUG
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	printf("%s", aButter);
#endif
}

void TodHesitationTrace(...)
{
}

void TodTraceAndLog(const char* theFormat, ...)
{
#ifdef _DEBUG
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	printf("%s", aButter);
	TodLogString(aButter);
#endif
}

void TodTraceWithoutSpamming(const char* theFormat, ...)
{
#ifdef _DEBUG
	static time_t gLastTraceTime = 0;
	time_t aTime = time(nullptr);
	if (aTime < gLastTraceTime)
	{
		return;
	}

	gLastTraceTime = aTime;
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	printf("%s", aButter);
#endif
}

void (*gBetaSubmitFunc)() = nullptr;

void TodAssertInitForApp()
{
	MkDir(GetAppDataFolder());
	MkDir(GetAppDataFolder() + "userdata");
	std::string aRelativeUserPath = GetAppDataFolder() + "userdata/";
	strcpy(gDebugDataFolder, aRelativeUserPath.c_str());
	std::string logPath = GetAppDataFolder() + "log.txt";
	strcpy(gLogFileName, logPath.c_str());
	TOD_ASSERT(strlen(gLogFileName) < 512);

	time_t aclock = time(nullptr);
	struct tm* timeinfo = localtime(&aclock);
	char timeStr[64];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
	TodLog("Started %s\n", timeStr);
}
