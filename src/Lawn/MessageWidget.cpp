#include "Board.h"
#include "Challenge.h"
#include "../LawnApp.h"
#include "../Resources.h"
#include "MessageWidget.h"
#include "SexyAppFramework/graphics/Font.h"
#include "../Sexy.TodLib/TodCommon.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Sexy.TodLib/TodStringFile.h"

//(0x4081F1)
MessageWidget::MessageWidget(LawnApp* theApp)
{
	mApp = theApp;
	mDuration = 0;
	mLabel[0] = __S('\0');
	mMessageStyle = MessageStyle::MESSAGE_STYLE_OFF;
	mLabelNext[0] = __S('\0');
	mMessageStyleNext = MessageStyle::MESSAGE_STYLE_OFF;
	mSlideOffTime = 100;
	memset(mTextReanimID, (int)ReanimationID::REANIMATIONID_NULL, MAX_MESSAGE_LENGTH);
}

//0x458FC0
void MessageWidget::ClearReanim()
{
	for (int i = 0; i < MAX_MESSAGE_LENGTH; i++)
	{
		Reanimation* aReanim = mApp->ReanimationTryToGet(mTextReanimID[i]);
		if (aReanim)
		{
			aReanim->ReanimationDie();
			mTextReanimID[i] = ReanimationID::REANIMATIONID_NULL;
		}
	}
}

void MessageWidget::ClearLabel()
{
	if (mReanimType != ReanimationType::REANIM_NONE)
	{
		mDuration = std::min(mDuration, 100 + mSlideOffTime + 1);
	}
	else
	{
		mDuration = 0;
	}
}

//0x459010
// GOTY @Patoke: inlined 0x459715
void MessageWidget::SetLabel(const SexyString& theNewLabel, MessageStyle theMessageStyle)
{
    SexyString aLabel = TodStringTranslate(theNewLabel);

    // 检查 aLabel 长度是否超出限制
    TOD_ASSERT(aLabel.length() < MAX_MESSAGE_LENGTH - 1);

    if (mReanimType != ReanimationType::REANIM_NONE && mDuration > 0)
    {
        mMessageStyleNext = theMessageStyle;

        // 检查 mLabelNext 的长度
        if (aLabel.length() < MAX_MESSAGE_LENGTH)
        {
            strncpy(mLabelNext, aLabel.c_str(), MAX_MESSAGE_LENGTH - 1);
            mLabelNext[MAX_MESSAGE_LENGTH - 1] = '\0'; // 确保字符串以 null 结尾
        }
        ClearLabel();
    }
    else
    {
        ClearReanim();

        // 检查 mLabel 的长度
        if (aLabel.length() < MAX_MESSAGE_LENGTH)
        {
            strncpy(mLabel, aLabel.c_str(), MAX_MESSAGE_LENGTH - 1);
            mLabel[MAX_MESSAGE_LENGTH - 1] = '\0';
        }
        mMessageStyle = theMessageStyle;
        mReanimType = ReanimationType::REANIM_NONE;

        switch (theMessageStyle)
        {
            // 根据不同的 MessageStyle 设置 mDuration
            case MessageStyle::MESSAGE_STYLE_HINT_LONG:
            case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE:
            case MessageStyle::MESSAGE_STYLE_ZEN_GARDEN_LONG:
            case MessageStyle::MESSAGE_STYLE_HINT_TALL_LONG:
                mDuration = 1500;
                break;

            case MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE:
                mDuration = 500;
                break;

            case MessageStyle::MESSAGE_STYLE_HINT_FAST:
            case MessageStyle::MESSAGE_STYLE_HINT_TALL_FAST:
            case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST:
            case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1:
            case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2:
            case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER:
                mDuration = 500;
                break;

            case MessageStyle::MESSAGE_STYLE_HINT_STAY:
            case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY:
            case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER_STAY:
                mDuration = 10000;
                break;

            case MessageStyle::MESSAGE_STYLE_HOUSE_NAME:
                mDuration = 250;
                break;

            case MessageStyle::MESSAGE_STYLE_HUGE_WAVE:
                mDuration = 750;
                mReanimType = ReanimationType::REANIM_TEXT_FADE_ON;
                break;

            case MessageStyle::MESSAGE_STYLE_SLOT_MACHINE:
                mDuration = 750;
                break;

            case MessageStyle::MESSAGE_STYLE_ACHIEVEMENT: // @Patoke: implemented
                mDuration = 250;
                break;

            default:
            TOD_ASSERT();
                break;
        }

        if (mReanimType != ReanimationType::REANIM_NONE)
        {
            LayoutReanimText();
        }
        mDisplayTime = mDuration;
    }
}


