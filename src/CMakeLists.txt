set(PROJECT_NAME LawnProject)

################################################################################
# Source groups
################################################################################

set(Src__Common__Common_Include
        "ConstEnums.h"
        "GameConstants.h"
        "Lawn/BoardInclude.h"
        "Lawn/LawnCommon.h"
        "Resources.h"
)
source_group("Src\\Common\\Common Include" FILES ${Src__Common__Common_Include})

set(Src__Common__Common_Source
        "Lawn/LawnCommon.cpp"
        "Resources.cpp"
)
source_group("Src\\Common\\Common Source" FILES ${Src__Common__Common_Source})

set(Lawn__Common__Common_Include
    "Lawn/Board.h"
    "Lawn/Challenge.h"
    "Lawn/Coin.h"
    "Lawn/CursorObject.h"
    "Lawn/Cutscene.h"
    "Lawn/GameObject.h"
    "Lawn/GridItem.h"
    "Lawn/LawnMower.h"
    "Lawn/MessageWidget.h"
    "Lawn/Plant.h"
    "Lawn/Projectile.h"
    "Lawn/SeedPacket.h"
    "Lawn/ToolTipWidget.h"
    "Lawn/ZenGarden.h"
    "Lawn/Zombie.h"
)
source_group("Lawn\\Common\\Common Include" FILES ${Lawn__Common__Common_Include})

set(Lawn__Common__Common_Source
    "Lawn/Board.cpp"
    "Lawn/Challenge.cpp"
    "Lawn/Coin.cpp"
    "Lawn/CursorObject.cpp"
    "Lawn/CutScene.cpp"
    "Lawn/GameObject.cpp"
    "Lawn/GridItem.cpp"
    "Lawn/LawnMower.cpp"
    "Lawn/MessageWidget.cpp"
    "Lawn/Plant.cpp"
    "Lawn/Projectile.cpp"
    "Lawn/SeedPacket.cpp"
    "Lawn/ToolTipWidget.cpp"
    "Lawn/ZenGarden.cpp"
    "Lawn/Zombie.cpp"
)
source_group("Lawn\\Common\\Common Source" FILES ${Lawn__Common__Common_Source})

set(Lawn__System__System_Include
    "Lawn/System/DataSync.h"
    "Lawn/System/Music.h"
    "Lawn/System/PlayerInfo.h"
    "Lawn/System/PoolEffect.h"
    "Lawn/System/PopDRMComm.h"
    "Lawn/System/ProfileMgr.h"
    "Lawn/System/ReanimationLawn.h"
    "Lawn/System/SaveGame.h"
)
source_group("Lawn\\System\\System Include" FILES ${Lawn__System__System_Include})

set(Lawn__System__System_Source
    "Lawn/System/DataSync.cpp"
    "Lawn/System/Music.cpp"
    "Lawn/System/PlayerInfo.cpp"
    "Lawn/System/PoolEffect.cpp"
    "Lawn/System/PopDRMComm.cpp"
    "Lawn/System/ProfileMgr.cpp"
    "Lawn/System/ReanimationLawn.cpp"
    "Lawn/System/SaveGame.cpp"
)
source_group("Lawn\\System\\System Source" FILES ${Lawn__System__System_Source})

set(Lawn__Widget__Widget_Include
    "Lawn/Widget/AchievementsScreen.h"
    "Lawn/Widget/AlmanacDialog.h"
    "Lawn/Widget/AwardScreen.h"
    "Lawn/Widget/ChallengeScreen.h"
    "Lawn/Widget/CheatDialog.h"
    "Lawn/Widget/ContinueDialog.h"
    "Lawn/Widget/CreditScreen.h"
    "Lawn/Widget/GameButton.h"
    "Lawn/Widget/GameSelector.h"
    "Lawn/Widget/ImitaterDialog.h"
    "Lawn/Widget/LawnDialog.h"
    "Lawn/Widget/NewOptionsDialog.h"
    "Lawn/Widget/NewUserDialog.h"
    "Lawn/Widget/SeedChooserScreen.h"
    "Lawn/Widget/StoreScreen.h"
    "Lawn/Widget/TitleScreen.h"
    "Lawn/Widget/UserDialog.h"
)
source_group("Lawn\\Widget\\Widget Include" FILES ${Lawn__Widget__Widget_Include})

