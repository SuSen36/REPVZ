cmake_minimum_required(VERSION 3.2.2...3.5)
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
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/zlib/*.c
)

# SDL_image Configuration
option(LINK_SDL_STATIC "Link SDL libraries statically" ON)

if(LINK_SDL_STATIC)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SDL_MIXER_X_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_MIXER_X_SHARED OFF CACHE BOOL "" FORCE)
else()
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    set(SDL_MIXER_X_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_MIXER_X_SHARED ON CACHE BOOL "" FORCE)
endif()

set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
# Disable AVIF to avoid NASM requirement
set(SDL2IMAGE_AVIF OFF CACHE BOOL "Disable AVIF" FORCE)
# Disable other complex formats not likely needed to speed up build and avoid errors
set(SDL2IMAGE_WEBP OFF CACHE BOOL "Disable WebP" FORCE)
set(SDL2IMAGE_TIF OFF CACHE BOOL "Disable TIFF" FORCE)
set(SDL2IMAGE_JXL OFF CACHE BOOL "Disable JXL" FORCE)
# SDL_mixer Configuration

set(USE_MIDI OFF CACHE BOOL "SDL Mixer X midi" FORCE)
set(USE_MODPLUG OFF CACHE BOOL "SDL Mixer X modplug" FORCE)
set(USE_XMP OFF CACHE BOOL "SDL Mixer X xmp" FORCE)
set(USE_OPUS OFF CACHE BOOL "SDL Mixer X opus" FORCE)
set(USE_DRFLAC OFF CACHE BOOL "SDL Mixer X flac" FORCE)
set(USE_PXTONE OFF CACHE BOOL "SDL Mixer X pxtone" FORCE)

# For Emscripten, SDL is usually handled by the toolchain itself
if(NOT EMSCRIPTEN)
    add_subdirectory(Libs/SDL)
    add_subdirectory(Libs/SDL_image)
    add_subdirectory(Libs/SDL_mixer_x)
    add_subdirectory(ck/PvZ-Quality-of-the-Lawn/Libs/SDL_ttf)
endif()

include(FetchContent)

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
	${PROJECT_SOURCE_DIR}/Libs/SDL_mixer_x/include
	${PROJECT_SOURCE_DIR}/ck/PvZ-Quality-of-the-Lawn/Libs/SDL_ttf
)

#target_compile_definitions(xp-plants-vs-zombies PRIVATE _DEBUG)


if (NOT EMSCRIPTEN)
    if(LINK_SDL_STATIC)
         target_link_libraries(xp-plants-vs-zombies 
             SDL2::SDL2-static 
             SDL2::SDL2main
             SDL2_image::SDL2_image-static 
             SDL2_mixer_ext_Static
             SDL2_ttf
         )
     else()
         target_link_libraries(xp-plants-vs-zombies 
             SDL2::SDL2 
             SDL2::SDL2main
             SDL2_image::SDL2_image 
             SDL2_mixer_ext
             SDL2_ttf
         )
     endif()
else()
    target_link_libraries(xp-plants-vs-zombies SDL2) # Emscripten provides its own SDL2
endif()

if (WIN32)
	target_link_options(xp-plants-vs-zombies PRIVATE -static -static-libgcc -static-libstdc++)
	target_link_libraries(xp-plants-vs-zombies mingw32 opengl32 ws2_32 ddraw dinput8 dxguid dxerr8 user32 gdi32 winmm imm32)
	target_compile_definitions(xp-plants-vs-zombies PRIVATE WINDOWS)

	if(NOT CONSOLE)
		target_link_options(xp-plants-vs-zombies PRIVATE -mwindows)
	endif()
elseif (ANDROID)  # 增加安卓平台的支持
    target_link_libraries(xp-plants-vs-zombies android log EGL GLESv2)
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

# 复制资源文件到构建目录
if (WIN32)
    if(EXISTS "${PAK_OUTPUT_DIR}/main.pak")
        file(COPY ${PAK_OUTPUT_DIR}/main.pak DESTINATION ${CMAKE_BINARY_DIR}/paks)
    endif()
    
    # 如果是动态连接，则复制 DLL
    if (NOT LINK_SDL_STATIC)
        if(EXISTS "${CMAKE_BINARY_DIR}/Libs/SDL2/SDL2.dll")
            file(COPY ${CMAKE_BINARY_DIR}/Libs/SDL2/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR})
        endif()
        if(EXISTS "${CMAKE_BINARY_DIR}/Libs/SDL2_image/SDL2_image.dll")
            file(COPY ${CMAKE_BINARY_DIR}/Libs/SDL2_image/SDL2_image.dll DESTINATION ${CMAKE_BINARY_DIR})
        endif()
        if(EXISTS "${CMAKE_BINARY_DIR}/Libs/SDL2_mixer/SDL2_mixer.dll")
            file(COPY ${CMAKE_BINARY_DIR}/Libs/SDL2_mixer/SDL2_mixer.dll DESTINATION ${CMAKE_BINARY_DIR})
        endif()
    endif()
endif()