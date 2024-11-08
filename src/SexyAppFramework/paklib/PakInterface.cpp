#include <unistd.h>
#include <filesystem>
#include <fstream>
#include "../Common.h"
#include "PakInterface.h"
#include "SexyAppFramework/fcaseopen/fcaseopen.h"

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

enum
{
	FILEFLAGS_END = 0x80
};

PakInterface* gPakInterface = new PakInterface();

static std::string StringToUpper(const std::string& theString)
{
	std::string aString;

	for (unsigned i = 0; i < theString.length(); i++)
		aString += toupper(theString[i]);

	return aString;
}

PakInterface::PakInterface()
{
	//if (GetPakPtr() == NULL)
		//*gPakInterfaceP = this;
}

PakInterface::~PakInterface()
{
}

//0x5D84D0
static void FixFileName(const char* theFileName, char* theUpperName)
{
#ifdef ANDROID
    // 在 Android 平台上不使用工作目录，只处理斜杠和大写
    const char* aSrc = theFileName;
    char* aDest = theUpperName;

    bool lastSlash = false;
    for (;;)
    {
        char c = *(aSrc++);

        if ((c == '\\') || (c == '/'))
        {
            // 统一转为右斜杠，且多个斜杠的情况下只保留一个
            if (!lastSlash)
                *(aDest++) = '/';
            lastSlash = true;
        }
        else
        {
            *(aDest++) = toupper((uchar)c);
            if (c == 0)
                break;
            lastSlash = false;
        }
    }
#else
    // 检测路径是否为从盘符开始的绝对路径
    if ((theFileName[0] != 0) && (theFileName[1] == ':'))
    {
        char aDir[256];
        getcwd(aDir, 256);  // 取得当前工作路径
        int aLen = strlen(aDir);
        aDir[aLen++] = '/';
        aDir[aLen] = 0;

        // 判断 theFileName 文件是否位于当前目录下
        if (strncasecmp(aDir, theFileName, aLen) == 0)
            theFileName += aLen;  // 若是，则跳过从盘符到当前目录的部分，转化为相对路径
    }

    const char* aSrc = theFileName;
    char* aDest = theUpperName;
    bool lastSlash = false;

    for (;;)
    {
        char c = *(aSrc++);

        if ((c == '\\') || (c == '/'))
        {
            // 统一转为右斜杠，且多个斜杠的情况下只保留一个
            if (!lastSlash)
                *(aDest++) = '/';
            lastSlash = true;
        }
        else if ((c == '.') && (lastSlash) && (*aSrc == '.'))
        {
            // We have a '/..' on our hands
            aDest--;
            while ((aDest > theUpperName + 1) && (*(aDest - 1) != '\\'))  // 回退到上一层目录
                --aDest;
            aSrc++;
            // 此处将形如“a\b\..\c”的路径简化为“a\c”
        }
        else
        {
            *(aDest++) = toupper((uchar)c);
            if (c == 0)
                break;
            lastSlash = false;
        }
    }
#endif

    // 结束字符串并确保以 null 结尾
    *aDest = '\0';
}