set(Lawn__Widget__Widget_Source
    "Lawn/Widget/AchievementsScreen.cpp"
    "Lawn/Widget/AlmanacDialog.cpp"
    "Lawn/Widget/AwardScreen.cpp"
    "Lawn/Widget/ChallengeScreen.cpp"
    "Lawn/Widget/CheatDialog.cpp"
    "Lawn/Widget/ContinueDialog.cpp"
    "Lawn/Widget/CreditScreen.cpp"
    "Lawn/Widget/GameButton.cpp"
    "Lawn/Widget/GameSelector.cpp"
    "Lawn/Widget/ImitaterDialog.cpp"
    "Lawn/Widget/LawnDialog.cpp"
    "Lawn/Widget/NewOptionsDialog.cpp"
    "Lawn/Widget/NewUserDialog.cpp"
    "Lawn/Widget/SeedChooserScreen.cpp"
    "Lawn/Widget/StoreScreen.cpp"
    "Lawn/Widget/TitleScreen.cpp"
    "Lawn/Widget/UserDialog.cpp"
)
source_group("Lawn\\Widget\\Widget Source" FILES ${Lawn__Widget__Widget_Source})

set(TodLib__Common__TodCommon_Include
    "Sexy.TodLib/DataArray.h"
    "Sexy.TodLib/Definition.h"
    "Sexy.TodLib/TodCommon.h"
    "Sexy.TodLib/TodDebug.h"
    "Sexy.TodLib/TodFoley.h"
    "Sexy.TodLib/TodList.h"
    "Sexy.TodLib/TodStringFile.h"
)
source_group("TodLib\\Common\\TodCommon Include" FILES ${TodLib__Common__TodCommon_Include})

set(TodLib__Common__TodCommon_Source
    "Sexy.TodLib/Definition.cpp"
    "Sexy.TodLib/TodCommon.cpp"
    "Sexy.TodLib/TodDebug.cpp"
    "Sexy.TodLib/TodFoley.cpp"
    "Sexy.TodLib/TodList.cpp"
    "Sexy.TodLib/TodStringFile.cpp"
)
source_group("TodLib\\Common\\TodCommon Source" FILES ${TodLib__Common__TodCommon_Source})

set(TodLib__Effect__Effect_Include
    "Sexy.TodLib/Attachment.h"
    "Sexy.TodLib/EffectSystem.h"
    "Sexy.TodLib/FilterEffect.h"
    "Sexy.TodLib/ReanimAtlas.h"
    "Sexy.TodLib/Reanimator.h"
    "Sexy.TodLib/TodParticle.h"
    "Sexy.TodLib/Trail.h"
)
source_group("TodLib\\Effect\\Effect Include" FILES ${TodLib__Effect__Effect_Include})

set(TodLib__Effect__Effect_Source
    "Sexy.TodLib/Attachment.cpp"
    "Sexy.TodLib/EffectSystem.cpp"
    "Sexy.TodLib/FilterEffect.cpp"
    "Sexy.TodLib/ReanimAtlas.cpp"
    "Sexy.TodLib/Reanimator.cpp"
    "Sexy.TodLib/TodParticle.cpp"
    "Sexy.TodLib/Trail.cpp"
)
source_group("TodLib\\Effect\\Effect Source" FILES ${TodLib__Effect__Effect_Source})

set(TodLib__Effect__Effect_Include
        "Sexy.TodLib/Attachment.h"
        "Sexy.TodLib/EffectSystem.h"
        "Sexy.TodLib/FilterEffect.h"
        "Sexy.TodLib/ReanimAtlas.h"
        "Sexy.TodLib/Reanimator.h"
        "Sexy.TodLib/TodParticle.h"
        "Sexy.TodLib/Trail.h"
)
source_group("TodLib\\Effect\\Effect Include" FILES ${TodLib__Effect__Effect_Include})

