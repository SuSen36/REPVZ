#include "TodCommon.h"
#include "TodParticle.h"
#include "Trail.h"
#include <cassert>
#include <cstring>
#include <cstddef>
#include "TodDebug.h"
#include "Definition.h"
#include "SexyAppFramework/misc/XMLParser.h"
#include "Resources.h"
#include "SexyAppFramework/SexyAppBase.h"

DefSymbol gTrailFlagDefSymbols[] = {
    { 0, "Loops" },                 { -1, nullptr }
};
DefField gTrailDefFields[] = {
    { "Image",            offsetof(TrailDefinition, mImage),           DefFieldType::DT_IMAGE,         nullptr },
    { "MaxPoints",        offsetof(TrailDefinition, mMaxPoints),       DefFieldType::DT_INT,           nullptr },
    { "MinPointDistance", offsetof(TrailDefinition, mMinPointDistance),DefFieldType::DT_FLOAT,         nullptr },
    { "TrailFlags",       offsetof(TrailDefinition, mTrailFlags),      DefFieldType::DT_FLAGS,         gTrailFlagDefSymbols },
    { "WidthOverLength",  offsetof(TrailDefinition, mWidthOverLength), DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "WidthOverTime",    offsetof(TrailDefinition, mWidthOverTime),   DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "AlphaOverLength",  offsetof(TrailDefinition, mAlphaOverLength), DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "AlphaOverTime",    offsetof(TrailDefinition, mAlphaOverTime),   DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "TrailDuration",    offsetof(TrailDefinition, mTrailDuration),   DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "",                 0x0,                                         DefFieldType::DT_INVALID,       nullptr }
};
DefMap gTrailDefMap = { gTrailDefFields, sizeof(TrailDefinition), TrailDefinitionConstructor };

DefSymbol gParticleFlagSymbols[] = {
    {  0, "RandomLaunchSpin" },     {  1, "AlignLaunchSpin" },  {  2, "AlignToPixel" },     {  4, "ParticleLoops" },    {  3, "SystemLoops" },
    {  5, "ParticlesDontFollow" },  {  6, "RandomStartTime" },  {  7, "DieIfOverloaded" },  {  8, "Additive" },         {  9, "FullScreen" },
    { 10, "SoftwareOnly" },         { 11, "HardwareOnly" },     { -1, nullptr }
};
DefSymbol gEmitterTypeSymbols[] = {
    {  0, "Circle" },               {  1, "Box" },              {  2, "BoxPath" },          {  3, "CirclePath" },       {  4, "CircleEvenSpacing" },
    { -1, nullptr }
};
DefSymbol gParticleTypeSymbols[] = {
    {  1, "Friction" },             {  2, "Acceleration" },     {  3, "Attractor" },        {  4, "MaxVelocity" },      {  5, "Velocity" },
    {  6, "Position" },             {  7, "SystemPosition" },   {  8, "GroundConstraint" }, {  9, "Shake" },            { 10, "Circle" },
    { 11, "Away" },                 { -1, nullptr }
};

DefField gParticleFieldDefFields[] = {
    { "FieldType",          offsetof(ParticleField, mFieldType), DefFieldType::DT_ENUM,          gParticleTypeSymbols },
    { "x",                  offsetof(ParticleField, mX),         DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "y",                  offsetof(ParticleField, mY),         DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "",                   0x0,                                 DefFieldType::DT_INVALID,       nullptr },
};
DefMap gParticleFieldDefMap = { gParticleFieldDefFields, sizeof(ParticleField), ParticleFieldConstructor };