bool PakInterface::AddPakFile(const std::string& theFileName)
{
#ifdef ANDROID
    // 在 Android 平台上使用 AAssetManager 来打开和读取 PAK 文件
    AAsset* asset = AAssetManager_open(Sexy::gAssetsManager, theFileName.c_str(), AASSET_MODE_STREAMING);
    if (!asset) return false;

    off_t aFileSize = AAsset_getLength(asset);
    mPakCollectionList.emplace_back(aFileSize);
    PakCollection* aPakCollection = &mPakCollectionList.back();

    AAsset_read(asset, aPakCollection->mDataPtr, aFileSize);
    AAsset_close(asset);
#else
    // 在其他平台上使用标准文件操作
    FILE *aFileHandle = fcaseopen(theFileName.c_str(), "rb");
    if (!aFileHandle) return false;

    fseek(aFileHandle, 0, SEEK_END);
    size_t aFileSize = ftell(aFileHandle);
    fseek(aFileHandle, 0, SEEK_SET);

    mPakCollectionList.emplace_back(aFileSize);
    PakCollection* aPakCollection = &mPakCollectionList.back();

    if (fread(aPakCollection->mDataPtr, 1, aFileSize, aFileHandle) != aFileSize) {
        fclose(aFileHandle);
        return false;
    }
    fclose(aFileHandle);
#endif

    // 对读取的数据进行异或处理
    {
        auto *aDataPtr = static_cast<uint8_t *>(aPakCollection->mDataPtr);
        for (size_t i = 0; i < aFileSize; i++)
            *aDataPtr++ ^= 0xF7;
    }

    PakRecordMap::iterator aRecordItr = mPakRecordMap.insert(PakRecordMap::value_type(StringToUpper(theFileName), PakRecord())).first;
    PakRecord* aPakRecord = &(aRecordItr->second);
    aPakRecord->mCollection = aPakCollection;
    aPakRecord->mFileName = theFileName;
    aPakRecord->mStartPos = 0;
    aPakRecord->mSize = aFileSize;

    // 使用 FOpen 函数处理 PAK 文件读取（此处可能需要适配）
    PFILE* aFP = FOpen(theFileName.c_str(), "rb");
    if (aFP == NULL)
        return false;

    uint32_t aMagic = 0;
    FRead(&aMagic, sizeof(uint32_t), 1, aFP);
    if (aMagic != 0xBAC04AC0)
    {
        FClose(aFP);
        return false;
    }

    uint32_t aVersion = 0;
    FRead(&aVersion, sizeof(uint32_t), 1, aFP);
    if (aVersion > 0)
    {
        FClose(aFP);
        return false;
    }

    int aPos = 0;

    for (;;)
    {
        uchar aFlags = 0;
        int aCount = FRead(&aFlags, 1, 1, aFP);
        if ((aFlags & FILEFLAGS_END) || (aCount == 0))
            break;

        uchar aNameWidth = 0;
        char aName[256];
        FRead(&aNameWidth, 1, 1, aFP);
        FRead(aName, 1, aNameWidth, aFP);
        aName[aNameWidth] = 0;
        int aSrcSize = 0;
        FRead(&aSrcSize, sizeof(int), 1, aFP);
        int64_t aFileTime;
        FRead(&aFileTime, sizeof(int64_t), 1, aFP);

        for (int i = 0; i < aNameWidth; i++)
        {
            if (aName[i] == '\\')
                aName[i] = '/'; // lol
        }

        char anUpperName[256];
        FixFileName(aName, anUpperName);

        PakRecordMap::iterator aRecordItr = mPakRecordMap.insert(PakRecordMap::value_type(StringToUpper(aName), PakRecord())).first;
        PakRecord* aPakRecord = &(aRecordItr->second);
        aPakRecord->mCollection = aPakCollection;
        aPakRecord->mFileName = anUpperName;
        aPakRecord->mStartPos = aPos;
        aPakRecord->mSize = aSrcSize;
        aPakRecord->mFileTime = aFileTime;

        aPos += aSrcSize;
    }

    int anOffset = FTell(aFP);

    // 现在修正文件的起始位置
    aRecordItr = mPakRecordMap.begin();
    while (aRecordItr != mPakRecordMap.end())
    {
        PakRecord* aPakRecord = &(aRecordItr->second);
        if (aPakRecord->mCollection == aPakCollection)
            aPakRecord->mStartPos += anOffset;
        ++aRecordItr;
    }

    FClose(aFP);

    return true;
}

