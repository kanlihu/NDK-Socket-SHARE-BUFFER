//
// Created by intel on 2/4/2023.
//

#ifndef AHARDWAREBUFFER_IPC_SERVER_OUTBUFFER_H
#define AHARDWAREBUFFER_IPC_SERVER_OUTBUFFER_H

#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/looper.h>
#include <android/log.h>

#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include "esUtil.h"
#include "outBuffer.h"

// Android log function wrappers
static const char* kTAG = "ClientIPC";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

// 0xAABBGGRR for pixel
#define RED_SHIFT 0
#define GREEN_SHIFT 8
#define BLUE_SHIFT 16

const uint32_t color_wheel[4] = {
        0x000000FF, // red
        0x0000FF00, // green
        0x00FF0000, // blue
        0x00000FFFF // yellow
};
void setupClient();
void* setupServer(ESContext *esContext);
void sendSharedMem(ESContext *esContext);
GLuint LoadOutTexture(ESContext *esContext);
void setupAHardwareBuffer01(ESContext * context);
GLuint setupAHardwareBuffer02(ESContext *esContext);
void gl_setup_scene();
void gl_draw_scene(GLuint texture);
#endif //AHARDWAREBUFFER_IPC_SERVER_OUTBUFFER_H