DefField gEmitterDefFields[] = {
    { "Image",              offsetof(TodEmitterDefinition,mImage)               ,  DefFieldType::DT_IMAGE,         nullptr },
    { "ImageRow",           offsetof(TodEmitterDefinition,mImageRow)            ,  DefFieldType::DT_INT,           nullptr },
    { "ImageCol",           offsetof(TodEmitterDefinition,mImageCol)            ,  DefFieldType::DT_INT,           nullptr },
    { "ImageFrames",        offsetof(TodEmitterDefinition,mImageFrames)         ,  DefFieldType::DT_INT,           nullptr },
    { "Animated",           offsetof(TodEmitterDefinition,mAnimated)            ,  DefFieldType::DT_INT,           nullptr },
    { "ParticleFlags",      offsetof(TodEmitterDefinition,mParticleFlags)       ,  DefFieldType::DT_FLAGS,         gParticleFlagSymbols },
    { "EmitterType",        offsetof(TodEmitterDefinition,mEmitterType)         ,  DefFieldType::DT_ENUM,          gEmitterTypeSymbols },
    { "Name",               offsetof(TodEmitterDefinition,mName)                ,  DefFieldType::DT_STRING,        nullptr },
    { "SystemDuration",     offsetof(TodEmitterDefinition,mSystemDuration)      ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "OnDuration",         offsetof(TodEmitterDefinition,mOnDuration)          ,  DefFieldType::DT_STRING,        nullptr },
    { "CrossFadeDuration",  offsetof(TodEmitterDefinition,mCrossFadeDuration)  ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SpawnRate",          offsetof(TodEmitterDefinition,mSpawnRate)          ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SpawnMinActive",     offsetof(TodEmitterDefinition,mSpawnMinActive)     ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SpawnMaxActive",     offsetof(TodEmitterDefinition,mSpawnMaxActive)     ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SpawnMaxLaunched",   offsetof(TodEmitterDefinition,mSpawnMaxLaunched)   ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "EmitterRadius",      offsetof(TodEmitterDefinition,mEmitterRadius)      ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "EmitterOffsetX",     offsetof(TodEmitterDefinition,mEmitterOffsetX)     ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "EmitterOffsetY",     offsetof(TodEmitterDefinition,mEmitterOffsetY)     ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "EmitterBoxX",        offsetof(TodEmitterDefinition,mEmitterBoxX)        ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "EmitterBoxY",        offsetof(TodEmitterDefinition,mEmitterBoxY)        ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "EmitterPath",        offsetof(TodEmitterDefinition,mEmitterPath)        ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "EmitterSkewX",       offsetof(TodEmitterDefinition,mEmitterSkewX)       ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "EmitterSkewY",       offsetof(TodEmitterDefinition,mEmitterSkewY)       ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleDuration",   offsetof(TodEmitterDefinition,mParticleDuration)   ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SystemRed",          offsetof(TodEmitterDefinition,mSystemRed)          ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SystemGreen",        offsetof(TodEmitterDefinition,mSystemGreen)        ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SystemBlue",         offsetof(TodEmitterDefinition,mSystemBlue)         ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SystemAlpha",        offsetof(TodEmitterDefinition,mSystemAlpha)        ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "SystemBrightness",   offsetof(TodEmitterDefinition,mSystemBrightness)   ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "LaunchSpeed",        offsetof(TodEmitterDefinition,mLaunchSpeed)        ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "LaunchAngle",        offsetof(TodEmitterDefinition,mLaunchAngle)        ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "Field",              offsetof(TodEmitterDefinition,mParticleFields)     ,  DefFieldType::DT_ARRAY,         &gParticleFieldDefMap },
    { "SystemField",        offsetof(TodEmitterDefinition,mSystemFields)       ,  DefFieldType::DT_ARRAY,         &gParticleFieldDefMap },
    { "ParticleRed",        offsetof(TodEmitterDefinition,mParticleRed)        ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleGreen",      offsetof(TodEmitterDefinition,mParticleGreen)      ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleBlue",       offsetof(TodEmitterDefinition,mParticleBlue)       ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleAlpha",      offsetof(TodEmitterDefinition,mParticleAlpha)      ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleBrightness", offsetof(TodEmitterDefinition,mParticleBrightness) ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleSpinAngle",  offsetof(TodEmitterDefinition,mParticleSpinAngle)  ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleSpinSpeed",  offsetof(TodEmitterDefinition,mParticleSpinSpeed)  ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleScale",      offsetof(TodEmitterDefinition,mParticleScale)      ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ParticleStretch",    offsetof(TodEmitterDefinition,mParticleStretch)    ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "CollisionReflect",   offsetof(TodEmitterDefinition,mCollisionReflect)   ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "CollisionSpin",      offsetof(TodEmitterDefinition,mCollisionSpin)      ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ClipTop",            offsetof(TodEmitterDefinition,mClipTop)            ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ClipBottom",         offsetof(TodEmitterDefinition,mClipBottom)         ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ClipLeft",           offsetof(TodEmitterDefinition,mClipLeft)           ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "ClipRight",          offsetof(TodEmitterDefinition,mClipRight)          ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "AnimationRate",      offsetof(TodEmitterDefinition,mAnimationRate)      ,  DefFieldType::DT_TRACK_FLOAT,   nullptr },
    { "",                   0x0,                                                  DefFieldType::DT_INVALID,       nullptr },
};
DefMap gEmitterDefMap = { gEmitterDefFields, sizeof(TodEmitterDefinition), TodEmitterDefinitionConstructor };

DefField gParticleDefFields[] = {
    { "Emitter",            offsetof(TodParticleDefinition,mEmitterDefs),        DefFieldType::DT_ARRAY,         &gEmitterDefMap },
    { "",                   0x0,        DefFieldType::DT_INVALID,       nullptr }
};
DefMap gParticleDefMap = { gParticleDefFields, sizeof(TodParticleDefinition), TodParticleDefinitionConstructor };

DefField gReanimatorTransformDefFields[] = {
    { "x",    offsetof(ReanimatorTransform,mTransX), DefFieldType::DT_FLOAT,         nullptr },
    { "y",    offsetof(ReanimatorTransform,mTransY), DefFieldType::DT_FLOAT,         nullptr },
    { "kx",   offsetof(ReanimatorTransform,mSkewX), DefFieldType::DT_FLOAT,         nullptr },
    { "ky",   offsetof(ReanimatorTransform,mSkewY), DefFieldType::DT_FLOAT,         nullptr },
    { "sx",   offsetof(ReanimatorTransform,mScaleX), DefFieldType::DT_FLOAT,         nullptr },
    { "sy",   offsetof(ReanimatorTransform,mScaleY), DefFieldType::DT_FLOAT,         nullptr },
    { "f",    offsetof(ReanimatorTransform,mFrame), DefFieldType::DT_FLOAT,         nullptr },
    { "a",    offsetof(ReanimatorTransform,mAlpha), DefFieldType::DT_FLOAT,         nullptr },
    { "i",    offsetof(ReanimatorTransform,mImage), DefFieldType::DT_IMAGE,         nullptr },
    { "font", offsetof(ReanimatorTransform,mFont), DefFieldType::DT_FONT,          nullptr },
    { "text", offsetof(ReanimatorTransform,mText), DefFieldType::DT_STRING,        nullptr },
    { "",     0, DefFieldType::DT_INVALID,       nullptr }
};
DefMap gReanimatorTransformDefMap = { gReanimatorTransformDefFields, sizeof(ReanimatorTransform), ReanimatorTransformConstructor};

DefField gReanimatorTrackDefFields[] = {
    { "name",               offsetof(ReanimatorTrack,mName),      DefFieldType::DT_STRING,        nullptr },
    { "t",                  offsetof(ReanimatorTrack,mTransforms),DefFieldType::DT_ARRAY,         &gReanimatorTransformDefMap },
    { "",                   0x0,                                  DefFieldType::DT_INVALID,       nullptr }
};
DefMap gReanimatorTrackDefMap = { gReanimatorTrackDefFields, sizeof(ReanimatorTrack), ReanimatorTrackConstructor };

DefField gReanimatorDefFields[] = {
    { "track",              offsetof(ReanimatorDefinition,mTracks),DefFieldType::DT_ARRAY,         &gReanimatorTrackDefMap },
    { "fps",                offsetof(ReanimatorDefinition,mFPS),   DefFieldType::DT_FLOAT,         nullptr },
    { "",                   0x0,                                   DefFieldType::DT_INVALID,       nullptr }
};
DefMap gReanimatorDefMap = { gReanimatorDefFields, sizeof(ReanimatorDefinition), ReanimatorDefinitionConstructor };

static DefLoadResPath gDefLoadResPaths[4] = { {"IMAGE_", ""}, {"IMAGE_", "particles/"}, {"IMAGE_REANIM_", "reanim/"}, {"IMAGE_REANIM_", "images/"} };

void* ParticleFieldConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((ParticleField*)thePointer)->mX.mNodes = nullptr;
        ((ParticleField*)thePointer)->mX.mCountNodes = 0;
        ((ParticleField*)thePointer)->mY.mNodes = nullptr;
        ((ParticleField*)thePointer)->mY.mCountNodes = 0;
        ((ParticleField*)thePointer)->mFieldType = ParticleFieldType::FIELD_INVALID;
    }
    return thePointer;
}