bool PakInterface::AddDirectory(const std::string& theFileName)
{
#ifdef ANDROID
    // 在 Android 平台上使用 AAssetManager 来打开和读取文件夹中的资产
    AAssetManager* assetManager = Sexy::gAssetsManager; // 获取资产管理器
    AAssetDir* assetDir = AAssetManager_openDir(assetManager, theFileName.c_str());
    if (!assetDir) return false;

    const char* fileName;
    while ((fileName = AAssetDir_getNextFileName(assetDir)) != nullptr) {
        std::string fullPath = theFileName + "/" + fileName;
        AAsset* asset = AAssetManager_open(assetManager, fullPath.c_str(), AASSET_MODE_STREAMING);
        if (!asset) {
            continue; // 如果无法打开资产，跳过
        }

        off_t fileSize = AAsset_getLength(asset);
        mPakCollectionList.emplace_back(fileSize);
        PakCollection* aPakCollection = &mPakCollectionList.back();

        AAsset_read(asset, aPakCollection->mDataPtr, fileSize);
        AAsset_close(asset);

        // 对读取的数据进行异或处理
        auto *aDataPtr = static_cast<uint8_t *>(aPakCollection->mDataPtr);
        for (size_t i = 0; i < fileSize; i++) {
            *aDataPtr++ ^= 0xF7;
        }

        // 生成文件记录
        PakRecordMap::iterator aRecordItr = mPakRecordMap.insert(PakRecordMap::value_type(StringToUpper(fullPath), PakRecord())).first;
        PakRecord* aPakRecord = &(aRecordItr->second);
        aPakRecord->mCollection = aPakCollection;
        aPakRecord->mFileName = fullPath;
        aPakRecord->mStartPos = 0;
        aPakRecord->mSize = fileSize;
        aPakRecord->mFileTime = 0; // Android AssetManager 不提供时间信息
    }

    AAssetDir_close(assetDir);
#else
    // 其他平台使用标准文件操作
    std::filesystem::path dirPath(theFileName);
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        return false;
    }

    // 遍历目录及其子目录
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (std::filesystem::is_regular_file(entry)) {
            const std::string filePath = entry.path().string();
            const auto fileSize = std::filesystem::file_size(entry);
            mPakCollectionList.emplace_back(fileSize);
            PakCollection* aPakCollection = &mPakCollectionList.back();

            // 读取文件内容
            std::ifstream fileStream(filePath, std::ios::binary);
            if (!fileStream) {
                return false;
            }
            fileStream.read(reinterpret_cast<char*>(aPakCollection->mDataPtr), fileSize);
            fileStream.close();

            // 对读取的数据进行异或处理
            auto *aDataPtr = static_cast<uint8_t *>(aPakCollection->mDataPtr);
            for (size_t i = 0; i < fileSize; i++) {
                *aDataPtr++ ^= 0xF7;
            }

            // 生成文件记录
            PakRecordMap::iterator aRecordItr = mPakRecordMap.insert(PakRecordMap::value_type(StringToUpper(filePath), PakRecord())).first;
            PakRecord* aPakRecord = &(aRecordItr->second);
            aPakRecord->mCollection = aPakCollection;
            aPakRecord->mFileName = filePath;
            aPakRecord->mStartPos = 0;
            aPakRecord->mSize = fileSize;
            aPakRecord->mFileTime = std::filesystem::last_write_time(entry).time_since_epoch().count();
        }
    }
#endif

    return true;
}


//0x5D85C0
PFILE* PakInterface::FOpen(const char* theFileName, const char* anAccess)
{
	if ((strcasecmp(anAccess, "r") == 0) || (strcasecmp(anAccess, "rb") == 0) || (strcasecmp(anAccess, "rt") == 0))
	{
		char anUpperName[256];
		FixFileName(theFileName, anUpperName);

		PakRecordMap::iterator anItr = mPakRecordMap.find(anUpperName);
		if (anItr != mPakRecordMap.end())
		{
			PFILE* aPFP = new PFILE;
			aPFP->mRecord = &anItr->second;
			aPFP->mPos = 0;
			aPFP->mFP = NULL;
			return aPFP;
		}

		anItr = mPakRecordMap.find(theFileName);
		if (anItr != mPakRecordMap.end())
		{
			PFILE* aPFP = new PFILE;
			aPFP->mRecord = &anItr->second;
			aPFP->mPos = 0;
			aPFP->mFP = NULL;
			return aPFP;
		}
	}

	FILE* aFP = fcaseopen(theFileName, anAccess);
	if (aFP == NULL)
		return NULL;
	PFILE* aPFP = new PFILE;
	aPFP->mRecord = NULL;
	aPFP->mPos = 0;
	aPFP->mFP = aFP;
	return aPFP;
}

