//
// Created by intel on 10/4/2023.
//

#ifndef AHARDWAREBUFFER_IPC_SERVER_OFFSCREENRENDER_H
#define AHARDWAREBUFFER_IPC_SERVER_OFFSCREENRENDER_H
#ifdef __cplusplus
extern "C" {
#endif
#include "esUtil.h"

void onSurfaceCreated(ESContext *esContext, EGLConfig eglConfig);
void onSurfaceChanged(ESContext *esContext, int width, int height);
void onDrawFrame(ESContext *esContext);
#ifdef __cplusplus
}
#endif
#endif //AHARDWAREBUFFER_IPC_SERVER_OFFSCREENRENDER_H