void* TodEmitterDefinitionConstructor(void* thePointer)
{
    if (thePointer)
    {
        memset(thePointer, 0, sizeof(TodEmitterDefinition));
        ((TodEmitterDefinition*)thePointer)->mImageFrames = 1;
        ((TodEmitterDefinition*)thePointer)->mEmitterType = EmitterType::EMITTER_BOX;
        ((TodEmitterDefinition*)thePointer)->mName = "";
        ((TodEmitterDefinition*)thePointer)->mOnDuration = "";
        ((TodEmitterDefinition*)thePointer)->mImageRow = 0;
        ((TodEmitterDefinition*)thePointer)->mImageCol = 0;
        ((TodEmitterDefinition*)thePointer)->mAnimated = 0;
        ((TodEmitterDefinition*)thePointer)->mImage = nullptr;
        ((TodEmitterDefinition*)thePointer)->mParticleFields.count = 0;
    }
    return thePointer;
}

void* TodParticleDefinitionConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((TodParticleDefinition*)thePointer)->mEmitterDefs = nullptr;
        ((TodParticleDefinition*)thePointer)->mEmitterDefCount = 0;
    }
    return thePointer;
}

void* TrailDefinitionConstructor(void* thePointer)
{
    if (thePointer)
    {
        memset(thePointer, 0, sizeof(TrailDefinition));
        ((TrailDefinition*)thePointer)->mMinPointDistance = 1.0f;
        ((TrailDefinition*)thePointer)->mMaxPoints = 2;
        ((TrailDefinition*)thePointer)->mTrailFlags = 0U;
        ((TrailDefinition*)thePointer)->mImage = nullptr;
    }
    return thePointer;
}

void* ReanimatorTransformConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((ReanimatorTransform*)thePointer)->mTransX = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mTransY = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mSkewX = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mSkewY = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mScaleX = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mScaleY = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mFrame = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mAlpha = DEFAULT_FIELD_PLACEHOLDER;
        ((ReanimatorTransform*)thePointer)->mImage = nullptr;
        ((ReanimatorTransform*)thePointer)->mFont = nullptr;
        ((ReanimatorTransform*)thePointer)->mText = "";
    }
    return thePointer;
}

void* ReanimatorTrackConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((ReanimatorTrack*)thePointer)->mName = "";
        ((ReanimatorTrack*)thePointer)->mTransforms = {nullptr, 0};
    }
    return thePointer;
}

void* ReanimatorDefinitionConstructor(void* thePointer)
{
    if (thePointer)
    {
        ((ReanimatorDefinition*)thePointer)->mTracks = {nullptr, 0};
        ((ReanimatorDefinition*)thePointer)->mFPS = 12.0f;
        ((ReanimatorDefinition*)thePointer)->mReanimAtlas = nullptr;
    }
    return thePointer;
}



void* DefinitionAlloc(int theSize)
{
    void* aPtr = operator new[](theSize);
    TOD_ASSERT(aPtr);
    memset(aPtr, 0, theSize);
    return aPtr;
}

