//
// Created by intel on 30/3/2023.
//

#ifndef AHARDWAREBUFFER_IPC_CLIENT_ANDROID_UTIL_H
#define AHARDWAREBUFFER_IPC_CLIENT_ANDROID_UTIL_H

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdint.h>
#include <array>

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
