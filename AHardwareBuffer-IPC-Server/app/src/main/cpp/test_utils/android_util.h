//
// Created by intel on 30/3/2023.
//

#ifndef AHARDWAREBUFFER_IPC_CLIENT_ANDROID_UTIL_H
#define AHARDWAREBUFFER_IPC_CLIENT_ANDROID_UTIL_H

#include "../../../../../../../../Android/Sdk/ndk/21.4.7075529/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/GLES3/gl3.h"
#include "../../../../../../../../Android/Sdk/ndk/21.4.7075529/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/EGL/egl.h"
#include "../../../../../../../../Android/Sdk/ndk/21.4.7075529/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/EGL/eglext.h"
#include "../../../../../../../../Android/Sdk/ndk/21.4.7075529/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/stdint.h"
#include "../../../../../../../../Android/Sdk/ndk/21.4.7075529/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/array"


// Taken from cutils/native_handle.h:
// https://android.googlesource.com/platform/system/core/+/master/libcutils/include/cutils/native_handle.h


/* Declare a char array for use with native_handle_init */
#define NATIVE_HANDLE_DECLARE_STORAGE(name, maxFds, maxInts) \
    alignas(native_handle_t) char (name)[                            \
      sizeof(native_handle_t) + sizeof(int) * ((maxFds) + (maxInts))]

typedef struct native_handle
{
    int version;        /* sizeof(native_handle_t) */
    int numFds;         /* number of file-descriptors at &data[0] */
    int numInts;        /* number of ints at &data[numFds] */
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-length-array"
#endif
    int data[0];        /* numFds + numInts ints */
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
} native_handle_t;

typedef const native_handle_t* buffer_handle_t;

struct ANativeWindowBuffer;
struct AHardwareBuffer;

    namespace android
    {

        constexpr std::array<GLenum, 3> kSupportedSizedInternalFormats = {GL_RGBA8, GL_RGB8, GL_RGB565};

        ANativeWindowBuffer *ClientBufferToANativeWindowBuffer(EGLClientBuffer clientBuffer);
        EGLClientBuffer AHardwareBufferToClientBuffer(const AHardwareBuffer *hardwareBuffer);
        AHardwareBuffer *ClientBufferToAHardwareBuffer(EGLClientBuffer clientBuffer);
        ANativeWindowBuffer *AHardwareBufferToANativeWindowBuffer(const AHardwareBuffer *hardwareBuffer);

        void GetANativeWindowBufferProperties(const ANativeWindowBuffer *buffer,
                                              int *width,
                                              int *height,
                                              int *depth,
                                              int *pixelFormat,
                                              uint64_t *usage,
                                              int *fd);

        AHardwareBuffer *ANativeWindowBufferToAHardwareBuffer(ANativeWindowBuffer *windowBuffer);

    }  // namespace android
#endif //AHARDWAREBUFFER_IPC_CLIENT_ANDROID_UTIL_H
