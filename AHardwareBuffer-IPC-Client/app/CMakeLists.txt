cmake_minimum_required(VERSION 3.4.1)

set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library( app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)

add_library( ClientIPC
             SHARED
        src/main/cpp/esUtil.h
        src/main/cpp/esUtil.c
        src/main/cpp/esShade.c
        src/main/cpp/esUtil_Android.c
        src/main/cpp/socket.c
        src/main/cpp/android_util.cpp
        src/main/cpp/render.cpp
        src/main/cpp/outBuffer.cpp
        )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

target_link_libraries( ClientIPC
                       app-glue
                       log
                       EGL
                       GLESv3
                       android)