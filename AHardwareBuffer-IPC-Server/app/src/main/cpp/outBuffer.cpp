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

// Server to get socket data with information of SharedMem's file descriptor
void *setupServer(ESContext *esContext) {
    int ret;
    struct sockaddr_un server_addr;
    int socket_fd;
    char socket_name[108]; // 108 sun_path length max

    LOGI("Start server setup");

    // AF_UNIX for domain unix IPC and SOCK_STREAM since it works for the example
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        LOGE("socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOGI("Socket made");

    // NDK needs abstract namespace by leading with '\0'
    // Ya I was like WTF! too... http://www.toptip.ca/2013/01/unix-domain-socket-with-abstract-socket.html?m=1
    // Note you don't need to unlink() the socket then
    memcpy(&socket_name[0], "\0", 1);
    strcpy(&socket_name[1], SOCKET_NAME);

    // clear for safty
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX; // Unix Domain instead of AF_INET IP domain
    strncpy(server_addr.sun_path, socket_name, sizeof(server_addr.sun_path) - 1); // 108 char max

    ret = bind(socket_fd, (const struct sockaddr *) &server_addr, sizeof(struct sockaddr_un));
    if (ret < 0) {
        LOGE("bind: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOGI("Bind made");

    // Open 8 back buffers for this demo
    ret = listen(socket_fd, 8);
    if (ret < 0) {
        LOGE("listen: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOGI("Socket listening for packages");

    // Wait for incoming connection.
    data_socket = accept(socket_fd, NULL, NULL);
    if (data_socket < 0) {
        LOGE("accept: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    LOGI("Accepted data");
    // This is the main loop for handling connections
    // Assuming in example connection is established only once
    // Would be better to refactor this for robustness
#if 1
    for (;;) {
        char buf[100];
        ret = read(data_socket, buf, 1);
        if (ret < 0) {
            LOGE("Failed to read errno %d", -errno);
            goto out;
        }
        LOGI("buf[0] %d", buf[0]);
        switch (buf[0]) {
            case 0x00: {
                break;
            }
            case 0x01: {
                // Blocks until sent data
                if (esContext && esContext->h_buffer) {
                    int ret = AHardwareBuffer_sendHandleToUnixSocket(esContext->h_buffer,
                                                                     data_socket);
                    if (ret < 0) {
                        LOGE("Failed to AHardwareBuffer_sendHandleToUnixSocket");
                    }
                }
                break;
            }
            case 0x02: {
                write_fd(data_socket, esContext->texture_dmabuf_fd, esContext->texture_metadata,
                         sizeof(struct texture_storage_metadata_t));
                break;
            }
        }

    }
#endif
    out:
    close(data_socket);
    close(socket_fd);

    return NULL;
}


constexpr EGLint kDefaultAttribs[] = {
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_NONE,
};

AHardwareBuffer *createAndroidHardwareBuffer(size_t width,
                                             size_t height,
                                             size_t depth,
                                             int androidFormat) {
    // The height and width are number of pixels of size format
    AHardwareBuffer_Desc aHardwareBufferDescription = {};
    aHardwareBufferDescription.width = width;
    aHardwareBufferDescription.height = height;
    aHardwareBufferDescription.layers = depth;
    aHardwareBufferDescription.format = androidFormat;
    aHardwareBufferDescription.usage =
            AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
    aHardwareBufferDescription.stride = 0;
    aHardwareBufferDescription.rfu0 = 0;
    aHardwareBufferDescription.rfu1 = 0;

    // Allocate memory from Android Hardware Buffer
    AHardwareBuffer *aHardwareBuffer = nullptr;
    int ret = AHardwareBuffer_allocate(&aHardwareBufferDescription, &aHardwareBuffer);
    if (ret < 0) {
        return nullptr;
    }
    return aHardwareBuffer;
}

void createEGLImageAndroidHardwareBufferSource(
        ESContext *esContext,
        size_t width,
        size_t height,
        size_t depth,
        int androidPixelFormat,
        const EGLint *attribs,
        AHardwareBuffer **outSourceAHB,
        EGLImageKHR *outSourceImage) {
    // Set Android Memory
    AHardwareBuffer *aHardwareBuffer =
            createAndroidHardwareBuffer(width, height, depth, androidPixelFormat);

    EGLImageKHR image = eglCreateImageKHR(
            esContext->eglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
            android::AHardwareBufferToClientBuffer(aHardwareBuffer), attribs);

    LOGI("width:%d height:%d aHardwareBuffer :%p", width, height, aHardwareBuffer);
    *outSourceAHB = aHardwareBuffer;
    *outSourceImage = image;
}

void setupAHardwareBuffer01(ESContext *context) {
    createEGLImageAndroidHardwareBufferSource(
            context,
            context->width,
            context->height,
            1,
            AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
            kDefaultAttribs,
            &context->h_buffer,
            &context->h_egl_buffer
    );

    AHardwareBuffer *h_buffer = context->h_buffer;
    AHardwareBuffer_Desc h_buffer_desc;
    void *shared_buffer;
    int ret;
    // need to pull out description of size info
    // currently assuming in demo, should use this to error check and stuff
    AHardwareBuffer_describe(h_buffer, &h_buffer_desc);

    LOGI("AHardwareBuffer Size: %d", h_buffer_desc.height * h_buffer_desc.stride * 4);

    ret = AHardwareBuffer_lock(h_buffer,
                               AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                               AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN,
                               -1, // no fence in demo
                               NULL,
                               &shared_buffer);
    if (ret != 0) {
        LOGE("Failed to AHardwareBuffer_lock");
    }
    LOGI("kanli width:%d height:%d stride:%d", h_buffer_desc.height, h_buffer_desc.stride,
         h_buffer_desc.stride);
    // 3 columns of color
    for (int i = 0; i < h_buffer_desc.height; i++) {
        for (int j = 0; j < h_buffer_desc.stride; j++) {
            if (j < h_buffer_desc.width / 3) {
                memcpy((char *) shared_buffer + (((i * h_buffer_desc.stride) + j) * 4),
                       &color_wheel[0], sizeof(uint32_t));
            } else if (j < h_buffer_desc.width * 2 / 3) {
                memcpy((char *) shared_buffer + (((i * h_buffer_desc.stride) + j) * 4),
                       &color_wheel[1], sizeof(uint32_t));
            } else {
                memcpy((char *) shared_buffer + (((i * h_buffer_desc.stride) + j) * 4),
                       &color_wheel[2], sizeof(uint32_t));
            }
        }
    }


    ret = AHardwareBuffer_unlock(h_buffer, NULL);
    if (ret != 0) {
        LOGE("Failed to AHardwareBuffer_unlock");
    }

}


void sendSharedMem(ESContext *esContext) {
    if (esContext && esContext->h_buffer) {
        int ret = AHardwareBuffer_sendHandleToUnixSocket(esContext->h_buffer, data_socket);
        if (ret != 0) {
            LOGE("Failed to AHardwareBuffer_sendHandleToUnixSocket");
        }
    }
}


GLuint LoadOutTexture(ESContext *esContext) {

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, esContext->h_egl_buffer);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    LOGI("kanli %s texId %d", __FUNCTION__, texId);
    return texId;
}


GLuint setupAHardwareBuffer02(ESContext *esContext) {

    int width,
            height;

    char *buffer = esLoadTGA(esContext->platformData, "lightmap.tga", &width, &height);

    GLuint texId;

    if (buffer == NULL) {
        esLogMessage("Error loading (%s) image.\n", "");
        return 0;
    } else {
        esLogMessage("OK loading (%s) image.\n", "");
    }

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    LOGI("kanli texId %d", texId);
    glFlush();

    EGLImageKHR image = eglCreateImageKHR(esContext->eglDisplay,
                                          esContext->eglContext,
                                          EGL_GL_TEXTURE_2D,
                                          (EGLClientBuffer) (uint64_t) texId,
                                          NULL);
    if (image == EGL_NO_IMAGE_KHR) {
        LOGE("create failed %x", eglGetError());
    }
    esContext->h_egl_buffer02 = image;
    LOGI("kanli esContext->h_egl_buffer02 %p", esContext->h_egl_buffer02);
// EGL (extension: EGL_MESA_image_dma_buf_export): Get file descriptor (texture_dmabuf_fd) for the EGL image and get its
    // storage data (texture_storage_metadata - fourcc, stride, offset)
    int fd;
    struct texture_storage_metadata_t *metadata = (struct texture_storage_metadata_t *)
            malloc(sizeof(struct texture_storage_metadata_t));

    PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC eglExportDMABUFImageQueryMESA =
            (PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC) eglGetProcAddress(
                    "eglExportDMABUFImageQueryMESA");
    if (eglExportDMABUFImageQueryMESA == NULL) {
        LOGE("can not use eglExportDMABUFImageQueryMESA");
    }
    EGLBoolean queried = eglExportDMABUFImageQueryMESA(esContext->eglDisplay,
                                                       esContext->h_egl_buffer02,
                                                       &metadata->fourcc,
                                                       &metadata->num_planes,
                                                       &metadata->modifiers);
    if (queried > 0) {
        LOGI("queried OK");
    }
    PFNEGLEXPORTDMABUFIMAGEMESAPROC eglExportDMABUFImageMESA =
            (PFNEGLEXPORTDMABUFIMAGEMESAPROC) eglGetProcAddress("eglExportDMABUFImageMESA");
    if (eglExportDMABUFImageMESA == NULL) {
        LOGE("can not use eglExportDMABUFImageMESA");
    }
    EGLBoolean exported = eglExportDMABUFImageMESA(esContext->eglDisplay,
                                                   esContext->h_egl_buffer02,
                                                   &fd,
                                                   &metadata->stride,
                                                   &metadata->offset);
    if (exported > 0) {
        metadata->width = width;
        metadata->height = height;
        esContext->texture_dmabuf_fd = fd;
        esContext->texture_metadata = metadata;

        LOGI("exorpted > 0 kanli fd %d forcc %x num_planes %d modifiers %" PRIx64 " stride %d offset %d",
             fd, metadata->fourcc, metadata->num_planes, metadata->modifiers,
             metadata->stride, metadata->offset);
    } else {
        esContext->texture_metadata = NULL;
    }

    return texId;
}

const char *offscreen_vertexshader =  "#version 330 core                                                 \n"
                                      "// Input vertex data, different for all executions of this shader.\n"
                                      "layout(location = 0) in vec3 vertexPosition_modelspace;           \n"
                                      "// Output data ; will be interpolated for each fragment.          \n"
                                      "void main(){                                                      \n"
                                      "    gl_Position =  vec4(vertexPosition_modelspace,1);             \n"
                                      "}                                                                 \n";
const char *offscreen_fragmentshader = "#version 330 core\n"
                                       "in vec2 UV;\n"
                                       "out vec3 color;\n"
                                       "uniform sampler2D renderedTexture;\n"
                                       "uniform float time;\n"
                                       "void main(){\n"
                                       "    color = texture( renderedTexture, UV + 0.005*vec2( sin(time+1024.0*UV.x),cos(time+768.0*UV.y)) ).xyz ;\n"
                                       "}";


const char vShadowMapShaderStr[] =
        "#version 300 es                                  \n"
        "uniform mat4 u_mvpLightMatrix;                   \n"
        "layout(location = 0) in vec4 a_position;         \n"
        "out vec4 v_color;                                \n"
        "void main()                                      \n"
        "{                                                \n"
        "   gl_Position = u_mvpLightMatrix * a_position;  \n"
        "}                                                \n";

const char fShadowMapShaderStr[] =
        "#version 300 es                                  \n"
        "precision lowp float;                            \n"
        "void main()                                      \n"
        "{                                                \n"
        "}                                                \n";

int offsceenRender(ESContext *esContext) {

    return 0;
}