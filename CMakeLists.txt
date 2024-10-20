cmake_minimum_required(VERSION 3.2.2)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)  # 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(re-plants-vs-zombies LANGUAGES C CXX ASM)

file(GLOB SOURCES
		src/*.cpp
		src/SexyAppFramework/*.cpp
		src/SexyAppFramework/graphics/*.cpp
		src/SexyAppFramework/imagelib/*.cpp
		src/SexyAppFramework/misc/*.cpp
		src/SexyAppFramework/paklib/*.cpp
		src/SexyAppFramework/sound/*.cpp
		src/SexyAppFramework/widget/*.cpp
		src/SexyAppFramework/fcaseopen/*.c
		src/Sexy.TodLib/*.cpp
		src/Lawn/*.cpp
		src/Lawn/Widget/*.cpp
		src/Lawn/System/*.cpp
		src/SexyAppFramework/glad/*.c
		src/SexyAppFramework/imagelib/png/*.c
		src/SexyAppFramework/imagelib/jpeg/*.c
)

# these files are included and must not be built in the project
list(REMOVE_ITEM SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sexy.TodLib/TodDrawTriangle.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Sexy.TodLib/TodDrawTriangleInc.cpp
)

set(LOW_MEMORY OFF)

if(ANDROID)
	list(APPEND SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/android/graphics/Window.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/android/graphics/GLInterface.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/pc/Input.cpp
	)
	list(APPEND PLAT_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/android
	)
else()
	list(APPEND SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/pc/graphics/Window.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/pc/graphics/GLInterface.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/pc/Input.cpp
	)
	list(APPEND PLAT_INCLUDES
		${CMAKE_CURRENT_SOURCE_DIR}/src/SexyAppFramework/platform/pc
	)
endif()

option(CONSOLE "Show console on Windows" OFF)

set(SDL_STATIC OFF CACHE BOOL "Build static library of SDL" FORCE)
set(SDL_SHARED ON CACHE BOOL "Build shared library of SDL" FORCE)
set(USE_MIDI OFF CACHE BOOL "SDL midi" FORCE)
set(USE_MODPLUG OFF CACHE BOOL "SDL modplug" FORCE)
set(USE_XMP OFF CACHE BOOL "SDL xmp" FORCE)
set(USE_OPUS OFF CACHE BOOL "SDL opus" FORCE)
set(USE_DRFLAC OFF CACHE BOOL "SDL flac" FORCE)
set(USE_PXTONE OFF CACHE BOOL "SDL pxtone" FORCE)

add_subdirectory(Libs/SDL)


set(JPEG_STATIC ON CACHE BOOL "Build static library of JPEG" FORCE)
set(JPEG_SHARED OFF CACHE BOOL "Build shared library of JPEG" FORCE)
add_subdirectory(Libs/zlib)
include(FetchContent)

if (WIN32)
	set(BASS_SRC http://www.un4seen.com/files/bass24.zip)
	set(BASS_LIB c/x64/bass.lib)
	set(BASS_INCLUDE c)
elseif (UNIX)
	if (APPLE)
		set(BASS_SRC http://www.un4seen.com/files/bass24-osx.zip)
		set(BASS_LIB libbass.dylib)
	elseif (LINUX)
		set(BASS_SRC http://www.un4seen.com/files/bass24-linux.zip)
		set(BASS_LIB libs/x86_64/libbass.so)
	elseif (ANDROID)
		set(BASS_INCLUDE c)
		# 使用ANDROID_ABI来动态识别当前架构
		if (ANDROID_ABI STREQUAL "arm64-v8a")
			set(BASS_SRC http://www.un4seen.com/files/bass24-android.zip)
			set(BASS_LIB libs/arm64-v8a/libbass.so)
		elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
			set(BASS_SRC http://www.un4seen.com/files/bass24-android.zip)
			set(BASS_LIB libs/armeabi-v7a/libbass.so)
		elseif (ANDROID_ABI STREQUAL "x86")
			set(BASS_SRC http://www.un4seen.com/files/bass24-android.zip)
			set(BASS_LIB libs/x86/libbass.so)
		elseif (ANDROID_ABI STREQUAL "x86_64")
			set(BASS_SRC http://www.un4seen.com/files/bass24-android.zip)
			set(BASS_LIB libs/x86_64/libbass.so)
		else()
			message(FATAL_ERROR "不支持的Android架构: ${ANDROID_ABI}.")
		endif()  # 结束针对 Android 的判断
	else()
		message(FATAL_ERROR "仅支持Android平台的构建.")
	endif()  # 结束针对 UNIX 的判断

endif()  # 结束针对 WIN32 的判断

message(STATUS "Current ABI: ${ANDROID_ABI}")


FetchContent_Declare(
		bass
		URL ${BASS_SRC}
)

FetchContent_GetProperties(bass)
if (NOT bass_POPULATED)
	FetchContent_Populate(bass)
	file(COPY ${bass_SOURCE_DIR}/${BASS_LIB} DESTINATION ${CMAKE_BINARY_DIR})
endif()

get_filename_component(bassfile ${BASS_LIB} NAME)

add_library(bass STATIC IMPORTED GLOBAL)
set_target_properties(bass PROPERTIES
		IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/${bassfile}
)

if (WIN32)
	configure_file("${bass_SOURCE_DIR}/x64/bass.dll" ${CMAKE_BINARY_DIR} COPYONLY)
endif()

if(WIN32)
add_executable(re-plants-vs-zombies ${SOURCES})
else (ANDROID)
add_library(re-plants-vs-zombies SHARED ${SOURCES})
endif ()

target_include_directories(re-plants-vs-zombies PRIVATE
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/Libs/SDL/include
	${PROJECT_SOURCE_DIR}/Libs/zlib
	${PLAT_INCLUDES}
	${bass_SOURCE_DIR}/${BASS_INCLUDE}
)
target_compile_definitions(re-plants-vs-zombies PRIVATE _DEBUG)

if (LOW_MEMORY)
	target_compile_definitions(re-plants-vs-zombies PRIVATE LOW_MEMORY)
endif()

target_link_libraries(re-plants-vs-zombies zlibstatic SDL2::SDL2 bass)

if (WIN32)
	target_link_options(re-plants-vs-zombies PRIVATE -static -static-libgcc -static-libstdc++)
	target_link_libraries(re-plants-vs-zombies opengl32 ws2_32 ddraw dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 shlwapi)
	target_compile_definitions(re-plants-vs-zombies PRIVATE WINDOWS)

	if(NOT CONSOLE)
		target_link_options(re-plants-vs-zombies PRIVATE -mwindows)
	endif()
elseif (ANDROID)  # 增加安卓平台的支持
	target_link_libraries(re-plants-vs-zombies log EGL GLESv1_CM)  # 链接需要的库
	target_compile_definitions(re-plants-vs-zombies PRIVATE ANDROID)  # 定义ANDROID宏
else()
	target_link_libraries(re-plants-vs-zombies GL)
endif()

# 设置资源目录
set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

if(WIN32)
	# 获取资源目录
	set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")

	# 使用 GLOB_RECURSE 获取资源文件
	file(GLOB_RECURSE RESOURCES
			"${RESOURCE_DIR}/*"
	)

	# 将资源文件拷贝到构建目录
	foreach(RES_FILE ${RESOURCES})
		# 获取文件的相对路径
		file(RELATIVE_PATH RELATIVE_PATH "${RESOURCE_DIR}" "${RES_FILE}")
		# 创建目标路径
		set(TARGET_PATH "${CMAKE_CURRENT_BINARY_DIR}/${RELATIVE_PATH}")
		# 创建目标文件的目录
		get_filename_component(TARGET_DIR "${TARGET_PATH}" DIRECTORY)

		# 添加目标路径的命令
		add_custom_command(
				TARGET re-plants-vs-zombies POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E make_directory "${TARGET_DIR}"
				COMMAND ${CMAKE_COMMAND} -E copy
				"${RES_FILE}"
				"${TARGET_PATH}"
				COMMENT "Copying resource file ${RELATIVE_PATH} to build directory"
		)
	endforeach()
endif()
