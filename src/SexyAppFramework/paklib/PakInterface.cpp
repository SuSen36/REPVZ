#include <unistd.h>
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

std::vector<std::string> PakInterface::GetPakFileNames() const
{
    std::vector<PakCollection> collections(mPakCollectionList.begin(), mPakCollectionList.end());

    std::sort(collections.begin(), collections.end(), [](const PakCollection& a, const PakCollection& b) {
        return a.mPriority > b.mPriority; // 根据 mPriority 排序
    });

    // 收集排序后的文件名
    std::vector<std::string> pakFileNames;
    for (const auto& collection : collections)
    {
        pakFileNames.push_back(collection.mFileName); // 推送文件名
    }
    return pakFileNames;
}

//0x5D84D0
static void FixFileName(const char* theFileName, char* theUpperName)
{
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

	bool lastSlash = false;
	const char* aSrc = theFileName;
	char* aDest = theUpperName;

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
			while ((aDest > theUpperName + 1) && (*(aDest-1) != '\\'))  // 回退到上一层目录
				--aDest;
			aSrc++;
			// 此处将形如“a\b\..\c”的路径简化为“a\c”
		}
		else
		{
			*(aDest++) = toupper((uchar) c);
			if (c == 0)
				break;
			lastSlash = false;				
		}
	}
}

bool PakInterface::AddPakFile(const std::string& theFileName, int thePriority) {
    // 打开文件
    FILE *aFileHandle = fcaseopen(theFileName.c_str(), "rb");
    if (!aFileHandle) return false;

    // 获取文件大小
    fseek(aFileHandle, 0, SEEK_END);
    size_t aFileSize = ftell(aFileHandle);
    fseek(aFileHandle, 0, SEEK_SET);

    // 存储文件数据
    mPakCollectionList.emplace_back(aFileSize,theFileName, thePriority);
    PakCollection* aPakCollection = &mPakCollectionList.back();

    if (fread(aPakCollection->mDataPtr, 1, aFileSize, aFileHandle) != aFileSize) {
        fclose(aFileHandle);
        return false;
    }
    fclose(aFileHandle);

    // 数据异或处理
    for (size_t i = 0; i < aFileSize; i++) {
        static_cast<uint8_t *>(aPakCollection->mDataPtr)[i] ^= 0xF7;
    }

    // 生成唯一文件名标识符以支持多 PAK 文件共存
    std::string upperFileName = StringToUpper(theFileName);
    std::string uniqueFileName = upperFileName;
    int count = 1;

    while (mPakRecordMap.find(uniqueFileName) != mPakRecordMap.end()) {
        uniqueFileName = upperFileName + "_" + std::to_string(count++);
    }

    // 插入文件记录
    PakRecord newPakRecord;
    newPakRecord.mCollection = aPakCollection;
    newPakRecord.mFileName = theFileName;
    newPakRecord.mStartPos = 0; // 初始化时偏移量设置为 0
    newPakRecord.mSize = aFileSize;
    newPakRecord.mPriority = thePriority; // 设置优先级

    // 如果文件名已存在，追加到相应的向量中
    mPakRecordMap[uniqueFileName].emplace_back(newPakRecord);

    // 验证文件的魔数和版本
    uint32_t aMagic = 0, aVersion = 0;
    PFILE* aFP = FOpen(theFileName.c_str(), "rb");
    if (!aFP || (FRead(&aMagic, sizeof(uint32_t), 1, aFP) != 1) || (aMagic != 0xBAC04AC0) ||
        (FRead(&aVersion, sizeof(uint32_t), 1, aFP) != 1) || (aVersion > 0)) {
        if (aFP) FClose(aFP);
        return false;
    }

    // 读取文件记录
    int aPos = 0;
    while (true) {
        uchar aFlags = 0;
        if ((FRead(&aFlags, 1, 1, aFP) != 1) || (aFlags & FILEFLAGS_END)) break;

        uchar aNameWidth = 0;
        char aName[256] = {0};
        FRead(&aNameWidth, 1, 1, aFP);
        FRead(aName, 1, aNameWidth, aFP);
        aName[aNameWidth] = 0;

        int aSrcSize = 0;
        FRead(&aSrcSize, sizeof(int), 1, aFP);
        int64_t aFileTime;
        FRead(&aFileTime, sizeof(int64_t), 1, aFP);

        // 处理文件名
        for (int i = 0; i < aNameWidth; i++) {
            if (aName[i] == '\\') aName[i] = '/';
        }
        char anUpperName[256];
        FixFileName(aName, anUpperName);

        // 创建额外的文件记录
        PakRecord additionalPakRecord;
        additionalPakRecord.mCollection = aPakCollection;
        additionalPakRecord.mFileName = anUpperName;
        additionalPakRecord.mStartPos = aPos;
        additionalPakRecord.mSize = aSrcSize;
        additionalPakRecord.mFileTime = aFileTime;
        additionalPakRecord.mPriority = thePriority; // 维持原优先级

        // 插入新的文件记录
        mPakRecordMap[StringToUpper(anUpperName)].emplace_back(additionalPakRecord);

        aPos += aSrcSize;
    }

    int anOffset = FTell(aFP);
    for (auto& record : mPakRecordMap) {
        for (auto& pakRecord : record.second) {
            if (pakRecord.mCollection == aPakCollection) {
                pakRecord.mStartPos += anOffset;
            }
        }
    }

    FClose(aFP);
    return true;
}