bool DefinitionLoadImage(Image** theImage, const SexyString& theName)
{
    // 当贴图文件路径不存在时，无须获取贴图
    if (theName.size() == 0)
    {
        *theImage = nullptr;
        return true;
    }

    // 尝试借助资源管理器，从 XML 中加载贴图
    Image* anImage = (Image*)gSexyAppBase->mResourceManager->LoadImage(theName);
    if (anImage)
    {
        *theImage = anImage;
        return true;
    }

    // This for loop's performance is HORRIBLE
    // 从可能的贴图路径中手动加载贴图
    for (const DefLoadResPath& aLoadResPath : gDefLoadResPaths)
    {
        int aNameLen = theName.size();
        int aPrefixLen = strlen(aLoadResPath.mPrefix);
        if (aPrefixLen < aNameLen)
        {
            SexyString aPathToTry = aLoadResPath.mDirectory + theName.substr(aPrefixLen, aNameLen);
            SharedImageRef aImageRef = gSexyAppBase->GetSharedImage(aPathToTry);
            if ((Image*)aImageRef != nullptr)
            {
                TodHesitationTrace("Load Image '%s'", theName.c_str());
                TodAddImageToMap(&aImageRef, theName);
                *theImage = (Image*)aImageRef;
                return true;
            }
        }
    }
    return false;
}

bool DefinitionLoadFont(Font** theFont, const SexyString& theName)
{
    Font* aFont = gSexyAppBase->mResourceManager->LoadFont(SexyStringToString(theName));
    *theFont = aFont;
    return aFont != nullptr;
}

bool DefinitionLoadXML(const SexyString& theFileName, DefMap* theDefMap, void* theDefinition)
{
    return DefinitionCompileAndLoad(theFileName, theDefMap, theDefinition);
}







void DefinitionFillWithDefaults(DefMap* theDefMap, void* theDefinition)
{
    memset(theDefinition, 0, theDefMap->mDefSize);  // 将 theDefinition 初始化填充为 0
    for (DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++)  // 遍历 theDefinition 的每一个成员变量
        if (aField->mFieldType == DefFieldType::DT_STRING)
            *(char**)((uintptr_t)theDefinition + aField->mFieldOffset) = (char *)"";  // 将所有 char* 类型的成员变量赋值为空字符数组的指针
}

void DefinitionXmlError(XMLParser* theXmlParser, const char* theFormat, ...)
{
    va_list argList;
    va_start(argList, theFormat);
    std::string aFormattedMessage = SexyStringToString(vformat(theFormat, argList));
    va_end(argList);

    int aLine = theXmlParser->GetCurrentLineNum();
    std::string aFileName = theXmlParser->GetFileName();
    TodTraceAndLog("%s(%d): XML Definition Error: %s\n", aFileName.c_str(), aLine, aFormattedMessage.c_str());
}

bool DefinitionReadXMLString(XMLParser* theXmlParser, SexyString& theValue)
{
    XMLElement aXMLElement;
    if (!theXmlParser->NextElement(&aXMLElement))  // 读取下一个 XML 元素
    {
        DefinitionXmlError(theXmlParser, "Missing element value");
        return false;
    }
    if (aXMLElement.mType == XMLElement::TYPE_END)  // 读取到结束标签则结束处理
        return true;
    else if (aXMLElement.mType != XMLElement::TYPE_ELEMENT)  // 除结束标签外，正常情况下，此处读取到的应是定义的正片内容
    {
        DefinitionXmlError(theXmlParser, "unknown element type");
        return false;
    }

    theValue = aXMLElement.mValue;  // ☆ 赋值出参

    if (!theXmlParser->NextElement(&aXMLElement))  // 继续读取下一个 XML 元素
    {
        DefinitionXmlError(theXmlParser, "Can't read element end");
        return false;
    }
    if (aXMLElement.mType != XMLElement::TYPE_END)  // 正常情况下，此处读取到的应是结束标签
    {
        DefinitionXmlError(theXmlParser, "Missing element end");
        return false;
    }
    return true;
}

bool DefSymbolValueFromString(DefSymbol* theSymbolMap, const char* theName, int* theResultValue)
{
    while (theSymbolMap->mSymbolName != nullptr)
    {
        if (strcasecmp(theName, theSymbolMap->mSymbolName) == 0)
        {
            *theResultValue = theSymbolMap->mSymbolValue;
            return true;
        }
        theSymbolMap++;
    }
    return false;
}

bool DefinitionReadIntField(XMLParser* theXmlParser, int* theValue)
{
    SexyString aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (sexysscanf(aStringValue.c_str(), __S("%d"), theValue) == 1)
        return true;

    DefinitionXmlError(theXmlParser, "Can't parse int value '%s'", aStringValue.c_str());
    return false;
}

bool DefinitionReadFloatField(XMLParser* theXmlParser, float* theValue)
{
    SexyString aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (sexysscanf(aStringValue.c_str(), __S("%f"), theValue) == 1)
        return true;

    DefinitionXmlError(theXmlParser, "Can't parse float value '%s'", aStringValue.c_str());
    return false;
}

bool DefinitionReadStringField(XMLParser* theXmlParser, char** theValue)
{
    SexyString aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (aStringValue.size() == 0)
    {
        *theValue = (char *)"";
    }
    else
    {
        *theValue = (char*)DefinitionAlloc(aStringValue.size());
        strcpy(*theValue, aStringValue.c_str());
    }
    return true;
}

bool DefinitionReadEnumField(XMLParser* theXmlParser, int* theValue, DefSymbol* theSymbolMap)
{
    SexyString aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (DefSymbolValueFromString(theSymbolMap, aStringValue.c_str(), theValue))
        return true;

    DefinitionXmlError(theXmlParser, "Can't parse enum value '%s'", aStringValue.c_str());
    return false;
}

bool DefinitionReadVector2Field(XMLParser* theXmlParser, SexyVector2* theValue)
{
    SexyString aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (sexysscanf(aStringValue.c_str(), __S("%f %f"), theValue) == 1)
        return true;

    DefinitionXmlError(theXmlParser, "Can't parse vector2 value '%s'", aStringValue.c_str());
    return false;
}

