//
// Created by intel on 29/3/2023.
//
#include "outBuffer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <android/hardware_buffer.h>
#include "esUtil.h"
#include "android_util.h"
#include "socket.h"

// Can be anything if using abstract namespace
#define SOCKET_NAME "sharedServerSocket"
static int data_socket;

void setupClient(void) {
    char socket_name[108]; // 108 sun_path length max
    static struct sockaddr_un server_addr;

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket < 0) {
        LOGE("socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // NDK needs abstract namespace by leading with '\0'
    // Ya I was like WTF! too... http://www.toptip.ca/2013/01/unix-domain-socket-with-abstract-socket.html?m=1
    // Note you don't need to unlink() the socket then
    memcpy(&socket_name[0], "\0", 1);
    strcpy(&socket_name[1], SOCKET_NAME);

    // clear for safty
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX; // Unix Domain instead of AF_INET IP domain
    strncpy(server_addr.sun_path, socket_name, sizeof(server_addr.sun_path) - 1); // 108 char max

    // Assuming only one init connection for demo
    int ret = connect(data_socket, (const struct sockaddr *) &server_addr,
                      sizeof(struct sockaddr_un));
    if (ret < 0) {
        LOGE("connect: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    LOGI("Client Setup Complete");
}

constexpr EGLint kDefaultAttribs[] = {
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_NONE,
};

void createEGLImageAndroidHardwareBufferSource(
        ESContext *esContext,
        size_t width,
        size_t height,
        size_t depth,
        int androidPixelFormat,
        const EGLint *attribs,
        EGLImageKHR *outSourceImage) {
    // Set Android Memory

    EGLImageKHR image = eglCreateImageKHR(
            esContext->eglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
            android::AHardwareBufferToClientBuffer(esContext->h_buffer), attribs);
    if (image == EGL_NO_IMAGE_KHR) {
        LOGI("EGL_NO_IMAGE_KHR error %x", eglGetError());
    }

    esContext->h_egl_buffer = image;

    LOGI("width:%zu height:%zu aHardwareBuffer %p egl_buffer %p", width, height,
         esContext->h_buffer,
         esContext->h_egl_buffer);
    *outSourceImage = image;
}

struct command {
    int32_t type;
    int32_t para1;
    int32_t para2;
    int32_t para3;
};

void getSharedMem01(ESContext *esContext) {
    int ret;
    struct command command_buf;
    command_buf.type = 0x01;
    ret = write(data_socket, &command_buf, sizeof(command_buf));
    if (ret < 0) {
        LOGE("write error");
    }

    ret = AHardwareBuffer_recvHandleFromUnixSocket(data_socket, &esContext->h_buffer);
    if (ret < 0) {
        LOGE("Failed to AHardwareBuffer_sendHandleToUnixSocket");
    }
}

void setupAHardwareBuffer01(ESContext *esContext) {
    createEGLImageAndroidHardwareBufferSource(
            esContext,
            esContext->width,
            esContext->height,
            1,
            AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
            kDefaultAttribs,
            &esContext->h_egl_buffer
    );
}


void getSharedMem02(ESContext *esContext, int index) {
    int ret;
    struct command command_buf;
    command_buf.type = 0x02;
    command_buf.para1 = index;
    ret = write(data_socket, &command_buf, sizeof(command_buf));
    LOGI("getShareMem02 ret %d", ret);
    if (ret < 0) {
        LOGE("write error");
    }
    read_fd(data_socket, &esContext->eglDmaImage[index].texture_dma_fd,
            &esContext->eglDmaImage[index].texture_metadata,
            sizeof(struct texture_storage_metadata_t));
    LOGI("getShareMem02 OK");
}

void setupAHardwareBuffer02(ESContext *esContext, int index) {
    struct texture_storage_metadata_t *texture_metadata = &esContext->eglDmaImage[index].texture_metadata;
    EGLint attrs[] = {
            EGL_WIDTH, texture_metadata->width,
            EGL_HEIGHT, texture_metadata->height,
            EGL_LINUX_DRM_FOURCC_EXT, texture_metadata->fourcc,
            EGL_DMA_BUF_PLANE0_FD_EXT, esContext->eglDmaImage[index].texture_dma_fd,
            EGL_DMA_BUF_PLANE0_OFFSET_EXT, texture_metadata->offset,
            EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT,
            (EGLint) ((texture_metadata->modifiers >> 32) & 0x00000000FFFFFFFF),
            EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT,
            (EGLint) (texture_metadata->modifiers & 0x00000000FFFFFFFF),
            EGL_DMA_BUF_PLANE0_PITCH_EXT, texture_metadata->stride,
            EGL_NONE,
    };

    EGLImageKHR image = eglCreateImageKHR(esContext->eglDisplay, EGL_NO_CONTEXT,
                                          EGL_LINUX_DMA_BUF_EXT,
                                          (EGLClientBuffer) (uint64_t) 0, attrs);
    if (image == EGL_NO_IMAGE_KHR) {
        LOGI("EGL_NO_IMAGE_KHR error %x", eglGetError());
    }
    esContext->h_egl_buffer = image;
    esContext->eglDmaImage[index].h_egl_buffer = image;

    LOGI("width:%d height:%d modifiers %" PRIx64 " high %x low %x egl_buffer %p",
         texture_metadata->width,
         texture_metadata->height, texture_metadata->modifiers,
         (EGLint) ((texture_metadata->modifiers >> 32) & 0x00000000FFFFFFFF),
         (EGLint) (texture_metadata->modifiers & 0x00000000FFFFFFFF),
         (void *) esContext->eglDmaImage[index].h_egl_buffer);
}