#include "DataSync.h"
#include "ProfileMgr.h"
#include "PlayerInfo.h"
#include "../../SexyAppFramework/SexyAppBase.h"

using namespace Sexy;
static int gProfileVersion = 14;

PlayerInfo* ProfileMgr::GetAnyProfile()
{
    if (mProfileMap.size() == 0)
        return nullptr;

    PlayerInfo* aPlayerInfo = &mProfileMap.begin()->second;
    aPlayerInfo->LoadDetails();
    aPlayerInfo->mUseSeq = mNextProfileUseSeq++;
    return aPlayerInfo;
}

void ProfileMgr::Clear()
{
    mProfileMap.clear();
    mNextProfileId = 1U;
    mNextProfileUseSeq = 1U;
}

void ProfileMgr::SyncState(DataSync& theSync)
{
    DataReader* aReader = theSync.GetReader();
    DataWriter* aWriter = theSync.GetWriter();

    int aVersion = gProfileVersion;
    theSync.SyncLong(aVersion);
    theSync.SetVersion(aVersion);
    if (aVersion == gProfileVersion)
    {
        if (aReader)
        {
            mProfileMap.clear();

            ulong aMaxProfileId = 0;
            ulong aMaxUseSeq = 0;
            for (int aProfileCount = aReader->ReadShort(); aProfileCount > 0; aProfileCount--)
            {
                PlayerInfo aProfile;
                aProfile.SyncSummary(theSync);

                if (aProfile.mId > aMaxProfileId)
                    aMaxProfileId = aProfile.mId;
                if (aProfile.mUseSeq > aMaxUseSeq)
                    aMaxUseSeq = aProfile.mUseSeq;

                mProfileMap[aProfile.mName] = aProfile;
            }

            mNextProfileId = aMaxProfileId + 1;
            mNextProfileUseSeq = aMaxUseSeq + 1;
        }
        else
        {
            aWriter->WriteShort((short)mProfileMap.size());
            
            for (auto anItr = mProfileMap.begin(); anItr != mProfileMap.end(); anItr++)
                anItr->second.SyncSummary(theSync);
        }
    }
}

void ProfileMgr::Load()
{
    Buffer aBuffer;
    std::string aFileName = GetAppDataFolder() + "userdata/users.dat";

    try
    {
        if (gSexyAppBase->ReadBufferFromFile(aFileName, &aBuffer))
        {
            DataReader aReader;
            aReader.OpenMemory(aBuffer.GetDataPtr(), aBuffer.GetDataLen(), false);
            DataSync aSync(aReader);
            SyncState(aSync);
        }
    }
    catch (DataReaderException&)
    {
        Clear();
    }
}

void ProfileMgr::Save()
{
    DataWriter aWriter;
    aWriter.OpenMemory(0x20);
    DataSync aSync(aWriter);
    SyncState(aSync);

    MkDir(GetAppDataFolder() + "userdata");
    std::string aFileName = GetAppDataFolder() + "userdata/users.dat";
    gSexyAppBase->WriteBytesToFile(aFileName, aWriter.GetDataPtr(), aWriter.GetDataLen());
}

void ProfileMgr::DeleteProfile(ProfileMap::iterator theProfile)
{
    theProfile->second.DeleteUserFiles();
    mProfileMap.erase(theProfile);
}

bool ProfileMgr::DeleteProfile(const SexyString& theName)
{
    auto anItr = mProfileMap.find(theName);
    if (anItr == mProfileMap.end())
        return false;

    DeleteProfile(anItr);
    return true;
}

bool ProfileMgr::RenameProfile(const SexyString& theOldName, const SexyString& theNewName)
{
    auto anOldItr = mProfileMap.find(theOldName);
    if (anOldItr == mProfileMap.end())
        return false;
    else
    {
        // �ж��޸�ǰ����û����Ƿ�һ�£�һ����ֱ����ԭ�浵�н����޸ģ�������Ҫ�������
        if (_stricmp(theOldName.c_str(), theNewName.c_str()) == 0)
            anOldItr->second.mName = theNewName;
        else
        {
            // �� mProfileMap �в���һ�������û������ɴ浵��ɵĶ���
            auto aRet = mProfileMap.emplace(theNewName, anOldItr->second);  // auto aRet = mProfileMap.insert({theNewName, anOldItr->second});
            // ͨ������ֵ������û����Ƿ���ԭ�д浵�ظ����ظ��򷵻� false������ɹ����������
            if (!aRet.second)
                return false;
            else
            {
                // ɾ�� mProfileMap ��ԭ�û������ɴ浵�ļ�ֵ��
                mProfileMap.erase(anOldItr);
                // �޸��²���ļ�ֵ���д浵���û���
                aRet.first->second.mName = theNewName;
            }
        }
        return true;
    }
}

void ProfileMgr::DeleteOldestProfile()
{
    if (mProfileMap.size() == 0)
        return;

    //�� mUseSeq ��С�Ĵ浵������λ�ü�¼�� anOldest ��������
    auto anOldest = mProfileMap.begin();
    for (auto anItr = anOldest; anItr != mProfileMap.end(); anItr++)
        if (anItr->second.mUseSeq < anOldest->second.mUseSeq)
            anOldest = anItr;
    //ɾ����¼�� mUseSeq ��С�Ĵ浵
    DeleteProfile(anOldest);
}

PlayerInfo* ProfileMgr::GetProfile(const SexyString& theName)
{
    auto anItr = mProfileMap.find(theName);
    if (anItr != mProfileMap.end())
    {
        PlayerInfo* aProfile = &anItr->second;
        aProfile->LoadDetails();
        aProfile->mUseSeq = mNextProfileUseSeq++;
        return aProfile;
    }
    return nullptr;
}

PlayerInfo* ProfileMgr::AddProfile(const SexyString& theName)
{
    auto aRet = mProfileMap.emplace(theName, PlayerInfo());
    if (aRet.second)
    {
        PlayerInfo* aProfile = &aRet.first->second;
        aProfile->mName = theName;
        aProfile->mId = mNextProfileId++;
        aProfile->mUseSeq = mNextProfileUseSeq++;
        DeleteOldProfiles();
        return aProfile;
    }
    return nullptr;
}