bool DefinitionReadArrayField(XMLParser* theXmlParser, DefinitionArrayDef* theArray, DefField* theField)
{
    DefMap* aDefMap = (DefMap*)theField->mExtraData;

    if (theArray->mArrayCount == 0)
    {
        theArray->mArrayCount = 1;
        theArray->mArrayData = DefinitionAlloc(aDefMap->mDefSize);
    }
    else
    {
        // 当 theArray 中已存在元素，且元素的个数为 2 的整数次幂时
        // TODO Potential error with the bracketing for the &
        if (theArray->mArrayCount >= 1 && (theArray->mArrayCount == 1 || ((theArray->mArrayCount & (theArray->mArrayCount - 1)) == 0)))
        {
            void* anOldData = theArray->mArrayData;
            theArray->mArrayData = DefinitionAlloc(2 * theArray->mArrayCount * aDefMap->mDefSize);
            memcpy(theArray->mArrayData, anOldData, theArray->mArrayCount * aDefMap->mDefSize);
            delete[] (char *)anOldData;
        }
        theArray->mArrayCount++;
    }

    if (DefinitionLoadMap(theXmlParser, aDefMap, (unsigned char*)theArray->mArrayData + aDefMap->mDefSize * (theArray->mArrayCount - 1)))
        return true;

    DefinitionXmlError(theXmlParser, "failed to read sub def");
    return false;
}

/*
Float Track Field EBNF:
    <test>     ::= <trackdef> ("\n"+ <trackdef>?)*
    <trackdef> ::= <node> (" " <node>)*
    <node>     ::= <norange> | <range>
    <norange>  ::= <float> ("," <float>)? (" " <curve>)?
    <range>    ::= "[" (<float> | <float> (" " <curve>)? " " <float>) "]" ("," <float>)?
    <curve>    ::= "Bounce" | "EaseIn" | "EaseInOutWeak" | "EaseOut" | "FastInOutWeak"
    <float>    ::= "-"? (<natural> ("." <natural>?)? | ("." <natural>))
    <natural>  ::= <digit>+
    <digit>    ::= [0-9]
*/

/*
struct TodCurveStringMap {
    char *mString;
    TodCurves mCurveType;
};

const TodCurveStringMap TodCurveStrings[] = {
    {(char *)"Bounce",        TodCurves::CURVE_BOUNCE},
    {(char *)"FastInOutWeak", TodCurves::CURVE_FAST_IN_OUT_WEAK},
    {(char *)"EaseInOutWeak", TodCurves::CURVE_EASE_IN_OUT_WEAK},
    {(char *)"EaseOut",       TodCurves::CURVE_EASE_OUT},
    {(char *)"EaseIn",        TodCurves::CURVE_EASE_IN},
};
*/

DefSymbol gDefTrackEaseSymbols[] = {
    {TodCurves::CURVE_EASE_IN_OUT_WEAK,   "EaseInOutWeak"},
    {TodCurves::CURVE_FAST_IN_OUT_WEAK,   "FastInOutWeak"},
    {TodCurves::CURVE_EASE_IN_OUT,        "EaseInOut"},
    {TodCurves::CURVE_FAST_IN_OUT,        "FastInOut"},
    {TodCurves::CURVE_EASE_IN,            "EaseIn"},
    {TodCurves::CURVE_EASE_OUT,           "EaseOut"},
    {TodCurves::CURVE_EASE_SIN_WAVE,      "EaseSinWave"},
    {TodCurves::CURVE_BOUNCE_FAST_MIDDLE, "BounceFastMiddle"},
    {TodCurves::CURVE_BOUNCE_SLOW_MIDDLE, "BounceSlowMiddle"},
    {TodCurves::CURVE_BOUNCE,             "Bounce"},
    {TodCurves::CURVE_SIN_WAVE,           "SinWave"},
    {TodCurves::CURVE_LINEAR,             "Linear"},
};