//0x4591E0
void MessageWidget::LayoutReanimText()
{
	float aMaxWidth = 0;
	int aCurLine = 0, aCurPos = 0;
	_Font* aFont = GetFont();
	int aLabelLen = sexystrlen(mLabel);
	mSlideOffTime = aLabelLen + 100;

	float aLineWidth[MAX_REANIM_LINES];
	for (int aPos = 0; aPos <= aLabelLen; aPos++)
	{
		if (aPos == aLabelLen || mLabel[aPos] == __S('\n'))
		{
			TOD_ASSERT(aCurLine < MAX_REANIM_LINES);

			int aLen = aPos - aCurPos;
			int aOff = aCurPos;
			aCurPos = aPos + 1;
			SexyString aLine(&mLabel[aOff], aLen);
			
			aLineWidth[aCurLine] = aFont->StringWidth(aLine);
			aMaxWidth = std::max(aMaxWidth, aLineWidth[aCurLine]);
			aCurLine++;
		}
	}

	aCurLine = 0;
	float aCurPosY = 0.0f;
	float aCurPosX = -aLineWidth[0] * 0.5f;
	// 以下遍历字幕中的所有文本，分别在适当的位置创建每一个文字的动画
	for (int aPos = 0; aPos < aLabelLen; aPos++)
	{
		// 创建文字的动画
		Reanimation* aReanimText = mApp->AddReanimation(aCurPosX, aCurPosY, 0, mReanimType);
		aReanimText->mIsAttachment = true;
		aReanimText->PlayReanim("anim_enter", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0.0f, 0.0f);
		mTextReanimID[aPos] = mApp->ReanimationGetID(aReanimText);

		aCurPosX += aFont->CharWidth(mLabel[aPos]);  // 坐标调整至下一个文字的位置
		if (mLabel[aPos] == __S('\n'))  // 换行处理
		{
			aCurLine++;
			TOD_ASSERT(aCurLine < MAX_REANIM_LINES);
			aCurPosX = -aLineWidth[aCurLine] * 0.5f;
			aCurPosY += aFont->GetLineSpacing();
		}
	}
}

//0x4594B0
void MessageWidget::Update()
{
	if (!mApp->mBoard || mApp->mBoard->mPaused)
		return;

	// 更新字幕的剩余时间倒计时和下一轮字幕的切换
	if (mDuration < 10000 && mDuration > 0)
	{
		mDuration--;
		if (mDuration == 0)
		{
			mMessageStyle = MessageStyle::MESSAGE_STYLE_OFF;
			if (mMessageStyleNext != MessageStyle::MESSAGE_STYLE_OFF)
			{
				SetLabel(mLabelNext, mMessageStyleNext);
				mMessageStyleNext = MessageStyle::MESSAGE_STYLE_OFF;
			}
		}
	}

	int aLabelLen = sexystrlen(mLabel);
	// 以下遍历每个文字的动画，设置其动画速率并更新其动画
	for (int aPos = 0; aPos < aLabelLen; aPos++)
	{
		Reanimation* aTextReanim = mApp->ReanimationTryToGet(mTextReanimID[aPos]);
		if (aTextReanim == nullptr)
		{
			break;  // 当不存在文本动画时，跳出循环，直接返回
		}

		// 设置动画速率
		int aTextSpeed = mReanimType == ReanimationType::REANIM_TEXT_FADE_ON ? 100 : 1;
		if (mDuration > mSlideOffTime)
		{
			if (mReanimType == ReanimationType::REANIM_TEXT_FADE_ON)
			{
				aTextReanim->mAnimRate = 60.0f;
			}
			else
			{
				aTextReanim->mAnimRate = TodAnimateCurveFloat(0, 50, (mDisplayTime - mDuration) * aTextSpeed - aPos, 0.0f, 40.0f, TodCurves::CURVE_LINEAR);
			}
		}
		else
		{
			if (mDuration == mSlideOffTime)
			{
				aTextReanim->PlayReanim("anim_leave", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 0.0f);
			}
			aTextReanim->mAnimRate = TodAnimateCurveFloat(0, 50, (mSlideOffTime - mDuration) * aTextSpeed - aPos, 0.0f, 40.0f, TodCurves::CURVE_LINEAR);
		}

		aTextReanim->Update();  //更新动画
	}
}

