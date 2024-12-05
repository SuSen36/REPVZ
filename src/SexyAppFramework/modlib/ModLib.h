#ifndef __MODLIB_H__
#define __MODLIB_H__

#include <string>

namespace ModLib
{

class Mod
{
public:
    std::string				mName;       // Ãû×Ö
    int						mVersion;    // °æ±¾
	uint32_t*				mBits;

public:
	Mod();
	virtual ~Mod();

	std::basic_string<char> GetName();
	int						GetVersion();
	uint32_t*				GetBits();
};

bool WriteDLLMod(const std::string& theFileName, Mod* theMod);
bool WriteSOMod(const std::string& theFileName, Mod* theMod);

extern int gAlphaComposeColor;
extern bool gLoadDLLs;
extern bool gLoadSOs;


Mod* GetMod(const std::string& theFileName, bool lookForAlphaImage = true);

}

#endif //__MODLIB_H__