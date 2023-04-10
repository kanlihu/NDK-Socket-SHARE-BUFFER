//
// Created by intel on 2/4/2023.
//

#ifndef AHARDWAREBUFFER_IPC_SERVER_OUTBUFFER_H
#define AHARDWAREBUFFER_IPC_SERVER_OUTBUFFER_H
#ifdef __cplusplus

extern "C" {
#endif

#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include "esUtil.h"

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
void *setupServer(ESContext *esContext);
void sendSharedMem(ESContext *esContext);
GLuint LoadOutTexture01(ESContext *esContext);
void setupAHardwareBuffer01(ESContext *context);
GLuint setupAHardwareBuffer02(ESContext *esContext, int index);
GLuint genOutTexture02(ESContext *esContext, int index);
GLuint LoadOutTexture02(ESContext *esContext, int index);
#ifdef __cplusplus
}
#endif
#endif //AHARDWAREBUFFER_IPC_SERVER_OUTBUFFER_H