//0x459710
void MessageWidget::DrawReanimatedText(Graphics* g, _Font* theFont, const Color& theColor, float thePosY)
{
	int aLabelLen = sexystrlen(mLabel);
	for (int aPos = 0; aPos < aLabelLen; aPos++)
	{
		Reanimation* aTextReanim = mApp->ReanimationTryToGet(mTextReanimID[aPos]);
		if (aTextReanim == nullptr)
		{
			break;  // 当不存在文本动画时，跳出循环，直接返回
		}

		ReanimatorTransform aTransform;
		aTextReanim->GetCurrentTransform(2, &aTransform);

		int anAlpha = std::clamp(FloatRoundToInt(theColor.mAlpha * aTransform.mAlpha), 0, 255);
		if (anAlpha <= 0)
		{
			break;  // 文本动画完全透明时，直接返回
		}
		Color aFinalColor(theColor);
		aFinalColor.mAlpha = anAlpha;

		aTransform.mTransX += aTextReanim->mOverlayMatrix.m02;
		aTransform.mTransY += aTextReanim->mOverlayMatrix.m12 + thePosY - BOARD_HEIGHT / 2;
		if (mReanimType == ReanimationType::REANIM_TEXT_FADE_ON && mDisplayTime - mDuration < mSlideOffTime)
		{
			float aStretch = 1.0f - aTextReanim->mAnimTime;
			aTransform.mTransX += aStretch * aTextReanim->mOverlayMatrix.m02;
		}

		SexyMatrix3 aMatrix;
		Reanimation::MatrixFromTransform(aTransform, aMatrix);
		SexyString aLetter;
		aLetter.append(1, mLabel[aPos]);
		TodDrawStringMatrix(g, theFont, aMatrix, aLetter, aFinalColor);
	}
}

//0x459990
// GOTY @Patoke: inlined 0x45CAEF
_Font* MessageWidget::GetFont()
{
	switch (mMessageStyle)
	{
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER_STAY:
	case MessageStyle::MESSAGE_STYLE_HINT_LONG:
	case MessageStyle::MESSAGE_STYLE_HINT_FAST:
	case MessageStyle::MESSAGE_STYLE_HINT_STAY:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_FAST:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_LONG:
	case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE:
	case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST:
	case MessageStyle::MESSAGE_STYLE_HOUSE_NAME:
	case MessageStyle::MESSAGE_STYLE_HUGE_WAVE:
	case MessageStyle::MESSAGE_STYLE_ZEN_GARDEN_LONG:
	case MessageStyle::MESSAGE_STYLE_ACHIEVEMENT: // @Patoke: implemented
		return Sexy::FONT_HOUSEOFTERROR28;

	case MessageStyle::MESSAGE_STYLE_SLOT_MACHINE:
		return Sexy::FONT_HOUSEOFTERROR16;
	case MessageStyle::MESSAGE_STYLE_OFF:
		break;
	}

	TOD_ASSERT();
	unreachable();
}

