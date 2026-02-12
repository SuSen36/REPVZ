cmake_minimum_required(VERSION 3.2.2)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)  # 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(xp-plants-vs-zombies LANGUAGES C CXX ASM)

# 启用并行编译
set(CMAKE_JOB_POOL_COMPILE compile_job_pool)
set(CMAKE_JOB_POOL_LINK link_job_pool)
set_property(GLOBAL PROPERTY JOB_POOLS compile_job_pool=8 link_job_pool=2)

# 根据构建类型设置默认优化级别
#if(NOT CMAKE_BUILD_TYPE)
#	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
#endif()

file(GLOB SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/graphics/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/imagelib/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/misc/*.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/misc/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/paklib/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/sound/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/widget/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/Sexy.TodLib/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/Lawn/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/Lawn/widgets/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Lawn/system/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/glad/*.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/zlib/*.c
)

# these files are included and must not be built in the project
list(REMOVE_ITEM SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sexy.TodLib/TodDrawTriangle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sexy.TodLib/TodDrawTriangleInc.cpp
)


if(ANDROID)
	list(APPEND SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/android/Window.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/android/Input.cpp
	)
	list(APPEND PLAT_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/android
	)
elseif (EMSCRIPTEN)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/emscripten/Window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/emscripten/Input.cpp
    )
    list(APPEND PLAT_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/emscripten
    )
else()
	list(APPEND SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/pc/Window.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/pc/Input.cpp
	)
	list(APPEND PLAT_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/pc
	)
endif()

set(SDL_STATIC OFF CACHE BOOL "Build static library of SDL" FORCE)
set(SDL_SHARED ON CACHE BOOL "Build shared library of SDL" FORCE)

# For Emscripten, SDL is usually handled by the toolchain itself
if(NOT EMSCRIPTEN)
    add_subdirectory(Libs/SDL)
    add_subdirectory(Libs/SDL_image)
endif()

include(FetchContent)

if (WIN32)
	set(BASS_SRC https://www.un4seen.com/files/bass24.zip)
	set(BASS_LIB x64/bass.dll)
	set(BASS_INCLUDE c)
elseif (UNIX)
	if (APPLE)
		set(BASS_SRC https://www.un4seen.com/files/bass24-osx.zip)
		set(BASS_LIB libbass.dylib)
	elseif (LINUX)
		set(BASS_SRC https://www.un4seen.com/files/bass24-linux.zip)
		set(BASS_LIB libs/x86_64/libbass.so)
		set(BASS_INCLUDE ./)
	elseif (ANDROID)
		set(BASS_INCLUDE c)
		# 使用ANDROID_ABI来动态识别当前架构
		if (ANDROID_ABI STREQUAL "arm64-v8a")
			set(BASS_SRC https://www.un4seen.com/files/bass24-android.zip)
			set(BASS_LIB libs/arm64-v8a/libbass.so)
		elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
			set(BASS_SRC https://www.un4seen.com/files/bass24-android.zip)
			set(BASS_LIB libs/armeabi-v7a/libbass.so)
		elseif (ANDROID_ABI STREQUAL "x86")
			set(BASS_SRC https://www.un4seen.com/files/bass24-android.zip)
			set(BASS_LIB libs/x86/libbass.so)
		elseif (ANDROID_ABI STREQUAL "x86_64")
			set(BASS_SRC https://www.un4seen.com/files/bass24-android.zip)
			set(BASS_LIB libs/x86_64/libbass.so)
		else()
			message(FATAL_ERROR "不支持的Android架构: ${ANDROID_ABI}.")
		endif()  # 结束针对 Android 的判断
	else()
		message(FATAL_ERROR "仅支持Android平台的构建.")
	endif()  # 结束针对 UNIX 的判断
elseif (EMSCRIPTEN)
    # BASS is generally not supported directly in Emscripten.
    # You would typically use Web Audio API directly from C++ or a JS wrapper.
    message(WARNING "BASS audio library is not directly supported by Emscripten. Audio will need to be handled via Web Audio API.")
endif()  # 结束针对 WIN32 的判断

message(STATUS "Current ABI: ${ANDROID_ABI}")

if(NOT EMSCRIPTEN) # Fetch bass only if not Emscripten
    FetchContent_Declare(
            bass
            URL ${BASS_SRC}
            #DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    FetchContent_GetProperties(bass)
    if (NOT bass_POPULATED)
        FetchContent_Populate(bass)
        file(COPY ${bass_SOURCE_DIR}/${BASS_LIB} DESTINATION ${CMAKE_BINARY_DIR})
    endif()

    get_filename_component(bassfile ${BASS_LIB} NAME)

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        add_library(bass STATIC IMPORTED GLOBAL)
    else ()
        add_library(bass STATIC IMPORTED GLOBAL)
    endif()

    set_target_properties(bass PROPERTIES
            IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/${bassfile}
    )
endif()

if (WIN32)
	add_executable(xp-plants-vs-zombies ${SOURCES})
elseif (ANDROID)
	add_library(xp-plants-vs-zombies SHARED ${SOURCES})
elseif (EMSCRIPTEN)
    add_executable(xp-plants-vs-zombies ${SOURCES}) # Emscripten output is an executable (.js or .html)
    set_target_properties(xp-plants-vs-zombies PROPERTIES SUFFIX ".html") # Set suffix to .html for convenience
else ()
	add_executable(xp-plants-vs-zombies ${SOURCES})
endif()

target_compile_definitions(xp-plants-vs-zombies PRIVATE $<$<CONFIG:Debug>:_DEBUG>)


target_include_directories(xp-plants-vs-zombies PRIVATE
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/Libs/SDL/include
	${PROJECT_SOURCE_DIR}/Libs/SDL_image/include
	${PLAT_INCLUDES}
	${bass_SOURCE_DIR}/${BASS_INCLUDE}
)

#target_compile_definitions(xp-plants-vs-zombies PRIVATE _DEBUG)


if (NOT EMSCRIPTEN)
    target_link_libraries(xp-plants-vs-zombies SDL2::SDL2 SDL2_image::SDL2_image bass)
else()
    target_link_libraries(xp-plants-vs-zombies SDL2) # Emscripten provides its own SDL2
endif()

if (WIN32)
	target_link_options(xp-plants-vs-zombies PRIVATE -static -static-libgcc -static-libstdc++)
	target_link_libraries(xp-plants-vs-zombies opengl32 ws2_32 ddraw dinput8 dxguid dxerr8 user32 gdi32 winmm imm32)
	target_compile_definitions(xp-plants-vs-zombies PRIVATE WINDOWS)

	if(NOT CONSOLE)
		target_link_options(xp-plants-vs-zombies PRIVATE -mwindows)
	endif()
elseif (ANDROID)  # 增加安卓平台的支持
    target_link_libraries(xp-plants-vs-zombies android log EGL GLESv2)  # 链接需要的库
	target_compile_definitions(xp-plants-vs-zombies PRIVATE ANDROID)  # 定义ANDROID宏
elseif (EMSCRIPTEN)
    target_link_libraries(xp-plants-vs-zombies GLESv2) # For WebGL
else()
	target_link_libraries(xp-plants-vs-zombies GL)
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(PAK_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/PAK)
file(MAKE_DIRECTORY ${PAK_OUTPUT_DIR})

set(PAK_SCRIPT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/create_pak.py)

# 添加自定义目标来创建 PAK 文件
add_custom_target(create_resources_pak
    COMMAND ${CMAKE_COMMAND} -E echo "正在创建资源 PAK 文件..."
    COMMAND ${Python3_EXECUTABLE} ${PAK_SCRIPT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Resources ${PAK_OUTPUT_DIR}/main.pak
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Resources ${PAK_SCRIPT_PATH}
    COMMENT "正在将 Resources 文件夹打包为 PAK 文件"
    VERBATIM
)

# 将 PAK 创建目标添加为主目标的依赖
add_dependencies(xp-plants-vs-zombies create_resources_pak)

install(FILES ${PAK_OUTPUT_DIR}/main.pak DESTINATION . OPTIONAL)