set(SexyAppFramework__Widget__Widget_Source
        "SexyAppFramework/widget/ButtonWidget.cpp"
        "SexyAppFramework/widget/Checkbox.cpp"
        "SexyAppFramework/widget/Dialog.cpp"
        "SexyAppFramework/widget/DialogButton.cpp"
        "SexyAppFramework/widget/EditWidget.cpp"
        "SexyAppFramework/widget/HyperlinkWidget.cpp"
        "SexyAppFramework/widget/Insets.cpp"
        "SexyAppFramework/widget/ListWidget.cpp"
        "SexyAppFramework/widget/ScrollbarWidget.cpp"
        "SexyAppFramework/widget/ScrollbuttonWidget.cpp"
        "SexyAppFramework/widget/Slider.cpp"
        "SexyAppFramework/widget/TextWidget.cpp"
        "SexyAppFramework/widget/Widget.cpp"
        "SexyAppFramework/widget/WidgetContainer.cpp"
        "SexyAppFramework/widget/WidgetManager.cpp"
)
source_group("SexyAppFramework\\Widget\\Widget Source" FILES ${SexyAppFramework__Widget__Widget_Source})

set(SexyAppFramework__Widget__Widget_Include
        "SexyAppFramework/widget/ButtonListener.h"
        "SexyAppFramework/widget/ButtonWidget.h"
        "SexyAppFramework/widget/Checkbox.h"
        "SexyAppFramework/widget/CheckboxListener.h"
        "SexyAppFramework/widget/Dialog.h"
        "SexyAppFramework/widget/DialogButton.h"
        "SexyAppFramework/widget/DialogListener.h"
        "SexyAppFramework/widget/EditListener.h"
        "SexyAppFramework/widget/EditWidget.h"
        "SexyAppFramework/widget/HyperlinkWidget.h"
        "SexyAppFramework/widget/Insets.h"
        "SexyAppFramework/widget/ListListener.h"
        "SexyAppFramework/widget/ListWidget.h"
        "SexyAppFramework/widget/ScrollbarWidget.h"
        "SexyAppFramework/widget/ScrollbuttonWidget.h"
        "SexyAppFramework/widget/ScrollListener.h"
        "SexyAppFramework/widget/Slider.h"
        "SexyAppFramework/widget/SliderListener.h"
        "SexyAppFramework/widget/TextWidget.h"
        "SexyAppFramework/widget/Widget.h"
        "SexyAppFramework/widget/WidgetContainer.h"
        "SexyAppFramework/widget/WidgetManager.h"
)
source_group("SexyAppFramework\\Widget\\Widget Include" FILES ${SexyAppFramework__Widget__Widget_Include})

set(SexyAppFramework__Sound__Sound_Source
        "SexyAppFramework/sound/BassMusicInterface.cpp"
        "SexyAppFramework/sound/BassSoundInstance.cpp"
        "SexyAppFramework/sound/BassSoundManager.cpp"
)
source_group("SexyAppFramework\\Sound\\Sound Source" FILES ${SexyAppFramework__Sound__Sound_Source})

set(SexyAppFramework__Sound__Sound_Include
        "SexyAppFramework/sound/BassMusicInterface.h"
        "SexyAppFramework/sound/BassSoundInstance.h"
        "SexyAppFramework/sound/BassSoundManager.h"
        "SexyAppFramework/sound/DummyMusicInterface.h"
        "SexyAppFramework/sound/DummySoundManager.h"
        "SexyAppFramework/sound/MusicInterface.h"
        "SexyAppFramework/sound/SoundInstance.h"
        "SexyAppFramework/sound/SoundManager.h"
)
source_group("SexyAppFramework\\Sound\\Sound Include" FILES ${SexyAppFramework__Sound__Sound_Include})

set(SexyAppFramework__PakLib__PakLib_Source
        "SexyAppFramework/paklib/PakInterface.cpp"
)
source_group("SexyAppFramework\\PakLib\\PakLib Source" FILES ${SexyAppFramework__PakLib__PakLib_Source})

set(SexyAppFramework__PakLib__PakLib_Include
        "SexyAppFramework/paklib/PakInterface.h"
)
source_group("SexyAppFramework\\PakLib\\PakLib Include" FILES ${SexyAppFramework__PakLib__PakLib_Include})