//0x5D85C0
PFILE* PakInterface::FOpen(const char* theFileName, const char* anAccess)
{
    // 处理读取模式
    if ((strcasecmp(anAccess, "r") == 0) ||
        (strcasecmp(anAccess, "rb") == 0) ||
        (strcasecmp(anAccess, "rt") == 0))
    {
        char anUpperName[256];
        FixFileName(theFileName, anUpperName); // 处理文件名以统一格式

        // 查找对应的 PakRecord
        auto it = mPakRecordMap.find(anUpperName);
        if (it != mPakRecordMap.end())
        {
            // 找到对应的记录，获取优先级最高的记录
            const PakRecord* highestPriorityRecord = nullptr;
            int highestPriority = -1;

            for (const auto& record : it->second) {
                if (record.mPriority > highestPriority) {
                    highestPriority = record.mPriority;
                    highestPriorityRecord = &record;
                }
            }

            if (highestPriorityRecord) {
                PFILE* aPFP = new PFILE;
                aPFP->mRecord = highestPriorityRecord; // 使用优先级最高的记录
                aPFP->mPos = 0; // 初始化读取位置
                aPFP->mFP = NULL; // PAK 文件没有标准文件指针
                return aPFP;
            }
        }

        // 再次尝试查找原始文件名
        it = mPakRecordMap.find(theFileName);
        if (it != mPakRecordMap.end())
        {
            const PakRecord* highestPriorityRecord = nullptr;
            int highestPriority = -1;

            for (const auto& record : it->second) {
                if (record.mPriority > highestPriority) {
                    highestPriority = record.mPriority;
                    highestPriorityRecord = &record;
                }
            }

            if (highestPriorityRecord) {
                PFILE* aPFP = new PFILE;
                aPFP->mRecord = highestPriorityRecord;
                aPFP->mPos = 0;
                aPFP->mFP = NULL; // PAK 文件没有标准文件指针
                return aPFP;
            }
        }
    }

    // 如果没有在 PAK 中找到，尝试使用标准方式打开文件
    FILE* aFP = fcaseopen(theFileName, anAccess);
    if (aFP == nullptr)
        return nullptr; // 打开失败

    // 为标准文件创建 PFILE 结构
    PFILE* aPFP = new PFILE;
    aPFP->mRecord = nullptr; // 没有对应的 PakRecord
    aPFP->mPos = 0; // 初始化读取位置
    aPFP->mFP = aFP; // 关联的标准文件指针
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

