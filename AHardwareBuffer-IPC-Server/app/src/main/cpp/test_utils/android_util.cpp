//
// Created by intel on 30/3/2023.
//

//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// android_util.cpp: Utilities for the using the Android platform

#include "android_util.h"

#include <stdint.h>

#if defined(ANGLE_PLATFORM_ANDROID) && __ANDROID_API__ >= 26
#    define ANGLE_AHARDWARE_BUFFER_SUPPORT
// NDK header file for access to Android Hardware Buffers
#    include <android/hardware_buffer.h>
#endif

// Taken from nativebase/nativebase.h
// https://android.googlesource.com/platform/frameworks/native/+/master/libs/nativebase/include/nativebase/nativebase.h
typedef const native_handle_t *buffer_handle_t;

typedef struct android_native_base_t
{
    /* a magic value defined by the actual EGL native type */
    int magic;
    /* the sizeof() of the actual EGL native type */
    int version;
    void *reserved[4];
    /* reference-counting interface */
    void (*incRef)(struct android_native_base_t *base);
    void (*decRef)(struct android_native_base_t *base);
} android_native_base_t;

typedef struct ANativeWindowBuffer
{
    struct android_native_base_t common;
    int width;
    int height;
    int stride;
    int format;
    int usage_deprecated;
    uintptr_t layerCount;
    void *reserved[1];
    const native_handle_t *handle;
    uint64_t usage;
    // we needed extra space for storing the 64-bits usage flags
    // the number of slots to use from reserved_proc depends on the
    // architecture.
    void *reserved_proc[8 - (sizeof(uint64_t) / sizeof(void *))];
} ANativeWindowBuffer_t;


// In the Android system:
// - AHardwareBuffer is essentially a typedef of GraphicBuffer. Conversion functions simply
// reinterpret_cast.
// - GraphicBuffer inherits from two base classes, ANativeWindowBuffer and RefBase.
//
// GraphicBuffer implements a getter for ANativeWindowBuffer (getNativeBuffer) by static_casting
// itself to its base class ANativeWindowBuffer. The offset of the ANativeWindowBuffer pointer
// from the GraphicBuffer pointer is 16 bytes. This is likely due to two pointers: The vtable of
// GraphicBuffer and the one pointer member of the RefBase class.
//
// This is not future proof at all. We need to look into getting utilities added to Android to
// perform this cast for us.
    constexpr int kAHardwareBufferToANativeWindowBufferOffset =
            static_cast<int>(sizeof(void *)) * 2;

    template<typename T1, typename T2>
    T1 *offsetPointer(T2 *ptr, int bytes) {
        return reinterpret_cast<T1 *>(reinterpret_cast<intptr_t>(ptr) + bytes);
    }


    namespace android {

        ANativeWindowBuffer *ClientBufferToANativeWindowBuffer(EGLClientBuffer clientBuffer) {
            return reinterpret_cast<ANativeWindowBuffer *>(clientBuffer);
        }


        void GetANativeWindowBufferProperties(const ANativeWindowBuffer *buffer,
                                              int *width,
                                              int *height,
                                              int *depth,
                                              int *pixelFormat,
                                              uint64_t *usage,
                                              int *fd) {
            *width = buffer->width;
            *height = buffer->height;
            *depth = static_cast<int>(buffer->layerCount);
            *height = buffer->height;
            *pixelFormat = buffer->format;
            *usage = buffer->usage;
            *fd = buffer->handle->data[0];
        }

        AHardwareBuffer *ANativeWindowBufferToAHardwareBuffer(ANativeWindowBuffer *windowBuffer) {
            return offsetPointer<AHardwareBuffer>(windowBuffer,
                                                  -kAHardwareBufferToANativeWindowBufferOffset);
        }

        ANativeWindowBuffer *AHardwareBufferToANativeWindowBuffer(const AHardwareBuffer *hardwareBuffer) {
            return offsetPointer<ANativeWindowBuffer>(hardwareBuffer,
                                                  kAHardwareBufferToANativeWindowBufferOffset);
        }

        EGLClientBuffer AHardwareBufferToClientBuffer(const AHardwareBuffer *hardwareBuffer) {
            return offsetPointer<EGLClientBuffer>(hardwareBuffer,
                                                  kAHardwareBufferToANativeWindowBufferOffset);
        }

        AHardwareBuffer *ClientBufferToAHardwareBuffer(EGLClientBuffer clientBuffer) {
            return offsetPointer<AHardwareBuffer>(clientBuffer,
                                                  -kAHardwareBufferToANativeWindowBufferOffset);
        }
    }