set(SexyAppFramework__Misc__Misc_Source
        "SexyAppFramework/misc/Buffer.cpp"
        "SexyAppFramework/misc/CritSect.cpp"
        "SexyAppFramework/misc/Debug.cpp"
        "SexyAppFramework/misc/DescParser.cpp"
        "SexyAppFramework/misc/DirectXErrorString.cpp"
        "SexyAppFramework/misc/Flags.cpp"
        "SexyAppFramework/misc/HTTPTransfer.cpp"
        "SexyAppFramework/misc/KeyCodes.cpp"
        "SexyAppFramework/misc/ModVal.cpp"
        "SexyAppFramework/misc/MTRand.cpp"
        "SexyAppFramework/misc/PerfTimer.cpp"
        "SexyAppFramework/misc/PropertiesParser.cpp"
        "SexyAppFramework/misc/Ratio.cpp"
        "SexyAppFramework/misc/ResourceManager.cpp"
        "SexyAppFramework/misc/SEHCatcher.cpp"
        "SexyAppFramework/misc/SexyMatrix.cpp"
        "SexyAppFramework/misc/XMLParser.cpp"
)
source_group("SexyAppFramework\\Misc\\Misc Source" FILES ${SexyAppFramework__Misc__Misc_Source})

set(SexyAppFramework__Misc__Misc_Include
        "SexyAppFramework/misc/AutoCrit.h"
        "SexyAppFramework/misc/Buffer.h"
        "SexyAppFramework/misc/CritSect.h"
        "SexyAppFramework/misc/Debug.h"
        "SexyAppFramework/misc/DescParser.h"
        "SexyAppFramework/misc/DirectXErrorString.h"
        "SexyAppFramework/misc/Flags.h"
        "SexyAppFramework/misc/HTTPTransfer.h"
        "SexyAppFramework/misc/KeyCodes.h"
        "SexyAppFramework/misc/memmgr.h"
        "SexyAppFramework/misc/ModVal.h"
        "SexyAppFramework/misc/MTRand.h"
        "SexyAppFramework/misc/PerfTimer.h"
        "SexyAppFramework/misc/Point.h"
        "SexyAppFramework/misc/PropertiesParser.h"
        "SexyAppFramework/misc/Ratio.h"
        "SexyAppFramework/misc/Rect.h"
        "SexyAppFramework/misc/ResourceManager.h"
        "SexyAppFramework/misc/SEHCatcher.h"
        "SexyAppFramework/misc/SexyMatrix.h"
        "SexyAppFramework/misc/SexyVector.h"
        "SexyAppFramework/misc/SmartPtr.h"
        "SexyAppFramework/misc/XMLParser.h"
)
source_group("SexyAppFramework\\Misc\\Misc Include" FILES ${SexyAppFramework__Misc__Misc_Include})

set(SexyAppFramework__Imagelib__Imagelib_Source
        "SexyAppFramework/imagelib/ImageLib.cpp"
)
source_group("SexyAppFramework\\Imagelib\\Imagelib Source" FILES ${SexyAppFramework__Imagelib__Imagelib_Source})

set(SexyAppFramework__Imagelib__Imagelib_Include
        "SexyAppFramework/imagelib/ImageLib.h"
)
source_group("SexyAppFramework\\Imagelib\\Imagelib Include" FILES ${SexyAppFramework__Imagelib__Imagelib_Include})

set(SexyAppFramework__Graphics__Graphics_Source
        "SexyAppFramework/graphics/Color.cpp"
        "SexyAppFramework/graphics/Font.cpp"
        "SexyAppFramework/graphics/GLImage.cpp"
        "SexyAppFramework/graphics/Graphics.cpp"
        "SexyAppFramework/graphics/Image.cpp"
        "SexyAppFramework/graphics/ImageFont.cpp"
        "SexyAppFramework/graphics/MemoryImage.cpp"
        "SexyAppFramework/graphics/NativeDisplay.cpp"
        "SexyAppFramework/graphics/Quantize.cpp"
        "SexyAppFramework/graphics/SharedImage.cpp"
        "SexyAppFramework/graphics/SWTri.cpp"
)
source_group("SexyAppFramework\\Graphics\\Graphics Source" FILES ${SexyAppFramework__Graphics__Graphics_Source})