bool DefinitionReadFloatTrackField(XMLParser* theXmlParser, FloatParameterTrack* theTrack)
{
    SexyString aStringValue;

    if (!DefinitionReadXMLString(theXmlParser, aStringValue)) return false;
    
    float aValue = 0;
    int aLen = 0;

    const char *aStringChars = aStringValue.c_str();
    size_t anIdx = 0;

    theTrack->mCountNodes = 0;

    std::vector<FloatParameterTrackNode> aFloatTrackVec = std::vector<FloatParameterTrackNode>();
    FloatParameterTrackNode aTrackNode = FloatParameterTrackNode();
    while(true) {
        if (anIdx >= aStringValue.length()) {
            return false;
        }
        if (aStringChars[anIdx] == '\0') goto _m_break; // No empty strings allowed

        aTrackNode.mTime = -1;
        aTrackNode.mCurveType = TodCurves::CURVE_LINEAR;
        aTrackNode.mDistribution = TodCurves::CURVE_LINEAR;

        if (aStringChars[anIdx] == '[') {
            // <range>
            anIdx++;
            if (sexysscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen) != 1) return false; // mLowValue
            anIdx += aLen;
            aTrackNode.mLowValue = aValue;
            aTrackNode.mHighValue = aValue;
            if (aStringChars[anIdx] != ']') {
                anIdx++; // space (' ')
                // <curve>
                for (size_t i = 0; i < sizeof(gDefTrackEaseSymbols)/sizeof(gDefTrackEaseSymbols[0]); ++i) {
                    size_t aStrLen = strlen(gDefTrackEaseSymbols[i].mSymbolName);
                    if (strncmp(gDefTrackEaseSymbols[i].mSymbolName, aStringChars + anIdx, aStrLen) == 0) // could be the distribution?
                    {
                        aTrackNode.mDistribution = (TodCurves)gDefTrackEaseSymbols[i].mSymbolValue;
                        anIdx += aStrLen + 1; // Accounts for space (' '), expressions never end with a curve
                        break;
                    }
                }
                switch(sexysscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen)) // mHighValue
                { 
                case 1: // Float read successfully
                    anIdx += aLen; 
                    aTrackNode.mHighValue = aValue;
                    break;
                case 0: // No float to read just continue
                    break;
                default: // Something bad happened, panic!
                    return false;
                }
            }
            if (aStringChars[anIdx] != ']') return false; // Invalid format
            anIdx++;
            if (aStringChars[anIdx] == '\0') goto _m_break; // Done!

            if (aStringChars[anIdx] == ',') {
                anIdx++;
                if (sexysscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen) != 1) return false; // mTime
                anIdx += aLen;
                aTrackNode.mTime = aValue * 0.01;
            }
            if (aStringChars[anIdx] == '\0') goto _m_break; // Done!
            anIdx++;
        } else {
            // <norange>
            if (sexysscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen) != 1) return false; // mLow/HighValue
            anIdx += aLen;
            aTrackNode.mLowValue = aValue;
            aTrackNode.mHighValue = aValue;

            if (aStringChars[anIdx] == '\0') goto _m_break; // Done!
    
            if (aStringChars[anIdx] == ',') {
                anIdx++;
                if (sexysscanf(aStringChars + anIdx, "%f%n", &aValue, &aLen) != 1) return false; // mTime
                anIdx += aLen;
                aTrackNode.mTime = aValue * 0.01;
            }
            if (aStringChars[anIdx] == '\0') goto _m_break; // Done!
            anIdx++;
            // <curve>
            for (size_t i = 0; i < sizeof(gDefTrackEaseSymbols)/sizeof(gDefTrackEaseSymbols[0]); ++i) {
                size_t aStrLen = strlen(gDefTrackEaseSymbols[i].mSymbolName);
                if (strncmp(gDefTrackEaseSymbols[i].mSymbolName, aStringChars + anIdx, aStrLen) == 0) // mCurveType
                {
                    aTrackNode.mCurveType = (TodCurves)gDefTrackEaseSymbols[i].mSymbolValue;
                    anIdx += aStrLen;
                    if (aStringChars[anIdx] == '\0') goto _m_break; // Done!
                    anIdx++;
                    break;
                }
            }
        }
        
        aFloatTrackVec.push_back(aTrackNode);
    }
    _m_break:
    aFloatTrackVec.push_back(aTrackNode);
    
    // Search forward for a timestamp:
    size_t aBaseIdx = 0;
    anIdx = 0;
    float high = 0.0;
    float low = 0.0;
    do {
        for(anIdx = aBaseIdx; anIdx < aFloatTrackVec.size(); ++anIdx) {
            if (aFloatTrackVec[anIdx].mTime >= 0.0){
                // Found a timestamp!
                high = aFloatTrackVec[anIdx].mTime;
                goto _m_found; // Since we break out anIdx isn't incremented.
            }
        }
        // Didn't find another value, we're finished.
        // Since we did break, anIdx == aFloatTrackVec.size(), which means final value is set
        high = 1.0;
    _m_found:
        // Going backwards set previous timestamps
        for(size_t i = aBaseIdx; i < anIdx; ++i) { // Iterate up to anIdx - 1
            float interp;
            if (((anIdx - 1) - aBaseIdx) != 0) interp = ((float)(i - aBaseIdx))/((float)((anIdx - 1) - aBaseIdx));
            else if (aBaseIdx == 0) interp = 0.0;
            else interp = 1.0;
            aFloatTrackVec[i].mTime = high*interp + low*(1 - interp);
        }
        // Start again
        aBaseIdx = anIdx + 1;
        low = high;
    } while (aBaseIdx < aFloatTrackVec.size());

    
    /*
    TodTraceAndLog("%s | %d", aStringChars, aFloatTrackVec.size());
    for (auto &i : aFloatTrackVec) {
        TodTraceAndLog("%f", i.mTime);
    }
    */

    size_t alloc_size = aFloatTrackVec.size() * sizeof(FloatParameterTrackNode);
    theTrack->mNodes = (FloatParameterTrackNode*)DefinitionAlloc(alloc_size);
    if (!theTrack->mNodes) return false;

    ::memcpy(theTrack->mNodes, aFloatTrackVec.data(), alloc_size);
    theTrack->mCountNodes = aFloatTrackVec.size();

    return true;
}

bool DefinitionReadFlagField(XMLParser* theXmlParser, const SexyString& theElementName, uint* theResultValue, DefSymbol* theSymbolMap)
{
    int aValue;
    if (!DefSymbolValueFromString(theSymbolMap, theElementName.c_str(), &aValue))
        return false;

    SexyString aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    float aFlag; // This was obviously a bug, the casting is wrong, although amusingly it just woks since it's just a bit
    if (sexysscanf(aStringValue.c_str(), __S("%f %f"), &aFlag) != 1)
    {
        DefinitionXmlError(theXmlParser, "Can't parse int value '%s'", aStringValue.c_str());
        return false;
    }

    // Still I'll let the compiler work out the optimisation
    int aFlag_int = aFlag;

    if (theResultValue) {
        if (aFlag_int)
        {
            *theResultValue |= 1 << aValue;
        }
        else
        {
            *theResultValue &= ~(1 << aValue);
        }
    }
    
    return true;
}

