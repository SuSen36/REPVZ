#ifndef __PAKINTERFACE_H__
#define __PAKINTERFACE_H__

#include <map>
#include <list>
#include <string>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <utility>
#include <vector>
#include "../Common.h"


class PakCollection;

// ====================================================================================================
// ★ 一个 PakRecord 实例对应资源包内的一个资源文件的数据，包括文件名，地址，大小等信息
// ====================================================================================================
struct PakRecord
{
public:
    PakCollection* mCollection;         // 指向该资源文件所在的资源包
    std::string mFileName;               // 资源文件的名称及路径
    int64_t mFileTime;                   // 资源文件的时间戳
    int mStartPos;                       // 该资源文件在资源包中的位置（偏移量）
    int mSize;                           // 资源文件的大小，单位为 Byte（字节数）
    int mPriority;                       // 文件优先级
};

typedef std::map<std::string, std::vector<PakRecord>> PakRecordMap; // 支持同名文件的映射

// ====================================================================================================
// ★ 一个 PakCollection 实例对应一个 pak 资源包在内存中的映射文件
// ====================================================================================================
class PakCollection
{
public:
    void* mDataPtr;                     // 资源包中的所有数据
    std::string mFileName;               //
    int mPriority;                       // 资源包的优先级

    PakCollection(size_t size,std::string fileName,int priority) {
        mDataPtr = malloc(size);
        mFileName = std::move(fileName);
        mPriority = priority;
    }
};

typedef std::list<PakCollection> PakCollectionList;

struct PFILE
{
    const PakRecord* mRecord;           // 指向资源记录
    int mPos;                            // 当前读取位置
    FILE* mFP;                           // 链接的标准文件指针
};

class PakInterfaceBase
{
public:
    virtual PFILE* FOpen(const char* theFileName, const char* theAccess) = 0;
    virtual int FClose(PFILE* theFile) = 0;
    virtual int FSeek(PFILE* theFile, long theOffset, int theOrigin) = 0;
    virtual int FTell(PFILE* theFile) = 0;
    virtual size_t FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile) = 0;
    virtual int FGetC(PFILE* theFile) = 0;
    virtual int UnGetC(int theChar, PFILE* theFile) = 0;
    virtual char* FGetS(char* thePtr, int theSize, PFILE* theFile) = 0;
    virtual int FEof(PFILE* theFile) = 0;
};

class PakInterface : public PakInterfaceBase
{
public:
    PakCollectionList         mPakCollectionList; // 通过 AddPakFile() 添加的各个资源包
    PakRecordMap              mPakRecordMap;           // 所有已添加的资源包中的资源文件映射

public:
    PakInterface();
    ~PakInterface();

    std::vector<std::string> GetPakFileNames() const;

    bool            AddPakFile(const std::string& theFileName, int thePriority);
    void            StorePakPriority(const std::string& theFileName, int thePriority); // 新增函数用于存储优先级
    PFILE*          FOpen(const char* theFileName, const char* theAccess) override;
    int             FClose(PFILE* theFile) override;
    int             FSeek(PFILE* theFile, long theOffset, int theOrigin) override;
    int             FTell(PFILE* theFile) override;
    size_t          FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile) override;
    int             FGetC(PFILE* theFile) override;
    int             UnGetC(int theChar, PFILE* theFile) override;
    char*           FGetS(char* thePtr, int theSize, PFILE* theFile) override;
    int             FEof(PFILE* theFile) override;

    PakRecord*      FindFile(const std::string& fileName); // 查找文件，返回优先级最高的记录

};

extern PakInterface* gPakInterface;

// 文件操作的静态函数封装
static PFILE* p_fopen(const char* theFileName, const char* theAccess) {
    return gPakInterface->FOpen(theFileName, theAccess);
}

static int p_fclose(PFILE* theFile) {
    return gPakInterface->FClose(theFile);
}

static int p_fseek(PFILE* theFile, long theOffset, int theOrigin) {
    return gPakInterface->FSeek(theFile, theOffset, theOrigin);
}

static int p_ftell(PFILE* theFile) {
    return gPakInterface->FTell(theFile);
}

static size_t p_fread(void* thePtr, int theSize, int theCount, PFILE* theFile) {
    return gPakInterface->FRead(thePtr, theSize, theCount, theFile);
}

static int p_fgetc(PFILE* theFile) {
    return gPakInterface->FGetC(theFile);
}

static int p_ungetc(int theChar, PFILE* theFile) {
    return gPakInterface->UnGetC(theChar, theFile);
}

static char* p_fgets(char* thePtr, int theSize, PFILE* theFile) {
    return gPakInterface->FGetS(thePtr, theSize, theFile);
}

static int p_feof(PFILE* theFile) {
    return gPakInterface->FEof(theFile);
}

#endif // __PAKINTERFACE_H__