set(SexyAppFramework__Graphics__Graphics_Include
        "SexyAppFramework/graphics/Color.h"
        "SexyAppFramework/graphics/Font.h"
        "SexyAppFramework/graphics/GLImage.h"
        "SexyAppFramework/graphics/Graphics.h"
        "SexyAppFramework/graphics/Image.h"
        "SexyAppFramework/graphics/ImageFont.h"
        "SexyAppFramework/graphics/MemoryImage.h"
        "SexyAppFramework/graphics/NativeDisplay.h"
        "SexyAppFramework/graphics/Quantize.h"
        "SexyAppFramework/graphics/SharedImage.h"
        "SexyAppFramework/graphics/SWTri.h"
        "SexyAppFramework/graphics/TriVertex.h"
)
source_group("SexyAppFramework\\Graphics\\Graphics Include" FILES ${SexyAppFramework__Graphics__Graphics_Include})

set(SexyAppFramework__Common__Common_Include
        "SexyAppFramework/Common.h"
        "SexyAppFramework/CursorWidget.h"
        "SexyAppFramework/SexyApp.h"
        "SexyAppFramework/SexyAppBase.h"
)
source_group("SexyAppFramework\\Common\\Common Include" FILES ${SexyAppFramework__Common__Common_Include})

set(SexyAppFramework__Common__Common_Source
        "SexyAppFramework/Common.cpp"
        "SexyAppFramework/SexyApp.cpp"
        "SexyAppFramework/SexyAppBase.cpp"
)
source_group("SexyAppFramework\\Common\\Common Source" FILES ${SexyAppFramework__Common__Common_Source})

set(ALL_FILES
    ${no_group_source_files}
    ${Src__Common__Common_Include}
    ${Src__Common__Common_Source}
    ${Lawn__Common__Common_Include}
    ${Lawn__Common__Common_Source}
    ${Lawn__System__System_Include}
    ${Lawn__System__System_Source}
    ${Lawn__Widget__Widget_Include}
    ${Lawn__Widget__Widget_Source}
    ${TodLib__Common__TodCommon_Include}
    ${TodLib__Common__TodCommon_Source}
    ${TodLib__Effect__Effect_Include}
    ${TodLib__Effect__Effect_Source}
    ${SexyAppFramework__Common__Common_Include}
    ${SexyAppFramework__Common__Common_Source}
    ${SexyAppFramework__Widget__Widget_Include}
    ${SexyAppFramework__Widget__Widget_Source}
    ${SexyAppFramework__Sound__Sound_Include}
    ${SexyAppFramework__Sound__Sound_Source}
    ${SexyAppFramework__PakLib__PakLib_Include}
    ${SexyAppFramework__PakLib__PakLib_Source}
    ${SexyAppFramework__Misc__Misc_Include}
    ${SexyAppFramework__Misc__Misc_Source}
    ${SexyAppFramework__Imagelib__Imagelib_Include}
    ${SexyAppFramework__Imagelib__Imagelib_Source}
    ${SexyAppFramework__Graphics_Include}
    ${SexyAppFramework__Graphics_Source}
)

################################################################################
# Target
################################################################################
if(MSVC)
    add_executable(${PROJECT_NAME} WIN32 ${ALL_FILES})
else()
    add_executable(${PROJECT_NAME} ${ALL_FILES})
endif()

add_subdirectory(Lawn)
add_subdirectory(SexyAppFramework)
add_subdirectory(Sexy.TodLib)


target_sources(${PROJECT_NAME} PRIVATE

    # No group sources
    "Cursor13.cur"
    "LawnApp.cpp"
    "LawnApp.h"
    "main.cpp"
    ".gitattributes"
    ".gitignore"
    "resource.h"
    "LawnProject.rc"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE SexyAppBase)

################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_DEBUG            "src"
    TARGET_NAME_RELEASE          "src"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG            "${CMAKE_BINARY_DIR}/"
    OUTPUT_DIRECTORY_RELEASE          "${CMAKE_BINARY_DIR}/"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG;"
        #"_WINDOWS;"
        #"_VISUALC_;"
        #"_JPEGLIB_;"
        #"_CRT_SECURE_NO_WARNINGS"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG;"
        "RELEASEFINAL;"
        #"_WINDOWS;"
        #"_VISUALC_;"
        #"_JPEGLIB_"
    ">"
    "_LIB;"
    "_MBCS"
)

################################################################################
# Dependencies
################################################################################

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE ".") # SexyAppFramework
target_link_libraries(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++ "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