bool DefinitionReadImageField(XMLParser* theXmlParser, Image** theImage)
{
    SexyString aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (DefinitionLoadImage(theImage, aStringValue))
        return true;

    std::string aMessgae = StrFormat("Failed to find image '%s' in %s", SexyStringToStringFast(aStringValue).c_str(), theXmlParser->GetFileName().c_str());
    TodErrorMessageBox(aMessgae.c_str(), "Missing image");

    return false;
}

bool DefinitionReadFontField(XMLParser* theXmlParser, Font** theFont)
{
    SexyString aStringValue;
    if (!DefinitionReadXMLString(theXmlParser, aStringValue))
        return false;

    if (DefinitionLoadFont(theFont, aStringValue))
        return true;

    std::string aMessgae = StrFormat("Failed to find font '%s' in %s", SexyStringToStringFast(aStringValue).c_str(), theXmlParser->GetFileName().c_str());
    TodErrorMessageBox(aMessgae.c_str(), "Missing font");

    return false;
}

bool DefinitionReadField(XMLParser* theXmlParser, DefMap* theDefMap, void* theDefinition, bool* theDone)
{
    if (theXmlParser->HasFailed())
        return false;

    XMLElement aXMLElement = XMLElement();
    if (!theXmlParser->NextElement(&aXMLElement) || aXMLElement.mType == XMLElement::TYPE_END)  // 读取下一个 XML 元素
    {
        *theDone = true;  // 没有下一个元素则表示读取完成
        return true;
    }
    if (aXMLElement.mType != XMLElement::TYPE_START)  // 正常情况下，此处读取到的应是开始标签，而其他内容在后续的相应函数中读取
    {
        DefinitionXmlError(theXmlParser, "Missing element start");
        return false;
    }

    for (DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++)
    {
        void* pVar = (void*)((uintptr_t)theDefinition + aField->mFieldOffset);
        // Missing pvar field for some reason!
        if (aField->mFieldType == DefFieldType::DT_FLAGS && DefinitionReadFlagField(theXmlParser, aXMLElement.mValue, (uint*)pVar, (DefSymbol*)aField->mExtraData))
            return true;
        
        if (strcasecmp(aXMLElement.mValue.c_str(), aField->mFieldName) == 0)  // 判断 aXMLElement 定义的是否为该成员变量
        {
            bool aSuccess;
            switch (aField->mFieldType)
            {
            case DefFieldType::DT_INT:
                aSuccess = DefinitionReadIntField(theXmlParser, (int*)pVar);
                break;
            case DefFieldType::DT_FLOAT:
                aSuccess = DefinitionReadFloatField(theXmlParser, (float*)pVar);
                break;
            case DefFieldType::DT_STRING:
                aSuccess = DefinitionReadStringField(theXmlParser, (char**)pVar);
                break;
            case DefFieldType::DT_ENUM:
                aSuccess = DefinitionReadEnumField(theXmlParser, (int*)pVar, (DefSymbol*)aField->mExtraData);
                break;
            case DefFieldType::DT_VECTOR2:
                aSuccess = DefinitionReadVector2Field(theXmlParser, (SexyVector2*)pVar);
                break;
            case DefFieldType::DT_ARRAY:
                aSuccess = DefinitionReadArrayField(theXmlParser, (DefinitionArrayDef*)pVar, aField);
                break;
            case DefFieldType::DT_TRACK_FLOAT:
                aSuccess = DefinitionReadFloatTrackField(theXmlParser, (FloatParameterTrack*)pVar);
                break;
            case DefFieldType::DT_IMAGE:
                aSuccess = DefinitionReadImageField(theXmlParser, (Image**)pVar);
                break;
            case DefFieldType::DT_FONT:
                aSuccess = DefinitionReadFontField(theXmlParser, (Font**)pVar);
                break;
            default:
                aSuccess = false;
                TOD_ASSERT(false);
                break;
            }
            if (aSuccess)
                return true;

            DefinitionXmlError(theXmlParser, "Failed to read '%s' field", aXMLElement.mValue.c_str());
            return false;
        }
    }
    DefinitionXmlError(theXmlParser, "Ignoring unknown element '%s'", aXMLElement.mValue.c_str());  // aXMLElement 未定义任何成员变量时
    return false;
}

bool DefinitionLoadMap(XMLParser* theXmlParser, DefMap* theDefMap, void* theDefinition)
{
    if (theDefMap->mConstructorFunc)
        theDefMap->mConstructorFunc(theDefinition);  // 利用构造函数构造 theDefinition
    else
        DefinitionFillWithDefaults(theDefMap, theDefinition);  // 以默认值填充 theDefinition

    bool aDone = false;
    while (!aDone)
        if (!DefinitionReadField(theXmlParser, theDefMap, theDefinition, &aDone))
            return false;
    return true;
}


//0x4447F0 : (void* def, *defMap, string& xmlFilePath)  //esp -= 0xC
bool DefinitionCompileAndLoad(const SexyString& theXMLFilePath, DefMap* theDefMap, void* theDefinition)
{
    TodHesitationTrace(__S("predef"));

    XMLParser aXMLParser;
    if (!aXMLParser.OpenFile(theXMLFilePath))
    {
        TodTrace(__S("XML file not found: %s\n"), theXMLFilePath.c_str());
        return false;
    }

    if (!DefinitionLoadMap(&aXMLParser, theDefMap, theDefinition))
        return false;

    TodHesitationTrace(__S("loaded %s"), theXMLFilePath.c_str());
    return true;
}

