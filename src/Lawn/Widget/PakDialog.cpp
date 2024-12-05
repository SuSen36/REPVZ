#include "PakDialog.h"
#include "GameButton.h"
#include <filesystem>
#include "../../LawnApp.h"
#include "../../Resources.h"
#include "../System/ProfileMgr.h"
#include "../System/PlayerInfo.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "SexyAppFramework/widget/ListWidget.h"
#include "SexyAppFramework/paklib/PakInterface.h"
#include "Sexy.TodLib/TodDebug.h"

static int gPakListWidgetColors[][3] = {
        {  23,  24,  35 },
        {   0,   0,   0 },
        { 235, 225, 180 },
        { 255, 255, 255 },
        {  20, 180,  15 }
};

PakDialog::PakDialog(LawnApp* theApp) : LawnDialog(theApp, Dialogs::DIALOG_PAKDIALOG, true, __S("Select Pak"), __S(""), __S(""), Dialog::BUTTONS_OK_CANCEL)
{
    mVerticalCenterText = false;
    mPakList = new ListWidget(0, FONT_BRIANNETOD16, this);
    mPakList->SetColors(gPakListWidgetColors, LENGTH(gPakListWidgetColors));
    mPakList->mDrawOutline = true;
    mPakList->mJustify = ListWidget::JUSTIFY_CENTER;
    mPakList->mItemHeight = 24;

    mMoveUpButton = MakeButton(PakDialog::PakDialog_MoveUpPak, this, __S("Move Up"));
    mMoveDownButton = MakeButton(PakDialog::PakDialog_MoveDownPak, this, __S("Move Down"));

    // 获取 PAK 文件并添加到列表
    const auto& pakFiles = gPakInterface->GetPakFileNames(); // 从 PakInterface 获取 PAK 文件名列表
    mNumPaks = pakFiles.size();

    for (const auto& pakName : pakFiles)
    {
        mPakList->AddLine(pakName.c_str(), false); // 添加 PAK 文件名到列表
    }

    //if (mNumPaks < 8)
    //{
    //    mPakList->AddLine(TodStringTranslate(__S("(Add a New Pak)")), false);
    //}

    mTallBottom = true;
    CalcSize(210, 270);
}

PakDialog::~PakDialog()
{
    delete mPakList;
    delete mMoveUpButton;
    delete mMoveDownButton;
}

void PakDialog::Resize(int theX, int theY, int theWidth, int theHeight)
{
    LawnDialog::Resize(theX, theY, theWidth, theHeight);
    mPakList->Resize(GetLeft() + 30, GetTop() + 4, GetWidth() - 60, 200);
    mMoveUpButton->Layout(LayoutFlags::LAY_SameLeft | LayoutFlags::LAY_Above | LayoutFlags::LAY_SameHeight | LayoutFlags::LAY_SameWidth, mLawnYesButton, 0, 0, 0, 0);
    mMoveDownButton->Layout(LayoutFlags::LAY_SameLeft | LayoutFlags::LAY_Above | LayoutFlags::LAY_SameHeight | LayoutFlags::LAY_SameWidth, mLawnNoButton, 0, 0, 0, 0);
}

int PakDialog::GetPreferredHeight(int theWidth)
{
    return LawnDialog::GetPreferredHeight(theWidth) + 190;
}

void PakDialog::AddedToManager(WidgetManager* theWidgetManager)
{
    LawnDialog::AddedToManager(theWidgetManager);
    AddWidget(mPakList);
    AddWidget(mMoveDownButton);
    AddWidget(mMoveUpButton);
}

void PakDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
    LawnDialog::RemovedFromManager(theWidgetManager);
    RemoveWidget(mPakList);
    RemoveWidget(mMoveDownButton);
    RemoveWidget(mMoveUpButton);
}

SexyString PakDialog::GetSelName()
{
    if (mPakList->mSelectIdx < 0 || mPakList->mSelectIdx >= mNumPaks)
    {
        return __S("");
    }
    return mPakList->GetStringAt(mPakList->mSelectIdx);
}

void PakDialog::MovePakUp()
{
    int selectedIndex = mPakList->mSelectIdx;
    if (selectedIndex > 0) // 确保不是第一个元素
    {
        // 交换选中项与上面的项
        SexyString selectedPakName = mPakList->GetStringAt(selectedIndex);
        SexyString upperPakName = mPakList->GetStringAt(selectedIndex - 1);

        mPakList->SetLine(selectedIndex - 1, selectedPakName); // 设置上方行
        mPakList->SetLine(selectedIndex, upperPakName); // 设置当前行为上方的行

        // 更新选择索引
        mPakList->SetSelect(selectedIndex - 1);
    }
}

void PakDialog::MovePakDown()
{
    int selectedIndex = mPakList->mSelectIdx;
    if (selectedIndex < mPakList->GetLineCount() - 1) // 确保不是最后一个元素
    {
        // 交换选中项与下面的项
        SexyString selectedPakName = mPakList->GetStringAt(selectedIndex);
        SexyString lowerPakName = mPakList->GetStringAt(selectedIndex + 1);

        mPakList->SetLine(selectedIndex + 1, selectedPakName); // 设置下方行
        mPakList->SetLine(selectedIndex, lowerPakName); // 设置当前行为下方的行

        // 更新选择索引
        mPakList->SetSelect(selectedIndex + 1);
    }
}

void PakDialog::Draw(Graphics* g)
{
    LawnDialog::Draw(g);
}

void PakDialog::ListClicked(int theId, int theIdx, int theClickCount)
{
    (void)theId;
    mPakList->SetSelect(theIdx);
    if (theClickCount == 2) {
        mApp->FinishPakDialog(true);
    }
}

void PakDialog::ButtonDepress(int theId)
{
    LawnDialog::ButtonDepress(theId);
    SexyString aSelName = GetSelName();
    if (!aSelName.empty())
    {
        switch (theId)
        {
            case PakDialog::PakDialog_MoveUpPak:
                MovePakUp();
                break;
            case PakDialog::PakDialog_MoveDownPak:
                MovePakDown();
                break;
            case Dialog::ID_OK:
                SavePakList();
                break;
        }
    }
}

void PakDialog::EditWidgetText(int theId, const SexyString& theString)
{
    (void)theId;(void)theString;
    mApp->ButtonDepress(mId + 2000);
}

void PakDialog::SavePakList()
{
    // 获取 PAK 文件夹路径
    std::string pakFolder = GetPakFolder();

    // 确保文件夹存在
    MkDir(pakFolder); // 使用已经定义的 MkDir

    // 创建完整的文件路径
    std::string filePath = pakFolder + "pak_list.txt";

    FILE* f = fopen(filePath.c_str(), "w");
    if (f == nullptr)
    {
        TodTrace( "Failed to open log file '%s'\n", filePath.c_str());
        return;
    }

    for (int i = 0; i < mPakList->GetLineCount(); ++i)
    {
        SexyString pakName = mPakList->GetStringAt(i);
        // 把每个 PAK 文件名写入文件
        fprintf(f, "%s\n", pakName.c_str()); // 写入格式为 "pakName [i]"
    }

    fclose(f); // 关闭文件
}


bool PakDialog::AllowChar(int theId, SexyChar theChar)
{
    (void)theId;
    return sexyisdigit(theChar);
}