//0x5D8780
int PakInterface::FClose(PFILE* theFile)
{
	if (theFile->mRecord == NULL)
		fclose(theFile->mFP);
	delete theFile;
	return 0;
}

//0x5D87B0
int PakInterface::FSeek(PFILE* theFile, long theOffset, int theOrigin)
{
	if (theFile->mRecord != NULL)
	{
		if (theOrigin == SEEK_SET)
			theFile->mPos = theOffset;
		else if (theOrigin == SEEK_END)
			theFile->mPos = theFile->mRecord->mSize - theOffset;
		else if (theOrigin == SEEK_CUR)
			theFile->mPos += theOffset;

		// 当前指针位置不能超过整个文件的大小，且不能小于 0
		theFile->mPos = std::max(std::min(theFile->mPos, theFile->mRecord->mSize), 0);
		return 0;
	}
	else
		return fseek(theFile->mFP, theOffset, theOrigin);
}

//0x5D8830
int PakInterface::FTell(PFILE* theFile)
{
	if (theFile->mRecord != NULL)
		return theFile->mPos;
	else
		return ftell(theFile->mFP);
}

//0x5D8850
size_t PakInterface::FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile)
{
	if (theFile->mRecord != NULL)
	{
		// 实际读取的字节数不能超过当前资源文件剩余可读取的字节数
		int aSizeBytes = std::min(theElemSize*theCount, theFile->mRecord->mSize - theFile->mPos);

		// 取得在整个 pak 中开始读取的位置的指针
		uchar* src = (uchar*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos;
		uchar* dest = (uchar*) thePtr;
		memcpy(dest, src, aSizeBytes);
		theFile->mPos += aSizeBytes;  // 读取完成后，移动当前读取位置的指针
		return aSizeBytes / theElemSize;  // 返回实际读取的项数
	}

	return fread(thePtr, theElemSize, theCount, theFile->mFP);
}

int PakInterface::FGetC(PFILE* theFile)
{
	if (theFile->mRecord != NULL)
	{
		for (;;)
		{
			if (theFile->mPos >= theFile->mRecord->mSize)
				return EOF;
			char aChar = *((char*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos++);
			if (aChar != '\r')
				return (uchar) aChar;
		}
	}

	return fgetc(theFile->mFP);
}

int PakInterface::UnGetC(int theChar, PFILE* theFile)
{
	if (theFile->mRecord != NULL)
	{
		// This won't work if we're not pushing the same chars back in the stream
		theFile->mPos = std::max(theFile->mPos - 1, 0);
		return theChar;
	}

	return ungetc(theChar, theFile->mFP);
}

char* PakInterface::FGetS(char* thePtr, int theSize, PFILE* theFile)
{
	if (theFile->mRecord != NULL)
	{
		int anIdx = 0;
		while (anIdx < theSize)
		{
			if (theFile->mPos >= theFile->mRecord->mSize)
			{
				if (anIdx == 0)
					return NULL;
				break;
			}
			char aChar = *((char*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos++);
			if (aChar != '\r')
				thePtr[anIdx++] = aChar;
			if (aChar == '\n')
				break;
		}
		thePtr[anIdx] = 0;
		return thePtr;
	}

	return fgets(thePtr, theSize, theFile->mFP);
}

int PakInterface::FEof(PFILE* theFile)
{
	if (theFile->mRecord != NULL)
		return theFile->mPos >= theFile->mRecord->mSize;
	else
		return feof(theFile->mFP);
}