//0x4599E0
// GOTY @Patoke: 0x45D2B0
void MessageWidget::Draw(Graphics* g)
{
	if (mDuration <= 0)
		return;
	
	_Font* aFont = GetFont();
	_Font* aOutlineFont = nullptr;
	int aPosX = BOARD_WIDTH / 2;
	int aPosY = 596;
	int aTextOffsetY = 0;
	int aRectHeight = 0;
	int aMinAlpha = 255;
	Color aColor(250, 250, 0, 255);
	Color aOutlineColor(0, 0, 0, 255);
	bool aFadeOut = false;
	if (aFont == Sexy::FONT_CONTINUUMBOLD14)
	{
		aOutlineFont = Sexy::FONT_CONTINUUMBOLD14OUTLINE;
	}

	switch (mMessageStyle)
	{
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL1_STAY:
		aPosY = 400;
		aRectHeight = 110;
		aTextOffsetY = -4;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LEVEL2:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER:
	case MessageStyle::MESSAGE_STYLE_TUTORIAL_LATER_STAY:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_FAST:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_UNLOCKMESSAGE:
	case MessageStyle::MESSAGE_STYLE_HINT_TALL_LONG:
	case MessageStyle::MESSAGE_STYLE_ACHIEVEMENT: // @Patoke: implemented
		aPosY = 476;
		aRectHeight = 100;
		aTextOffsetY = -4;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	case MessageStyle::MESSAGE_STYLE_HINT_LONG:
	case MessageStyle::MESSAGE_STYLE_HINT_FAST:
	case MessageStyle::MESSAGE_STYLE_HINT_STAY:
		aPosY = 527;
		aRectHeight = 55;
		aTextOffsetY = -4;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE:
	case MessageStyle::MESSAGE_STYLE_BIG_MIDDLE_FAST:
		aPosY = 300;
		aRectHeight = 110;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	case MessageStyle::MESSAGE_STYLE_HOUSE_NAME:
		aPosY = 550;
		aColor = Color(255, 255, 255, 255);
		aFadeOut = true;
		break;

	case MessageStyle::MESSAGE_STYLE_HUGE_WAVE:
		aPosY = 330;
		aColor = Color(255, 0, 0);
		break;

	case MessageStyle::MESSAGE_STYLE_SLOT_MACHINE:
		aPosY = 93;
		aPosX = 340;
		aMinAlpha = 64;
		break;

	case MessageStyle::MESSAGE_STYLE_ZEN_GARDEN_LONG:
		aPosY = 514;
		aRectHeight = 55;
		aTextOffsetY = -4;
		aColor = Color(253, 245, 173);
		aMinAlpha = 192;
		break;

	default:
		TOD_ASSERT();
		break;
	}

	if (mReanimType != ReanimationType::REANIM_NONE)
	{
		if (aFont == Sexy::FONT_CONTINUUMBOLD14)
		{
			DrawReanimatedText(g, Sexy::FONT_CONTINUUMBOLD14OUTLINE, Color::Black, aPosY);
		}
		DrawReanimatedText(g, aFont, aColor, aPosY);
	}
	else
	{
		if (aMinAlpha != 255)
		{
			aColor.mAlpha = TodAnimateCurve(75, 0, mApp->mBoard->mMainCounter % 75, aMinAlpha, 255, TodCurves::CURVE_BOUNCE_SLOW_MIDDLE);
			aOutlineColor.mAlpha = aColor.mAlpha;
		}
		if (aFadeOut)
		{
			aColor.mAlpha = std::clamp(mDuration * 15, 0, 255);
			aOutlineColor.mAlpha = aColor.mAlpha;
		}

		if (aRectHeight > 0)
		{
			aOutlineColor = Color(0, 0, 0, 128);
			Rect aRect(0, aPosY, BOARD_WIDTH, aRectHeight);
			g->SetColor(aOutlineColor);
			g->FillRect(aRect);

			aRect.mY += aTextOffsetY;
			TodDrawStringWrapped(g, mLabel, aRect, aFont, aColor, DrawStringJustification::DS_ALIGN_CENTER_VERTICAL_MIDDLE);
		}
		else
		{
			Rect aRect(aPosX - mApp->mBoard->mX - BOARD_WIDTH / 2, aPosY - aFont->mAscent, BOARD_WIDTH, BOARD_HEIGHT);
			if (aOutlineFont)
			{
				TodDrawStringWrapped(g, mLabel, aRect, aOutlineFont, aOutlineColor, DrawStringJustification::DS_ALIGN_CENTER);
			}
			TodDrawStringWrapped(g, mLabel, aRect, aFont, aColor, DrawStringJustification::DS_ALIGN_CENTER);
		}

		if (mMessageStyle == MessageStyle::MESSAGE_STYLE_HOUSE_NAME)
		{
			SexyString aSubStr;
			if (mApp->IsSurvivalMode() && mApp->mBoard->mChallenge->mSurvivalStage > 0)
			{
				int aFlags = mApp->mBoard->GetNumWavesPerSurvivalStage() * mApp->mBoard->mChallenge->mSurvivalStage / mApp->mBoard->GetNumWavesPerFlag();
				SexyString aFlagStr = mApp->Pluralize(aFlags, __S("[ONE_FLAG]"), __S("[COUNT_FLAGS]"));
				SexyString aSubStr = TodReplaceString(__S("[FLAGS_COMPLETED]"), __S("{FLAGS}"), aFlagStr);
			}

			if (aSubStr.size() > 0)
			{
				TodDrawString(
					g, 
					aSubStr, 
					BOARD_WIDTH / 2 - mApp->mBoard->mX, 
					aPosY + 26, 
					Sexy::FONT_HOUSEOFTERROR16, 
					Color(224, 187, 62, aColor.mAlpha), 
					DrawStringJustification::DS_ALIGN_CENTER
				);
			}
		}
	}
}