float FloatTrackEvaluate(FloatParameterTrack& theTrack, float theTimeValue, float theInterp)
{
    if (theTrack.mCountNodes == 0)
        return 0.0f;

    if (theTimeValue < theTrack.mNodes[0].mTime)  // 如果当前时间小于第一个节点的开始时间
        return TodCurveEvaluate(theInterp, theTrack.mNodes[0].mLowValue, theTrack.mNodes[0].mHighValue, theTrack.mNodes[0].mDistribution);

    for (int i = 1; i < theTrack.mCountNodes; i++)
    {
        FloatParameterTrackNode* aNodeNxt = &theTrack.mNodes[i];
        if (theTimeValue <= aNodeNxt->mTime)  // 寻找首个开始时间大于当前时间的节点
        {
            FloatParameterTrackNode* aNodeCur = &theTrack.mNodes[i - 1];
            // 计算当前时间在〔当前节点至下一节点〕的过程中的进度
            float aTimeFraction = (theTimeValue - aNodeCur->mTime) / (aNodeNxt->mTime - aNodeCur->mTime);
            float aLeftValue = TodCurveEvaluate(theInterp, aNodeCur->mLowValue, aNodeCur->mHighValue, aNodeCur->mDistribution);
            float aRightValue = TodCurveEvaluate(theInterp, aNodeNxt->mLowValue, aNodeNxt->mHighValue, aNodeNxt->mDistribution);
            return TodCurveEvaluate(aTimeFraction, aLeftValue, aRightValue, aNodeCur->mCurveType);
        }
    }

    FloatParameterTrackNode* aLastNode = &theTrack.mNodes[theTrack.mCountNodes - 1];  // 如果当前时间大于最后一个节点的开始时间
    return TodCurveEvaluate(theInterp, aLastNode->mLowValue, aLastNode->mHighValue, aLastNode->mDistribution);
}

void FloatTrackSetDefault(FloatParameterTrack& theTrack, float theValue)
{
    if (theTrack.mNodes == nullptr && theValue != 0.0f)  // 确保该参数轨道无节点（未被赋值过）且给定的默认值不为 0
    {
        theTrack.mCountNodes = 1;  // 默认参数轨道有且仅有 1 个节点
        FloatParameterTrackNode* aNode = (FloatParameterTrackNode*)DefinitionAlloc(sizeof(FloatParameterTrackNode));
        theTrack.mNodes = aNode;
        aNode->mTime = 0.0f;
        aNode->mLowValue = theValue;
        aNode->mHighValue = theValue;
        aNode->mCurveType = TodCurves::CURVE_CONSTANT;
        aNode->mDistribution = TodCurves::CURVE_LINEAR;
    } else if (theTrack.mNodes == nullptr) {
        theTrack.mCountNodes = 0;
    }
}

bool FloatTrackIsSet(const FloatParameterTrack& theTrack)
{
    return theTrack.mCountNodes != 0 && theTrack.mNodes[0].mCurveType != TodCurves::CURVE_CONSTANT;
}

bool FloatTrackIsConstantZero(FloatParameterTrack& theTrack)
{
    // 当轨道无节点，或仅存在一个节点且该节点的最大、最小值均为 0 时，认为该轨道上的值恒为零
    return theTrack.mCountNodes == 0 || (theTrack.mCountNodes == 1 && theTrack.mNodes[0].mLowValue == 0.0f && theTrack.mNodes[0].mHighValue == 0.0f);
}

float FloatTrackEvaluateFromLastTime(FloatParameterTrack& theTrack, float theTimeValue, float theInterp)
{
    return theTimeValue < 0.0f ? 0.0f : FloatTrackEvaluate(theTrack, theTimeValue, theInterp);
}

void DefinitionFreeArrayField(DefinitionArrayDef* theArray, DefMap* theDefMap)
{
    for (int i = 0; i < theArray->mArrayCount; i++)
        DefinitionFreeMap(theDefMap, (void*)((intptr_t)theArray->mArrayData + theDefMap->mDefSize * i));  // 最后一个参数表示 pData[i]
    delete[] (char *)theArray->mArrayData;
    theArray->mArrayData = nullptr;
}

void DefinitionFreeMap(DefMap* theDefMap, void* theDefinition)
{
    // 根据 theDefMap 遍历 theDefinition 的每个成员变量
    for (DefField* aField = theDefMap->mMapFields; *aField->mFieldName != '\0'; aField++)
    {
        void* aVar = (void*)((intptr_t)theDefinition + aField->mFieldOffset);  // 指向该成员变量的指针
        switch (aField->mFieldType)
        {
        case DefFieldType::DT_STRING:
            //if (**(char**)aVar != '\0')
            //    delete[] *(char**)aVar;  // 释放字符数组
            *(char**)aVar = nullptr;
            break;
        case DefFieldType::DT_ARRAY:
            DefinitionFreeArrayField((DefinitionArrayDef*)aVar, (DefMap*)aField->mExtraData);
            break;
        case DefFieldType::DT_TRACK_FLOAT:
            if (((FloatParameterTrack*)aVar)->mCountNodes != 0)
                delete[]((FloatParameterTrack*)aVar)->mNodes;  // 释放浮点参数轨道的节点
            ((FloatParameterTrack*)aVar)->mNodes = nullptr;
            break;
        default:
            break;
        }
    }
}
