//
// Created by intel on 5/4/2023.
//

#ifndef AHARDWAREBUFFER_IPC_SERVER_ANDROID_LOG_H
#define AHARDWAREBUFFER_IPC_SERVER_ANDROID_LOG_H
#include <android/log.h>
// Android log function wrappers
static const char* kTAG = "ClientIPC";
#define ALOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define ALOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define ALOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

#define ALOGV(...) \
  ((void)__android_log_print(ANDROID_LOG_VERBOSE, kTAG, __VA_ARGS__))

#define ALOGD(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))
#endif //AHARDWAREBUFFER_IPC_SERVER_ANDROID_LOG_H
